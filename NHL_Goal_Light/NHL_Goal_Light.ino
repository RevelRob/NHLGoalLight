#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <Update.h>

#include "Config.h"
#include "Teams.h"
#include "LedEngine.h"
#include "AudioEngine.h"
#include "NhlClient.h"
#include "WebPortal.h"

// =====================================================
// GLOBAL OBJECTS
// =====================================================
Preferences prefs;
WebServer server(80);
DNSServer dnsServer;

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

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println("\n=================================");
  Serial.println("🚨 NHL Goal Light Initializing");
  Serial.printf("📦 Firmware Version: %s\n", FIRMWARE_VERSION);
  Serial.println("=================================");

#ifdef PIN_STATUS_LED
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW); // Turn on built-in blue LED (active LOW)
#endif

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP); // On-board BOOT button on ESP32-S3 Super Mini

  // Load NVS config
  loadConfiguration();

  // Initialize LED Engine
  const TeamData* team = findTeam(config.primary_team);
  leds.begin();
  leds.setBrightness(config.brightness);
  leds.setTeam(team);

  // Initialize Audio
  audio.begin();
  audio.setVolumeIndex(config.volume_index);

  // Initialize NHL Client
  nhl.setTeam(config.primary_team);

  // Check if either external button (GPIO 9) or on-board BOOT button (GPIO 0) is held during boot
  if (digitalRead(PIN_BUTTON) == LOW || digitalRead(0) == LOW) {
    Serial.println("[Boot] Button held on power up: Forcing AP setup mode and clearing stored Wi-Fi.");
    isApMode = true;
    config.wifi_ssid[0] = '\0';
    config.wifi_pass[0] = '\0';
    saveConfiguration();
  }

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

  // Periodic Serial heartbeat so Serial Monitor confirms the device is running
  static unsigned long lastHeartbeat = 0;
  if (now - lastHeartbeat >= 3000) {
    lastHeartbeat = now;
    Serial.printf("🏒 [NHL Goal Light] Alive! Mode: %s | Free RAM: %u bytes | IP: %s\n",
                  isApMode ? "AP (SSID: NHL-Goal-Light-Setup)" : "Station Mode",
                  ESP.getFreeHeap(),
                  isApMode ? "192.168.4.1" : WiFi.localIP().toString().c_str());
  }

  // 5. If celebrating or in AP mode, skip live polling
  if (isApMode || leds.isCelebrating()) {
    return;
  }

  // 6. Check Wi-Fi reconnection if lost
  if (WiFi.status() != WL_CONNECTED && !isApMode) {
    return;
  }

  // 7. Poll NHL Web API
  bool opponentGoal = false;
  bool gameWin = false;
  bool periodStart = false;

  bool preferredGoal = nhl.poll(opponentGoal, gameWin, periodStart);

  if (preferredGoal) {
    triggerGoalCelebration();
  } else if (opponentGoal && config.alert_opponent_goal) {
    triggerOpponentGoal();
  } else if (gameWin && config.alert_game_win) {
    triggerGameWin();
  } else if (periodStart && config.alert_period_buzzer) {
    audio.play(TRACK_PERIOD_HORN);
  }
}

// =====================================================
// EVENT ACTIONS
// =====================================================
void triggerGoalCelebration() {
  Serial.println("[Event] GOAL CELEBRATION TRIGGERED!");
  audio.play(TRACK_GOAL_HORN);

  LedMode mode = (config.effect_style == 1) ? LED_STROBE_GOAL : LED_BEACON_GOAL;
  leds.setMode(mode, config.celebration_sec * 1000UL);
}

void triggerOpponentGoal() {
  Serial.println("[Event] Opponent Goal Alert.");
  leds.setMode(LED_OPPONENT_GOAL, 3000);
}

void triggerGameWin() {
  Serial.println("[Event] VICTORY CELEBRATION!");
  audio.play(TRACK_WIN_FANFARE);
  leds.setMode(LED_VICTORY_RAINBOW, 20000);
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
      // Single press: cycle volume
      uint8_t nextVol = (audio.getVolumeIndex() + 1) % 6;
      audio.setVolumeIndex(nextVol);
      config.volume_index = nextVol;
      saveConfiguration();
      Serial.printf("[Button] Single press: Volume index %d\n", nextVol);
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

  // Set conservative Wi-Fi TX power (11dBm) to prevent voltage drop on compact SuperMini regulator
  WiFi.setTxPower(WIFI_POWER_11dBm);

  if (isApMode || strlen(config.wifi_ssid) == 0) {
    isApMode = true;
    Serial.println("[Network] Starting Access Point: " AP_SSID);
    WiFi.mode(WIFI_AP_STA); // AP_STA allows background scanning while AP is active
    delay(50);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    bool apSuccess = WiFi.softAP(AP_SSID);
    delay(100);

    dnsServer.start(DNS_PORT, "*", apIP);
    Serial.printf("[Network] AP '%s' status: %s (IP: 192.168.4.1)\n", AP_SSID, apSuccess ? "ACTIVE" : "FAILED");

    // Pre-cache nearby Wi-Fi networks immediately
    WiFi.scanNetworks(true);

    leds.setMode(LED_AP_CONFIG);
#ifdef PIN_STATUS_LED
    digitalWrite(PIN_STATUS_LED, LOW); // Blue LED ON solid in AP mode
#endif
  } else {
    // Both SoftAP and Station Mode: SoftAP remains available for phone connection while connecting to router
    Serial.printf("[Network] Starting AP '%s' & connecting to '%s'...\n", AP_SSID, config.wifi_ssid);
    WiFi.mode(WIFI_AP_STA);
    delay(50);
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(AP_SSID);
    dnsServer.start(DNS_PORT, "*", apIP);

    WiFi.setHostname(MDNS_HOSTNAME);
    WiFi.begin(config.wifi_ssid, config.wifi_pass);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) {
      delay(200);
      Serial.print(".");
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED)); // Blink while connecting
#endif
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n[Network] Wi-Fi Connected!");
      Serial.print("[Network] IP Address: ");
      Serial.println(WiFi.localIP());

      // Turn off SoftAP once connected so phone re-joins home Wi-Fi automatically
      WiFi.softAPdisconnect(true);
      WiFi.mode(WIFI_STA);
      isApMode = false;

      if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.println("[Network] mDNS responder started: http://" MDNS_HOSTNAME ".local");
      }
      leds.setMode(LED_IDLE_BREATHE);
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, HIGH); // Blue LED OFF when connected online
#endif
    } else {
      Serial.println("\n[Network] Wi-Fi Connection failed. AP remains broadcast on 192.168.4.1.");
      isApMode = true;
      WiFi.scanNetworks(true);
      leds.setMode(LED_AP_CONFIG);
#ifdef PIN_STATUS_LED
      digitalWrite(PIN_STATUS_LED, LOW); // Blue LED ON solid in AP mode
#endif
    }
  }
}

void setupWebServer() {
  // Main page
  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
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
      if (ssid.length() == 0) continue; // Skip hidden/blank SSIDs

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
    json += "\"team\":\"" + String(config.primary_team) + "\",";
    json += "\"brightness\":" + String(config.brightness) + ",";
    json += "\"volume\":" + String(config.volume_index) + ",";
    json += "\"hornDuration\":" + String(config.celebration_sec) + ",";
    json += "\"effectStyle\":" + String(config.effect_style) + ",";
    json += "\"alertOpponent\":" + String(config.alert_opponent_goal ? "true" : "false") + ",";
    json += "\"alertPowerPlay\":" + String(config.alert_power_play ? "true" : "false") + ",";
    json += "\"alertBuzzer\":" + String(config.alert_period_buzzer ? "true" : "false") + ",";
    json += "\"alertWin\":" + String(config.alert_game_win ? "true" : "false") + ",";
    json += "\"ssid\":\"" + String(config.wifi_ssid) + "\"";
    json += "}";
    server.send(200, "application/json", json);
  });

  // REST API: Config POST
  server.on("/api/config", HTTP_POST, []() {
    if (server.hasArg("plain")) {
      String body = server.arg("plain");

      bool wifiConfigured = false;

      // Extract fields simply without large JSON parser overhead
      int teamPos = body.indexOf("\"team\":\"");
      if (teamPos >= 0) {
        String t = body.substring(teamPos + 8, body.indexOf("\"", teamPos + 8));
        strncpy(config.primary_team, t.c_str(), sizeof(config.primary_team));
        const TeamData* team = findTeam(config.primary_team);
        leds.setTeam(team);
        nhl.setTeam(config.primary_team);
      }

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

      config.alert_opponent_goal = (body.indexOf("\"alertOpponent\":true") >= 0);
      config.alert_power_play = (body.indexOf("\"alertPowerPlay\":true") >= 0);
      config.alert_period_buzzer = (body.indexOf("\"alertBuzzer\":true") >= 0);
      config.alert_game_win = (body.indexOf("\"alertWin\":true") >= 0);

      int sPos = body.indexOf("\"ssid\":\"");
      if (sPos >= 0) {
        String s = body.substring(sPos + 8, body.indexOf("\"", sPos + 8));
        s.trim();
        if (s.length() > 0) {
          if (strcmp(config.wifi_ssid, s.c_str()) != 0) {
            wifiConfigured = true;
            config.wifi_pass[0] = '\0';
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

      saveConfiguration();

      bool shouldRestart = (isApMode && strlen(config.wifi_ssid) > 0) || wifiConfigured;
      if (shouldRestart) {
        scheduledRestartTime = millis() + 1200;
        Serial.printf("[Config] Wi-Fi settings saved. Rebooting in 1.2s to connect to '%s'...\n", config.wifi_ssid);
        server.send(200, "application/json", "{\"success\":true,\"restart\":true}");
      } else {
        server.send(200, "application/json", "{\"success\":true,\"restart\":false}");
      }
      return;
    }
    server.send(400, "text/plain", "Bad Request");
  });

  // REST API: Test
  server.on("/api/test", HTTP_POST, []() {
    triggerGoalCelebration();
    server.send(200, "text/plain", "Test Triggered");
  });

  // REST API: Status
  server.on("/api/status", HTTP_GET, []() {
    const GameStatus &gs = nhl.getStatus();
    String json = "{";
    json += "\"system\":{";
    json += "\"connected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
    json += "\"team\":\"" + String(config.primary_team) + "\",";
    json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"version\":\"" + String(FIRMWARE_VERSION) + "\"";
    json += "},";
    json += "\"game\":{";
    json += "\"state\":\"" + String(gs.state == STATE_LIVE ? "LIVE" : (gs.state == STATE_FINAL ? "FINAL" : "IDLE")) + "\",";
    json += "\"homeAbbrev\":\"" + gs.homeAbbrev + "\",";
    json += "\"awayAbbrev\":\"" + gs.awayAbbrev + "\",";
    json += "\"homeScore\":" + String(gs.homeScore) + ",";
    json += "\"awayScore\":" + String(gs.awayScore);
    json += "}}";
    server.send(200, "application/json", json);
  });

  // REST API: OTA Firmware Update
  server.on("/api/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "application/json", Update.hasError() ? "{\"success\":false,\"error\":\"Update failed\"}" : "{\"success\":true}");
    delay(500);
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("[OTA] Update started: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) {
        Serial.printf("[OTA] Update Success: %u bytes\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
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

  String ssid = prefs.getString("ssid", "");
  String pass = prefs.getString("pass", "");
  String team = prefs.getString("team", "NYR");

  if (ssid.length() > 0) strncpy(config.wifi_ssid, ssid.c_str(), sizeof(config.wifi_ssid));
  if (pass.length() > 0) strncpy(config.wifi_pass, pass.c_str(), sizeof(config.wifi_pass));
  if (team.length() > 0) strncpy(config.primary_team, team.c_str(), sizeof(config.primary_team));

  config.brightness = prefs.getUChar("bright", DEFAULT_BRIGHTNESS);
  config.volume_index = prefs.getUChar("vol", DEFAULT_VOLUME_IDX);
  config.celebration_sec = prefs.getUChar("dur", DEFAULT_HORN_SEC);
  config.effect_style = prefs.getUChar("effect", 0);
  config.alert_opponent_goal = prefs.getBool("opp", false);
  config.alert_power_play = prefs.getBool("pp", true);
  config.alert_period_buzzer = prefs.getBool("buzzer", true);
  config.alert_game_win = prefs.getBool("win", true);

  prefs.end();
  Serial.println("[Config] Settings loaded from NVS flash.");
}

void saveConfiguration() {
  prefs.begin("goallight", false);
  prefs.putString("ssid", config.wifi_ssid);
  prefs.putString("pass", config.wifi_pass);
  prefs.putString("team", config.primary_team);
  prefs.putUChar("bright", config.brightness);
  prefs.putUChar("vol", config.volume_index);
  prefs.putUChar("dur", config.celebration_sec);
  prefs.putUChar("effect", config.effect_style);
  prefs.putBool("opp", config.alert_opponent_goal);
  prefs.putBool("pp", config.alert_power_play);
  prefs.putBool("buzzer", config.alert_period_buzzer);
  prefs.putBool("win", config.alert_game_win);
  prefs.end();
  Serial.println("[Config] Settings saved to NVS flash.");
}
