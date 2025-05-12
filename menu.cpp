#include "menu.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

MenuSystem::MenuSystem(SDL_Renderer* renderer, const std::string& fontPath, ActionHandler handler) 
    : renderer(renderer), actionHandler(handler) {
    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        return;
    }
    
    font = TTF_OpenFont(fontPath.c_str(), 24);
    if (!font) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << std::endl;
    }
}

MenuSystem::~MenuSystem() {
    if (font) TTF_CloseFont(font);
    TTF_Quit();
}

void MenuSystem::loadResources(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open menu file: " << filename << std::endl;
        return;
    }

    std::string line, currentSection;
    
    while (getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line.empty()) continue;
        
        if (line[0] == '[') {
            currentSection = line.substr(1, line.find(']') - 1);
            continue;
        }
        
        std::istringstream iss(line);
        std::string key;
        getline(iss, key, '=');
        
        if (key == "type") {
            std::string type;
            getline(iss, type);
            if (type == "menu") parseMenu(file, currentSection);
            else if (type == "dialog") parseDialog(file, currentSection);
        }
    }
}

void MenuSystem::parseMenu(std::istream& stream, const std::string& name) {
    MenuSection section{MenuSection::MENU, name};
    std::string line;
    
    while (getline(stream, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line == "]") break;
        
        if (line.find("{text=") != std::string::npos) {
            MenuItem item;
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            item.text = line.substr(start, end - start);
            
            start = line.find("action=\"", end) + 8;
            end = line.find("\"", start);
            item.action = line.substr(start, end - start);
            
            start = line.find("submenu=\"", end);
            if (start != std::string::npos) {
                start += 9;
                end = line.find("\"", start);
                item.target = line.substr(start, end - start);
            }
            
            section.items.push_back(item);
        }
    }
    
    sections[name] = section;
}

void MenuSystem::parseDialog(std::istream& stream, const std::string& name) {
    MenuSection section{MenuSection::DIALOG, name};
    std::string line;
    
    while (getline(stream, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
        if (line == "]") break;
        
        if (line.find("message=") != std::string::npos) {
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            section.message = line.substr(start, end - start);
        }
        else if (line.find("{text=") != std::string::npos) {
            MenuItem item;
            size_t start = line.find("\"") + 1;
            size_t end = line.find("\"", start);
            item.text = line.substr(start, end - start);
            
            start = line.find("action=\"", end) + 8;
            end = line.find("\"", start);
            item.action = line.substr(start, end - start);
            
            section.items.push_back(item);
        }
    }
    
    sections[name] = section;
}

void MenuSystem::pushMenu(const std::string& name) {
    if (sections.find(name) == sections.end() || sections[name].type != MenuSection::MENU) {
        std::cerr << "Menu not found: " << name << std::endl;
        return;
    }
    menuStack.push(name);
}

void MenuSystem::showDialog(const std::string& name) {
    if (sections.find(name) == sections.end() || sections[name].type != MenuSection::DIALOG) {
        std::cerr << "Dialog not found: " << name << std::endl;
        return;
    }
    activeDialog = name;
}

void MenuSystem::handleEvent(const SDL_Event& event) {
    if (!activeDialog.empty()) {
        handleDialogEvent(event);
    } else if (!menuStack.empty()) {
        handleMenuEvent(event);
    }
}

void MenuSystem::back() {
    if (menuStack.size() > 1) {
        menuStack.pop();
    }
}

void MenuSystem::handleMenuEvent(const SDL_Event& event) {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const MenuSection& currentMenu = sections[menuStack.top()];
    SDL_Point mousePoint = {event.button.x, event.button.y};
    
    for (size_t i = 0; i < currentMenu.items.size(); ++i) {
        SDL_Rect itemRect = {
            100,
            100 + static_cast<int>(i) * 50,
            200,
            40
        };
        
        if (SDL_PointInRect(&mousePoint, &itemRect)) {
            const MenuItem& item = currentMenu.items[i];
            
            if (!item.target.empty()) {
                if (sections[item.target].type == MenuSection::DIALOG) {
                    showDialog(item.target);
                } else {
                    pushMenu(item.target);
                }
            } else if (item.action == "back") {
                back();
            } else {
                actionHandler(item.action);
            }
            break;
        }
    }
}

void MenuSystem::handleDialogEvent(const SDL_Event& event) {
    if (event.type != SDL_MOUSEBUTTONDOWN || event.button.button != SDL_BUTTON_LEFT) {
        return;
    }

    const MenuSection& dialog = sections[activeDialog];
    SDL_Point mousePoint = {event.button.x, event.button.y};
    
    for (size_t i = 0; i < dialog.items.size(); ++i) {
        SDL_Rect buttonRect = {
            200 + static_cast<int>(i) * 150,
            300,
            120,
            40
        };
        
        if (SDL_PointInRect(&mousePoint, &buttonRect)) {
            const std::string& action = dialog.items[i].action;
            
            if (action == "close_dialog") {
                activeDialog.clear();
            } else {
                actionHandler(action);
            }
            break;
        }
    }
}

void MenuSystem::render() {
    if (!activeDialog.empty()) {
        drawDialog(sections[activeDialog]);
    } else if (!menuStack.empty()) {
        drawMenu(sections[menuStack.top()]);
    }
}

SDL_Texture* MenuSystem::createTextTexture(const std::string& text, SDL_Color color, int size) {
    TTF_SetFontSize(font, size);
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) {
        std::cerr << "TTF_RenderText Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    return texture;
}

void MenuSystem::drawText(const std::string& text, int x, int y, SDL_Color color, int size) {
    SDL_Texture* texture = createTextTexture(text, color, size);
    if (!texture) return;
    
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect dstRect = {x, y, w, h};
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
    SDL_DestroyTexture(texture);
}

void MenuSystem::drawMenu(const MenuSection& menu) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 100, 255);
    SDL_RenderClear(renderer);
    
    SDL_Color white = {255, 255, 255, 255};
    drawText(menu.name, 100, 50, white, 28);
    
    for (size_t i = 0; i < menu.items.size(); ++i) {
        const auto& item = menu.items[i];
        SDL_Rect rect = {100, 100 + static_cast<int>(i) * 50, 200, 40};
        
        SDL_SetRenderDrawColor(renderer, 70, 70, 120, 255);
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
        
        drawText(item.text, rect.x + 10, rect.y + 10, white, 20);
    }
}

void MenuSystem::drawDialog(const MenuSection& dialog) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
    SDL_Rect bg = {0, 0, 800, 600};
    SDL_RenderFillRect(renderer, &bg);
    
    SDL_SetRenderDrawColor(renderer, 70, 70, 120, 255);
    SDL_Rect windowRect = {150, 150, 500, 300};
    SDL_RenderFillRect(renderer, &windowRect);
    
    SDL_Color white = {255, 255, 255, 255};
    drawText(dialog.message, 200, 200, white, 24);
    
    for (size_t i = 0; i < dialog.items.size(); ++i) {
        const auto& item = dialog.items[i];
        SDL_Rect rect = {200 + static_cast<int>(i) * 150, 300, 120, 40};
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 150, 255);
        SDL_RenderFillRect(renderer, &rect);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &rect);
        
        drawText(item.text, rect.x + 10, rect.y + 10, white, 20);
    }
}