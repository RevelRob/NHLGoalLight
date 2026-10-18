#pragma once
#include <Arduino.h>
#include <ESP_I2S.h>
#include <math.h>
#include "Config.h"

#if ENABLE_MICROSD_MODULE
#include <SPI.h>
#include <SD.h>
#endif

// 6 volume presets (0 = Mute, 1 = 20%, 2 = 40%, 3 = 60%, 4 = 80%, 5 = 100%)
const float VOLUME_FACTORS[6] = {0.0f, 0.15f, 0.35f, 0.60f, 0.85f, 1.0f};

enum SoundTrack {
  TRACK_NONE         = 0,
  TRACK_GOAL_HORN    = 1,
  TRACK_GAME_START   = 2,
  TRACK_PERIOD_HORN  = 3,
  TRACK_WIN_FANFARE  = 4,
  TRACK_TEST_HORN    = 5
};

class AudioEngine {
private:
  I2SClass i2s;
  bool i2sReady;
  bool sdReady;
  uint8_t volumeIndex;
  volatile bool isPlaying;
  volatile SoundTrack currentTrack;
  unsigned long trackStartTime;
  uint32_t trackDurationMs;
  TaskHandle_t audioTaskHandle;

  // Background audio worker task on Core 0
  static void audioWorkerTask(void *param) {
    AudioEngine *engine = static_cast<AudioEngine*>(param);
    engine->runAudioLoop();
  }

  void runAudioLoop() {
    const size_t CHUNK_SAMPLES = 256;
    int16_t buffer[CHUNK_SAMPLES * 2]; // 16-bit stereo interleaved (Left, Right)
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;
    const float sampleRate = 22050.0f;
    const float twoPi = 6.283185307f;

    while (true) {
      if (!isPlaying || currentTrack == TRACK_NONE || volumeIndex == 0) {
        vTaskDelay(pdMS_TO_TICKS(15));
        continue;
      }

      unsigned long elapsed = millis() - trackStartTime;
      if (trackDurationMs > 0 && elapsed >= trackDurationMs) {
        stop();
        vTaskDelay(pdMS_TO_TICKS(10));
        continue;
      }

      float vol = VOLUME_FACTORS[volumeIndex];
      SoundTrack track = currentTrack;

      // Generate audio chunk based on track
      for (size_t i = 0; i < CHUNK_SAMPLES; i++) {
        int16_t sample = 0;

        if (track == TRACK_GOAL_HORN || track == TRACK_TEST_HORN) {
          // Authentic NHL Stadium Air Horn: Harmonics 285Hz (dominant) + 356Hz + 440Hz with stadium resonance
          float s1 = sinf(phase1);
          float s2 = sinf(phase2);
          float s3 = sinf(phase3);

          // Add subtle acoustic overtones & saturation
          float mixed = (s1 * 0.50f) + (s2 * 0.32f) + (s3 * 0.18f);
          if (mixed > 0.85f) mixed = 0.85f + (mixed - 0.85f) * 0.3f;
          else if (mixed < -0.85f) mixed = -0.85f + (mixed + 0.85f) * 0.3f;

          sample = (int16_t)(mixed * vol * 24000.0f);

          phase1 += (twoPi * 285.0f) / sampleRate;
          phase2 += (twoPi * 356.0f) / sampleRate;
          phase3 += (twoPi * 440.0f) / sampleRate;
          if (phase1 >= twoPi) phase1 -= twoPi;
          if (phase2 >= twoPi) phase2 -= twoPi;
          if (phase3 >= twoPi) phase3 -= twoPi;

        } else if (track == TRACK_PERIOD_HORN) {
          // Classic Arena Buzzer: Harsh 120Hz + 240Hz + 360Hz buzz
          float s1 = (sinf(phase1) > 0 ? 0.7f : -0.7f);
          float s2 = (sinf(phase2) > 0 ? 0.4f : -0.4f);
          float mixed = (s1 * 0.65f) + (s2 * 0.35f);
          sample = (int16_t)(mixed * vol * 22000.0f);

          phase1 += (twoPi * 120.0f) / sampleRate;
          phase2 += (twoPi * 360.0f) / sampleRate;
          if (phase1 >= twoPi) phase1 -= twoPi;
          if (phase2 >= twoPi) phase2 -= twoPi;

        } else if (track == TRACK_GAME_START || track == TRACK_WIN_FANFARE) {
          // Uplifting Arena Charge Fanfare: Dynamic arpeggiated melodic notes
          unsigned long tSec = elapsed / 280;
          float freq = 392.0f; // G4
          switch (tSec % 6) {
            case 0: freq = 392.0f; break; // G4
            case 1: freq = 523.25f; break; // C5
            case 2: freq = 659.25f; break; // E5
            case 3: freq = 783.99f; break; // G5
            case 4: freq = 659.25f; break; // E5
            case 5: freq = 783.99f; break; // G5
          }
          sample = (int16_t)(sinf(phase1) * vol * 20000.0f);
          phase1 += (twoPi * freq) / sampleRate;
          if (phase1 >= twoPi) phase1 -= twoPi;
        }

        buffer[i * 2]     = sample; // Left Channel
        buffer[i * 2 + 1] = sample; // Right Channel
      }

      if (i2sReady) {
        i2s.write((const uint8_t*)buffer, sizeof(buffer));
      }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
  }

public:
  AudioEngine() : i2sReady(false),
                  sdReady(false),
                  volumeIndex(DEFAULT_VOLUME_IDX),
                  isPlaying(false),
                  currentTrack(TRACK_NONE),
                  trackStartTime(0),
                  trackDurationMs(0),
                  audioTaskHandle(nullptr) {}

  void begin() {
    Serial.println("[Audio] Initialising MAX98357A I2S Class-D DAC...");
    i2s.setPins(PIN_I2S_BCLK, PIN_I2S_LRC, PIN_I2S_DOUT);

    if (i2s.begin(I2S_MODE_STD, 22050, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO)) {
      i2sReady = true;
      Serial.println("[Audio] MAX98357A I2S DAC connected successfully (BCLK:1, LRC:2, DIN:3).");
    } else {
      i2sReady = false;
      Serial.println("[Audio] Failed to initialise I2S interface. Audio output unavailable.");
    }

#if ENABLE_MICROSD_MODULE
    // Check optional MicroSD module
    SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
    if (SD.begin(PIN_SD_CS)) {
      sdReady = true;
      Serial.println("[Audio] MicroSD module detected. Custom audio clips supported.");
    } else {
      sdReady = false;
      Serial.println("[Audio] No MicroSD module detected. Using built-in I2S real-time audio synthesizer.");
    }
#else
    sdReady = false;
    Serial.println("[Audio] Real-time MAX98357A I2S harmonic audio synthesizer active.");
#endif

    // Start background FreeRTOS audio worker task on Core 0
    xTaskCreatePinnedToCore(
      audioWorkerTask,
      "audioWorker",
      4096,
      this,
      1,
      &audioTaskHandle,
      0
    );
  }

  bool isConnected() const { return i2sReady; }
  bool hasMicroSd() const { return sdReady; }

  void setVolumeIndex(uint8_t idx) {
    if (idx > 5) idx = 3;
    volumeIndex = idx;
    Serial.printf("[Audio] Volume set to level %d (%d%%)\n", volumeIndex, (int)(VOLUME_FACTORS[volumeIndex] * 100.0f));
  }

  uint8_t getVolumeIndex() const { return volumeIndex; }

  void play(SoundTrack track, uint32_t durationMs = 0) {
    if (!i2sReady || volumeIndex == 0) return;

    if (durationMs == 0) {
      if (track == TRACK_GOAL_HORN) durationMs = 12000;
      else if (track == TRACK_TEST_HORN) durationMs = 3500;
      else if (track == TRACK_PERIOD_HORN) durationMs = 3000;
      else if (track == TRACK_WIN_FANFARE) durationMs = 15000;
      else durationMs = 4000;
    }

    currentTrack = track;
    trackDurationMs = durationMs;
    trackStartTime = millis();
    isPlaying = true;
    Serial.printf("[Audio] Playing track %d for %u ms via MAX98357A\n", (int)track, (unsigned int)durationMs);
  }

  // Instant acoustic chirp for live volume slider feedback (macOS / Windows style cue)
  void playChirp() {
    if (!i2sReady || volumeIndex == 0) return;

    // Generate short, high-fidelity ascending 60ms chirp (1200 Hz to 2400 Hz)
    const size_t CHIRP_SAMPLES = 1320; // ~60ms at 22050 Hz
    int16_t chirpBuf[CHIRP_SAMPLES * 2];
    float phase = 0.0f;
    float vol = VOLUME_FACTORS[volumeIndex];
    const float twoPi = 6.283185307f;

    for (size_t i = 0; i < CHIRP_SAMPLES; i++) {
      float progress = (float)i / CHIRP_SAMPLES;
      float freq = 1200.0f + (progress * 1200.0f);
      // Bell envelope
      float env = sinf(progress * 3.14159f);
      int16_t s = (int16_t)(sinf(phase) * env * vol * 22000.0f);
      chirpBuf[i * 2]     = s;
      chirpBuf[i * 2 + 1] = s;

      phase += (twoPi * freq) / 22050.0f;
      if (phase >= twoPi) phase -= twoPi;
    }

    i2s.write((const uint8_t*)chirpBuf, sizeof(chirpBuf));
  }

  void stop() {
    isPlaying = false;
    currentTrack = TRACK_NONE;

    // Send brief silence frame to ensure clean DMA cutoff
    if (i2sReady) {
      int16_t zeroBuf[64 * 2] = {0};
      i2s.write((const uint8_t*)zeroBuf, sizeof(zeroBuf));
    }
  }
};
