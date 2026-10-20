#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "Config.h"
#include "Teams.h"
#include "EventLog.h"
#include "LedEngine.h"
#include "AudioEngine.h"
#include "NhlClient.h"
#include "WebPortal.h"
#include "AppIcon.h"

// =====================================================
// GLOBAL OBJECTS
// =====================================================
Preferences prefs;
WebServer server(80);
DNSServer dnsServer;
EventLogManager eventLog;

LedEngine leds;
AudioEngine audio;
NhlClient nhl;
SystemConfig config;

const byte DNS_PORT = 53;
bool isApMode = false;

// Button state tracking
bool buttonWasDown = false;
unsigned long buttonDownTime = 0;
unsigned long lastReleaseTime = 0;
unsigned long buttonLockoutUntil = 0;
int pressCount = 0;
unsigned long scheduledRestartTime = 0;

// TV Broadcast Delay Celebration Queue
unsigned long pendingGoalCelebrationTime = 0;
unsigned long pendingOpponentGoalTime = 0;

// =====================================================
// FORWARD DECLARATIONS
// =====================================================
void loadConfiguration();
void saveConfiguration();
void setupNetwork();
void setupWebServer();
void handleButton(unsigned long now);
void triggerGoalCelebration();
void triggerOpponentGoal();
void triggerGameWin();

void generateHostname(const char* name, char* out, size_t maxLen) {
  if (name == nullptr || strlen(name) == 0) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(out, maxLen, "%s-%02x%02x", MDNS_HOSTNAME, mac[4], mac[5]);
    return;
  }

  // Check if name begins with "NHL Goal Light" or "Goal Light"
  const char* suffix = nullptr;
  if (strncasecmp(name, "NHL Goal Light", 14) == 0) {
    const char* p = name + 14;
    while (*p == ' ' || *p == '-' || *p == '_') p++;
    if (*p != '\0') {
      suffix = p;
    } else {
      // Exactly "NHL Goal Light" with no suffix -> fallback to unique MAC hash
      uint8_t mac[6];
      WiFi.macAddress(mac);
      snprintf(out, maxLen, "%s-%02x%02x", MDNS_HOSTNAME, mac[4], mac[5]);
      return;
    }
  } else if (strncasecmp(name, "Goal Light", 10) == 0) {
    const char* p = name + 10;
    while (*p == ' ' || *p == '-' || *p == '_') p++;
    if (*p != '\0') {
      suffix = p;
    } else {
      uint8_t mac[6];
      WiFi.macAddress(mac);
      snprintf(out, maxLen, "%s-%02x%02x", MDNS_HOSTNAME, mac[4], mac[5]);
      return;
    }
  }

  if (suffix != nullptr) {
    // Generate "goallight-<suffix>"
    size_t j = 0;
    const char* prefix = MDNS_HOSTNAME;
    while (prefix[j] != '\0' && j < maxLen - 1) {
      out[j] = prefix[j];
      j++;
    }
    if (j < maxLen - 1) out[j++] = '-';

    for (size_t i = 0; suffix[i] != '\0' && j < maxLen - 1; i++) {
      char c = suffix[i];
      if (isalnum(c)) {
        out[j++] = tolower(c);
      } else if (c == ' ' || c == '_' || c == '-') {
        if (j > 0 && out[j - 1] != '-') {
          out[j++] = '-';
        }
      }
    }
    if (j > 0 && out[j - 1] == '-') j--;
    out[j] = '\0';
    return;
  }

  // Generic custom name (e.g. "Man Cave" -> "man-cave")
  size_t j = 0;
  for (size_t i = 0; name[i] != '\0' && j < maxLen - 1; i++) {
    char c = name[i];
    if (isalnum(c)) {
      out[j++] = tolower(c);
    } else if (c == ' ' || c == '_' || c == '-') {
      if (j > 0 && out[j - 1] != '-') {
        out[j++] = '-';
      }
    }
  }
  if (j > 0 && out[j - 1] == '-') j--;
  out[j] = '\0';
  if (strlen(out) == 0) {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(out, maxLen, "%s-%02x%02x", MDNS_HOSTNAME, mac[4], mac[5]);
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=================================");
  Serial.println("🚨 NHL Goal Light Initialising");
  Serial.printf("📦 Firmware Version: %s\n", FIRMWARE_VERSION);
  Serial.println("=================================");

#ifdef PIN_STATUS_LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // Turn on built-in blue LED (active LOW)
#endif

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP); // On-board BOOT button on ESP32-S3 Super Mini

  // Load persistent configuration from NVS
  loadConfiguration();

  // Initialise LED Engine
  const TeamData* team = findTeam(config.primary_team);
  leds.begin();
  leds.configure(config);
  leds.setTeam(team);

  // Initialise Audio
  audio.begin();
  audio.setVolumeIndex(config.volume_index);

  // Initialise NHL API Client
  nhl.setTeam(config.primary_team);

  // Network Setup
  setupNetwork();

  // Web Server Routes
  setupWebServer();

  Serial.println("[Boot] Setup complete. System ready.");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  unsigned long now = millis();

  // Check for scheduled restart (e.g. after Wi-Fi settings saved)
  if (scheduledRestartTime > 0 && now >= scheduledRestartTime) {
    Serial.println("[System] Executing scheduled reboot to apply network settings...");
    scheduledRestartTime = 0;
    delay(100);
    ESP.restart();
  }

  // 1. Process physical button (supports external pin 9 and on-board pin 0)
  handleButton(now);

  // 2. Process DNS server if in AP mode (captive portal)
  if (isApMode || WiFi.getMode() == WIFI_MODE_AP || WiFi.getMode() == WIFI_MODE_APSTA) {
    dnsServer.processNextRequest();
  }

  // 3. Process Web Server requests
  server.handleClient();

  // 4. Update LED animations (non-blocking)
  leds.update();

  // 5. Update NHL Game Clock ticker (decrements every 1s when active play is running)
  nhl.tick();

  // Periodic Serial heartbeat so Serial Monitor confirms the device is running
  static unsigned long lastHeartbeat = 0;
  if (now - lastHeartbeat >= 3000) {
    lastHeartbeat = now;
    Serial.printf("🏒 [NHL Goal Light] Alive! Mode: %s | Free RAM: %u bytes | IP: %s\n",
                  isApMode ? "AP (SSID: NHL-Goal-Light-Setup)" : "Station Mode",
                  ESP.getFreeHeap(),
                  isApMode ? "192.168.4.1" : WiFi.localIP().toString().c_str());
  }

  // Check buffered TV broadcast delay celebration
  if (pendingGoalCelebrationTime > 0 && now >= pendingGoalCelebrationTime) {
    pendingGoalCelebrationTime = 0;
    triggerGoalCelebration();
  }
  if (pendingOpponentGoalTime > 0 && now >= pendingOpponentGoalTime) {
    pendingOpponentGoalTime = 0;
    triggerOpponentGoal();
  }

  // 6. If celebrating or in AP mode, skip live polling
  if (isApMode || leds.isCelebrating()) {
    return;
  }

  // 7. Check Wi-Fi reconnection if lost; if lost for > 45s, activate Captive Portal AP
  static unsigned long disconnectStartTime = 0;
  if (!isApMode) {
    if (WiFi.status() != WL_CONNECTED) {
      if (disconnectStartTime == 0) {
        disconnectStartTime = now;
        Serial.println("[Network] Wi-Fi lost! Attempting auto-reconnect...");
      }
      if (now - disconnectStartTime > 45000) {
        Serial.println("[Network] Reconnection timed out (45s). Activating Captive Portal Access Point...");
        disconnectStartTime = 0;
        isApMode = true;
        setupNetwork();
      }
      return;
    } else {
      disconnectStartTime = 0;
    }
  }

  // 8. Poll NHL Web API
  bool opponentGoal = false;
  bool gameWin = false;
  bool periodStart = false;

  bool preferredGoal = nhl.poll(opponentGoal, gameWin, periodStart);

  if (preferredGoal || (opponentGoal && config.alert_opponent_goal)) {
    unsigned long elapsedSinceGoalMs = 0;
    if (nhl.getStatus().lastStoppageMillis > 0 && (now >= nhl.getStatus().lastStoppageMillis)) {
      elapsedSinceGoalMs = now - nhl.getStatus().lastStoppageMillis;
      if (elapsedSinceGoalMs > 120000UL) elapsedSinceGoalMs = 0;
    }
    long remainingDelayMs = ((long)config.broadcast_delay_sec * 1000L) - (long)elapsedSinceGoalMs;
    int logSec = (remainingDelayMs > 0) ? (int)((remainingDelayMs + 500) / 1000) : 0;

    if (preferredGoal) {
      if (config.broadcast_delay_sec == 0 || remainingDelayMs <= 0) {
        triggerGoalCelebration();
      } else {
        pendingGoalCelebrationTime = now + (unsigned long)remainingDelayMs;
        Serial.printf("[Event] Goal scored! Celebration held for %ld ms (TV broadcast sync, %lu ms since whistle)...\n", remainingDelayMs, elapsedSinceGoalMs);
        eventLog.log("[GOAL] Scored! Delayed %ds for TV", logSec);
      }
    } else {
      if (config.broadcast_delay_sec == 0 || remainingDelayMs <= 0) {
        triggerOpponentGoal();
      } else {
        pendingOpponentGoalTime = now + (unsigned long)remainingDelayMs;
        Serial.printf("[Event] Opponent goal! Alert held for %ld ms (TV broadcast sync, %lu ms since whistle)...\n", remainingDelayMs, elapsedSinceGoalMs);
        eventLog.log("[OPPONENT GOAL] Delayed %ds for TV", logSec);
      }
    }
  } else if (gameWin && config.alert_game_win) {
    triggerGameWin();
  } else if (periodStart && config.alert_period_buzzer) {
    if (config.hw_speaker_enabled) {
      audio.play(TRACK_PERIOD_HORN);
    }
  }
}

// =====================================================
// EVENT ACTIONS
// =====================================================
void triggerGoalCelebration() {
  Serial.println("[Event] GOAL CELEBRATION TRIGGERED!");

  if (config.hw_speaker_enabled) {
    audio.play(TRACK_GOAL_HORN);
  }

  if (config.hw_lights_enabled) {
    LedMode mode = LED_BEACON_GOAL;
    switch (config.effect_style) {
      case 0: mode = LED_BEACON_GOAL; break;
      case 1: mode = LED_STROBE_GOAL; break;
      case 2: mode = LED_BREATHE_CELEBRATE; break;
      case 3: mode = LED_SIREN_DUAL; break;
      case 4: mode = LED_HEARTBEAT; break;
      default: mode = LED_BEACON_GOAL; break;
    }
    leds.setMode(mode, config.celebration_sec * 1000UL);
  }
}

void triggerOpponentGoal() {
  Serial.println("[Event] Opponent Goal Alert.");
  if (config.hw_lights_enabled) {
    leds.setMode(LED_OPPONENT_GOAL, 3000);
  }
}

void triggerGameWin() {
  Serial.println("[Event] VICTORY CELEBRATION!");
  if (config.hw_speaker_enabled) {
    audio.play(TRACK_WIN_FANFARE);
  }
  if (config.hw_lights_enabled) {
    leds.setMode(LED_VICTORY_RAINBOW, 20000);
  }
}

// =====================================================
// BUTTON LOGIC
// =====================================================
void handleButton(unsigned long now) {
  if (now < buttonLockoutUntil) return;

  // Supports both external tactile button (PIN_BUTTON) and on-board Super Mini BOOT button (GPIO 0)
  bool isDown = (digitalRead(PIN_BUTTON) == LOW) || (digitalRead(0) == LOW);

  if (isDown && !buttonWasDown) {
    buttonWasDown = true;
    buttonDownTime = now;
  }

  if (!isDown && buttonWasDown) {
    buttonWasDown = false;
    unsigned long duration = now - buttonDownTime;

    // Long press (> 4 seconds): reboot into AP mode and wipe invalid Wi-Fi
    if (duration > 4000) {
      Serial.println("[Button] Long press detected. Switching to AP mode & resetting Wi-Fi...");
      isApMode = true;
      config.wifi_ssid[0] = '\0';
      config.wifi_pass[0] = '\0';
      saveConfiguration();
      setupNetwork();
      return;
    }

    // Debounce
    if (duration > 50) {
      pressCount++;
      lastReleaseTime = now;
    }
  }

  // Multi-press evaluator (400ms timeout)
  if (pressCount > 0 && now - lastReleaseTime > 400) {
    int count = pressCount;
    pressCount = 0;
    buttonLockoutUntil = now + 300;

    if (count == 1) {
      // Single press: cycle volume (if speaker enabled)
      if (config.hw_speaker_enabled) {
        uint8_t nextVol = (audio.getVolumeIndex() + 1) % 6;
        audio.setVolumeIndex(nextVol);
        config.volume_index = nextVol;
        saveConfiguration();
        Serial.printf("[Button] Single press: Volume index %d\n", nextVol);
      }
    } else if (count == 2) {
      // Double press: Test Goal Celebration
      Serial.println("[Button] Double press: Test Goal Celebration!");
      triggerGoalCelebration();
    }
  }
}

// =====================================================
// NETWORK & WEB SERVER
// =====================================================
void setupNetwork() {
  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMsk(255, 255, 255, 0);

  // Derive unique mDNS hostname from device name or MAC address
  char hostSlug[33];
  generateHostname(config.device_name, hostSlug, sizeof(hostSlug));

  // Set conservative Wi-Fi TX power (11dBm) to prevent voltage drop on compact SuperMini regulator
  WiFi.setTxPower(WIFI_POWER_11dBm);

  if (isApMode || strlen(config.wifi_ssid) == 0) {
    isApMode = true;
    Serial.println("[Network] Starting Access Point: " AP_SSID);
    WiFi.disconnect(true);
    delay(50);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    bool apSuccess = false;
    if (strlen(config.ap_password) >= 8) {
      apSuccess = WiFi.softAP(AP_SSID, config.ap_password);
      Serial.printf("[Network] AP '%s' (WPA2 secured) status: %s (IP: 192.168.4.1)\n", AP_SSID, apSuccess ? "ACTIVE" : "FAILED");
    } else {
      apSuccess = WiFi.softAP(AP_SSID);
      Serial.printf("[Network] AP '%s' (Open network) status: %s (IP: 192.168.4.1)\n", AP_SSID, apSuccess ? "ACTIVE" : "FAILED");
    }
    delay(100);

    dnsServer.start(DNS_PORT, "*", apIP);
    WiFi.scanNetworks(true);

    leds.setMode(LED_AP_CONFIG);
#ifdef PIN_STATUS_LED
    digitalWrite(PIN_STATUS_LED, LOW); // Blue LED ON solid in AP mode
#endif
  } else {
    Serial.printf("[Network] Connecting to Wi-Fi '%s' as '%s.local'...\n", config.wifi_ssid, hostSlug);
    // CRITICAL: Put radio strictly into Station mode first!
    // Do NOT run SoftAP simultaneously during initial association, as SoftAP on Channel 1
    // prevents the ESP32 RF synthesizer from locking onto home APs on other channels.
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostSlug);
    WiFi.begin(config.wifi_ssid, config.wifi_pass);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 16000) {
      delay(250);
      Serial.print(".");
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED)); // Blink while connecting
#endif
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[Network] Wi-Fi Connected!");
      Serial.print("[Network] IP Address: ");
      Serial.println(WiFi.localIP());

      // Synchronise system time via NTP (UTC)
      configTime(0, 0, "pool.ntp.org", "time.google.com");

      isApMode = false;

      if (MDNS.begin(hostSlug)) {
        MDNS.setInstanceName(config.device_name);
        MDNS.addService("http", "tcp", 80);
        Serial.printf("[Network] mDNS responder started: http://%s.local\n", hostSlug);
      }
      leds.returnToIdle();
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, HIGH); // Blue LED OFF when connected online
#endif
    } else {
      Serial.printf("\n[Network] Could not connect to '%s'. Starting fallback Captive Portal (192.168.4.1)...\n", config.wifi_ssid);
      isApMode = true;
      WiFi.disconnect(true);
      delay(50);
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, netMsk);
      if (strlen(config.ap_password) >= 8) {
        WiFi.softAP(AP_SSID, config.ap_password);
      } else {
        WiFi.softAP(AP_SSID);
      }
      dnsServer.start(DNS_PORT, "*", apIP);
      WiFi.scanNetworks(true);
      leds.setMode(LED_AP_CONFIG);
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, LOW); // Blue LED ON solid in AP mode
#endif
    }
  }
}

struct CloudOtaProgress {
  volatile bool inProgress = false;
  volatile int percent = 0;
  volatile size_t bytesWritten = 0;
  volatile size_t totalBytes = 0;
  volatile bool success = false;
  volatile bool failed = false;
  String message = "";
};
CloudOtaProgress otaProgress;
String cloudOtaTargetUrl = "";

bool performCloudOta(const String &url) {
  otaProgress.inProgress = true;
  otaProgress.percent = 0;
  otaProgress.bytesWritten = 0;
  otaProgress.totalBytes = 0;
  otaProgress.success = false;
  otaProgress.failed = false;
  otaProgress.message = "Connecting to firmware server...";

  Serial.printf("[Cloud-OTA] Connecting to: %s\n", url.c_str());
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(30000);

  if (!http.begin(client, url)) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "HTTP begin failed";
    Serial.println("[Cloud-OTA] HTTP begin failed.");
    return false;
  }

  http.setUserAgent("NHL-GoalLight-OTA/" FIRMWARE_VERSION);
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "HTTP error " + String(httpCode);
    Serial.printf("[Cloud-OTA] HTTP GET returned: %d\n", httpCode);
    http.end();
    return false;
  }

  int contentLength = http.getSize();
  if (contentLength <= 0) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "Invalid binary size";
    Serial.printf("[Cloud-OTA] Invalid binary size: %d\n", contentLength);
    http.end();
    return false;
  }

  otaProgress.totalBytes = contentLength;
  otaProgress.message = "Flashing partition...";
  Serial.printf("[Cloud-OTA] Binary size: %d bytes. Flashing partition...\n", contentLength);
  if (!Update.begin(contentLength)) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "Not enough OTA partition space";
    Serial.println("[Cloud-OTA] Not enough space for OTA.");
    Update.printError(Serial);
    http.end();
    return false;
  }

  WiFiClient *stream = http.getStreamPtr();
  uint8_t buff[2048];
  size_t written = 0;
  unsigned long lastActivity = millis();

  while (http.connected() && (written < (size_t)contentLength)) {
    size_t sizeAvailable = stream->available();
    if (sizeAvailable) {
      int c = stream->readBytes(buff, ((sizeAvailable > sizeof(buff)) ? sizeof(buff) : sizeAvailable));
      if (c > 0) {
        Update.write(buff, c);
        written += c;
        otaProgress.bytesWritten = written;
        otaProgress.percent = (written * 100) / contentLength;
        lastActivity = millis();
      }
    } else {
      if (millis() - lastActivity > 15000) {
        otaProgress.failed = true;
        otaProgress.inProgress = false;
        otaProgress.message = "Stream timeout after 15s";
        Serial.println("[Cloud-OTA] Stream timeout.");
        Update.abort();
        http.end();
        return false;
      }
      delay(1);
    }
    yield();
  }

  if (written != (size_t)contentLength) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "Download size mismatch";
    Serial.printf("[Cloud-OTA] Stream write mismatch: %u / %d\n", (unsigned int)written, contentLength);
    Update.printError(Serial);
    http.end();
    return false;
  }

  if (!Update.end(true)) {
    otaProgress.failed = true;
    otaProgress.inProgress = false;
    otaProgress.message = "Update verification failed";
    Serial.println("[Cloud-OTA] Update.end() failed.");
    Update.printError(Serial);
    http.end();
    return false;
  }

  otaProgress.percent = 100;
  otaProgress.success = true;
  otaProgress.inProgress = false;
  otaProgress.message = "Flash complete! Rebooting...";
  Serial.println("[Cloud-OTA] Flash successful! Rebooting in 1.5s...");
  http.end();
  delay(1500);
  ESP.restart();
  return true;
}

void cloudOtaTask(void *pv) {
  performCloudOta(cloudOtaTargetUrl);
  vTaskDelete(NULL);
}

void setupWebServer() {

  // Main page - Disable caching so browsers never serve stale HTML or version strings
  server.on("/", []() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");
    server.send_P(200, "text/html", INDEX_HTML);
  });

  // Static: App Icons for iOS & Android (GET and HEAD supported)
  auto sendAppIcon = []() {
    server.sendHeader("Cache-Control", "public, max-age=604800");
    server.send_P(200, "image/png", (const char*)ICON_192_PNG, sizeof(ICON_192_PNG));
  };
  server.on("/icon.png", sendAppIcon);
  server.on("/apple-touch-icon.png", sendAppIcon);
  server.on("/apple-touch-icon-precomposed.png", sendAppIcon);
  server.on("/apple-touch-icon-180x180.png", sendAppIcon);
  server.on("/apple-touch-icon-180x180-precomposed.png", sendAppIcon);
  server.on("/apple-touch-icon-120x120.png", sendAppIcon);
  server.on("/apple-touch-icon-120x120-precomposed.png", sendAppIcon);
  server.on("/favicon.ico", sendAppIcon);

  // Web App Manifest for PWA / Add to Home Screen
  server.on("/manifest.json", []() {
    server.sendHeader("Cache-Control", "public, max-age=86400");
    server.send_P(200, "application/manifest+json", PWA_MANIFEST_JSON);
  });

  // REST API: Wi-Fi Scanner (GET)
  server.on("/api/scan", HTTP_GET, []() {
    int16_t scanStatus = WiFi.scanComplete();
    if (scanStatus == WIFI_SCAN_RUNNING) {
      server.send(202, "application/json", "{\"scanning\":true}");
      return;
    }
    if (scanStatus == WIFI_SCAN_FAILED) {
      WiFi.scanNetworks(true);
      server.send(202, "application/json", "{\"scanning\":true}");
      return;
    }

    String json = "[";
    int count = scanStatus;
    int added = 0;
    for (int i = 0; i < count; ++i) {
      String ssid = WiFi.SSID(i);
      if (ssid.length() == 0) continue;

      if (added > 0) json += ",";
      ssid.replace("\\", "\\\\");
      ssid.replace("\"", "\\\"");

      json += "{\"ssid\":\"" + ssid + "\",";
      json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
      json += "\"secure\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false") + "}";
      added++;
    }
    json += "]";
    server.send(200, "application/json", json);
  });

  // REST API: Wi-Fi Scanner (POST - trigger fresh scan)
  server.on("/api/scan", HTTP_POST, []() {
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
    server.send(202, "application/json", "{\"scanning\":true}");
  });

  // REST API: Config GET
  server.on("/api/config", HTTP_GET, []() {
    String json = "{";
    json += "\"deviceName\":\"" + String(config.device_name) + "\",";
    json += "\"team\":\"" + String(config.primary_team) + "\",";
    json += "\"hwLights\":" + String(config.hw_lights_enabled ? "true" : "false") + ",";
    json += "\"hwSpeaker\":" + String(config.hw_speaker_enabled ? "true" : "false") + ",";
    json += "\"hwDisplay\":" + String(config.hw_display_enabled ? "true" : "false") + ",";
    json += "\"numLeds\":" + String(config.num_leds) + ",";
    json += "\"ledLayout\":" + String(config.led_layout) + ",";
    json += "\"ledReverse\":" + String(config.led_reverse ? "true" : "false") + ",";
    json += "\"idleEffect\":" + String(config.idle_effect) + ",";
    json += "\"brightness\":" + String(config.brightness) + ",";
    json += "\"volume\":" + String(config.volume_index) + ",";
    json += "\"hornDuration\":" + String(config.celebration_sec) + ",";
    json += "\"effectStyle\":" + String(config.effect_style) + ",";
    json += "\"goalClassicRed\":" + String(config.goal_classic_red ? "true" : "false") + ",";
    json += "\"alertOpponent\":" + String(config.alert_opponent_goal ? "true" : "false") + ",";
    json += "\"alertOpponentSound\":" + String(config.alert_opponent_sound ? "true" : "false") + ",";
    json += "\"alertPowerPlay\":" + String(config.alert_power_play ? "true" : "false") + ",";
    json += "\"alertPowerPlaySound\":" + String(config.alert_power_play_sound ? "true" : "false") + ",";
    json += "\"alertBuzzer\":" + String(config.alert_period_buzzer ? "true" : "false") + ",";
    json += "\"alertBuzzerSound\":" + String(config.alert_period_buzzer_sound ? "true" : "false") + ",";
    json += "\"alertIntermission\":" + String(config.alert_intermission ? "true" : "false") + ",";
    json += "\"alertWin\":" + String(config.alert_game_win ? "true" : "false") + ",";
    json += "\"alertWinSound\":" + String(config.alert_game_win_sound ? "true" : "false") + ",";
    json += "\"broadcastDelay\":" + String(config.broadcast_delay_sec) + ",";
    json += "\"ssid\":\"" + String(config.wifi_ssid) + "\",";
    json += "\"apPass\":\"" + String(config.ap_password) + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // REST API: Config POST
  server.on("/api/config", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      bool wifiConfigured = false;

      // Device Name
      int dPos = body.indexOf("\"deviceName\":\"");
      if (dPos >= 0) {
        String dn = body.substring(dPos + 14, body.indexOf("\"", dPos + 14));
        dn.trim();
        if (dn.length() > 0) {
          strncpy(config.device_name, dn.c_str(), sizeof(config.device_name));
        }
      }

      // Favourite Team
      int teamPos = body.indexOf("\"team\":\"");
      if (teamPos >= 0) {
        String t = body.substring(teamPos + 8, body.indexOf("\"", teamPos + 8));
        strncpy(config.primary_team, t.c_str(), sizeof(config.primary_team));
        const TeamData* team = findTeam(config.primary_team);
        leds.setTeam(team);
        nhl.setTeam(config.primary_team);
      }

      // Hardware Flags
      if (body.indexOf("\"hwLights\":") >= 0) {
        config.hw_lights_enabled = (body.indexOf("\"hwLights\":true") >= 0);
      }
      if (body.indexOf("\"hwSpeaker\":") >= 0) {
        config.hw_speaker_enabled = (body.indexOf("\"hwSpeaker\":true") >= 0);
      }
      if (body.indexOf("\"hwDisplay\":") >= 0) {
        config.hw_display_enabled = (body.indexOf("\"hwDisplay\":true") >= 0);
      }

      // LED Geometry
      int nlPos = body.indexOf("\"numLeds\":");
      if (nlPos >= 0) {
        config.num_leds = (uint16_t)body.substring(nlPos + 10).toInt();
      }

      int layPos = body.indexOf("\"ledLayout\":");
      if (layPos >= 0) {
        config.led_layout = (uint8_t)body.substring(layPos + 12).toInt();
      }

      if (body.indexOf("\"ledReverse\":") >= 0) {
        config.led_reverse = (body.indexOf("\"ledReverse\":true") >= 0);
      }

      int iePos = body.indexOf("\"idleEffect\":");
      if (iePos >= 0) {
        config.idle_effect = (uint8_t)body.substring(iePos + 13).toInt();
      }

      // Brightness & Volume
      int bPos = body.indexOf("\"brightness\":");
      if (bPos >= 0) {
        config.brightness = body.substring(bPos + 13).toInt();
        leds.setBrightness(config.brightness);
      }

      int vPos = body.indexOf("\"volume\":");
      if (vPos >= 0) {
        config.volume_index = body.substring(vPos + 9).toInt();
        audio.setVolumeIndex(config.volume_index);
      }

      int hPos = body.indexOf("\"hornDuration\":");
      if (hPos >= 0) {
        config.celebration_sec = body.substring(hPos + 15).toInt();
      }

      int ePos = body.indexOf("\"effectStyle\":");
      if (ePos >= 0) {
        config.effect_style = body.substring(ePos + 14).toInt();
      }

      if (body.indexOf("\"goalClassicRed\":") >= 0) {
        config.goal_classic_red = (body.indexOf("\"goalClassicRed\":true") >= 0);
      }

      // Event Toggles & Options
      if (body.indexOf("\"alertOpponent\":") >= 0) {
        config.alert_opponent_goal = (body.indexOf("\"alertOpponent\":true") >= 0);
      }
      if (body.indexOf("\"alertOpponentSound\":") >= 0) {
        config.alert_opponent_sound = (body.indexOf("\"alertOpponentSound\":true") >= 0);
      }
      if (body.indexOf("\"alertPowerPlay\":") >= 0) {
        config.alert_power_play = (body.indexOf("\"alertPowerPlay\":true") >= 0);
      }
      if (body.indexOf("\"alertPowerPlaySound\":") >= 0) {
        config.alert_power_play_sound = (body.indexOf("\"alertPowerPlaySound\":true") >= 0);
      }
      if (body.indexOf("\"alertBuzzer\":") >= 0) {
        config.alert_period_buzzer = (body.indexOf("\"alertBuzzer\":true") >= 0);
      }
      if (body.indexOf("\"alertBuzzerSound\":") >= 0) {
        config.alert_period_buzzer_sound = (body.indexOf("\"alertBuzzerSound\":true") >= 0);
      }
      if (body.indexOf("\"alertIntermission\":") >= 0) {
        config.alert_intermission = (body.indexOf("\"alertIntermission\":true") >= 0);
      }
      if (body.indexOf("\"alertWin\":") >= 0) {
        config.alert_game_win = (body.indexOf("\"alertWin\":true") >= 0);
      }
      if (body.indexOf("\"alertWinSound\":") >= 0) {
        config.alert_game_win_sound = (body.indexOf("\"alertWinSound\":true") >= 0);
      }

      // TV Broadcast Delay
      int bdPos = body.indexOf("\"broadcastDelay\":");
      if (bdPos >= 0) {
        config.broadcast_delay_sec = (uint16_t)body.substring(bdPos + 17).toInt();
      }

      // Wi-Fi Credentials
      int sPos = body.indexOf("\"ssid\":\"");
      if (sPos >= 0) {
        String s = body.substring(sPos + 8, body.indexOf("\"", sPos + 8));
        s.trim();
        if (s.length() > 0) {
          if (strcmp(config.wifi_ssid, s.c_str()) != 0) {
            wifiConfigured = true;
          }
          strncpy(config.wifi_ssid, s.c_str(), sizeof(config.wifi_ssid));
        }
      }

      int pPos = body.indexOf("\"pass\":\"");
      if (pPos >= 0) {
        String p = body.substring(pPos + 8, body.indexOf("\"", pPos + 8));
        if (p.length() > 0) {
          if (strcmp(config.wifi_pass, p.c_str()) != 0) {
            wifiConfigured = true;
          }
          strncpy(config.wifi_pass, p.c_str(), sizeof(config.wifi_pass));
        }
      }

      int apPos = body.indexOf("\"apPass\":\"");
      if (apPos >= 0) {
        String ap = body.substring(apPos + 10, body.indexOf("\"", apPos + 10));
        ap.trim();
        strncpy(config.ap_password, ap.c_str(), sizeof(config.ap_password));
      }

      Serial.printf("[Config-POST] Wi-Fi SSID: '%s', Pass len: %d, Device: '%s', Team: '%s', Red: %d\n",
                    config.wifi_ssid, strlen(config.wifi_pass), config.device_name, config.primary_team, config.goal_classic_red);

      // Apply runtime configuration
      leds.configure(config);
      saveConfiguration();

      bool shouldRestart = (isApMode && strlen(config.wifi_ssid) > 0) || wifiConfigured;
      if (shouldRestart) {
        scheduledRestartTime = millis() + 1500;
        Serial.printf("[Config] Wi-Fi settings saved. Rebooting in 1.5s to connect to '%s'...\n", config.wifi_ssid);
        server.send(200, "application/json", "{\"success\":true,\"restart\":true}");
      } else {
        server.send(200, "application/json", "{\"success\":true,\"restart\":false}");
      }
      return;
    }
    server.send(400, "text/plain", "Bad Request");
  });

  // REST API: Quick Levels Live Adjustment (Brightness & Volume with Chirp, Broadcast Delay)
  server.on("/api/quick-set", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      int bPos = body.indexOf("\"brightness\":");
      if (bPos >= 0) {
        int b = body.substring(bPos + 13).toInt();
        if (b >= 5 && b <= 255) {
          config.brightness = (uint8_t)b;
          leds.setBrightness(config.brightness);
          prefs.putUChar("bright", config.brightness);
        }
      }

      int vPos = body.indexOf("\"volume\":");
      if (vPos >= 0) {
        int v = body.substring(vPos + 9).toInt();
        if (v >= 0 && v <= 5) {
          config.volume_index = (uint8_t)v;
          audio.setVolumeIndex(config.volume_index);
          prefs.putUChar("vol", config.volume_index);
        }
      }

      int bdPos = body.indexOf("\"broadcastDelay\":");
      if (bdPos >= 0) {
        int bd = body.substring(bdPos + 17).toInt();
        if (bd >= 0 && bd <= 180) {
          config.broadcast_delay_sec = (uint16_t)bd;
          prefs.putUShort("b_delay", config.broadcast_delay_sec);
          eventLog.log("[USER] Delay set to %ds", config.broadcast_delay_sec);
        }
      }

      if (body.indexOf("\"chirp\":true") >= 0 && config.hw_speaker_enabled) {
        audio.playChirp();
      }

      server.send(200, "application/json", "{\"success\":true}");
      return;
    }
    server.send(400, "text/plain", "Bad Request");
  });

  // REST API: Verbose Diagnostics Log
  server.on("/api/logs", HTTP_GET, []() {
    server.send(200, "text/plain", eventLog.getText());
  });

  // REST API: Test Celebration & Events (supports horn, light, display, event types, effect override, and stop)
  server.on("/api/test", HTTP_POST, []() {
    bool horn = true;
    bool light = true;
    bool display = true;
    String eventType = "goal";
    uint8_t effect = config.effect_style;

    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      if (body.indexOf("\"horn\":false") >= 0) horn = false;
      if (body.indexOf("\"light\":false") >= 0) light = false;
      if (body.indexOf("\"display\":false") >= 0) display = false;
      int evPos = body.indexOf("\"event\":\"");
      if (evPos >= 0) {
        eventType = body.substring(evPos + 9, body.indexOf("\"", evPos + 9));
      }
      int effPos = body.indexOf("\"effect\":");
      if (effPos >= 0) {
        effect = (uint8_t)body.substring(effPos + 9).toInt();
      }
    }

    if (eventType == "stop") {
      leds.returnToIdle();
      audio.stop();
      server.send(200, "application/json", "{\"success\":true,\"stopped\":true}");
      return;
    }

    if (eventType == "opponent") {
      if (light && config.hw_lights_enabled) leds.setMode(LED_OPPONENT_GOAL, 4000);
      if (horn && config.hw_speaker_enabled) audio.play(TRACK_PERIOD_HORN);
    } else if (eventType == "powerplay") {
      if (light && config.hw_lights_enabled) leds.setMode(LED_POWER_PLAY, 8000);
      if (horn && config.hw_speaker_enabled) audio.play(TRACK_GAME_START);
    } else if (eventType == "buzzer") {
      if (light && config.hw_lights_enabled) leds.setMode(LED_STROBE_GOAL, 3000);
      if (horn && config.hw_speaker_enabled) audio.play(TRACK_PERIOD_HORN);
    } else if (eventType == "win") {
      if (light && config.hw_lights_enabled) leds.setMode(LED_VICTORY_RAINBOW, 20000);
      if (horn && config.hw_speaker_enabled) audio.play(TRACK_WIN_FANFARE);
    } else if (eventType == "intermission") {
      if (light && config.hw_lights_enabled) leds.setMode(LED_IDLE_BREATHE, 10000);
      if (horn && config.hw_speaker_enabled) audio.play(TRACK_PERIOD_HORN);
    } else { // default goal celebration
      if (horn && config.hw_speaker_enabled) {
        audio.play(TRACK_GOAL_HORN, config.celebration_sec * 1000UL);
      }
      if (light && config.hw_lights_enabled) {
        LedMode mode = LED_BEACON_GOAL;
        switch (effect) {
          case 0: mode = LED_BEACON_GOAL; break;
          case 1: mode = LED_STROBE_GOAL; break;
          case 2: mode = LED_BREATHE_CELEBRATE; break;
          case 3: mode = LED_SIREN_DUAL; break;
          case 4: mode = LED_HEARTBEAT; break;
          default: mode = LED_BEACON_GOAL; break;
        }
        leds.setMode(mode, config.celebration_sec * 1000UL);
      }
    }
    server.send(200, "application/json", "{\"success\":true}");
  });

  // REST API: Live Game & System Status
  server.on("/api/status", HTTP_GET, []() {
    const GameStatus &gs = nhl.getStatus();
    const UpcomingGame &up = nhl.getUpcoming();
    char hostSlug[33];
    generateHostname(config.device_name, hostSlug, sizeof(hostSlug));
    String json = "{";
    json += "\"system\":{";
    json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"isApMode\":" + String(isApMode ? "true" : "false") + ",";
    json += "\"hostname\":\"" + String(hostSlug) + ".local\",";
    json += "\"deviceName\":\"" + String(config.device_name) + "\",";
    json += "\"team\":\"" + String(config.primary_team) + "\",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"version\":\"" + String(FIRMWARE_VERSION) + "\"";
    json += "},";
    json += "\"game\":{";
    json += "\"state\":\"" + String(gs.state == STATE_LIVE ? "LIVE" : (gs.state == STATE_FINAL ? "FINAL" : (gs.state == STATE_PREGAME ? "PREGAME" : "IDLE"))) + "\",";
    json += "\"homeAbbrev\":\"" + gs.homeAbbrev + "\",";
    json += "\"awayAbbrev\":\"" + gs.awayAbbrev + "\",";
    json += "\"homeScore\":" + String(gs.homeScore) + ",";
    json += "\"awayScore\":" + String(gs.awayScore) + ",";
    json += "\"period\":" + String(gs.period) + ",";
    json += "\"timeRemaining\":\"" + gs.timeRemaining + "\",";
    json += "\"secondsRemaining\":" + String(gs.secondsRemaining) + ",";
    json += "\"running\":" + String(gs.clockRunning ? "true" : "false") + ",";
    json += "\"inIntermission\":" + String(gs.inIntermission ? "true" : "false") + ",";
    json += "\"lastPlay\":\"" + gs.lastPlay + "\",";
    json += "\"broadcastDelay\":" + String(config.broadcast_delay_sec) + ",";
    unsigned long stopElapsed = (gs.lastStoppageMillis > 0) ? ((millis() - gs.lastStoppageMillis) / 1000UL) : 9999;
    unsigned long resumeElapsed = (gs.lastResumeMillis > 0) ? ((millis() - gs.lastResumeMillis) / 1000UL) : 9999;
    unsigned long playElapsed = (gs.lastPlayMillis > 0) ? ((millis() - gs.lastPlayMillis) / 1000UL) : 0;
    String safeApiDate = gs.apiServerDate;
    safeApiDate.replace("\"", "");
    json += "\"stoppageElapsed\":" + String(stopElapsed) + ",";
    json += "\"resumeElapsed\":" + String(resumeElapsed) + ",";
    json += "\"stoppageSeconds\":" + String(gs.stoppageSecondsRemaining) + ",";
    json += "\"lastPlayElapsed\":" + String(playElapsed) + ",";
    json += "\"lastPlayEpoch\":" + String(gs.lastPlayEpoch) + ",";
    json += "\"espEpoch\":" + String((unsigned long)time(nullptr)) + ",";
    json += "\"apiDate\":\"" + safeApiDate + "\",";
    json += "\"apiAge\":" + String(gs.apiCacheAge) + ",";
    json += "\"recentPlays\":[";
    for (int i = 0; i < gs.recentPlayCount; i++) {
      if (i > 0) json += ",";
      unsigned long rElapsed = (gs.recentPlays[i].arrivalMillis > 0) ? ((millis() - gs.recentPlays[i].arrivalMillis) / 1000UL) : 0;
      String desc = gs.recentPlays[i].desc;
      desc.replace("\"", "'");
      json += "{\"desc\":\"" + desc + "\",\"elapsed\":" + String(rElapsed) + "}";
    }
    json += "]";
    json += "},";
    json += "\"upcoming\":{";
    json += "\"valid\":" + String(up.valid ? "true" : "false") + ",";
    json += "\"gameId\":" + String(up.gameId) + ",";
    json += "\"date\":\"" + up.date + "\",";
    json += "\"timeUTC\":\"" + up.timeUTC + "\",";
    json += "\"opponent\":\"" + up.opponent + "\",";
    json += "\"opponentName\":\"" + up.opponentName + "\",";
    json += "\"isHome\":" + String(up.isHome ? "true" : "false") + ",";
    json += "\"venue\":\"" + up.venue + "\",";
    json += "\"broadcasts\":\"" + up.broadcasts + "\",";
    json += "\"gameState\":\"" + up.gameState + "\"";
    json += "}}";
    server.send(200, "application/json", json);
  });

  // REST API: Manual OTA Firmware Upload
  server.on("/api/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    if (Update.hasError()) {
      server.send(500, "application/json", "{\"success\":false,\"error\":\"OTA flashing failed. Ensure you selected firmware.bin, not merged.bin.\"}");
      return;
    }
    server.send(200, "application/json", "{\"success\":true}");
    delay(1000);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] Manual upload started: %s\n", upload.filename.c_str());
      if (upload.filename.indexOf("merged") >= 0) {
        Serial.println("[OTA] Error: User selected merged.bin! Aborting OTA.");
        return;
      }
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("[OTA] Manual upload success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // REST API: Cloud 1-Click OTA Initiator (Background Task)
  server.on("/api/ota-cloud", HTTP_POST, []() {
    String url = "https://revelrob.github.io/NHLGoalLight/builds/firmware.bin";
    if (server.hasArg("plain")) {
      String body = server.arg("plain");
      int uPos = body.indexOf("\"url\"");
      if (uPos >= 0) {
        int httpPos = body.indexOf("http", uPos);
        int quoteEnd = body.indexOf("\"", httpPos);
        if (httpPos >= 0 && quoteEnd > httpPos) {
          url = body.substring(httpPos, quoteEnd);
        }
      }
    }

    cloudOtaTargetUrl = url;
    otaProgress.inProgress = true;
    otaProgress.percent = 0;
    otaProgress.bytesWritten = 0;
    otaProgress.totalBytes = 0;
    otaProgress.success = false;
    otaProgress.failed = false;
    otaProgress.message = "Initiating cloud OTA...";

    server.send(200, "application/json", "{\"success\":true,\"message\":\"Cloud OTA started\"}");

    // Spawn on Core 1 so WebServer stays alive and can report progress
    xTaskCreatePinnedToCore(cloudOtaTask, "CloudOtaTask", 8192, NULL, 1, NULL, 1);
  });

  // REST API: Real-Time OTA Progress Status
  server.on("/api/ota-status", HTTP_GET, []() {
    String json = "{";
    json += "\"inProgress\":" + String(otaProgress.inProgress ? "true" : "false") + ",";
    json += "\"percent\":" + String(otaProgress.percent) + ",";
    json += "\"bytesWritten\":" + String(otaProgress.bytesWritten) + ",";
    json += "\"totalBytes\":" + String(otaProgress.totalBytes) + ",";
    json += "\"success\":" + String(otaProgress.success ? "true" : "false") + ",";
    json += "\"failed\":" + String(otaProgress.failed ? "true" : "false") + ",";
    json += "\"message\":\"" + otaProgress.message + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // Captive portal fallback
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("[WebServer] HTTP server started on port 80.");
}

// =====================================================
// NVS STORAGE (PREFERENCES)
// =====================================================
void loadConfiguration() {
  initDefaultConfig(config);
  prefs.begin("goallight", false);

  String devName = prefs.getString("dev_name", "NHL Goal Light");
  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  String apPass = prefs.getString("ap_pass", "");
  String team = prefs.getString("team", "TOR");

  if (devName.length() > 0) strncpy(config.device_name, devName.c_str(), sizeof(config.device_name));
  if (ssid.length() > 0) strncpy(config.wifi_ssid, ssid.c_str(), sizeof(config.wifi_ssid));
  if (pass.length() > 0) strncpy(config.wifi_pass, pass.c_str(), sizeof(config.wifi_pass));
  if (apPass.length() > 0) strncpy(config.ap_password, apPass.c_str(), sizeof(config.ap_password));
  if (team.length() > 0) strncpy(config.primary_team, team.c_str(), sizeof(config.primary_team));

  config.hw_lights_enabled = prefs.getBool("hw_light", true);
  config.hw_speaker_enabled = prefs.getBool("hw_audio", true);
  config.hw_display_enabled = prefs.getBool("hw_disp", false);

  config.num_leds = prefs.getUShort("n_leds", NUM_LEDS_DEFAULT);
  config.led_layout = prefs.getUChar("led_lay", 0);
  config.led_reverse = prefs.getBool("led_rev", false);
  config.idle_effect = prefs.getUChar("idle_eff", 1);

  config.brightness = prefs.getUChar("bright", DEFAULT_BRIGHTNESS);
  config.volume_index = prefs.getUChar("vol", DEFAULT_VOLUME_IDX);
  config.celebration_sec = prefs.getUChar("dur", DEFAULT_HORN_SEC);
  config.effect_style = prefs.getUChar("effect", 0);
  config.goal_classic_red = prefs.getBool("goal_red", false);

  config.alert_opponent_goal = prefs.getBool("opp", false);
  config.alert_opponent_sound = prefs.getBool("opp_snd", true);
  config.alert_power_play = prefs.getBool("pp", true);
  config.alert_power_play_sound = prefs.getBool("pp_snd", true);
  config.alert_period_buzzer = prefs.getBool("buzzer", true);
  config.alert_period_buzzer_sound = prefs.getBool("buzz_snd", true);
  config.alert_intermission = prefs.getBool("interm", true);
  config.alert_game_win = prefs.getBool("win", true);
  config.alert_game_win_sound = prefs.getBool("win_snd", true);

  config.broadcast_delay_sec = prefs.getUShort("b_delay", 0);

  prefs.end();
  Serial.println("[Config] Settings loaded from NVS flash.");
}

void saveConfiguration() {
  prefs.begin("goallight", false);

  prefs.putString("dev_name", config.device_name);
  prefs.putString("ssid", config.wifi_ssid);
  prefs.putString("pass", config.wifi_pass);
  prefs.putString("ap_pass", config.ap_password);
  prefs.putString("team", config.primary_team);

  prefs.putBool("hw_light", config.hw_lights_enabled);
  prefs.putBool("hw_audio", config.hw_speaker_enabled);
  prefs.putBool("hw_disp", config.hw_display_enabled);

  prefs.putUShort("n_leds", config.num_leds);
  prefs.putUChar("led_lay", config.led_layout);
  prefs.putBool("led_rev", config.led_reverse);
  prefs.putUChar("idle_eff", config.idle_effect);

  prefs.putUChar("bright", config.brightness);
  prefs.putUChar("vol", config.volume_index);
  prefs.putUChar("dur", config.celebration_sec);
  prefs.putUChar("effect", config.effect_style);
  prefs.putBool("goal_red", config.goal_classic_red);

  prefs.putBool("opp", config.alert_opponent_goal);
  prefs.putBool("opp_snd", config.alert_opponent_sound);
  prefs.putBool("pp", config.alert_power_play);
  prefs.putBool("pp_snd", config.alert_power_play_sound);
  prefs.putBool("buzzer", config.alert_period_buzzer);
  prefs.putBool("buzz_snd", config.alert_period_buzzer_sound);
  prefs.putBool("interm", config.alert_intermission);
  prefs.putBool("win", config.alert_game_win);
  prefs.putBool("win_snd", config.alert_game_win_sound);

  prefs.putUShort("b_delay", config.broadcast_delay_sec);

  prefs.end();
  Serial.println("[Config] Settings saved to NVS flash.");
}
