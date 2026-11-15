#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class FrogGotchi {
 public:
  enum class Action : uint8_t { FEED, PLAY, CLEAN, SLEEP, LOVE, MEDICINE, FOCUS, COUNT };
  explicit FrogGotchi(Arduino_GFX *display);
  void begin();
  void enter();
  void stop();
  void update();
  bool isActive() const;
  bool consumeExitRequest();
  void performAction(Action action);
  bool setClassicMode(bool enabled);
  bool setBackground(uint8_t index, bool explicitChoice = true);
  bool setPetName(const char *name);
  void startFocus(uint8_t minutes, uint8_t breakMinutes = 5);
  void pauseTimer();
  void resumeTimer();
  void cancelTimer();
  void syncEpoch(uint32_t epoch);
  uint8_t hunger() const { return _hunger; }
  uint8_t energy() const { return _energy; }
  uint8_t happiness() const { return _happiness; }
  uint8_t health() const { return _health; }
  uint8_t cleanliness() const { return _cleanliness; }
  uint8_t background() const { return _background; }
  uint8_t expression() const;
  uint8_t focusMinutes() const { return _focusMinutes; }
  uint8_t breakMinutes() const { return _breakMinutes; }
  uint16_t coins() const { return _coins; }
  uint16_t focusSessions() const { return _focusSessions; }
  uint32_t ageMinutes() const { return _ageMinutes; }
  bool classicMode() const { return _classicMode; }
  bool sleeping() const { return _sleeping; }
  bool alive() const { return _alive; }
  bool focusRunning() const { return _focusRunning; }
  bool breakRunning() const { return _breakRunning; }
  bool timerPaused() const { return _timerPaused; }
  uint32_t timerSecondsRemaining() const;
  const char *petName() const { return _petName; }
  const char *stageName() const;
  const char *moodName() const;
  const char *needMessage() const;
  const char *selectedActionName() const;
  const char *backgroundName() const;
  const char *accessoryName() const;
  const char *quote() const;

 private:
  static constexpr uint8_t LEFT_BUTTON = 0;
  static constexpr uint8_t RIGHT_BUTTON = 14;
  static constexpr uint32_t FRAME_MS = 500;
  static constexpr uint32_t DEBOUNCE_MS = 30;
  static constexpr uint32_t EXIT_HOLD_MS = 1800;
  static constexpr uint32_t SAVE_MS = 300000;
  static constexpr uint8_t MENU_COUNT = 9;
  Arduino_GFX *_display;
  bool _active = false, _exitRequested = false, _classicMode = false;
  bool _sleeping = false, _alive = true, _focusRunning = false, _breakRunning = false;
  bool _timerPaused = false, _backgroundExplicit = false, _statsVisible = false;
  bool _leftRaw = false, _rightRaw = false, _leftPressed = false, _rightPressed = false;
  bool _previousLeft = false, _previousRight = false, _exitHandled = false;
  uint8_t _hunger = 82, _energy = 78, _happiness = 80, _health = 100, _cleanliness = 90;
  uint8_t _menuIndex = 0, _background = 0, _expressionOverride = 7, _quoteIndex = 0;
  uint8_t _effect = 0, _focusMinutes = 25, _breakMinutes = 5;
  uint16_t _coins = 0, _focusSessions = 0;
  uint32_t _ageMinutes = 0, _lastTickAt = 0, _lastFrameAt = 0, _lastSaveAt = 0;
  uint32_t _leftChangedAt = 0, _rightChangedAt = 0, _exitStartedAt = 0;
  uint32_t _overrideUntil = 0, _effectUntil = 0, _timerEndsAt = 0, _pausedSeconds = 0;
  uint32_t _savedEpoch = 0;
  char _petName[13] = "Mochi";
  char _feedback[24] = "WELCOME HOME";
  void load();
  void save();
  void applyElapsedMinutes(uint32_t minutes);
  void tickMinute();
  void updateButton(bool raw, bool &lastRaw, bool &stable, uint32_t &changedAt, uint32_t now);
  void updateControls(uint32_t now);
  void updateTimer(uint32_t now);
  void activateMenuItem();
  void render();
  void drawBackground();
  void drawPet(int16_t x, int16_t y);
  void drawEgg(int16_t x, int16_t y);
  void drawTadpole(int16_t x, int16_t y);
  void drawMainScreen();
  void drawStatsScreen();
  void drawFocusScreen();
  void drawBar(int16_t x, int16_t y, int16_t width, uint8_t value, uint16_t color);
  uint8_t expressionIndex() const;
  void setExpression(uint8_t index, uint32_t durationMs = 2200, uint8_t effect = 0);
  void hatchIfNeeded();
  static uint8_t addClamped(uint8_t value, int16_t delta);
};
