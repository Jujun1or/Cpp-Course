#include "space_invaders.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

// GameObject implementation
GameObject::GameObject(float x, float y, int w, int h) : x(x), y(y), width(w), height(h), active(true) {}

SDL_Rect GameObject::getRect() const { 
    return {static_cast<int>(x), static_cast<int>(y), width, height}; 
}

bool GameObject::isActive() const { 
    return active; 
}

void GameObject::setActive(bool a) { 
    active = a; 
}

// Player implementation
Player::Player(float x, float y) : GameObject(x, y, 50, 30), lives(3), speed(300.0f), canShoot(true), shootCooldown(0.0f) {}

void Player::update(float delta) {
    if (!canShoot) {
        shootCooldown -= delta;
        if (shootCooldown <= 0.0f) canShoot = true;
    }
}

void Player::render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Player::onCollision(GameObject* other) {
    lives--;
}

void Player::moveLeft(float delta) { 
    x -= speed * delta; 
    if (x < 0) x = 0; 
}

void Player::moveRight(float delta, int sw) { 
    x += speed * delta; 
    if (x > sw - width) x = sw - width; 
}

bool Player::shoot() {
    if (canShoot) {
        canShoot = false;
        shootCooldown = 0.5f;
        return true;
    }
    return false;
}

int Player::getLives() const { 
    return lives; 
}

// Enemy implementation
Enemy::Enemy(float x, float y, int w, int h, int p, bool ibe) 
    : GameObject(x, y, w, h), points(p), isBottomEnemy(ibe) {}

void Enemy::update(float delta) {
    // Можно добавить дополнительную логику при необходимости
}

void Enemy::render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Enemy::onCollision(GameObject* other) { 
    active = false; 
}

bool Enemy::canShoot() const { 
    return isBottomEnemy; 
}

int Enemy::getPoints() const { 
    return points; 
}

void Enemy::setBottomEnemy(bool ibe) { 
    isBottomEnemy = ibe; 
}

void Enemy::updatePosition(float dx, float dy) {
    x += dx;
    y += dy;
}

// Bullet implementation
Bullet::Bullet(float x, float y, bool ipb, float s) 
    : GameObject(x, y, 5, 15), isPlayerBullet(ipb), speed(s) {}

void Bullet::update(float delta) {
    y += (isPlayerBullet ? -1 : 1) * speed * delta;
    if (y < 0 || y > 600) active = false;
}

void Bullet::render(SDL_Renderer* renderer) {
    SDL_Rect rect = getRect();
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Bullet::onCollision(GameObject* other) { 
    active = false; 
}

bool Bullet::isFromPlayer() const { 
    return isPlayerBullet; 
}

// Level implementation
Level::Level() : config{100, 50.0f, 3.0f}, shootTimer(0.0f) {}

Level::~Level() {
    for (auto enemy : enemies) {
        delete enemy;
    }
}

bool Level::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        if (std::getline(iss, key, '=')) {
            std::string value;
            if (std::getline(iss, value)) {
                if (key == "startY") config.startY = std::stoi(value);
                else if (key == "enemySpeed") config.enemySpeed = std::stof(value);
                else if (key == "shootInterval") config.shootInterval = std::stof(value);
            }
        }
    }
    return true;
}

void Level::createEnemies(int screenWidth) {
    const int enemyWidth = 40;
    const int enemyHeight = 30;
    const int cols = 11;
    const int rows = 5;
    const int spacing = 10;
    
    int startX = (screenWidth - (cols * (enemyWidth + spacing))) / 2;
    
    for (int row = 0; row < rows; ++row) {
        std::vector<Enemy*> currentRow;
        for (int col = 0; col < cols; ++col) {
            int points = (rows - row) * 10;
            float x = startX + col * (enemyWidth + spacing);
            float y = config.startY + row * (enemyHeight + spacing);
            
            Enemy* enemy = new Enemy(x, y, enemyWidth, enemyHeight, points, row == rows - 1);
            enemies.push_back(enemy);
            currentRow.push_back(enemy);
        }
        enemyGrid.push_back(currentRow);
    }
    updateBottomEnemies();
}

void Level::updateBottomEnemies() {
    for (auto enemy : enemies) {
        enemy->setBottomEnemy(false);
    }
    
    for (int col = 0; col < 11; ++col) {
        for (int row = 4; row >= 0; --row) {
            if (row < enemyGrid.size() && col < enemyGrid[row].size() && 
                enemyGrid[row][col]->isActive()) {
                enemyGrid[row][col]->setBottomEnemy(true);
                break;
            }
        }
    }
}

void Level::update(float delta, int screenWidth, std::vector<Bullet*>& bullets) {
    static int lastEnemyCount = enemies.size();
    static bool movingRight = true;
    static bool shouldMoveDown = false;
    
    // Подсчет текущих врагов и ускорение
    int currentEnemyCount = 0;
    for (auto enemy : enemies) {
        if (enemy->isActive()) currentEnemyCount++;
    }
    
    if (currentEnemyCount < lastEnemyCount) {
        float speedIncrease = 1.0f + (lastEnemyCount - currentEnemyCount) * 0.05f;
        config.enemySpeed *= speedIncrease;
    }
    lastEnemyCount = currentEnemyCount;
    
    shootTimer += delta;
    
    if (enemies.empty()) {
        createEnemies(screenWidth);
        return;
    }
    
    // Проверка достижения края
    bool reachedEdge = false;
    for (auto enemy : enemies) {
        if (enemy->isActive()) {
            SDL_Rect rect = enemy->getRect();
            if ((movingRight && rect.x + rect.w >= screenWidth) || 
                (!movingRight && rect.x <= 0)) {
                reachedEdge = true;
                break;
            }
        }
    }
    
    if (reachedEdge) {
        movingRight = !movingRight;
        shouldMoveDown = true;
    }
    
    // Движение врагов
    for (auto enemy : enemies) {
        if (enemy->isActive()) {
            if (shouldMoveDown) {
                enemy->updatePosition(0, 20.0f);
            } else {
                float direction = movingRight ? 1.0f : -1.0f;
                enemy->updatePosition(direction * config.enemySpeed * delta, 0);
            }
        }
    }
    
    shouldMoveDown = false;
    
    // Стрельба врагов
    if (shootTimer >= config.shootInterval) {
        shootTimer = 0.0f;
        std::vector<Enemy*> shooters;
        for (auto enemy : enemies) {
            if (enemy->isActive() && enemy->canShoot()) {
                shooters.push_back(enemy);
            }
        }
        
        if (!shooters.empty()) {
            int idx = rand() % shooters.size();
            SDL_Rect rect = shooters[idx]->getRect();
            bullets.push_back(new Bullet(rect.x + rect.w/2 - 2, 
                                      rect.y + rect.h, 
                                      false, 
                                      300.0f));
        }
    }
    
    updateBottomEnemies();
}

void Level::render(SDL_Renderer* renderer) {
    for (auto enemy : enemies) {
        if (enemy->isActive()) {
            enemy->render(renderer);
        }
    }
}

std::vector<Enemy*>& Level::getEnemies() {
    return enemies;
}

bool Level::allEnemiesDestroyed() const {
    for (auto enemy : enemies) {
        if (enemy->isActive()) return false;
    }
    return true;
}

bool Level::enemiesReachedBottom(int screenHeight) const {
    for (auto enemy : enemies) {
        if (enemy->isActive()) {
            SDL_Rect rect = enemy->getRect();
            if (rect.y + rect.h >= screenHeight - 50) {
                return true;
            }
        }
    }
    return false;
}

// Game implementation
Game::Game() : window(nullptr), renderer(nullptr), running(true), player(nullptr), score(0), gameOver(false) {}

Game::~Game() {
    delete player;
    for (auto bullet : bullets) {
        delete bullet;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Game::init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    window = SDL_CreateWindow("Space Invaders", 
                            SDL_WINDOWPOS_CENTERED, 
                            SDL_WINDOWPOS_CENTERED, 
                            800, 600, 
                            SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 
                                SDL_RENDERER_ACCELERATED | 
                                SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return false;
    }
    
    if (!level.loadFromFile("level.txt")) {
        std::cerr << "Failed to load level config, using defaults" << std::endl;
    }
    
    player = new Player(400, 550);
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        
        if (event.type == SDL_KEYDOWN && !gameOver) {
            if (event.key.keysym.sym == SDLK_SPACE && player->shoot()) {
                SDL_Rect rect = player->getRect();
                bullets.push_back(new Bullet(rect.x + rect.w/2 - 2, rect.y, true, 500.0f));
            }
        }
    }
    
    if (!gameOver) {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_LEFT]) {
            player->moveLeft(0.016f);
        }
        if (keystates[SDL_SCANCODE_RIGHT]) {
            player->moveRight(0.016f, 800);
        }
    } else {
        const Uint8* keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_RETURN]) {
            resetGame();
        }
    }
}

void Game::update(float deltaTime) {
    if (gameOver) return;
    
    player->update(deltaTime);
    level.update(deltaTime, 800, bullets);
    
    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        (*it)->update(deltaTime);
        if (!(*it)->isActive()) {
            delete *it;
            it = bullets.erase(it);
        } else {
            ++it;
        }
    }
    
    checkCollisions();
    
    // Check game over conditions
    if (player->getLives() <= 0 || level.enemiesReachedBottom(600)) {
        gameOver = true;
    }
    
    // Check level complete
    if (level.allEnemiesDestroyed()) {
        score += 100;
        level = Level();
        if (!level.loadFromFile("level.txt")) {
            // Use defaults if file loading fails
        }
    }
}

void Game::checkCollisions() {
    // Player bullets vs enemies
    for (auto bullet : bullets) {
        if (bullet->isFromPlayer() && bullet->isActive()) {
            for (auto enemy : level.getEnemies()) {
                if (enemy->isActive()) {
                    SDL_Rect bulletRect = bullet->getRect();
                    SDL_Rect enemyRect = enemy->getRect();
                    
                    if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
                        bullet->onCollision(enemy);
                        enemy->onCollision(bullet);
                        score += enemy->getPoints();
                        break;
                    }
                }
            }
        }
    }
    
    // Enemy bullets vs player
    for (auto bullet : bullets) {
        if (!bullet->isFromPlayer() && bullet->isActive() && player->isActive()) {
            SDL_Rect bulletRect = bullet->getRect();
            SDL_Rect playerRect = player->getRect();
            
            if (SDL_HasIntersection(&bulletRect, &playerRect)) {
                bullet->onCollision(player);
                player->onCollision(bullet);
            }
        }
    }
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    if (player->isActive()) {
        player->render(renderer);
    }
    
    level.render(renderer);
    
    for (auto bullet : bullets) {
        if (bullet->isActive()) {
            bullet->render(renderer);
        }
    }
    
    // Render UI to console
    std::cout << "Score: " << score << " | Lives: " << player->getLives();
    if (gameOver) std::cout << " | GAME OVER - Press Enter to restart";
    std::cout << "\r";
    
    SDL_RenderPresent(renderer);
}

void Game::resetGame() {
    delete player;
    for (auto bullet : bullets) {
        delete bullet;
    }
    bullets.clear();
    
    player = new Player(400, 550);
    score = 0;
    gameOver = false;
    level = Level();
    if (!level.loadFromFile("level.txt")) {
        // Use defaults if file loading fails
    }
}

void Game::run() {
    Uint32 lastTime = SDL_GetTicks();
    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        
        handleEvents();
        update(deltaTime);
        render();
        
        SDL_Delay(16); // Cap at ~60 FPS
    }
}