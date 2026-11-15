#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class RocketRush {
 public:
  enum class State : uint8_t { INACTIVE, TITLE, PLAYING, PAUSED, GAME_OVER };

  explicit RocketRush(Arduino_GFX *display);
  void begin();
  void update();
  void enterTitle();
  void stop();
  bool isActive() const;
  bool consumeExitRequest();
  const char *stateName() const;

 private:
  static constexpr uint8_t LEFT_BUTTON_PIN = 0;
  static constexpr uint8_t RIGHT_BUTTON_PIN = 14;
  static constexpr uint8_t MAX_ENEMIES = 10;
  static constexpr uint8_t MAX_BULLETS = 8;
  static constexpr uint8_t MAX_PARTICLES = 24;
  static constexpr uint8_t STAR_COUNT = 28;
  static constexpr uint32_t FRAME_MS = 33;
  static constexpr uint32_t DEBOUNCE_MS = 25;
  static constexpr uint32_t EXIT_HOLD_MS = 1800;

  struct Button {
    bool rawPressed = false;
    bool pressed = false;
    uint32_t changedAt = 0;
  };
  struct Enemy {
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
    uint8_t color = 0;
    uint16_t yMilli = 0;
  };
  struct Bullet {
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
  };
  struct Particle {
    bool active = false;
    int16_t xMilli = 0;
    int16_t yMilli = 0;
    int8_t vx = 0;
    int8_t vy = 0;
    uint8_t life = 0;
    uint16_t color = 0;
  };
  struct Star {
    int16_t x = 0;
    int16_t y = 0;
    uint8_t speed = 1;
    uint16_t color = 0;
  };

  Arduino_GFX *_display;
  State _state = State::INACTIVE;
  Button _left;
  Button _right;
  Enemy _enemies[MAX_ENEMIES];
  Bullet _bullets[MAX_BULLETS];
  Particle _particles[MAX_PARTICLES];
  Star _stars[STAR_COUNT];
  int16_t _playerX = 160;
  uint32_t _score = 0;
  uint32_t _highScore = 0;
  uint8_t _lives = 3;
  uint32_t _lastFrameAt = 0;
  uint32_t _lastSpawnAt = 0;
  uint32_t _lastShotAt = 0;
  uint32_t _lifeFlashUntil = 0;
  uint32_t _chordStartedAt = 0;
  bool _chordLatched = false;
  bool _longHoldHandled = false;
  bool _exitRequested = false;

  void updateButton(Button &button, uint8_t pin, uint32_t now);
  void updateControls(uint32_t now);
  void handleChord();
  void startGame();
  void finishGame();
  void stepGame(uint32_t now);
  void updateStars();
  void spawnEnemy(uint32_t now);
  void spawnBullet(uint32_t now);
  void updateBullets();
  void updateEnemies(uint32_t now);
  void updateParticles();
  void checkCollisions();
  void loseLife(int16_t x, int16_t y);
  void explodeAt(int16_t x, int16_t y, uint16_t color);
  void clearActors();
  void initializeStars();
  void loadHighScore();
  void saveHighScore();

  void render();
  void drawBackground();
  void drawHud();
  void drawPlayer(int16_t x, int16_t y);
  void drawEnemy(const Enemy &enemy);
  void drawTitle();
  void drawPaused();
  void drawGameOver();
  void centerText(const char *text, int16_t y, uint8_t size, uint16_t color);
  bool overlaps(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                int16_t bx, int16_t by, int16_t bw, int16_t bh) const;
};
