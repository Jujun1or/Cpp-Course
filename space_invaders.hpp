#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>

class GameObject {
protected:
    float x, y;
    int width, height;
    bool active;
public:
    GameObject(float x, float y, int w, int h);
    virtual ~GameObject() = default;
    
    virtual void update(float delta) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;
    virtual void onCollision(GameObject* other) = 0;
    
    SDL_Rect getRect() const;
    bool isActive() const;
    void setActive(bool a);
};

class Player : public GameObject {
    int lives;
    float speed;
    bool canShoot;
    float shootCooldown;
public:
    Player(float x, float y);
    void update(float delta) override;
    void render(SDL_Renderer* renderer) override;
    void onCollision(GameObject* other) override;
    
    void moveLeft(float delta);
    void moveRight(float delta, int screenWidth);
    bool shoot();
    int getLives() const;
};

class Enemy : public GameObject {
    int points;
    bool isBottomEnemy;
public:
    Enemy(float x, float y, int w, int h, int p, bool ibe);
    void update(float delta) override;
    void render(SDL_Renderer* renderer) override;
    void onCollision(GameObject* other) override;
    
    bool canShoot() const;
    int getPoints() const;
    void setBottomEnemy(bool ibe);
    void updatePosition(float dx, float dy);
};

class Bullet : public GameObject {
    bool isPlayerBullet;
    float speed;
public:
    Bullet(float x, float y, bool ipb, float s);
    void update(float delta) override;
    void render(SDL_Renderer* renderer) override;
    void onCollision(GameObject* other) override;
    
    bool isFromPlayer() const;
};

struct LevelConfig { 
    int startY; 
    float enemySpeed; 
    float shootInterval; 
};

class Level {
    LevelConfig config;
    std::vector<Enemy*> enemies;
    std::vector<std::vector<Enemy*>> enemyGrid;
    float shootTimer;
    
    void createEnemies(int sw);
    void updateBottomEnemies();
public:
    Level();
    ~Level();
    
    bool loadFromFile(const std::string& filename);
    void update(float delta, int screenWidth, std::vector<Bullet*>& bullets);
    void render(SDL_Renderer* renderer);
    
    std::vector<Enemy*>& getEnemies();
    LevelConfig getConfig() const;
    bool allEnemiesDestroyed() const;
    bool enemiesReachedBottom(int screenHeight) const;
};

class Game {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Player* player;
    Level level;
    std::vector<Bullet*> bullets;
    int score;
    bool gameOver;
    
    void handleEvents();
    void update(float delta);
    void render();
    void checkCollisions();
    void resetGame();
    
public:
    Game();
    ~Game();
    
    bool init();
    void run();
};