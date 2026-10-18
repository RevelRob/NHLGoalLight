#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "Teams.h"

enum LedMode {
  LED_OFF,
  LED_AP_CONFIG,
  LED_IDLE_BREATHE,
  LED_IDLE_SOLID,
  LED_IDLE_CANDLE,
  LED_IDLE_ORBIT,
  LED_BEACON_GOAL,
  LED_STROBE_GOAL,
  LED_BREATHE_CELEBRATE,
  LED_SIREN_DUAL,
  LED_HEARTBEAT,
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
  uint16_t numLeds;
  bool reverseDirection;
  uint8_t layout;
  uint8_t idleEffect;
  bool lightsEnabled;
  bool goalClassicRed;
  const TeamData* activeTeam;

public:
  LedEngine() : strip(NUM_LEDS_DEFAULT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800),
                currentMode(LED_OFF),
                lastFrameTime(0),
                modeStartTime(0),
                durationMs(0),
                animStep(0),
                brightness(DEFAULT_BRIGHTNESS),
                numLeds(NUM_LEDS_DEFAULT),
                reverseDirection(false),
                layout(0),
                idleEffect(1),
                lightsEnabled(true),
                goalClassicRed(false),
                activeTeam(&NHL_TEAMS[18]) {} // Default to Toronto Maple Leafs (TOR)

  void begin() {
    strip.begin();
    strip.setBrightness(brightness);
    strip.clear();
    strip.show();
  }

  void configure(const SystemConfig &cfg) {
    lightsEnabled = cfg.hw_lights_enabled;
    reverseDirection = cfg.led_reverse;
    layout = cfg.led_layout;
    idleEffect = cfg.idle_effect;
    goalClassicRed = cfg.goal_classic_red;
    setBrightness(cfg.brightness);

    uint16_t targetLeds = cfg.num_leds;
    if (targetLeds < 4) targetLeds = 4;
    if (targetLeds > 300) targetLeds = 300;

    if (targetLeds != numLeds) {
      numLeds = targetLeds;
      strip.updateLength(numLeds);
    }

    if (!lightsEnabled || idleEffect == 0) {
      setMode(LED_OFF);
      strip.clear();
      strip.show();
    } else if (!isCelebrating() && currentMode != LED_AP_CONFIG) {
      returnToIdle();
    }
  }

  void setTeam(const TeamData* team) {
    if (team != nullptr) {
      activeTeam = team;
    }
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
           currentMode == LED_BREATHE_CELEBRATE ||
           currentMode == LED_SIREN_DUAL ||
           currentMode == LED_HEARTBEAT ||
           currentMode == LED_VICTORY_RAINBOW;
  }

  void returnToIdle() {
    switch (idleEffect) {
      case 0: setMode(LED_OFF); break;
      case 1: setMode(LED_IDLE_BREATHE); break;
      case 2: setMode(LED_IDLE_SOLID); break;
      case 3: setMode(LED_IDLE_CANDLE); break;
      case 4: setMode(LED_IDLE_ORBIT); break;
      default: setMode(LED_IDLE_BREATHE); break;
    }
  }

  void update() {
    if (!lightsEnabled && currentMode != LED_AP_CONFIG) {
      if (animStep == 0) {
        strip.clear();
        strip.show();
        animStep = 1;
      }
      return;
    }

    unsigned long now = millis();

    // Check duration expiration
    if (durationMs > 0 && (now - modeStartTime >= durationMs)) {
      returnToIdle();
      return;
    }

    switch (currentMode) {
      case LED_OFF:
        if (animStep == 0) {
          strip.clear();
          strip.show();
          animStep = 1;
        }
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

      case LED_BREATHE_CELEBRATE:
        renderCelebrationBreathe(now);
        break;

      case LED_SIREN_DUAL:
        renderDualSiren(now);
        break;

      case LED_HEARTBEAT:
        renderHeartbeat(now);
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

      case LED_IDLE_SOLID:
        renderSolidTeamColour(now);
        break;

      case LED_IDLE_CANDLE:
        renderCandleFlicker(now);
        break;

      case LED_IDLE_ORBIT:
        renderOrbit(now);
        break;

      case LED_VICTORY_RAINBOW:
        renderRainbow(now);
        break;
    }
  }

private:
  inline uint8_t goalR() const { return goalClassicRed ? 245 : activeTeam->primaryR; }
  inline uint8_t goalG() const { return goalClassicRed ? 15  : activeTeam->primaryG; }
  inline uint8_t goalB() const { return goalClassicRed ? 15  : activeTeam->primaryB; }

  int mapLedIndex(int idx) {
    if (numLeds == 0) return 0;
    int safeIdx = idx % numLeds;
    if (safeIdx < 0) safeIdx += numLeds;
    return reverseDirection ? ((numLeds - 1) - safeIdx) : safeIdx;
  }

  // 1. Rotating Halogen Beacon: Sharp focal beam with smooth parabolic decay simulating a revolving mirror
  void renderRotatingBeacon(unsigned long now) {
    if (now - lastFrameTime < 35) return; // ~28 FPS smooth rotation
    lastFrameTime = now;

    strip.clear();

    // Calculate dynamic beam half-width based on ring size
    int beamHalfWidth = (numLeds <= 16) ? 2 : (numLeds <= 24 ? 3 : (numLeds / 6));
    if (beamHalfWidth < 1) beamHalfWidth = 1;

    for (int offset = -beamHalfWidth; offset <= beamHalfWidth; offset++) {
      int pixelIdx = mapLedIndex(animStep + offset);
      float distance = abs(offset) / (float)beamHalfWidth;
      // Parabolic falloff curve
      float intensity = 1.0f - (distance * distance * 0.85f);
      if (intensity < 0.1f) intensity = 0.1f;

      uint8_t r = (uint8_t)(goalR() * intensity);
      uint8_t g = (uint8_t)(goalG() * intensity);
      uint8_t b = (uint8_t)(goalB() * intensity);

      // Core focus receives bright white highlight
      if (offset == 0) {
        r = qadd8(r, 60);
        g = qadd8(g, 60);
        b = qadd8(b, 60);
      }

      strip.setPixelColor(pixelIdx, strip.Color(r, g, b));
    }

    strip.show();
    animStep = (animStep + 1) % numLeds;
  }

  // 2. High-Speed Arena Strobe: Blinding stadium strobe with crisp alternating bursts
  void renderArenaStrobe(unsigned long now) {
    if (now - lastFrameTime < 45) return;
    lastFrameTime = now;

    strip.clear();
    bool flash = (animStep % 2 == 0);
    int phase = (animStep / 4) % 3;

    if (flash) {
      if (phase == 0) {
        // Full flash white
        for (int i = 0; i < numLeds; i++) {
          strip.setPixelColor(i, strip.Color(255, 255, 255));
        }
      } else if (phase == 1) {
        // Goal colour flash
        for (int i = 0; i < numLeds; i++) {
          strip.setPixelColor(i, strip.Color(goalR(), goalG(), goalB()));
        }
      } else {
        // Alternating split
        for (int i = 0; i < numLeds; i++) {
          if (i % 2 == 0) {
            strip.setPixelColor(i, strip.Color(255, 255, 255));
          } else {
            strip.setPixelColor(i, strip.Color(goalR(), goalG(), goalB()));
          }
        }
      }
    }
    strip.show();
    animStep = (animStep + 1) % 24;
  }

  // 3. Team Colour Breathe (Celebration): High energy pulsing across all LEDs in unison
  void renderCelebrationBreathe(unsigned long now) {
    if (now - lastFrameTime < 25) return;
    lastFrameTime = now;

    // Fast celebration rhythm (~1.1s cycle)
    float val = (exp(sin(now / 350.0 * PI)) - 0.36787944) * 108.0;
    uint8_t factor = (uint8_t)constrain(val, 20.0, 255.0);

    for (int i = 0; i < numLeds; i++) {
      uint8_t r = (goalR() * factor) / 255;
      uint8_t g = (goalG() * factor) / 255;
      uint8_t b = (goalB() * factor) / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }

  // 4. Dual Siren Flasher: Authentic emergency lightbar wig-wag (left half goal colour, right half white)
  void renderDualSiren(unsigned long now) {
    if (now - lastFrameTime < 60) return;
    lastFrameTime = now;

    strip.clear();
    int half = numLeds / 2;
    int step = animStep % 12;

    bool sideA = (step < 6);
    bool flashOn = (step % 2 == 0);

    if (flashOn) {
      int start = sideA ? 0 : half;
      int end = sideA ? half : numLeds;

      for (int i = start; i < end; i++) {
        int idx = mapLedIndex(i);
        if (sideA) {
          strip.setPixelColor(idx, strip.Color(goalR(), goalG(), goalB()));
        } else {
          strip.setPixelColor(idx, strip.Color(255, 255, 255));
        }
      }
    }

    strip.show();
    animStep++;
  }

  // 5. Heartbeat Pulse: Urgent double-thump goal surge
  void renderHeartbeat(unsigned long now) {
    if (now - lastFrameTime < 25) return;
    lastFrameTime = now;

    unsigned long cycleTime = now % 900;
    float intensity = 0.05f;

    if (cycleTime < 180) {
      // First pulse
      intensity = sin((cycleTime / 180.0f) * PI);
    } else if (cycleTime >= 240 && cycleTime < 420) {
      // Second pulse (stronger)
      intensity = sin(((cycleTime - 240) / 180.0f) * PI) * 1.0f;
    }

    uint8_t factor = (uint8_t)constrain(intensity * 255.0f, 12.0f, 255.0f);
    for (int i = 0; i < numLeds; i++) {
      uint8_t r = (goalR() * factor) / 255;
      uint8_t g = (goalG() * factor) / 255;
      uint8_t b = (goalB() * factor) / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }

  void renderOpponentFlash(unsigned long now) {
    if (now - lastFrameTime < 140) return;
    lastFrameTime = now;

    bool on = (animStep % 2 == 0);
    strip.clear();
    if (on) {
      for (int i = 0; i < numLeds; i++) {
        strip.setPixelColor(i, strip.Color(140, 30, 0)); // Subdued amber warning
      }
    }
    strip.show();
    animStep++;
  }

  void renderPowerPlay(unsigned long now) {
    if (now - lastFrameTime < 50) return;
    lastFrameTime = now;

    strip.clear();
    int lead = animStep % numLeds;
    for (int i = 0; i < numLeds; i++) {
      int idx = mapLedIndex(i);
      if (i == lead || i == (lead + 1) % numLeds) {
        strip.setPixelColor(idx, strip.Color(255, 180, 0)); // Vibrant Gold Accent
      } else {
        strip.setPixelColor(idx, strip.Color(activeTeam->primaryR / 3, activeTeam->primaryG / 3, activeTeam->primaryB / 3));
      }
    }
    strip.show();
    animStep++;
  }

  // AP Setup Mode: Subtle gentle breathing on first pixel (pixel 0) in warm amber/orange
  void renderApBeacon(unsigned long now) {
    if (now - lastFrameTime < 30) return;
    lastFrameTime = now;

    strip.clear();
    // Smooth, slow breathing cycle (~2.5s)
    float val = (exp(sin(now / 2500.0 * PI)) - 0.36787944) * 108.0;
    uint8_t factor = (uint8_t)constrain(val, 6.0, 180.0);

    // Warm amber / orange (RGB 255, 120, 0)
    uint8_t r = (255 * factor) / 255;
    uint8_t g = (120 * factor) / 255;
    uint8_t b = 0;

    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
  }

  // Idle: Subtle gentle team breathe
  void renderBreathe(unsigned long now) {
    if (now - lastFrameTime < 30) return;
    lastFrameTime = now;

    float val = (exp(sin(now / 2200.0 * PI)) - 0.36787944) * 108.0;
    uint8_t factor = (uint8_t)constrain(val, 10.0, 255.0);

    for (int i = 0; i < numLeds; i++) {
      uint8_t r = (activeTeam->primaryR * factor) / 255;
      uint8_t g = (activeTeam->primaryG * factor) / 255;
      uint8_t b = (activeTeam->primaryB * factor) / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }

  // Idle: Solid team colour
  void renderSolidTeamColour(unsigned long now) {
    if (now - lastFrameTime < 250) return;
    lastFrameTime = now;

    for (int i = 0; i < numLeds; i++) {
      strip.setPixelColor(i, strip.Color(activeTeam->primaryR, activeTeam->primaryG, activeTeam->primaryB));
    }
    strip.show();
  }

  // Idle: Candle / Ember flicker
  void renderCandleFlicker(unsigned long now) {
    if (now - lastFrameTime < 70) return;
    lastFrameTime = now;

    for (int i = 0; i < numLeds; i++) {
      uint8_t flicker = random(30, 230);
      uint8_t r = (activeTeam->primaryR * flicker) / 255;
      uint8_t g = (activeTeam->primaryG * flicker) / 255;
      uint8_t b = (activeTeam->primaryB * flicker) / 255;
      strip.setPixelColor(i, strip.Color(r, g, b));
    }
    strip.show();
  }

  // Idle: Slow gentle orbit
  void renderOrbit(unsigned long now) {
    if (now - lastFrameTime < 160) return;
    lastFrameTime = now;

    strip.clear();
    int head = animStep % numLeds;
    for (int offset = -1; offset <= 1; offset++) {
      int idx = mapLedIndex(head + offset);
      float factor = (offset == 0) ? 0.75f : 0.2f;
      uint8_t r = (uint8_t)(activeTeam->primaryR * factor);
      uint8_t g = (uint8_t)(activeTeam->primaryG * factor);
      uint8_t b = (uint8_t)(activeTeam->primaryB * factor);
      strip.setPixelColor(idx, strip.Color(r, g, b));
    }
    strip.show();
    animStep = (animStep + 1) % numLeds;
  }

  void renderRainbow(unsigned long now) {
    if (now - lastFrameTime < 25) return;
    lastFrameTime = now;

    for (int i = 0; i < numLeds; i++) {
      int idx = mapLedIndex(i);
      int pixelHue = (animStep * 256 + (i * 65536L / numLeds)) & 0xFFFF;
      strip.setPixelColor(idx, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show();
    animStep = (animStep + 1) % 256;
  }

  // Saturating 8-bit addition helper
  static inline uint8_t qadd8(uint8_t i, uint8_t j) {
    int t = i + j;
    if (t > 255) t = 255;
    return (uint8_t)t;
  }
};
