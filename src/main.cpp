#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WiFi.h>

// Verified LILYGO T-Display-S3 standard non-touch pin mapping.
constexpr int LCD_POWER = 15;
constexpr int LCD_BACKLIGHT = 38;
constexpr int LEFT_BUTTON = 0;
constexpr int RIGHT_BUTTON = 14;
constexpr uint32_t HELLO_DURATION_MS = 3000;
constexpr uint32_t CONNECT_TIMEOUT_MS = 15000;
constexpr uint8_t AP_CHANNEL = 6;
constexpr uint8_t AP_MAX_CLIENTS = 4;

Arduino_DataBus *bus = new Arduino_ESP32PAR8Q(
    7, 6, 8, 9, 39, 40, 41, 42, 45, 46, 47, 48);
Arduino_GFX *display = new Arduino_ST7789(
    bus, 5, 0, true, 170, 320, 35, 0, 35, 0);

Preferences preferences;
DNSServer dnsServer;
WebServer webServer(80);

String setupSsid;
const char *setupPassword = "cyberdeck";
String savedSsid;
String savedPassword;
bool networkStarted = false;
bool connectionPending = false;
bool connectedScreenDrawn = false;
uint32_t connectStartedAt = 0;

void drawCenteredText(const char *text, uint8_t size, int centerY, uint16_t color) {
  display->setTextSize(size);
  display->setTextColor(color);
  const int width = strlen(text) * 6 * size;
  const int height = 8 * size;
  display->setCursor((display->width() - width) / 2, centerY - height / 2);
  display->print(text);
}

void drawHelloScreen() {
  display->fillScreen(BLACK);
  drawCenteredText("HELLO WORLD!", 4, 61, WHITE);
  drawCenteredText("MY FIRST LILYGO", 2, 112,
                   display->color565(255, 105, 180));
}

void drawSetupScreen(const char *message = nullptr) {
  display->fillScreen(BLACK);
  drawCenteredText("Wi-Fi Setup", 3, 20, WHITE);
  drawCenteredText("Connect to:", 1, 51, WHITE);
  drawCenteredText(setupSsid.c_str(), 2, 72,
                   display->color565(255, 105, 180));
  drawCenteredText("Password: cyberdeck", 1, 102, WHITE);
  drawCenteredText("Then open 192.168.4.1", 1, 132, WHITE);
  if (message) drawCenteredText(message, 1, 157, display->color565(255, 216, 92));
}

void drawConnectedScreen() {
  display->fillScreen(BLACK);
  drawCenteredText("Wi-Fi Connected", 3, 40,
                   display->color565(130, 235, 160));
  drawCenteredText(savedSsid.c_str(), 2, 82, WHITE);
  drawCenteredText("Device IP:", 1, 111, WHITE);
  drawCenteredText(WiFi.localIP().toString().c_str(), 2, 136,
                   display->color565(255, 105, 180));
}

String escapeHtml(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 8);
  for (size_t i = 0; i < value.length(); ++i) {
    switch (value[i]) {
      case '&': escaped += F("&amp;"); break;
      case '<': escaped += F("&lt;"); break;
      case '>': escaped += F("&gt;"); break;
      case '"': escaped += F("&quot;"); break;
      case '\'': escaped += F("&#39;"); break;
      default: escaped += value[i];
    }
  }
  return escaped;
}

String setupPage() {
  const bool connected = WiFi.status() == WL_CONNECTED;
  String page;
  page.reserve(4200);
  page += F(R"HTML(<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>LILYGO Wi-Fi Setup</title><style>
:root{--ink:#292135;--pink:#ff76a8;--green:#b8ed9f;--cream:#fff7df}
*{box-sizing:border-box}body{margin:0;background:var(--cream);color:var(--ink);font:16px/1.5 system-ui,sans-serif;padding:22px}
main{max-width:560px;margin:4vh auto;background:white;border:3px solid var(--ink);padding:26px;box-shadow:7px 7px 0 var(--ink)}
h1{font-size:34px;line-height:1;margin:0 0 12px}label{display:grid;gap:6px;font-weight:800;margin:18px 0}
input{font:inherit;padding:12px;border:2px solid var(--ink);width:100%}button{font:inherit;font-weight:900;padding:12px 16px;border:2px solid var(--ink);background:var(--pink);box-shadow:3px 3px 0 var(--ink);cursor:pointer}
.status{padding:14px;border:2px solid var(--ink);background:var(--green);margin:18px 0}.hint{color:#6d6576;font-size:14px}code{font-weight:800}</style></head><body><main>
<h1>My First LILYGO</h1><p>Connect this Cyber Deck to your phone hotspot or Wi-Fi network.</p>)HTML");
  if (connected) {
    page += F("<div class=\"status\"><b>Wi-Fi connected!</b><br>Network: ");
    page += escapeHtml(WiFi.SSID());
    page += F("<br>Device IP: <code>");
    page += WiFi.localIP().toString();
    page += F("</code></div>");
  } else if (connectionPending) {
    page += F("<div class=\"status\">Trying to connect… Refresh this page in a few seconds.</div>");
  }
  page += F(R"HTML(<form method="post" action="/save">
<label>Wi-Fi or hotspot name<input name="ssid" maxlength="32" required autocomplete="off"></label>
<label>Password<input name="password" type="password" maxlength="63" autocomplete="new-password"></label>
<button type="submit">Save and connect</button></form>
<p class="hint">Your password is saved only on this ESP32. It is never printed to Serial logs or shown on this page.</p>
</main></body></html>)HTML");
  return page;
}

void beginStationConnection() {
  if (savedSsid.isEmpty()) return;
  WiFi.disconnect(false, false);
  delay(50);
  WiFi.begin(savedSsid.c_str(), savedPassword.c_str());
  connectionPending = true;
  connectedScreenDrawn = false;
  connectStartedAt = millis();
  drawSetupScreen("Connecting...");
  Serial.print(F("[wifi] connecting to saved network: "));
  Serial.println(savedSsid);
}

void configureWebServer() {
  webServer.on("/", HTTP_GET, []() { webServer.send(200, "text/html", setupPage()); });
  webServer.on("/health", HTTP_GET, []() { webServer.send(200, "text/plain", "OK"); });
  webServer.on("/save", HTTP_POST, []() {
    const String ssid = webServer.arg("ssid");
    const String password = webServer.arg("password");
    if (ssid.isEmpty() || ssid.length() > 32 || password.length() > 63) {
      webServer.send(400, "text/plain", "Invalid network details");
      return;
    }
    savedSsid = ssid;
    savedPassword = password;
    preferences.begin("workshop-wifi", false);
    preferences.putString("ssid", savedSsid);
    preferences.putString("password", savedPassword);
    preferences.end();
    webServer.sendHeader("Location", "/", true);
    webServer.send(303, "text/plain", "Connecting");
    beginStationConnection();
  });
  webServer.onNotFound([]() {
    webServer.sendHeader("Location", "http://192.168.4.1/", true);
    webServer.send(302, "text/plain", "Open setup page");
  });
  webServer.begin();
}

void startNetwork() {
  const uint64_t chipId = ESP.getEfuseMac();
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06X", static_cast<uint32_t>(chipId & 0xFFFFFF));
  setupSsid = "LILYGO-SETUP-" + String(suffix);

  preferences.begin("workshop-wifi", true);
  savedSsid = preferences.getString("ssid", "");
  savedPassword = preferences.getString("password", "");
  preferences.end();

  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
  WiFi.mode(WIFI_AP_STA);
  const bool apStarted = WiFi.softAP(setupSsid.c_str(), setupPassword,
                                     AP_CHANNEL, false, AP_MAX_CLIENTS);
  dnsServer.start(53, "*", WiFi.softAPIP());
  configureWebServer();
  networkStarted = true;

  Serial.print(F("[setup] access point: "));
  Serial.println(setupSsid);
  Serial.print(F("[setup] AP started: "));
  Serial.println(apStarted ? F("yes") : F("no"));
  Serial.print(F("[setup] page: http://"));
  Serial.println(WiFi.softAPIP());
  drawSetupScreen();
  if (!savedSsid.isEmpty()) beginStationConnection();
}

void setup() {
  Serial.begin(115200);
  pinMode(LCD_POWER, OUTPUT);
  digitalWrite(LCD_POWER, HIGH);
  pinMode(LCD_BACKLIGHT, OUTPUT);
  digitalWrite(LCD_BACKLIGHT, HIGH);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  display->begin();
  display->setRotation(1);
  display->setTextWrap(false);
  drawHelloScreen();
  Serial.println(F("[boot] HELLO WORLD screen ready"));
}

void loop() {
  if (!networkStarted && millis() >= HELLO_DURATION_MS) startNetwork();
  if (!networkStarted) return;

  dnsServer.processNextRequest();
  webServer.handleClient();

  if (connectionPending && WiFi.status() == WL_CONNECTED) {
    connectionPending = false;
    connectedScreenDrawn = true;
    drawConnectedScreen();
    Serial.print(F("[wifi] connected; IP: "));
    Serial.println(WiFi.localIP());
  } else if (connectionPending && millis() - connectStartedAt >= CONNECT_TIMEOUT_MS) {
    connectionPending = false;
    WiFi.disconnect(false, false);
    drawSetupScreen("Connection failed - retry");
    Serial.println(F("[wifi] connection timed out; setup AP remains available"));
  } else if (!connectionPending && WiFi.status() == WL_CONNECTED && !connectedScreenDrawn) {
    connectedScreenDrawn = true;
    drawConnectedScreen();
  }

  delay(2);
}
