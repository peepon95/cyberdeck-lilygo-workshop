#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include "FrogGotchi.h"
#include "RocketRush.h"
#include "cyberdeck_page.h"
#include "froggotchi_page.h"
#include "frog_backgrounds.h"
#include "frog_web_assets.h"
#include "studio_page.h"
#include "fonts/FreeMono8pt7b.h"
#include "fonts/FreeSansBold10pt7b.h"
#include "fonts/FreeSerifBoldItalic12pt7b.h"

// Official LILYGO T-Display-S3 pin map (standard 1.9-inch model).
constexpr int LCD_POWER = 15;
constexpr int LCD_BACKLIGHT = 38;
constexpr uint8_t SETUP_AP_CHANNEL = 6;
constexpr uint8_t SETUP_AP_MAX_CLIENTS = 4;

Preferences preferences;
DNSServer dnsServer;
WebServer webServer(80);
String setupSsid;
String setupPassword;
bool setupPortalRunning = false;
uint8_t lastStationCount = 0;

struct HairpinDesign {
  String text = "EE VON";
  String font = "bold";
  String textColor = "#FFFFFF";
  String backgroundColor = "#000000";
  uint8_t size = 3;
  String mode = "center";
  uint16_t speed = 60;
};

HairpinDesign design;
int32_t marqueePositionMilli = 320000;
uint32_t lastMarqueeMillis = 0;

Arduino_DataBus *bus = new Arduino_ESP32PAR8Q(
    7, 6, 8, 9, 39, 40, 41, 42, 45, 46, 47, 48);
Arduino_GFX *display = new Arduino_ST7789(
    bus, 5, 0, true, 170, 320, 35, 0, 35, 0);
RocketRush rocketRush(display);
FrogGotchi frogGotchi(display);

enum class DeviceMode : uint8_t { MENU, DESIGN, GAME, FROG };
DeviceMode deviceMode = DeviceMode::MENU;
uint8_t menuSelection = 0;
constexpr uint8_t LEFT_BUTTON = 0;
constexpr uint8_t RIGHT_BUTTON = 14;
constexpr uint32_t MENU_DEBOUNCE_MS = 30;
constexpr uint32_t MENU_RETURN_HOLD_MS = 1500;
bool leftStable = false;
bool rightStable = false;
bool leftRaw = false;
bool rightRaw = false;
bool menuChordLatched = false;
bool returnHandled = false;
uint32_t leftChangedAt = 0;
uint32_t rightChangedAt = 0;
uint32_t returnStartedAt = 0;

void drawCenteredText(const char *text, int textSize, int centerY, uint16_t color) {
  const int textWidth = strlen(text) * 6 * textSize;
  const int textHeight = 8 * textSize;
  display->setTextSize(textSize);
  display->setTextColor(color);
  display->setCursor((display->width() - textWidth) / 2, centerY - textHeight / 2);
  display->print(text);
}

void drawHelloScreen() {
  display->fillScreen(BLACK);
  const uint16_t pink = display->color565(255, 105, 180);
  drawCenteredText("HELLO WORLD!", 4, 62, WHITE);
  drawCenteredText("MY FIRST LILYGO", 2, 112, pink);
}

void drawSetupScreen() {
  display->fillScreen(BLACK);
  drawCenteredText("CYBERDECK OS", 3, 20, WHITE);
  drawCenteredText("Connect to:", 1, 55, WHITE);
  drawCenteredText(setupSsid.c_str(), 2, 76, display->color565(255, 105, 180));
  drawCenteredText("Password:", 1, 101, WHITE);
  drawCenteredText(setupPassword.c_str(), 2, 120, WHITE);
  drawCenteredText("Open: 192.168.4.1", 1, 151, WHITE);
}

void drawDeviceMenu() {
  static const char *items[] = {"DISPLAY STUDIO", "ROCKET RUSH", "CYBER PET"};
  display->setFont(nullptr);
  display->fillScreen(display->color565(8, 10, 19));
  drawCenteredText("CYBERDECK OS", 2, 16, display->color565(255, 92, 168));
  for (uint8_t i = 0; i < 3; ++i) {
    const int y = 48 + i * 34;
    const bool selected = i == menuSelection;
    if (selected) display->fillRoundRect(18, y - 8, 284, 28, 6, display->color565(94, 231, 238));
    display->setTextSize(2);
    display->setTextColor(selected ? BLACK : WHITE);
    display->setCursor(31, y);
    display->print("> ");
    display->print(items[i]);
  }
  drawCenteredText("LEFT/RIGHT + BOTH TO OPEN", 1, 158, display->color565(154, 163, 187));
}

bool isHexColor(const String &value) {
  if (value.length() != 7 || value[0] != '#') return false;
  for (size_t i = 1; i < 7; ++i) {
    if (!isxdigit(static_cast<unsigned char>(value[i]))) return false;
  }
  return true;
}

uint16_t colorFromHex(const String &value) {
  const uint32_t rgb = strtoul(value.substring(1).c_str(), nullptr, 16);
  return display->color565((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

bool isAllowedValue(const String &value, const char *const *allowed, size_t count) {
  for (size_t i = 0; i < count; ++i) if (value == allowed[i]) return true;
  return false;
}

bool isDisplayableText(const String &value) {
  if (value.isEmpty() || value.length() > 48) return false;
  for (size_t i = 0; i < value.length(); ++i) {
    const uint8_t c = value[i];
    if (c < 32 || c > 126) return false;
  }
  return true;
}

void selectDesignFont() {
  if (design.font == "serif") display->setFont(&FreeSerifBoldItalic12pt7b);
  else if (design.font == "mono") display->setFont(&FreeMono8pt7b);
  else if (design.font == "pixel") display->setFont(nullptr);
  else display->setFont(&FreeSansBold10pt7b);
}

uint8_t fitText(const String &text, uint8_t requestedScale, uint16_t maxWidth, uint16_t maxHeight,
                int16_t &x1, int16_t &y1, uint16_t &w, uint16_t &h) {
  uint8_t scale = requestedScale;
  while (true) {
    display->setTextSize(scale);
    display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    if ((w <= maxWidth && h <= maxHeight) || scale == 1) return scale;
    --scale;
  }
}

void drawTextAtCenter(const String &text, int centerX, int centerY, uint16_t color,
                      uint8_t requestedScale, uint16_t maxWidth, uint16_t maxHeight, bool outline) {
  int16_t x1, y1;
  uint16_t w, h;
  fitText(text, requestedScale, maxWidth, maxHeight, x1, y1, w, h);
  const int x = centerX - static_cast<int>(w) / 2 - x1;
  const int y = centerY - static_cast<int>(h) / 2 - y1;
  if (outline) {
    display->setTextColor(color);
    for (int dx = -2; dx <= 2; ++dx) {
      for (int dy = -2; dy <= 2; ++dy) {
        if (abs(dx) + abs(dy) < 2) continue;
        display->setCursor(x + dx, y + dy);
        display->print(text);
      }
    }
    display->setTextColor(colorFromHex(design.backgroundColor));
  } else {
    display->setTextColor(color);
  }
  display->setCursor(x, y);
  display->print(text);
}

void renderStaticDesign() {
  const uint16_t foreground = colorFromHex(design.textColor);
  const uint16_t background = colorFromHex(design.backgroundColor);
  display->fillScreen(background);
  selectDesignFont();
  uint8_t scale = design.font == "pixel" ? design.size * 2 : design.size;

  if (design.mode == "banner") {
    display->fillRect(0, 42, 320, 86, foreground);
    drawTextAtCenter(design.text, 160, 85, background, scale, 304, 76, false);
  } else if (design.mode == "outline") {
    drawTextAtCenter(design.text, 160, 85, foreground, scale, 304, 150, true);
  } else {
    int16_t x1, y1;
    uint16_t w, h;
    fitText(design.text, scale, 304, 150, x1, y1, w, h);
    if (w > 304 && design.text.indexOf(' ') > 0) {
      int split = design.text.length() / 2;
      int before = design.text.lastIndexOf(' ', split);
      int after = design.text.indexOf(' ', split);
      if (before < 0 || (after >= 0 && after - split < split - before)) split = after;
      else split = before;
      drawTextAtCenter(design.text.substring(0, split), 160, 55, foreground, scale, 304, 70, false);
      drawTextAtCenter(design.text.substring(split + 1), 160, 115, foreground, scale, 304, 70, false);
    } else {
      drawTextAtCenter(design.text, 160, 85, foreground, scale, 304, 150, false);
    }
  }
}

void resetMarquee() {
  marqueePositionMilli = 320000;
  lastMarqueeMillis = millis();
}

void renderMarqueeFrame() {
  const uint16_t foreground = colorFromHex(design.textColor);
  const uint16_t background = colorFromHex(design.backgroundColor);
  display->fillScreen(background);
  selectDesignFont();
  display->setTextSize(design.font == "pixel" ? design.size * 2 : design.size);
  int16_t x1, y1;
  uint16_t w, h;
  display->getTextBounds(design.text, 0, 0, &x1, &y1, &w, &h);
  const int x = marqueePositionMilli / 1000;
  const int y = 85 - static_cast<int>(h) / 2 - y1;
  display->setTextColor(foreground);
  display->setCursor(x - x1, y);
  display->print(design.text);
  if (x + static_cast<int>(w) < 0) marqueePositionMilli = 320000;
}

void renderDesign() {
  if (design.mode == "marquee") {
    resetMarquee();
    renderMarqueeFrame();
  } else {
    renderStaticDesign();
  }
}

void updateMarquee() {
  if (design.mode != "marquee") return;
  const uint32_t now = millis();
  const uint32_t elapsed = now - lastMarqueeMillis;
  if (elapsed < 25) return;
  lastMarqueeMillis = now;
  marqueePositionMilli -= static_cast<int32_t>(design.speed) * elapsed;
  renderMarqueeFrame();
}

void saveDesign() {
  preferences.begin("design", false);
  preferences.putString("text", design.text);
  preferences.putString("font", design.font);
  preferences.putString("fg", design.textColor);
  preferences.putString("bg", design.backgroundColor);
  preferences.putUChar("size", design.size);
  preferences.putString("mode", design.mode);
  preferences.putUShort("speed", design.speed);
  preferences.end();
}

void loadDesign() {
  preferences.begin("design", false);
  if (preferences.isKey("text")) design.text = preferences.getString("text", design.text);
  if (preferences.isKey("font")) design.font = preferences.getString("font", design.font);
  if (preferences.isKey("fg")) design.textColor = preferences.getString("fg", design.textColor);
  if (preferences.isKey("bg")) design.backgroundColor = preferences.getString("bg", design.backgroundColor);
  if (preferences.isKey("size")) design.size = preferences.getUChar("size", design.size);
  if (preferences.isKey("mode")) design.mode = preferences.getString("mode", design.mode);
  if (preferences.isKey("speed")) design.speed = preferences.getUShort("speed", design.speed);
  preferences.end();
  const char *fonts[] = {"bold", "serif", "pixel", "mono"};
  const char *modes[] = {"center", "banner", "outline", "marquee"};
  if (!isDisplayableText(design.text)) design.text = "EE VON";
  if (!isAllowedValue(design.font, fonts, 4)) design.font = "bold";
  if (!isAllowedValue(design.mode, modes, 4)) design.mode = "center";
  if (!isHexColor(design.textColor)) design.textColor = "#FFFFFF";
  if (!isHexColor(design.backgroundColor)) design.backgroundColor = "#000000";
  design.size = constrain(design.size, 1, 4);
  design.speed = constrain(design.speed, 10, 180);
}

const char *resetReasonName(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "power-on";
    case ESP_RST_EXT: return "external-pin";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt-watchdog";
    case ESP_RST_TASK_WDT: return "task-watchdog";
    case ESP_RST_WDT: return "other-watchdog";
    case ESP_RST_DEEPSLEEP: return "deep-sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "SDIO";
    default: return "unknown";
  }
}

void printMac(const uint8_t *mac) {
  char value[18];
  snprintf(value, sizeof(value), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(value);
}

void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("[event] Setup AP started");
      break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
      Serial.println("[event] Setup AP stopped");
      break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.print("[event] AP station associated: ");
      printMac(info.wifi_ap_staconnected.mac);
      Serial.print(", AID=");
      Serial.println(info.wifi_ap_staconnected.aid);
      break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.print("[event] AP station disconnected: ");
      printMac(info.wifi_ap_stadisconnected.mac);
      Serial.print(", AID=");
      Serial.println(info.wifi_ap_stadisconnected.aid);
      Serial.println("[event] AP disconnect reason is not exposed by Arduino-ESP32 2.0.14");
      break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.print("[event] DHCP assigned AP client IP: ");
      Serial.println(IPAddress(info.wifi_ap_staipassigned.ip.addr));
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.print("[event] Hotspot STA disconnected, reason=");
      Serial.println(info.wifi_sta_disconnected.reason);
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[event] Hotspot DHCP address: ");
      Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
      break;
    default:
      break;
  }
}

void logHttpClient(const char *route) {
  Serial.print("[http] ");
  Serial.print(route);
  Serial.print(" requested by ");
  Serial.println(webServer.client().remoteIP());
}

void redirectToPortal() {
  logHttpClient("captive redirect");
  webServer.sendHeader("Location", "http://192.168.4.1/", true);
  webServer.send(302, "text/plain", "");
}

void writeDesignJson(JsonObject target, const HairpinDesign &value) {
  target["text"] = value.text;
  target["font"] = value.font;
  target["textColor"] = value.textColor;
  target["backgroundColor"] = value.backgroundColor;
  target["size"] = value.size;
  target["mode"] = value.mode;
  target["speed"] = value.speed;
}

void sendCurrentDesign() {
  StaticJsonDocument<384> document;
  writeDesignJson(document.to<JsonObject>(), design);
  String response;
  serializeJson(document, response);
  webServer.send(200, "application/json", response);
}

void sendDesignError(const String &message) {
  StaticJsonDocument<160> document;
  document["error"] = message;
  String response;
  serializeJson(document, response);
  webServer.send(400, "application/json", response);
}

void handleDesignPost() {
  if (!webServer.hasArg("plain")) {
    sendDesignError("Missing JSON body");
    return;
  }

  StaticJsonDocument<512> document;
  const DeserializationError error = deserializeJson(document, webServer.arg("plain"));
  if (error) {
    sendDesignError("Invalid JSON");
    return;
  }

  HairpinDesign candidate = design;
  candidate.text = document["text"] | "";
  candidate.font = document["font"] | "";
  candidate.textColor = String(static_cast<const char *>(document["textColor"] | ""));
  candidate.backgroundColor = String(static_cast<const char *>(document["backgroundColor"] | ""));
  candidate.textColor.toUpperCase();
  candidate.backgroundColor.toUpperCase();
  candidate.size = document["size"] | 0;
  candidate.mode = document["mode"] | "";
  candidate.speed = document["speed"] | 0;

  const char *fonts[] = {"bold", "serif", "pixel", "mono"};
  const char *modes[] = {"center", "banner", "outline", "marquee"};
  if (!isDisplayableText(candidate.text)) {
    sendDesignError("Text must be 1-48 printable ASCII characters");
    return;
  }
  if (!isAllowedValue(candidate.font, fonts, 4)) {
    sendDesignError("Unknown font preset");
    return;
  }
  if (!isAllowedValue(candidate.mode, modes, 4)) {
    sendDesignError("Unknown display mode");
    return;
  }
  if (!isHexColor(candidate.textColor) || !isHexColor(candidate.backgroundColor)) {
    sendDesignError("Colours must use #RRGGBB format");
    return;
  }
  if (candidate.size < 1 || candidate.size > 4) {
    sendDesignError("Font size must be between 1 and 4");
    return;
  }
  if (candidate.speed < 10 || candidate.speed > 180) {
    sendDesignError("Marquee speed must be between 10 and 180");
    return;
  }

  design = candidate;
  saveDesign();
  rocketRush.stop();
  frogGotchi.stop();
  deviceMode = DeviceMode::DESIGN;
  renderDesign();
  Serial.println("[design] Validated, saved to NVS, and rendered");

  StaticJsonDocument<448> responseDocument;
  responseDocument["ok"] = true;
  writeDesignJson(responseDocument.createNestedObject("design"), design);
  String response;
  serializeJson(responseDocument, response);
  webServer.send(200, "application/json", response);
}

const char *deviceModeName() {
  if (deviceMode == DeviceMode::GAME) return "rocket_rush";
  if (deviceMode == DeviceMode::FROG) return "froggotchi";
  if (deviceMode == DeviceMode::DESIGN) return "display_studio";
  return "menu";
}

void setDeviceMode(DeviceMode mode) {
  deviceMode = mode;
  returnHandled = false;
  menuChordLatched = true;
  if (mode == DeviceMode::GAME) {
    frogGotchi.stop();
    rocketRush.enterTitle();
  } else if (mode == DeviceMode::FROG) {
    rocketRush.stop();
    frogGotchi.enter();
  } else {
    rocketRush.stop();
    frogGotchi.stop();
    if (mode == DeviceMode::DESIGN) renderDesign();
    else drawDeviceMenu();
  }
}

void sendModeJson() {
  StaticJsonDocument<128> document;
  document["mode"] = deviceModeName();
  document["state"] = deviceMode == DeviceMode::GAME ? rocketRush.stateName() : deviceModeName();
  String response;
  serializeJson(document, response);
  webServer.send(200, "application/json", response);
}

void handleModePost() {
  StaticJsonDocument<96> document;
  if (!webServer.hasArg("plain") || deserializeJson(document, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  const String mode = document["mode"] | "";
  if (mode == "menu") setDeviceMode(DeviceMode::MENU);
  else if (mode == "design") setDeviceMode(DeviceMode::DESIGN);
  else if (mode == "game") setDeviceMode(DeviceMode::GAME);
  else if (mode == "frog") setDeviceMode(DeviceMode::FROG);
  else {
    webServer.send(400, "application/json", "{\"error\":\"Unknown mode\"}");
    return;
  }
  sendModeJson();
}

void sendFrogJson() {
  StaticJsonDocument<896> document;
  document["hunger"] = frogGotchi.hunger();
  document["energy"] = frogGotchi.energy();
  document["happiness"] = frogGotchi.happiness();
  document["health"] = frogGotchi.health();
  document["cleanliness"] = frogGotchi.cleanliness();
  document["coins"] = frogGotchi.coins();
  document["sessions"] = frogGotchi.focusSessions();
  document["ageMinutes"] = frogGotchi.ageMinutes();
  document["classic"] = frogGotchi.classicMode();
  document["sleeping"] = frogGotchi.sleeping();
  document["alive"] = frogGotchi.alive();
  document["focusRunning"] = frogGotchi.focusRunning();
  document["breakRunning"] = frogGotchi.breakRunning();
  document["timerSeconds"] = frogGotchi.timerSecondsRemaining();
  document["timerPaused"] = frogGotchi.timerPaused();
  document["focusMinutes"] = frogGotchi.focusMinutes();
  document["breakMinutes"] = frogGotchi.breakMinutes();
  document["background"] = frogGotchi.background();
  document["backgroundName"] = frogGotchi.backgroundName();
  document["expression"] = frogGotchi.expression();
  document["name"] = frogGotchi.petName();
  document["stage"] = frogGotchi.stageName();
  document["mood"] = frogGotchi.moodName();
  document["need"] = frogGotchi.needMessage();
  document["quote"] = frogGotchi.quote();
  document["accessory"] = frogGotchi.accessoryName();
  String response;
  serializeJson(document, response);
  webServer.send(200, "application/json", response);
}

void handleFrogAction() {
  StaticJsonDocument<256> document;
  if (!webServer.hasArg("plain") || deserializeJson(document, webServer.arg("plain"))) {
    webServer.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  if (deviceMode != DeviceMode::FROG) setDeviceMode(DeviceMode::FROG);
  const String action = document["action"] | "";
  if (action == "feed") frogGotchi.performAction(FrogGotchi::Action::FEED);
  else if (action == "play") frogGotchi.performAction(FrogGotchi::Action::PLAY);
  else if (action == "pet" || action == "love") frogGotchi.performAction(FrogGotchi::Action::LOVE);
  else if (action == "clean") frogGotchi.performAction(FrogGotchi::Action::CLEAN);
  else if (action == "sleep") frogGotchi.performAction(FrogGotchi::Action::SLEEP);
  else if (action == "medicine") frogGotchi.performAction(FrogGotchi::Action::MEDICINE);
  else if (action == "focus") frogGotchi.startFocus(constrain(document["minutes"] | 25, 1, 60), constrain(document["breakMinutes"] | 5, 1, 30));
  else if (action == "pause") frogGotchi.pauseTimer();
  else if (action == "resume") frogGotchi.resumeTimer();
  else if (action == "cancel") frogGotchi.cancelTimer();
  else if (action == "background") {
    if (!frogGotchi.setBackground(document["index"] | 255)) {
      webServer.send(400, "application/json", "{\"error\":\"Background must be 0, 1 or 2\"}");
      return;
    }
  }
  else if (action == "rename") {
    if (!frogGotchi.setPetName(document["name"] | "")) {
      webServer.send(400, "application/json", "{\"error\":\"Name must be 1-12 letters, numbers, spaces or hyphens\"}");
      return;
    }
  }
  else if (action == "classic") frogGotchi.setClassicMode(document["enabled"] | false);
  else if (action == "sync") frogGotchi.syncEpoch(document["epoch"] | 0UL);
  else {
    webServer.send(400, "application/json", "{\"error\":\"Unknown frog action\"}");
    return;
  }
  sendFrogJson();
}

void sendBackgroundPng(uint8_t index) {
  if (index == 0) webServer.send_P(200, "image/png", reinterpret_cast<PGM_P>(FROG_BG_COZY_ROOM_PNG), FROG_BG_COZY_ROOM_PNG_LEN);
  else if (index == 1) webServer.send_P(200, "image/png", reinterpret_cast<PGM_P>(FROG_BG_STUDY_DESK_PNG), FROG_BG_STUDY_DESK_PNG_LEN);
  else if (index == 2) webServer.send_P(200, "image/png", reinterpret_cast<PGM_P>(FROG_BG_FROG_POND_PNG), FROG_BG_FROG_POND_PNG_LEN);
  else webServer.send(404, "text/plain", "Background not found");
}

void sendFrogSpritePng(uint8_t index) {
  if (index >= 12) { webServer.send(404, "text/plain", "Sprite not found"); return; }
  const uint8_t *data = reinterpret_cast<const uint8_t *>(pgm_read_ptr(&FROG_WEB_SPRITES[index]));
  const size_t length = pgm_read_dword(&FROG_WEB_SPRITE_LENGTHS[index]);
  webServer.send_P(200, "image/png", reinterpret_cast<PGM_P>(data), length);
}

void configurePortalRoutes() {
  webServer.on("/", HTTP_GET, []() {
    logHttpClient("FrogGotchi home");
    setDeviceMode(DeviceMode::FROG);
    webServer.send_P(200, "text/html", FROGGOTCHI_HTML);
  });
  webServer.on("/os", HTTP_GET, []() {
    logHttpClient("Cyberdeck OS");
    webServer.send_P(200, "text/html", CYBERDECK_HOME_HTML);
  });
  webServer.on("/studio", HTTP_GET, []() {
    logHttpClient("Display Studio");
    webServer.send_P(200, "text/html", HAIRPIN_STUDIO_HTML);
  });
  webServer.on("/frog", HTTP_GET, []() {
    logHttpClient("FrogGotchi");
    setDeviceMode(DeviceMode::FROG);
    webServer.send_P(200, "text/html", FROGGOTCHI_HTML);
  });
  webServer.on("/health", HTTP_GET, []() {
    logHttpClient("health");
    webServer.send(200, "text/plain", "OK");
  });
  webServer.on("/api/design", HTTP_GET, []() {
    logHttpClient("GET design");
    sendCurrentDesign();
  });
  webServer.on("/api/design", HTTP_POST, []() {
    logHttpClient("POST design");
    handleDesignPost();
  });
  webServer.on("/api/mode", HTTP_GET, []() {
    logHttpClient("GET mode");
    sendModeJson();
  });
  webServer.on("/api/mode", HTTP_POST, []() {
    logHttpClient("POST mode");
    handleModePost();
  });
  webServer.on("/api/frog", HTTP_GET, []() {
    logHttpClient("GET frog");
    sendFrogJson();
  });
  webServer.on("/api/frog/action", HTTP_POST, []() {
    logHttpClient("POST frog action");
    handleFrogAction();
  });
  webServer.on("/assets/background/0.png", HTTP_GET, []() { sendBackgroundPng(0); });
  webServer.on("/assets/background/1.png", HTTP_GET, []() { sendBackgroundPng(1); });
  webServer.on("/assets/background/2.png", HTTP_GET, []() { sendBackgroundPng(2); });
  for (uint8_t i = 0; i < 12; ++i) {
    const String path = "/assets/frog/" + String(i) + ".png";
    webServer.on(path, HTTP_GET, [i]() { sendFrogSpritePng(i); });
  }
  webServer.on("/generate_204", HTTP_GET, redirectToPortal);
  webServer.on("/hotspot-detect.html", HTTP_GET, redirectToPortal);
  webServer.on("/connecttest.txt", HTTP_GET, redirectToPortal);
  webServer.onNotFound([]() {
    redirectToPortal();
  });
}

void makeUniqueSetupCredentials() {
  const uint64_t chipId = ESP.getEfuseMac();
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06llX", chipId & 0xFFFFFFULL);
  setupSsid = "Cyberdeck-" + String(suffix).substring(2);
  setupPassword = "cyberdeck";
}

const char *authModeName(wifi_auth_mode_t mode) {
  switch (mode) {
    case WIFI_AUTH_OPEN: return "OPEN";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA-PSK";
    case WIFI_AUTH_WPA2_PSK: return "WPA2-PSK";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2-PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK: return "WPA3-PSK";
    case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3-PSK";
    default: return "UNKNOWN";
  }
}

void logSetupApConfiguration() {
  wifi_config_t config = {};
  const esp_err_t result = esp_wifi_get_config(WIFI_IF_AP, &config);
  Serial.print("[ap] softAP IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("[ap] channel: ");
  Serial.println(config.ap.channel);
  Serial.print("[ap] authentication: ");
  Serial.println(result == ESP_OK ? authModeName(config.ap.authmode) : "config read failed");
  Serial.print("[ap] cipher: ");
  Serial.println(config.ap.pairwise_cipher == WIFI_CIPHER_TYPE_CCMP ? "CCMP (AES)" : "other");
  Serial.print("[ap] connected stations: ");
  Serial.println(WiFi.softAPgetStationNum());
  Serial.print("[ap] setup password length: ");
  Serial.println(setupPassword.length());
}

void startHairpinNetwork() {
  makeUniqueSetupCredentials();
  // Stable AP-only provisioning: no STA reconnects or radio mode changes occur
  // until credentials are submitted and the board cleanly restarts.
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_AP);
  const bool started = WiFi.softAP(setupSsid.c_str(), setupPassword.c_str(),
                                   SETUP_AP_CHANNEL, 0, SETUP_AP_MAX_CLIENTS, false);
  Serial.print("[ap] WiFi.softAP() returned: ");
  Serial.println(started ? "success" : "FAILURE");
  if (!started) return;
  esp_wifi_set_ps(WIFI_PS_NONE);
  dnsServer.start(53, "*", WiFi.softAPIP());
  configurePortalRoutes();
  webServer.begin();
  setupPortalRunning = true;
  drawSetupScreen();
  Serial.print("[ap] SSID: ");
  Serial.println(setupSsid);
  Serial.println("[ap] password is shown only on the LCD");
  Serial.println("[http] FrogGotchi: http://192.168.4.1 (Cyberdeck OS at /os)");
  logSetupApConfiguration();
}

void updateMenuButton(bool raw, bool &lastRaw, bool &stable, uint32_t &changedAt, uint32_t now) {
  if (raw != lastRaw) {
    lastRaw = raw;
    changedAt = now;
  }
  if (now - changedAt >= MENU_DEBOUNCE_MS) stable = lastRaw;
}

void launchMenuSelection() {
  if (menuSelection == 0) {
    setDeviceMode(DeviceMode::DESIGN);
  } else if (menuSelection == 1) {
    setDeviceMode(DeviceMode::GAME);
  } else {
    setDeviceMode(DeviceMode::FROG);
  }
}

void updateDeviceControls() {
  static bool previousLeft = false;
  static bool previousRight = false;
  const uint32_t now = millis();
  updateMenuButton(digitalRead(LEFT_BUTTON) == LOW, leftRaw, leftStable, leftChangedAt, now);
  updateMenuButton(digitalRead(RIGHT_BUTTON) == LOW, rightRaw, rightStable, rightChangedAt, now);
  const bool both = leftStable && rightStable;

  if (deviceMode == DeviceMode::MENU) {
    if (both && !menuChordLatched) {
      menuChordLatched = true;
      launchMenuSelection();
    } else if (!both) {
      menuChordLatched = false;
      if (leftStable && !previousLeft) {
        menuSelection = (menuSelection + 2) % 3;
        drawDeviceMenu();
      } else if (rightStable && !previousRight) {
        menuSelection = (menuSelection + 1) % 3;
        drawDeviceMenu();
      }
    }
  } else if (deviceMode == DeviceMode::DESIGN) {
    if (both && returnStartedAt == 0) returnStartedAt = now;
    if (both && !returnHandled && now - returnStartedAt >= MENU_RETURN_HOLD_MS) {
      returnHandled = true;
      setDeviceMode(DeviceMode::MENU);
    }
    if (!both) {
      returnStartedAt = 0;
      returnHandled = false;
      menuChordLatched = false;
    }
  }
  previousLeft = leftStable;
  previousRight = rightStable;
}

void setup() {
  Serial.begin(115200);
  delay(300);
  const esp_reset_reason_t reason = esp_reset_reason();
  Serial.print("[boot] reset reason: ");
  Serial.print(resetReasonName(reason));
  Serial.print(" (");
  Serial.print(static_cast<int>(reason));
  Serial.println(")");
  esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  WiFi.onEvent(onWiFiEvent);

  pinMode(LCD_POWER, OUTPUT);
  digitalWrite(LCD_POWER, HIGH);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, HIGH);
  if (!display->begin()) {
    Serial.println("Display initialization failed");
    return;
  }
  display->setRotation(1);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  loadDesign();
  rocketRush.begin();
  frogGotchi.begin();
  drawHelloScreen();
  Serial.println("T-Display-S3 workshop starter ready");
  delay(900);
  startHairpinNetwork();
  delay(4200);
  setDeviceMode(DeviceMode::MENU);
}

void loop() {
  if (setupPortalRunning) dnsServer.processNextRequest();
  webServer.handleClient();
  if (deviceMode == DeviceMode::GAME) {
    rocketRush.update();
    if (rocketRush.consumeExitRequest()) setDeviceMode(DeviceMode::MENU);
  } else if (deviceMode == DeviceMode::FROG) {
    frogGotchi.update();
    if (frogGotchi.consumeExitRequest()) setDeviceMode(DeviceMode::MENU);
  } else {
    updateDeviceControls();
    if (deviceMode == DeviceMode::DESIGN) updateMarquee();
  }
  if (setupPortalRunning) {
    const uint8_t stationCount = WiFi.softAPgetStationNum();
    if (stationCount != lastStationCount) {
      lastStationCount = stationCount;
      Serial.print("[ap] connected station count changed to: ");
      Serial.println(stationCount);
    }
  }
  delay(2);
}
