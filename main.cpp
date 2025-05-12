#include <iostream>
#include "menu.hpp"
#include <SDL2/SDL.h>

class RenderSystem {
    SDL_Window* window;
    SDL_Renderer* renderer;
    
public:
    RenderSystem(int width, int height) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
        SDL_SetWindowTitle(window, "SDL Menu System");
    }
    
    ~RenderSystem() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    
    SDL_Renderer* getRenderer() { return renderer; }
    
    void clear() { 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }
    
    void present() { SDL_RenderPresent(renderer); }
};

int main() {
    RenderSystem renderer(800, 600);
    
    MenuSystem menu(
        renderer.getRenderer(),
        "arialmt.ttf",
        [&](const std::string& action) {
            std::cout << "Action: " << action << std::endl;
            if (action == "exit") {
                menu.showDialog("ExitDialog");
            }
            else if (action == "open_exit_dialog") {
                menu.showDialog("ExitDialog");
            }
            else if (action == "open_graphics_options") {
                menu.pushMenu("GraphicsOptions");
            }
        }
    );
    
    menu.loadResources("menu.txt");
    menu.pushMenu("MainMenu");
    
    SDL_Event event;
    bool running = true;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            menu.handleEvent(event);
        }
        
        renderer.clear();
        menu.render();
        renderer.present();
        
        SDL_Delay(16);
    }
    
    return 0;
}