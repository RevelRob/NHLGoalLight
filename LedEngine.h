#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "Teams.h"

enum LedMode {
  LED_OFF,
  LED_AP_CONFIG,
  LED_IDLE_BREATHE,
  LED_BEACON_GOAL,
  LED_STROBE_GOAL,
  LED_OPPONENT_GOAL,
  LED_POWER_PLAY,
  LED_VICTORY_RAINBOW
};

class LedEngine {
private:
  Adafruit_NeoPixel strip;
  LedMode currentMode;
  unsigned long lastFrameTime;
  unsigned long modeStartTime;
  uint32_t durationMs;
  int animStep;
  uint8_t brightness;
  const TeamData* activeTeam;

public:
  LedEngine() : strip(NUM_LEDS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800),
                currentMode(LED_OFF),
                lastFrameTime(0),
                modeStartTime(0),
                durationMs(0),
                animStep(0),
                brightness(DEFAULT_BRIGHTNESS),
                activeTeam(&NHL_TEAMS[18]) {}

  void begin() {
    strip.begin();
    strip.setBrightness(brightness);
    strip.clear();
    strip.show();
  }

  void setTeam(const TeamData* team) {
    activeTeam = team;
  }

  void setBrightness(uint8_t b) {
    brightness = b;
    strip.setBrightness(b);
  }

  void setMode(LedMode mode, uint32_t duration = 0) {
    currentMode = mode;
    modeStartTime = millis();
    durationMs = duration;
    animStep = 0;
    lastFrameTime = 0;
  }

  LedMode getMode() const { return currentMode; }

  bool isCelebrating() const {
    return currentMode == LED_BEACON_GOAL || 
           currentMode == LED_STROBE_GOAL ||
           currentMode == LED_VICTORY_RAINBOW;
  }

  void update() {
    unsigned long now = millis();

    // Check duration expiration
    if (durationMs > 0 && (now - modeStartTime >= durationMs)) {
      setMode(LED_OFF);
      strip.clear();
      strip.show();
      return;
    }

    switch (currentMode) {
      case LED_OFF:
        strip.clear();
        strip.show();
        break;

      case LED_AP_CONFIG:
        renderApBeacon(now);
        break;

      case LED_BEACON_GOAL:
        renderRotatingBeacon(now);
        break;

      case LED_STROBE_GOAL:
        renderArenaStrobe(now);
        break;

      case LED_OPPONENT_GOAL:
        renderOpponentFlash(now);
        break;

      case LED_POWER_PLAY:
        renderPowerPlay(now);
        break;

      case LED_IDLE_BREATHE:
        renderBreathe(now);
        break;

      case LED_VICTORY_RAINBOW:
        renderRainbow(now);
        break;
    }
  }

private:
  void renderRotatingBeacon(unsigned long now) {
    if (now - lastFrameTime < 40) return; // ~25 FPS
    lastFrameTime = now;

    strip.clear();
    // 5 LEDs lit with brightness falloff traveling around the ring
    for (int i = 0; i < 5; i++) {
      int idx = (animStep + i) % NUM_LEDS;
      uint8_t intensity = (i == 2) ? 255 : (i == 1 || i == 3 ? 160 : 60);
      uint8_t r = (activeTeam->primaryR * intensity) / 255;
      uint8_t g = (activeTeam->primaryG * intensity) / 255;
      uint8_t b = (activeTeam->primaryB * intensity) / 255;
      strip.setPixelColor(idx, strip.Color(r, g, b));
    }
    strip.show();
    animStep = (animStep + 1) % NUM_LEDS;
  }

  void renderArenaStrobe(unsigned long now) {
    if (now - lastFrameTime < 50) return;
    lastFrameTime = now;

    strip.clear();
    bool strobeOn = (animStep % 2 == 0);
    int quadrant = (animStep / 4) % 2;

    if (strobeOn) {
      int half = NUM_LEDS / 2;
      int start = (quadrant == 0) ? 0 : half;
      int end = start + half;
      for (int i = start; i < end; i++) {
        strip.setPixelColor(i, strip.Color(255, 255, 255));
      }
    }
    strip.show();
    animStep = (animStep + 1) % 16;
  }

  void renderOpponentFlash(unsigned long now) {
    if (now - lastFrameTime < 150) return;
    lastFrameTime = now;

    bool on = (animStep % 2 == 0);
    strip.clear();
    if (on) {
      for (int i = 0; i < NUM_LEDS; i++) {
        strip.setPixelColor(i, strip.Color(120, 40, 0)); // Dim amber
      }
    }
    strip.show();
    animStep++;
  }

  void renderPowerPlay(unsigned long now) {
    if (now - lastFrameTime < 60) return;
    lastFrameTime = now;

    strip.clear();
    int lead = animStep % NUM_LEDS;
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i == lead || i == (lead + 1) % NUM_LEDS) {
        strip.setPixelColor(i, strip.Color(255, 180, 0)); // Amber accent
      } else {
        strip.setPixelColor(i, strip.Color(activeTeam->primaryR / 3, activeTeam->primaryG / 3, activeTeam->primaryB / 3));
      }
    }
    strip.show();
    animStep++;
  }

  void renderApBeacon(unsigned long now) {
    if (now - lastFrameTime < 100) return;
    lastFrameTime = now;

    strip.clear();
    int idx = animStep % NUM_LEDS;
    strip.setPixelColor(idx, strip.Color(255, 140, 0));
    strip.show();
    animStep++;
  }

  void renderBreathe(unsigned long now) {
    if (now - lastFrameTime < 30) return;
    lastFrameTime = now;

    float val = (exp(sin(now / 2000.0 * PI)) - 0.36787944) * 108.0;
    uint8_t factor = (uint8_t)constrain(val, 15.0, 255.0);

    for (int i = 0; i < NUM_LEDS; i++) {
      uint8_t r = (activeTeam->primaryR * factor) / 255;
      uint8_t g = (activeTeam->primaryG * factor) / 255;
      uint8_t b = (activeTeam->primaryB * factor) / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }

  void renderRainbow(unsigned long now) {
    if (now - lastFrameTime < 25) return;
    lastFrameTime = now;

    for (int i = 0; i < NUM_LEDS; i++) {
      int pixelHue = (animStep * 256 + (i * 65536L / NUM_LEDS)) & 0xFFFF;
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    animStep = (animStep + 1) % 256;
  }
};
