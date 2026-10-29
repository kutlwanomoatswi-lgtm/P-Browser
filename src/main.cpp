#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include "browser_ui.h"

class PBrowserEngine {
private:
    std::string current_url = "https://www.google.com";
    std::string default_search = "https://www.google.com/search?q=";

public:
    void navigate(const std::string& input, bool adblock_active, int& blocked_counter) {
        if (input.rfind("http://", 0) == 0 || input.rfind("https://", 0) == 0) {
            current_url = input;
        } else {
            std::string query = input;
            for (char &c : query) if (c == ' ') c = '+';
            current_url = default_search + query;
        }

        if (adblock_active) {
            blocked_counter += 3; // Simulate blocking tracker domains on navigate
        }

        std::cout << "[P-Browser Engine] Navigated to: " << current_url 
                  << " (AdBlock: " << (adblock_active ? "ENABLED" : "DISABLED") << ")" << std::endl;
    }

    std::string getUrl() const { return current_url; }
};

int main(int argc, char* argv[]) {
    // Initialize SDL2 with Video, Controller, and Events
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("P-Browser [PPSA99080]",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);

    if (!window) {
        std::cerr << "SDL Window Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // PS5 Software Renderer Fallback: Try Software Renderer first, fallback to default if unavailable
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        // Fallback for standard desktop runners
        renderer = SDL_CreateRenderer(window, -1, 0);
    }

    if (!renderer) {
        std::cerr << "SDL Renderer Creation Failed: " << SDL_GetError() << std::endl;
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
    snprintf(state.url_buf, sizeof(state.url_buf), "%s", browser.getUrl().c_str());

    bool running = true;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) running = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        RenderPBrowserUI(state);

        if (state.navigate_requested) {
            browser.navigate(state.url_buf, state.adblock_enabled, state.blocked_ads_count);
            state.navigate_requested = false;
        }

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
