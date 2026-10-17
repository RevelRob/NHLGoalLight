#if __has_include(<DFRobot_DF1201S.h>)
  #include <DFRobot_DF1201S.h>
  #define HAS_DFPLAYER_LIB 1
#else
  #define HAS_DFPLAYER_LIB 0
#endif

#include "Config.h"

// 6 volume presets (0 = Mute, 5 = Max 30)
const int VOLUME_LEVELS[6] = {0, 6, 12, 18, 24, 30};

enum SoundTrack {
  TRACK_GOAL_HORN    = 1,
  TRACK_GAME_START   = 2,
  TRACK_PERIOD_HORN  = 3,
  TRACK_WIN_FANFARE  = 4,
  TRACK_TEST_HORN    = 5
};

class AudioEngine {
private:
  HardwareSerial dfSerial;
#if HAS_DFPLAYER_LIB
  DFRobot_DF1201S dfplayer;
#endif
  bool connected;
  uint8_t volumeIndex;

public:
  AudioEngine() : dfSerial(1), connected(false), volumeIndex(DEFAULT_VOLUME_IDX) {}

  void begin() {
#if HAS_DFPLAYER_LIB
    Serial.println("[Audio] Initializing DFPlayer Pro UART...");
    
    // Begin hardware serial with specified pins
    dfSerial.begin(115200, SERIAL_8N1, PIN_DFPLAYER_RX, PIN_DFPLAYER_TX);
    delay(500);

    if (!dfplayer.begin(dfSerial)) {
      Serial.println("[Audio] DFPlayer Pro not detected. Audio will be disabled.");
      connected = false;
      return;
    }

    connected = true;
    Serial.println("[Audio] DFPlayer Pro connected successfully.");
    dfplayer.switchFunction(dfplayer.MUSIC);
    dfplayer.setPlayMode(dfplayer.SINGLE);
    setVolumeIndex(volumeIndex);
#else
    Serial.println("[Audio] DFRobot_DF1201S library not installed. Running in silent/light-only mode.");
    connected = false;
#endif
  }

  bool isConnected() const { return connected; }

  void setVolumeIndex(uint8_t idx) {
    if (idx > 5) idx = 3;
    volumeIndex = idx;
#if HAS_DFPLAYER_LIB
    if (connected) {
      int vol = VOLUME_LEVELS[volumeIndex];
      dfplayer.setVol(vol);
      Serial.print("[Audio] Volume set to: ");
      Serial.println(vol);
    }
#endif
  }

  uint8_t getVolumeIndex() const { return volumeIndex; }

  void play(SoundTrack track) {
#if HAS_DFPLAYER_LIB
    if (!connected) return;
    int vol = VOLUME_LEVELS[volumeIndex];
    if (vol == 0) return; // Muted

    Serial.print("[Audio] Playing track: ");
    Serial.println(track);
    dfplayer.playFileNum(track);
#endif
  }

  void stop() {
#if HAS_DFPLAYER_LIB
    if (connected) {
      dfplayer.pause();
    }
#endif
  }
};
