#include <iostream>
#include <string>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>

class PBrowserEngine {
private:
    std::string current_url = "https://www.google.com";
    std::string default_search = "https://www.google.com/search?q=";
    bool is_media_active = false;

public:
    void navigate(const std::string& input) {
        if (input.rfind("http://", 0) == 0 || input.rfind("https://", 0) == 0) {
            current_url = input;
        } else {
            std::string query = input;
            for (char &c : query) if (c == ' ') c = '+';
            current_url = default_search + query;
        }
        std::cout << "[P-Browser Engine] Navigated to: " << current_url << std::endl;
    }

    std::string getUrl() const { return current_url; }
};

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_CaptureMouse(SDL_TRUE);
    SDL_GameController* controller = nullptr;
    if (SDL_NumJoysticks() > 0) {
        controller = SDL_GameControllerOpen(0);
    }

    PBrowserEngine browser;
    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
            }
        }
    }

    if (controller) SDL_GameControllerClose(controller);
    SDL_Quit();
    return 0;
}
