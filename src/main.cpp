#include <iostream>
#include <string>
#include <vector>
#include <future>
#include <atomic>
#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include "browser_ui.h"

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    std::string* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), real_size);
    return real_size;
}

class PSBrowserEngine {
private:
    std::string default_search = "https://www.google.com/search?q=";

    bool isFilteredDomain(const std::string& url) {
        static const std::vector<std::string> blocked_patterns = {
            "doubleclick.net", "googlesyndication.com", "adservice", 
            "telemetry", "analytics"
        };
        for (const auto& pattern : blocked_patterns) {
            if (url.find(pattern) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // Bounded Linear HTML Parser & Title Extractor
    void parseHTML(const std::string& raw, std::string& out_text, std::string& out_title, std::vector<std::string>& out_links) {
        out_text.clear();
        out_title.clear();
        out_links.clear();

        size_t limit = std::min(raw.size(), size_t(8 * 1024 * 1024)); // 8 MB OOM cap
        bool in_script = false;
        bool in_style = false;
        bool in_head = false;
        bool in_title_tag = false;
        std::string current_title_candidate = "";

        for (size_t i = 0; i < limit; ++i) {
            if (raw[i] == '<') {
                // Read tag name
                size_t j = i + 1;
                while (j < limit && raw[j] != '>' && !isspace(raw[j])) {
                    j++;
                }
                std::string tag = raw.substr(i + 1, j - (i + 1));
                // Convert tag to lowercase
                for (auto& c : tag) c = tolower(c);

                if (tag == "script") in_script = true;
                else if (tag == "/script") in_script = false;
                else if (tag == "style") in_style = true;
                else if (tag == "/style") in_style = false;
                else if (tag == "head") in_head = true;
                else if (tag == "/head") in_head = false;
                else if (tag == "title") in_title_tag = true;
                else if (tag == "/title") in_title_tag = false;

                // Extract href if tag is 'a'
                if (tag == "a") {
                    size_t href_pos = raw.find("href=\"", i);
                    if (href_pos != std::string::npos && href_pos < j + 100) {
                        size_t start_quote = href_pos + 6;
                        size_t end_quote = raw.find("\"", start_quote);
                        if (end_quote != std::string::npos) {
                            std::string link = raw.substr(start_quote, end_quote - start_quote);
                            if (out_links.size() < 15 && (link.rfind("http://", 0) == 0 || link.rfind("https://", 0) == 0)) {
                                out_links.push_back(link);
                            }
                        }
                    }
                }

                // Skip until '>'
                while (i < limit && raw[i] != '>') {
                    i++;
                }
            } else {
                if (in_title_tag) {
                    current_title_candidate += raw[i];
                } else if (!in_script && !in_style && !in_head) {
                    out_text += raw[i];
                }
            }
        }

        if (!current_title_candidate.empty()) {
            out_title = current_title_candidate;
        } else {
            out_title = "Untitled Web Page";
        }
    }

public:
    PSBrowserEngine() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~PSBrowserEngine() {
        curl_global_cleanup();
    }

    bool fetchPage(BrowserState& state) {
        std::string target_url = state.url_buf;

        if (target_url.rfind("http://", 0) != 0 && target_url.rfind("https://", 0) != 0) {
            std::string query = target_url;
            for (char &c : query) if (c == ' ') c = '+';
            target_url = default_search + query;
            snprintf(state.url_buf, sizeof(state.url_buf), "%s", target_url.c_str());
        }

        if (state.url_filter_enabled && isFilteredDomain(target_url)) {
            state.filtered_count++;
            state.page_title = "Blocked by PSBrowser Filter";
            state.rendered_text_body = "Access to " + target_url + " was blocked by active URL filter rules.";
            return false;
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            state.rendered_text_body = "Failed to initialize libcurl network context.";
            return false;
        }

        std::string response_string;
        curl_easy_setopt(curl, CURLOPT_URL, target_url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "PSBrowser/1.0 (PlayStation 5; PS5 Payload SDK)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_MAXFILESIZE, 8 * 1024 * 1024L); // 8 MB cap
        curl_easy_setopt(curl, CURLOPT_CAINFO, "/data/homebrew/PSBrowser/cacert.pem");

        CURLcode res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            char* effective_url = nullptr;
            curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effective_url);
            if (effective_url) {
                snprintf(state.url_buf, sizeof(state.url_buf), "%s", effective_url);
            }
        }

        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            state.page_title = "Network Error";
            state.rendered_text_body = "Failed to fetch page. CURL Error: " + std::string(curl_easy_strerror(res)) + 
                                       "\n(Check if cacert.pem is present or use HTTP for unencrypted testing).";
            return false;
        }

        std::string extracted_title;
        parseHTML(response_string, state.rendered_text_body, extracted_title, state.parsed_links);
        state.page_title = extracted_title;
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("PSBrowser [PPSA99080]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        SDL_Quit();
        return -1;
    }

    // Renderer Selection with Fallback Logging
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }

    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    fprintf(stderr, "[PSBrowser] Renderer: %s\n", info.name);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    SDL_GameController* controller = nullptr;
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
    }

    PSBrowserEngine browser;
    BrowserState state;

    static std::future<bool> fetch_future;
    static std::atomic<bool> fetch_running{false};

    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        // Handle Asynchronous Network Fetching
        if (state.navigate_requested && !fetch_running) {
            state.navigate_requested = false;
            state.is_loading = true;
            fetch_running = true;
            fetch_future = std::async(std::launch::async, [&browser, &state]() {
                bool ok = browser.fetchPage(state);
                state.is_loading = false;
                fetch_running = false;
                return ok;
            });
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderPSBrowserUI(state);

        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (controller) SDL_GameControllerClose(controller);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
