#pragma once
#include <Arduino.h>

#define FIRMWARE_VERSION     "1.1.0.2"
#define GITHUB_REPO          "RevelRob/NHLGoalLight"

// =====================================================
// PIN DEFINITIONS - Tailored for ESP32-S3 Super Mini
// =====================================================

#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV)
  // ESP32-S3 Super Mini Default Pinout
  #define PIN_STATUS_LED   8   // Built-in blue LED on Super Mini (active LOW)
  #define PIN_NEOPIXEL     4   // GPIO 4 -> NeoPixel DIN (via 330 ohm resistor)
  #define PIN_BUTTON       9   // GPIO 9 -> Tactile Switch to GND (INPUT_PULLUP)
  #define PIN_DFPLAYER_TX  1   // GPIO 1 -> DFPlayer RX (ESP32 TX)
  #define PIN_DFPLAYER_RX  2   // GPIO 2 -> DFPlayer TX (ESP32 RX)

  // Future Phase 3: GC9A01 Round Display SPI Pins
  #define PIN_LCD_SCL      12  // SPI Clock (SCL / SCK)
  #define PIN_LCD_SDA      11  // SPI Data (MOSI / SDA)
  #define PIN_LCD_CS       10  // Chip Select (CS)
  #define PIN_LCD_DC       7   // Data/Command (DC)
  #define PIN_LCD_RST      6   // Reset (RST)
  #define PIN_LCD_BLK      3   // Backlight PWM (BLK)

#elif defined(CONFIG_IDF_TARGET_ESP32C3) || defined(ARDUINO_SEEED_XIAO_ESP32C3)
  // Seeed XIAO ESP32-C3 Pins
  #define PIN_NEOPIXEL     D3  // GPIO 5
  #define PIN_BUTTON       D4  // GPIO 6
  #define PIN_DFPLAYER_RX  D7  // GPIO 21
  #define PIN_DFPLAYER_TX  D6  // GPIO 20

#else
  // Standard ESP32-WROOM-32 DevKit Pins
  #define PIN_NEOPIXEL     18
  #define PIN_BUTTON       4
  #define PIN_DFPLAYER_RX  16
  #define PIN_DFPLAYER_TX  17
#endif

// =====================================================
// HARDWARE DEFAULTS
// =====================================================
#define NUM_LEDS_DEFAULT     16     // 16 or 24 LED NeoPixel ring
#define DEFAULT_BRIGHTNESS   180    // 0-255 scale
#define DEFAULT_VOLUME_IDX   3      // 0-5 volume preset
#define DEFAULT_HORN_SEC     12     // Goal celebration duration

// =====================================================
// NETWORK DEFAULTS
// =====================================================
#define AP_SSID              "NHL-Goal-Light-Setup"
#define AP_PASSWORD          ""     // Open network for quick mobile setup
#define MDNS_HOSTNAME        "goallight"

// =====================================================
// PERSISTENT CONFIGURATION STRUCT
// =====================================================
struct SystemConfig {
  char device_name[33];      // Friendly name (e.g. "NHL Goal Light", "Living Room Beacon")
  char wifi_ssid[33];
  char wifi_pass[65];
  char ap_password[33];      // SoftAP password (empty = open, min 8 chars = WPA2)
  char primary_team[4];      // e.g. "TOR"

  // Hardware Module Enablement
  bool hw_lights_enabled;    // WS2812B RGB LEDs attached
  bool hw_speaker_enabled;   // DFPlayer Mini audio module attached
  bool hw_display_enabled;   // GC9A01 circular LCD attached (Phase 3)

  // LED Geometry & Setup (WLED style)
  uint16_t num_leds;         // Total pixel count (8 to 300)
  uint8_t led_layout;        // 0=Single Ring, 1=Spiral Strip, 2=Parallel Columns, 3=Linear Strip
  bool led_reverse;          // Reverse rotation direction
  uint8_t idle_effect;       // 0=Off, 1=Team Colour Breathe, 2=Solid Team Colour, 3=Candle Flicker

  // Master Levels & Celebration
  uint8_t brightness;        // 0-255 scale
  uint8_t volume_index;      // 0-5 scale
  uint8_t celebration_sec;   // 5-30 seconds
  uint8_t effect_style;      // 0=Rotating Halogen Beacon, 1=Arena Strobe, 2=Team Colour Breathe, 3=Dual Siren Flasher, 4=Heartbeat Pulse

  // Event Action Flags
  bool alert_opponent_goal;  // Alert on opponent goals
  bool alert_power_play;     // Alert on power play
  bool alert_period_buzzer;  // Buzzer on period end/start
  bool alert_intermission;   // Intermission lighting mode
  bool alert_game_win;       // Extended victory fanfare

  // Broadcast Synchronisation
  uint16_t broadcast_delay_sec; // TV broadcast delay buffer (0 to 120 seconds)
};

inline void initDefaultConfig(SystemConfig &cfg) {
  memset(&cfg, 0, sizeof(SystemConfig));
  strncpy(cfg.device_name, "NHL Goal Light", sizeof(cfg.device_name));
  strncpy(cfg.wifi_ssid, "", sizeof(cfg.wifi_ssid));
  strncpy(cfg.wifi_pass, "", sizeof(cfg.wifi_pass));
  strncpy(cfg.ap_password, "", sizeof(cfg.ap_password));
  strncpy(cfg.primary_team, "TOR", sizeof(cfg.primary_team));

  cfg.hw_lights_enabled = true;
  cfg.hw_speaker_enabled = true;
  cfg.hw_display_enabled = false;

  cfg.num_leds = NUM_LEDS_DEFAULT;
  cfg.led_layout = 0; // Single Ring
  cfg.led_reverse = false;
  cfg.idle_effect = 1; // Team Colour Breathe

  cfg.brightness = DEFAULT_BRIGHTNESS;
  cfg.volume_index = DEFAULT_VOLUME_IDX;
  cfg.celebration_sec = DEFAULT_HORN_SEC;
  cfg.effect_style = 0; // Rotating Halogen Beacon

  cfg.alert_opponent_goal = false;
  cfg.alert_power_play = true;
  cfg.alert_period_buzzer = true;
  cfg.alert_intermission = true;
  cfg.alert_game_win = true;

  cfg.broadcast_delay_sec = 0;
}

