#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include "browser_ui.h"

// Memory callback for libcurl response handling
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t real_size = size * nmemb;
    std::string* mem = static_cast<std::string*>(userp);
    mem->append(static_cast<char*>(contents), real_size);
    return real_size;
}

class PBrowserEngine {
private:
    std::string default_search = "https://www.google.com/search?q=";

    // Basic domain blocklist filter
    bool isAdDomain(const std::string& url) {
        static const std::vector<std::string> ad_patterns = {
            "doubleclick.net", "googlesyndication.com", "adservice", 
            "telemetry", "analytics", "ads.", "/ads/", "popads.net"
        };
        for (const auto& pattern : ad_patterns) {
            if (url.find(pattern) != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    // Strip script, style, and HTML markup into text content
    void parseHTML(const std::string& raw_html, std::string& out_text, std::vector<std::string>& out_links) {
        out_text.clear();
        out_links.clear();

        // 1. Remove <script> and <style> blocks
        std::string cleaned = std::regex_replace(raw_html, std::regex("<script[\\s\\S]*?</script>", std::regex::icase), "");
        cleaned = std::regex_replace(cleaned, std::regex("<style[\\s\\S]*?</style>", std::regex::icase), "");

        // 2. Extract href links
        std::smatch match;
        std::regex link_regex("href=\"(https?://[^\"]+)\"", std::regex::icase);
        std::string::const_iterator search_start(cleaned.cbegin());
        while (std::regex_search(search_start, cleaned.cend(), match, link_regex)) {
            if (out_links.size() < 15) {
                out_links.push_back(match[1].str());
            }
            search_start = match[0].second;
        }

        // 3. Strip remaining HTML tags
        std::string text_only = std::regex_replace(cleaned, std::regex("<[^>]*>"), " ");
        
        // 4. Clean up whitespace
        text_only = std::regex_replace(text_only, std::regex("\\s+"), " ");
        
        out_text = text_only;
    }

public:
    PBrowserEngine() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    ~PBrowserEngine() {
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

        if (state.adblock_enabled && isAdDomain(target_url)) {
            state.blocked_ads_count++;
            state.page_title = "Blocked by P-Browser Shield";
            state.rendered_text_body = "Access to " + target_url + " was blocked by your active AdBlock rule.";
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
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "P-Browser/1.0 (PlayStation 5; PS5 Payload SDK)");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Allow self-signed or console network certs

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            state.page_title = "Network Error";
            state.rendered_text_body = "Failed to fetch page. CURL Error: " + std::string(curl_easy_strerror(res));
            return false;
        }

        state.page_title = "Page: " + target_url;
        parseHTML(response_string, state.rendered_text_body, state.parsed_links);
        return true;
    }
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("P-Browser [PPSA99080]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

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

    PBrowserEngine browser;
    BrowserState state;

    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        if (state.navigate_requested && !state.is_loading) {
            state.is_loading = true;
            state.navigate_requested = false;
            
            // Render loading frame
            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            RenderPBrowserUI(state);
            ImGui::Render();
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);

            // Network Fetch
            browser.fetchPage(state);
            state.is_loading = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderPBrowserUI(state);

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
