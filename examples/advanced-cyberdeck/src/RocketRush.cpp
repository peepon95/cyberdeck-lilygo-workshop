#include "RocketRush.h"

#include <Preferences.h>

#undef NAVY
#undef CYAN
#undef PURPLE
#undef GREEN
#undef YELLOW
#undef ORANGE

namespace {
constexpr uint16_t NAVY = 0x0005;
constexpr uint16_t PINK = 0xFB56;
constexpr uint16_t CYAN = 0x37FF;
constexpr uint16_t PURPLE = 0xA15F;
constexpr uint16_t GREEN = 0x4FE9;
constexpr uint16_t YELLOW = 0xFFE7;
constexpr uint16_t ORANGE = 0xFD20;
constexpr uint16_t DIM = 0x3186;
constexpr uint16_t WHITE16 = 0xFFFF;
constexpr uint16_t RED16 = 0xF800;
}

RocketRush::RocketRush(Arduino_GFX *display) : _display(display) {}

void RocketRush::begin() {
  pinMode(LEFT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON_PIN, INPUT_PULLUP);
  randomSeed(esp_random());
  loadHighScore();
  initializeStars();
}

bool RocketRush::isActive() const { return _state != State::INACTIVE; }

bool RocketRush::consumeExitRequest() {
  const bool requested = _exitRequested;
  _exitRequested = false;
  return requested;
}

const char *RocketRush::stateName() const {
  switch (_state) {
    case State::INACTIVE: return "design";
    case State::TITLE: return "title";
    case State::PLAYING: return "playing";
    case State::PAUSED: return "paused";
    case State::GAME_OVER: return "game_over";
  }
  return "unknown";
}

void RocketRush::enterTitle() {
  clearActors();
  _state = State::TITLE;
  _chordLatched = true;
  _longHoldHandled = false;
  _lastFrameAt = 0;
  render();
}

void RocketRush::stop() {
  _state = State::INACTIVE;
  _chordLatched = true;
  _longHoldHandled = false;
  clearActors();
}

void RocketRush::updateButton(Button &button, uint8_t pin, uint32_t now) {
  const bool raw = digitalRead(pin) == LOW;
  if (raw != button.rawPressed) {
    button.rawPressed = raw;
    button.changedAt = now;
  }
  if (now - button.changedAt >= DEBOUNCE_MS) button.pressed = button.rawPressed;
}

void RocketRush::handleChord() {
  if (_state == State::INACTIVE) enterTitle();
  else if (_state == State::TITLE || _state == State::GAME_OVER) startGame();
  else if (_state == State::PLAYING) _state = State::PAUSED;
  else if (_state == State::PAUSED) _state = State::PLAYING;
  _lastFrameAt = 0;
  render();
}

void RocketRush::updateControls(uint32_t now) {
  updateButton(_left, LEFT_BUTTON_PIN, now);
  updateButton(_right, RIGHT_BUTTON_PIN, now);
  const bool both = _left.pressed && _right.pressed;
  if (both && !_chordLatched) {
    _chordLatched = true;
    _longHoldHandled = false;
    _chordStartedAt = now;
    handleChord();
  }
  if (both && !_longHoldHandled && isActive() && now - _chordStartedAt >= EXIT_HOLD_MS) {
    _longHoldHandled = true;
    _exitRequested = true;
  }
  if (!both) {
    _chordLatched = false;
    _longHoldHandled = false;
  }
}

void RocketRush::startGame() {
  clearActors();
  _score = 0;
  _lives = 3;
  _playerX = 160;
  _lastSpawnAt = millis();
  _lastShotAt = millis();
  _lifeFlashUntil = 0;
  _state = State::PLAYING;
}

void RocketRush::finishGame() {
  if (_score > _highScore) {
    _highScore = _score;
    saveHighScore();
  }
  _state = State::GAME_OVER;
}

void RocketRush::update() {
  const uint32_t now = millis();
  updateControls(now);
  if (_state == State::INACTIVE) return;
  if (_state != State::PLAYING) {
    if (_lastFrameAt == 0 || now - _lastFrameAt >= 120) {
      _lastFrameAt = now;
      updateStars();
      render();
    }
    return;
  }
  if (now - _lastFrameAt < FRAME_MS) return;
  _lastFrameAt = now;
  stepGame(now);
  render();
}

void RocketRush::stepGame(uint32_t now) {
  if (_left.pressed && !_right.pressed) _playerX -= 4;
  if (_right.pressed && !_left.pressed) _playerX += 4;
  _playerX = constrain(_playerX, 12, 307);
  updateStars();
  spawnEnemy(now);
  spawnBullet(now);
  updateBullets();
  updateEnemies(now);
  updateParticles();
  checkCollisions();
}

void RocketRush::initializeStars() {
  for (auto &star : _stars) {
    star.x = random(2, 318);
    star.y = random(18, 169);
    star.speed = random(1, 4);
    star.color = star.speed == 3 ? CYAN : (star.speed == 2 ? PURPLE : DIM);
  }
}

void RocketRush::updateStars() {
  for (auto &star : _stars) {
    star.y += star.speed;
    if (star.y >= 170) {
      star.y = 19;
      star.x = random(2, 318);
    }
  }
}

void RocketRush::spawnEnemy(uint32_t now) {
  const uint32_t interval = max<uint32_t>(260, 900 - min<uint32_t>(_score / 2, 640));
  if (now - _lastSpawnAt < interval) return;
  _lastSpawnAt = now;
  for (auto &enemy : _enemies) {
    if (!enemy.active) {
      enemy.active = true;
      enemy.x = random(10, 297);
      enemy.y = 20;
      enemy.yMilli = 0;
      enemy.color = random(0, 3);
      return;
    }
  }
}

void RocketRush::spawnBullet(uint32_t now) {
  const uint32_t interval = max<uint32_t>(170, 320 - min<uint32_t>(_score / 20, 150));
  if (now - _lastShotAt < interval) return;
  _lastShotAt = now;
  for (auto &bullet : _bullets) {
    if (!bullet.active) {
      bullet.active = true;
      bullet.x = _playerX;
      bullet.y = 139;
      return;
    }
  }
}

void RocketRush::updateBullets() {
  for (auto &bullet : _bullets) {
    if (!bullet.active) continue;
    bullet.y -= 7;
    if (bullet.y < 18) bullet.active = false;
  }
}

void RocketRush::updateEnemies(uint32_t) {
  const uint16_t speedMilli = min<uint16_t>(3000, 900 + _score * 2);
  for (auto &enemy : _enemies) {
    if (!enemy.active) continue;
    enemy.yMilli += speedMilli;
    enemy.y += enemy.yMilli / 1000;
    enemy.yMilli %= 1000;
    if (enemy.y > 158) {
      const int16_t x = enemy.x;
      enemy.active = false;
      loseLife(x, 157);
    }
  }
}

void RocketRush::updateParticles() {
  for (auto &particle : _particles) {
    if (!particle.active) continue;
    particle.xMilli += particle.vx;
    particle.yMilli += particle.vy;
    if (particle.life > 0) --particle.life;
    if (particle.life == 0) particle.active = false;
  }
}

bool RocketRush::overlaps(int16_t ax, int16_t ay, int16_t aw, int16_t ah,
                          int16_t bx, int16_t by, int16_t bw, int16_t bh) const {
  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

void RocketRush::checkCollisions() {
  for (auto &bullet : _bullets) {
    if (!bullet.active) continue;
    for (auto &enemy : _enemies) {
      if (!enemy.active) continue;
      if (overlaps(bullet.x - 1, bullet.y, 3, 7, enemy.x - 7, enemy.y - 5, 15, 11)) {
        bullet.active = false;
        enemy.active = false;
        _score += 100;
        const uint16_t color = enemy.color == 0 ? CYAN : (enemy.color == 1 ? PURPLE : GREEN);
        explodeAt(enemy.x, enemy.y, color);
        break;
      }
    }
  }
  for (auto &enemy : _enemies) {
    if (!enemy.active) continue;
    if (overlaps(_playerX - 9, 143, 19, 19, enemy.x - 7, enemy.y - 5, 15, 11)) {
      const int16_t x = enemy.x;
      const int16_t y = enemy.y;
      enemy.active = false;
      loseLife(x, y);
    }
  }
}

void RocketRush::loseLife(int16_t x, int16_t y) {
  if (_state != State::PLAYING || millis() < _lifeFlashUntil) {
    explodeAt(x, y, RED16);
    return;
  }
  explodeAt(x, y, RED16);
  _lifeFlashUntil = millis() + 650;
  if (_lives > 0) --_lives;
  if (_lives == 0) finishGame();
}

void RocketRush::explodeAt(int16_t x, int16_t y, uint16_t color) {
  uint8_t spawned = 0;
  for (auto &particle : _particles) {
    if (particle.active) continue;
    particle.active = true;
    particle.xMilli = x * 10;
    particle.yMilli = y * 10;
    particle.vx = random(-16, 17);
    particle.vy = random(-16, 17);
    particle.life = random(7, 15);
    particle.color = random(0, 2) ? color : YELLOW;
    if (++spawned == 10) break;
  }
}

void RocketRush::clearActors() {
  for (auto &enemy : _enemies) enemy.active = false;
  for (auto &bullet : _bullets) bullet.active = false;
  for (auto &particle : _particles) particle.active = false;
}

void RocketRush::loadHighScore() {
  Preferences scores;
  scores.begin("rocket", false);
  if (scores.isKey("high")) _highScore = scores.getUInt("high", 0);
  scores.end();
}

void RocketRush::saveHighScore() {
  Preferences scores;
  scores.begin("rocket", false);
  scores.putUInt("high", _highScore);
  scores.end();
}

void RocketRush::centerText(const char *text, int16_t y, uint8_t size, uint16_t color) {
  _display->setFont(nullptr);
  _display->setTextSize(size);
  _display->setTextColor(color);
  const int16_t width = strlen(text) * 6 * size;
  _display->setCursor((320 - width) / 2, y);
  _display->print(text);
}

void RocketRush::drawBackground() {
  _display->fillScreen(NAVY);
  for (const auto &star : _stars) {
    _display->drawPixel(star.x, star.y, star.color);
    if (star.speed == 3) _display->drawPixel(star.x, star.y + 1, star.color);
  }
}

void RocketRush::drawPlayer(int16_t x, int16_t y) {
  _display->fillTriangle(x, y - 11, x - 10, y + 8, x + 10, y + 8, PINK);
  _display->fillRect(x - 3, y - 5, 7, 12, WHITE16);
  _display->fillRect(x - 2, y - 3, 5, 5, CYAN);
  _display->fillTriangle(x - 3, y + 8, x + 3, y + 8, x, y + 14, YELLOW);
  _display->drawPixel(x, y + 15, ORANGE);
}

void RocketRush::drawEnemy(const Enemy &enemy) {
  const uint16_t color = enemy.color == 0 ? CYAN : (enemy.color == 1 ? PURPLE : GREEN);
  _display->fillRect(enemy.x - 5, enemy.y - 4, 11, 8, color);
  _display->fillRect(enemy.x - 8, enemy.y - 1, 17, 5, color);
  _display->drawPixel(enemy.x - 4, enemy.y - 5, color);
  _display->drawPixel(enemy.x + 4, enemy.y - 5, color);
  _display->drawPixel(enemy.x - 3, enemy.y, NAVY);
  _display->drawPixel(enemy.x + 3, enemy.y, NAVY);
  _display->drawPixel(enemy.x - 7, enemy.y + 5, color);
  _display->drawPixel(enemy.x + 7, enemy.y + 5, color);
}

void RocketRush::drawHud() {
  _display->fillRect(0, 0, 320, 18, 0x0000);
  _display->setFont(nullptr);
  _display->setTextSize(1);
  _display->setTextColor(CYAN);
  _display->setCursor(4, 5);
  _display->print("SCORE ");
  _display->setTextColor(WHITE16);
  _display->printf("%05lu", static_cast<unsigned long>(_score));
  _display->setTextColor(PINK);
  _display->setCursor(116, 5);
  _display->print("HI ");
  _display->setTextColor(WHITE16);
  _display->printf("%05lu", static_cast<unsigned long>(_highScore));
  for (uint8_t i = 0; i < 3; ++i) {
    const int x = 278 + i * 13;
    const uint16_t color = i < _lives ? PINK : DIM;
    _display->fillRect(x - 3, 5, 7, 5, color);
    _display->fillTriangle(x - 3, 10, x + 4, 10, x, 15, color);
  }
}

void RocketRush::drawTitle() {
  centerText("ROCKET", 35, 4, WHITE16);
  centerText("RUSH", 70, 4, PINK);
  drawPlayer(160, 115);
  centerText("PRESS BOTH TO START", 147, 1, CYAN);
}

void RocketRush::drawPaused() {
  _display->fillRoundRect(104, 65, 112, 40, 5, 0x0000);
  _display->drawRoundRect(104, 65, 112, 40, 5, YELLOW);
  centerText("PAUSED", 77, 2, YELLOW);
  centerText("BOTH: RESUME", 96, 1, WHITE16);
}

void RocketRush::drawGameOver() {
  centerText("GAME OVER", 40, 3, PINK);
  char scoreLine[24];
  snprintf(scoreLine, sizeof(scoreLine), "SCORE %05lu", static_cast<unsigned long>(_score));
  centerText(scoreLine, 79, 2, CYAN);
  snprintf(scoreLine, sizeof(scoreLine), "HI %05lu", static_cast<unsigned long>(_highScore));
  centerText(scoreLine, 101, 2, WHITE16);
  centerText("PRESS BOTH TO RESTART", 137, 1, YELLOW);
}

void RocketRush::render() {
  drawBackground();
  if (_state == State::TITLE) {
    drawTitle();
    return;
  }
  if (_state == State::GAME_OVER) {
    drawGameOver();
    return;
  }
  drawHud();
  for (const auto &enemy : _enemies) if (enemy.active) drawEnemy(enemy);
  for (const auto &bullet : _bullets) {
    if (!bullet.active) continue;
    _display->fillRect(bullet.x - 1, bullet.y, 3, 7, YELLOW);
  }
  for (const auto &particle : _particles) {
    if (!particle.active) continue;
    _display->fillRect(particle.xMilli / 10, particle.yMilli / 10, 2, 2, particle.color);
  }
  drawPlayer(_playerX, 154);
  if (millis() < _lifeFlashUntil) _display->drawRect(0, 18, 320, 152, RED16);
  if (_state == State::PAUSED) drawPaused();
}
