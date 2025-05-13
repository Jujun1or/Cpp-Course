#include "space_invaders.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    Game game;
    if (!game.init()) {
        std::cerr << "Game initialization failed!\n";
        return 1;
    }
    game.run();
    return 0;
}