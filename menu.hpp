#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <string>
#include <map>
#include <stack>
#include <functional>
#include <memory>

struct MenuItem {
    std::string text;
    std::string action;
    std::string target;
};

struct MenuSection {
    enum Type { MENU, DIALOG } type;
    std::string name;
    std::vector<MenuItem> items;
    std::string message;
};

class MenuSystem {
public:
    using ActionHandler = std::function<void(const std::string&)>;
    
    MenuSystem(SDL_Renderer* renderer, const std::string& fontPath, ActionHandler handler);
    ~MenuSystem();
    
    void loadResources(const std::string& filename);
    void pushMenu(const std::string& name);
    void showDialog(const std::string& name);
    void handleEvent(const SDL_Event& event);
    void render();
    void back();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    ActionHandler actionHandler;
    std::map<std::string, MenuSection> sections;
    std::stack<std::string> menuStack;
    std::string activeDialog;
    
    void parseMenu(std::istream& stream, const std::string& name);
    void parseDialog(std::istream& stream, const std::string& name);
    void handleMenuEvent(const SDL_Event& event);
    void handleDialogEvent(const SDL_Event& event);
    void drawMenu(const MenuSection& menu);
    void drawDialog(const MenuSection& dialog);
    void drawText(const std::string& text, int x, int y, SDL_Color color, int size);
    SDL_Texture* createTextTexture(const std::string& text, SDL_Color color, int size);
};