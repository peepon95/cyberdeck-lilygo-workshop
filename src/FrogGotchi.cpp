#include "FrogGotchi.h"

#include <Preferences.h>
#include "frog_backgrounds.h"
#include "frog_sprites.h"

namespace {
constexpr uint16_t INK = 0x2963;
constexpr uint16_t CREAM = 0xFF5A;
constexpr uint16_t PINK = 0xFB56;
constexpr uint16_t FROG_CYAN = 0x37FF;
constexpr uint16_t FROG_YELLOW = 0xFFE7;
constexpr uint16_t FROG_RED = 0xF986;
constexpr uint16_t FROG_GREEN = 0x866F;
constexpr uint16_t WHITE16 = 0xFFFF;
const char *const MENU_NAMES[] = {"FEED", "PLAY", "CLEAN", "SLEEP", "LOVE", "MEDICINE", "FOCUS", "STATS", "BACKGROUND"};
const char *const BACKGROUND_NAMES[] = {"Cozy Room", "Study Desk", "Frog Pond"};
const char *const QUOTES[] = {"Tiny steps count.", "You are doing great!", "Focus, then frolic.",
                              "One task at a time.", "Progress over perfect.", "Your frog believes!"};
}

FrogGotchi::FrogGotchi(Arduino_GFX *display) : _display(display) {}

void FrogGotchi::begin() {
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  load();
}

void FrogGotchi::enter() {
  _active = true;
  _exitRequested = _exitHandled = false;
  _lastFrameAt = 0;
  _leftRaw = _rightRaw = _leftPressed = _rightPressed = false;
  _previousLeft = _previousRight = false;
  render();
}

void FrogGotchi::stop() { if (_active) save(); _active = false; }
bool FrogGotchi::isActive() const { return _active; }
bool FrogGotchi::consumeExitRequest() { bool value = _exitRequested; _exitRequested = false; return value; }
uint8_t FrogGotchi::addClamped(uint8_t value, int16_t delta) { return constrain((int16_t)value + delta, 0, 100); }

void FrogGotchi::hatchIfNeeded() {
  if (_ageMinutes == 0) {
    _ageMinutes = 1;
    _happiness = 90;
    snprintf(_feedback, sizeof(_feedback), "HELLO, %s!", _petName);
    setExpression(7, 3500, 3);
  }
}

void FrogGotchi::performAction(Action action) {
  if (!_alive && action != Action::LOVE && action != Action::MEDICINE) return;
  if (!_alive) {
    _alive = true; _health = 60; _hunger = _energy = _happiness = 50; _cleanliness = 70; _ageMinutes = 1;
  }
  hatchIfNeeded();
  switch (action) {
    case Action::FEED:
      _hunger = addClamped(_hunger, 24); _health = addClamped(_health, 3); ++_coins;
      strcpy(_feedback, "YUMMY!"); setExpression(9, 2500, 2); break;
    case Action::PLAY:
      _happiness = addClamped(_happiness, 20); _energy = addClamped(_energy, -6); _hunger = addClamped(_hunger, -3);
      strcpy(_feedback, "FUN!"); setExpression(6, 2800, 2); break;
    case Action::CLEAN:
      _cleanliness = 100; _health = addClamped(_health, 4);
      strcpy(_feedback, "SQUEAKY CLEAN"); setExpression(3, 2400, 3); break;
    case Action::SLEEP:
      _sleeping = !_sleeping;
      if (_sleeping) { cancelTimer(); strcpy(_feedback, "GOOD NIGHT"); setExpression(10, 5000); }
      else { strcpy(_feedback, "GOOD MORNING"); setExpression(7, 2200, 3); }
      break;
    case Action::LOVE:
      _happiness = addClamped(_happiness, 20); _health = addClamped(_health, 2);
      strcpy(_feedback, "LOVE YOU!"); setExpression(8, 3200, 1); break;
    case Action::MEDICINE:
      if (_health < 85) { _health = addClamped(_health, 25); _coins = _coins > 1 ? _coins - 2 : 0; strcpy(_feedback, "FEELING BETTER"); }
      else strcpy(_feedback, "ALREADY HEALTHY");
      setExpression(3, 2400, 3); break;
    case Action::FOCUS: startFocus(_focusMinutes); break;
    case Action::COUNT: break;
  }
  save();
  if (_active) render();
}

bool FrogGotchi::setClassicMode(bool enabled) {
  _classicMode = enabled;
  if (!enabled && !_alive) { _alive = true; _health = 20; }
  save(); if (_active) render(); return _classicMode;
}

bool FrogGotchi::setBackground(uint8_t index, bool explicitChoice) {
  if (index > 2) return false;
  _background = index;
  if (explicitChoice) _backgroundExplicit = true;
  snprintf(_feedback, sizeof(_feedback), "%s", BACKGROUND_NAMES[index]);
  save(); if (_active) render(); return true;
}

bool FrogGotchi::setPetName(const char *name) {
  if (!name) return false;
  size_t length = strlen(name);
  if (length < 1 || length > 12) return false;
  for (size_t i = 0; i < length; ++i) if (!isalnum((unsigned char)name[i]) && name[i] != ' ' && name[i] != '-') return false;
  strncpy(_petName, name, sizeof(_petName) - 1); _petName[sizeof(_petName) - 1] = '\0';
  strcpy(_feedback, "NAME SAVED"); save(); if (_active) render(); return true;
}

void FrogGotchi::startFocus(uint8_t minutes, uint8_t breakMinutes) {
  _focusMinutes = constrain(minutes, 1, 60);
  _breakMinutes = constrain(breakMinutes, 1, 30);
  if (!_backgroundExplicit) _background = 1;
  _sleeping = false; _focusRunning = true; _breakRunning = false; _timerPaused = false; _pausedSeconds = 0;
  _timerEndsAt = millis() + (uint32_t)_focusMinutes * 60000UL;
  strcpy(_feedback, "YOU'VE GOT THIS"); setExpression(2, 5000); save(); if (_active) render();
}

void FrogGotchi::pauseTimer() {
  if ((!_focusRunning && !_breakRunning) || _timerPaused) return;
  _pausedSeconds = timerSecondsRemaining(); _timerPaused = true; strcpy(_feedback, "TIMER PAUSED"); if (_active) render();
}

void FrogGotchi::resumeTimer() {
  if (!_timerPaused || (!_focusRunning && !_breakRunning)) return;
  _timerEndsAt = millis() + _pausedSeconds * 1000UL; _timerPaused = false; strcpy(_feedback, "BACK TO FOCUS"); if (_active) render();
}

void FrogGotchi::cancelTimer() {
  _focusRunning = _breakRunning = _timerPaused = false; _timerEndsAt = _pausedSeconds = 0;
  strcpy(_feedback, "TIMER STOPPED"); if (_active) render();
}

uint32_t FrogGotchi::timerSecondsRemaining() const {
  if ((!_focusRunning && !_breakRunning) || _timerEndsAt == 0) return _timerPaused ? _pausedSeconds : 0;
  if (_timerPaused) return _pausedSeconds;
  int32_t remaining = (int32_t)(_timerEndsAt - millis());
  return remaining > 0 ? (uint32_t)remaining / 1000 : 0;
}

void FrogGotchi::syncEpoch(uint32_t epoch) {
  if (_savedEpoch > 0 && epoch > _savedEpoch) applyElapsedMinutes(min<uint32_t>((epoch - _savedEpoch) / 60, 10080));
  _savedEpoch = epoch; save(); if (_active) render();
}

void FrogGotchi::applyElapsedMinutes(uint32_t minutes) { for (uint32_t i = 0; i < minutes; ++i) tickMinute(); }

void FrogGotchi::tickMinute() {
  if (!_alive || _ageMinutes == 0) return;
  ++_ageMinutes;
  if (_sleeping) { _energy = addClamped(_energy, 3); _hunger = addClamped(_hunger, -1); }
  else {
    if (_ageMinutes % 6 == 0) _hunger = addClamped(_hunger, -1);
    if (_ageMinutes % 5 == 0) _energy = addClamped(_energy, -1);
    if (_ageMinutes % 8 == 0) _happiness = addClamped(_happiness, -1);
  }
  if (_ageMinutes % 10 == 0) _cleanliness = addClamped(_cleanliness, -1);
  bool neglected = _hunger < 15 || _energy < 10 || _cleanliness < 15 || _happiness < 10;
  if (neglected) _health = addClamped(_health, -2);
  else if (_hunger > 55 && _cleanliness > 50 && _happiness > 50) _health = addClamped(_health, 1);
  if (_health == 0) { if (_classicMode) _alive = false; else _health = 1; }
}

void FrogGotchi::updateButton(bool raw, bool &lastRaw, bool &stable, uint32_t &changedAt, uint32_t now) {
  if (raw != lastRaw) { lastRaw = raw; changedAt = now; }
  if (now - changedAt >= DEBOUNCE_MS) stable = lastRaw;
}

void FrogGotchi::activateMenuItem() {
  _statsVisible = false;
  if (_menuIndex < (uint8_t)Action::COUNT) performAction((Action)_menuIndex);
  else if (_menuIndex == 7) { _statsVisible = true; strcpy(_feedback, "PET STATS"); render(); }
  else { setBackground((_background + 1) % 3); }
}

void FrogGotchi::updateControls(uint32_t now) {
  updateButton(digitalRead(LEFT_BUTTON) == LOW, _leftRaw, _leftPressed, _leftChangedAt, now);
  updateButton(digitalRead(RIGHT_BUTTON) == LOW, _rightRaw, _rightPressed, _rightChangedAt, now);
  bool both = _leftPressed && _rightPressed;
  if (both && _exitStartedAt == 0) _exitStartedAt = now;
  if (both && !_exitHandled && now - _exitStartedAt >= EXIT_HOLD_MS) { _exitHandled = true; _exitRequested = true; }
  if (!both) {
    _exitStartedAt = 0; _exitHandled = false;
    if (_leftPressed && !_previousLeft) { _statsVisible = false; _menuIndex = (_menuIndex + 1) % MENU_COUNT; render(); }
    if (_rightPressed && !_previousRight) activateMenuItem();
  }
  _previousLeft = _leftPressed; _previousRight = _rightPressed;
}

void FrogGotchi::updateTimer(uint32_t now) {
  if (_timerPaused || (!_focusRunning && !_breakRunning) || (int32_t)(now - _timerEndsAt) < 0) return;
  if (_focusRunning) {
    _focusRunning = false; _breakRunning = true; _timerEndsAt = now + (uint32_t)_breakMinutes * 60000UL;
    ++_focusSessions; _coins += 5; _happiness = addClamped(_happiness, 10);
    _quoteIndex = (_quoteIndex + 1) % (sizeof(QUOTES) / sizeof(QUOTES[0]));
    strcpy(_feedback, "FOCUS COMPLETE!"); setExpression(6, 7000, 3);
  } else {
    _breakRunning = false; _timerEndsAt = 0; strcpy(_feedback, "BREAK COMPLETE"); setExpression(3, 5000, 3);
  }
  save();
}

void FrogGotchi::update() {
  if (!_active) return;
  uint32_t now = millis(); updateControls(now); updateTimer(now);
  if (_lastTickAt == 0) _lastTickAt = now;
  while (now - _lastTickAt >= 60000UL) { _lastTickAt += 60000UL; tickMinute(); }
  if (now - _lastSaveAt >= SAVE_MS) { _lastSaveAt = now; save(); }
  if (now - _lastFrameAt >= FRAME_MS) { _lastFrameAt = now; render(); }
}

const char *FrogGotchi::stageName() const {
  if (!_alive) return "departed"; if (_ageMinutes == 0) return "egg"; if (_ageMinutes < 60) return "hatchling";
  if (_ageMinutes < 1440) return "young frog"; if (_ageMinutes < 10080) return "grown frog"; return "evolved frog";
}
const char *FrogGotchi::moodName() const {
  if (!_alive) return "gone"; if (_sleeping) return "sleeping"; if (_health < 20 || _hunger < 15) return "needs help";
  if (_energy < 25) return "tired"; if (_happiness < 30) return "sad"; if (_focusRunning) return "studying";
  if (_happiness > 85) return "delighted"; return "content";
}
const char *FrogGotchi::needMessage() const {
  if (!_alive) return "NEEDS MEDICINE"; if (_health < 30) return "NOT FEELING WELL"; if (_hunger < 30) return "FEED ME!";
  if (_energy < 25) return "SLEEPY!"; if (_cleanliness < 30) return "BATH TIME!"; if (_happiness < 35) return "PLAY WITH ME!"; return _feedback;
}
const char *FrogGotchi::selectedActionName() const { return MENU_NAMES[_menuIndex]; }
const char *FrogGotchi::backgroundName() const { return BACKGROUND_NAMES[_background]; }
const char *FrogGotchi::accessoryName() const { if (_focusSessions >= 10) return "focus headphones"; if (_focusSessions >= 5) return "star crown"; if (_focusSessions >= 2) return "cool glasses"; return "none yet"; }
const char *FrogGotchi::quote() const { return QUOTES[_quoteIndex]; }

void FrogGotchi::setExpression(uint8_t index, uint32_t durationMs, uint8_t effect) {
  _expressionOverride = index % 12; _overrideUntil = millis() + durationMs; _effect = effect; _effectUntil = millis() + durationMs;
}
uint8_t FrogGotchi::expression() const { return expressionIndex(); }
uint8_t FrogGotchi::expressionIndex() const {
  if ((int32_t)(_overrideUntil - millis()) > 0) return _expressionOverride;
  if (_sleeping) return 10; if (_health < 20 || _hunger < 12) return 11; if (_energy < 25) return 1;
  if (_happiness < 28) return 5; if (_focusRunning) return 2; if (_happiness > 90) return 6;
  if (_happiness > 80) return 3; if (_hunger < 40) return 4; return 7;
}

void FrogGotchi::drawBackground() {
  const uint16_t *background = reinterpret_cast<const uint16_t *>(pgm_read_ptr(&FROG_BACKGROUNDS[_background]));
  _display->draw16bitRGBBitmap(0, 0, background, FROG_BG_W, FROG_BG_H);
}

void FrogGotchi::drawBar(int16_t x, int16_t y, int16_t width, uint8_t value, uint16_t color) {
  _display->fillRoundRect(x, y, width, 8, 3, CREAM); _display->drawRoundRect(x, y, width, 8, 3, INK);
  _display->fillRect(x + 2, y + 2, (width - 4) * value / 100, 4, color);
}

void FrogGotchi::drawEgg(int16_t x, int16_t y) {
  _display->fillEllipse(x + 48, y + 48, 28, 38, CREAM); _display->drawEllipse(x + 48, y + 48, 28, 38, INK);
  _display->fillRect(x + 34, y + 45, 7, 7, FROG_GREEN); _display->fillRect(x + 53, y + 30, 8, 8, PINK);
}
void FrogGotchi::drawTadpole(int16_t x, int16_t y) {
  _display->fillCircle(x + 43, y + 49, 25, FROG_GREEN); _display->fillTriangle(x + 66, y + 48, x + 92, y + 31, x + 84, y + 68, FROG_GREEN);
  _display->fillCircle(x + 36, y + 43, 3, INK);
}

void FrogGotchi::drawPet(int16_t x, int16_t y) {
  if (!_alive) { _display->fillRoundRect(x, y + 24, 96, 48, 7, INK); _display->setTextColor(WHITE16); _display->setTextSize(1); _display->setCursor(x + 20, y + 38); _display->print("RESTING..."); _display->setCursor(x + 9, y + 55); _display->print("LOVE TO REVIVE"); return; }
  int16_t bounce = (_effect == 2 && (int32_t)(_effectUntil - millis()) > 0 && ((millis() / 180) & 1)) ? -5 : 0;
  if (_ageMinutes == 0) drawEgg(x, y + bounce);
  else if (_ageMinutes < 5) drawTadpole(x, y + bounce);
  else {
    uint8_t index = expressionIndex();
    const uint16_t *sprite = reinterpret_cast<const uint16_t *>(pgm_read_ptr(&FROG_SPRITES[index]));
    const uint8_t *mask = reinterpret_cast<const uint8_t *>(pgm_read_ptr(&FROG_SPRITE_MASKS[index]));
    _display->draw16bitRGBBitmapWithMask(x, y + bounce, sprite, mask, FROG_SPRITE_W, FROG_SPRITE_H);
    if (!_sleeping && millis() % 7000UL < 600UL) {
      _display->drawFastHLine(x + 29, y + 28 + bounce, 10, INK);
      _display->drawFastHLine(x + 57, y + 28 + bounce, 10, INK);
    }
  }
  if (_effect == 1 && (int32_t)(_effectUntil - millis()) > 0) {
    _display->setTextColor(PINK); _display->setTextSize(2); _display->setCursor(x + 3, y + 18); _display->print("<3"); _display->setCursor(x + 75, y + 7); _display->print("<3");
  }
  if (_sleeping) { _display->setTextColor(FROG_CYAN); _display->setTextSize(2); _display->setCursor(x + 77, y + 8); _display->print("Z"); }
  if (_effect == 3 && (int32_t)(_effectUntil - millis()) > 0) {
    _display->setTextColor(FROG_YELLOW); _display->setTextSize(2); _display->setCursor(x + 1, y + 2); _display->print("*"); _display->setCursor(x + 83, y + 23); _display->print("*");
  }
}

void FrogGotchi::drawMainScreen() {
  _display->fillRoundRect(5, 4, 310, 22, 6, INK); _display->setTextColor(WHITE16); _display->setTextSize(1);
  _display->setCursor(12, 11); _display->print(_petName); _display->setCursor(122, 11); _display->print(needMessage());
  _display->setTextColor(FROG_YELLOW); _display->setCursor(271, 11); _display->print(_coins); _display->print("c");
  drawPet(112, 31);
  _display->fillRoundRect(6, 132, 308, 33, 7, INK);
  _display->setTextColor(FROG_YELLOW); _display->setTextSize(2); _display->setCursor(13, 139); _display->print("< "); _display->print(selectedActionName());
  _display->setTextColor(WHITE16); _display->setTextSize(1); _display->setCursor(222, 140); _display->print("B: SELECT");
  _display->setCursor(222, 153); _display->print("HOLD BOTH: BACK");
  drawBar(13, 153, 86, _hunger, FROG_GREEN); drawBar(111, 153, 86, _energy, FROG_CYAN);
}

void FrogGotchi::drawStatsScreen() {
  _display->fillRoundRect(12, 8, 296, 154, 10, CREAM); _display->drawRoundRect(12, 8, 296, 154, 10, INK);
  _display->setTextColor(INK); _display->setTextSize(2); _display->setCursor(28, 20); _display->print(_petName); _display->setTextSize(1);
  const char *labels[] = {"FULLNESS", "ENERGY", "HAPPINESS", "HEALTH", "CLEAN"};
  uint8_t values[] = {_hunger, _energy, _happiness, _health, _cleanliness};
  uint16_t colors[] = {FROG_GREEN, FROG_CYAN, PINK, FROG_RED, FROG_YELLOW};
  for (uint8_t i = 0; i < 5; ++i) { int y = 48 + i * 19; _display->setCursor(28, y); _display->print(labels[i]); drawBar(110, y - 2, 170, values[i], colors[i]); }
  _display->setCursor(28, 145); _display->print(stageName()); _display->setCursor(190, 145); _display->print("LEFT: BACK");
}

void FrogGotchi::drawFocusScreen() {
  _display->fillRoundRect(8, 5, 304, 24, 7, INK); _display->setTextColor(WHITE16); _display->setTextSize(2); _display->setCursor(18, 9); _display->print(_breakRunning ? "BREAK TIME" : "FOCUS TIME");
  drawPet(23, 43);
  _display->fillRoundRect(131, 43, 178, 91, 9, CREAM); _display->drawRoundRect(131, 43, 178, 91, 9, INK);
  uint32_t seconds = timerSecondsRemaining(); char timer[8]; snprintf(timer, sizeof(timer), "%02lu:%02lu", seconds / 60, seconds % 60);
  _display->setTextColor(INK); _display->setTextSize(4); _display->setCursor(148, 61); _display->print(timer);
  _display->setTextSize(1); _display->setCursor(149, 108); _display->print(_timerPaused ? "PAUSED" : quote());
  _display->fillRoundRect(8, 140, 304, 25, 6, INK); _display->setTextColor(FROG_YELLOW); _display->setTextSize(1); _display->setCursor(17, 149); _display->print("WEB: PAUSE / RESUME / STOP");
}

void FrogGotchi::render() {
  drawBackground(); _display->setFont(nullptr);
  if (_focusRunning || _breakRunning) drawFocusScreen(); else if (_statsVisible) drawStatsScreen(); else drawMainScreen();
}

void FrogGotchi::load() {
  Preferences prefs; prefs.begin("frogpet", true);
  _hunger = prefs.getUChar("hunger", 82); _energy = prefs.getUChar("energy", 78); _happiness = prefs.getUChar("happy", 80);
  _health = prefs.getUChar("health", 100); _cleanliness = prefs.getUChar("clean", 90); _coins = prefs.getUShort("coins", 0);
  _focusSessions = prefs.getUShort("focus", 0); _ageMinutes = prefs.getULong("age", 0); _savedEpoch = prefs.getULong("epoch", 0);
  _classicMode = prefs.getBool("classic", false); _alive = prefs.getBool("alive", true); _quoteIndex = prefs.getUChar("quote", 0) % 6;
  _background = min<uint8_t>(prefs.getUChar("bg", 0), 2); _backgroundExplicit = prefs.getBool("bgSet", false);
  _focusMinutes = constrain(prefs.getUChar("focusMin", 25), 1, 60); _breakMinutes = constrain(prefs.getUChar("breakMin", 5), 1, 30);
  _statsVisible = prefs.getBool("statsView", false);
  String savedName = prefs.getString("name", "Mochi"); savedName.toCharArray(_petName, sizeof(_petName)); prefs.end();
}

void FrogGotchi::save() {
  Preferences prefs; prefs.begin("frogpet", false);
  prefs.putUChar("hunger", _hunger); prefs.putUChar("energy", _energy); prefs.putUChar("happy", _happiness);
  prefs.putUChar("health", _health); prefs.putUChar("clean", _cleanliness); prefs.putUShort("coins", _coins);
  prefs.putUShort("focus", _focusSessions); prefs.putULong("age", _ageMinutes); prefs.putULong("epoch", _savedEpoch);
  prefs.putBool("classic", _classicMode); prefs.putBool("alive", _alive); prefs.putUChar("quote", _quoteIndex);
  prefs.putUChar("bg", _background); prefs.putBool("bgSet", _backgroundExplicit); prefs.putUChar("focusMin", _focusMinutes); prefs.putUChar("breakMin", _breakMinutes);
  prefs.putBool("statsView", _statsVisible); prefs.putString("name", _petName); prefs.end();
}
