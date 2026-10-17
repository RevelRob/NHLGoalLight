# Remote Asset Sync & GitHub Asset Repository

This specification defines the architecture for hosting audio files and graphics in a GitHub repository, enabling the **Rob NHL Goal Light** to dynamically fetch, update, and cache team-specific assets over-the-air (OTA) via Wi-Fi.

---

## 1. System Overview

```mermaid
flowchart TD
    subgraph GitHub ["GitHub Asset Repository (Public/CDN)"]
        Manifest["manifest.json (Version & Checksums)"]
        CommonAudio["common/sounds/ (Siren, Buzzer, Win)"]
        CommonGraphics["common/graphics/ (NHL Logo, Puck)"]
        TeamPacks["teams/{ABBREV}/ (Horn MP3, 240x240 Logos)"]
    end

    subgraph ESP32 ["ESP32-S3 Super Mini (Firmware)"]
        WebUI["Web Configuration Portal"]
        SyncMgr["Asset Sync Engine (HTTPS Client)"]
        LittleFS["Onboard Flash Storage (/littlefs)"]
        AudioDrv["Audio Player (DFPlayer Pro / LittleFS)"]
        DisplayDrv["Round Display Driver (GC9A01 240x240)"]
    end

    WebUI -->|User Selects Team (e.g. BOS)| SyncMgr
    SyncMgr -->|1. Fetch Manifest| Manifest
    SyncMgr -->|2. Download Team Audio & Graphics| TeamPacks
    SyncMgr -->|3. Save to Flash| LittleFS
    LittleFS --> AudioDrv
    LittleFS --> DisplayDrv
```

When a user switches their preferred team in the web configuration portal (e.g. from `NYR` to `BOS`), the ESP32:
1. Connects to the GitHub repository over HTTPS.
2. Checks `manifest.json` for the selected team's available assets.
3. Downloads the team's custom goal horn (`goal_horn.mp3`) and 240×240 circular graphic (`logo_240.bin`) directly into onboard `LittleFS` flash storage.
4. Updates local configuration without requiring any USB flashing or manual SD card swapping!

---

## 2. GitHub Repository Structure

The asset repository should be structured as follows:

```text
nhl-goal-light-assets/
├── manifest.json                     <-- Master index of assets and versions
├── common/
│   ├── sounds/
│   │   ├── game_start.mp3            <-- Air-raid siren / buzzer (Track 2)
│   │   ├── period_end.mp3            <-- Period buzzer (Track 3)
│   │   ├── win_fanfare.mp3           <-- Arena victory song (Track 4)
│   │   └── test_horn.mp3             <-- Test horn sample (Track 5)
│   └── graphics/
│       ├── nhl_shield_240.bin        <-- 240x240 RGB565 raw image
│       └── puck_240.bin              <-- Spinning puck animation frame
└── teams/
    ├── ANA/
    │   ├── sounds/
    │   │   └── goal_horn.mp3         <-- Anaheim Ducks authentic horn + song
    │   └── graphics/
    │       ├── logo_240.bin          <-- 240x240 circular primary logo
    │       └── banner_240.bin        <-- Alternate goal celebration splash
    ├── BOS/
    │   ├── sounds/
    │   │   └── goal_horn.mp3
    │   └── graphics/
    │       └── logo_240.bin
    ├── NYR/
    │   ├── sounds/
    │   │   └── goal_horn.mp3
    │   └── graphics/
    │       └── logo_240.bin
    └── ... (All 32 NHL Teams)
```

---

## 3. Manifest Specification (`manifest.json`)

The `manifest.json` file resides in the root of the assets repository and serves as the single source of truth for file availability, URLs, sizes, and MD5 hashes:

```json
{
  "version": "1.0.0",
  "baseUrl": "https://raw.githubusercontent.com/username/nhl-goal-light-assets/main",
  "common": {
    "sounds": {
      "game_start": { "path": "/common/sounds/game_start.mp3", "size": 94200, "md5": "a1b2c3..." },
      "period_end": { "path": "/common/sounds/period_end.mp3", "size": 65400, "md5": "d4e5f6..." },
      "win_fanfare": { "path": "/common/sounds/win_fanfare.mp3", "size": 184500, "md5": "789abc..." }
    },
    "graphics": {
      "nhl_shield": { "path": "/common/graphics/nhl_shield_240.bin", "size": 115200, "md5": "def012..." }
    }
  },
  "teams": {
    "NYR": {
      "name": "New York Rangers",
      "version": "1.0.2",
      "horn": { "path": "/teams/NYR/sounds/goal_horn.mp3", "size": 210400, "md5": "c4ca42..." },
      "logo": { "path": "/teams/NYR/graphics/logo_240.bin", "size": 115200, "md5": "eccbc8..." }
    },
    "BOS": {
      "name": "Boston Bruins",
      "version": "1.0.1",
      "horn": { "path": "/teams/BOS/sounds/goal_horn.mp3", "size": 195200, "md5": "c81e72..." },
      "logo": { "path": "/teams/BOS/graphics/logo_240.bin", "size": 115200, "md5": "e4da3b..." }
    }
  }
}
```

---

## 4. Asset File Standards

### A. Audio Files (`.mp3`)
- **Format**: MPEG Audio Layer 3 (MP3).
- **Sampling Rate**: 44.1 kHz or 22.05 kHz.
- **Bitrate**: 96 kbps to 128 kbps constant bitrate (CBR).
- **Channels**: Mono or Stereo (Mono halves file size and is indistinguishable through a 3W speaker).
- **Duration**: 10 to 18 seconds (horn blast followed by goal celebration song).
- **Target Size**: ~120 KB – 220 KB per clip.

### B. Display Graphics (`.bin` / RGB565)
- **Dimensions**: Exactly 240 × 240 pixels (matching the GC9A01 round IPS display).
- **Format**: Uncompressed 16-bit RGB565 raw pixel array:
  $$\text{File Size} = 240 \times 240 \times 2 \text{ bytes} = 115{,}200 \text{ bytes (112.5 KB)}$$
- **Benefit of Raw RGB565**: Zero CPU overhead to decode—the ESP32 streams the bytes straight into the display's SPI DMA buffer in ~15 ms!
- **Optional Compressed Format**: PNG (requires `PNGdec` library, reducing file size to ~15–35 KB at the expense of slight decode RAM).

---

## 5. Storage & Flash Allocation (ESP32-S3 Super Mini)

The ESP32-S3 Super Mini typically features 4MB or 8MB of SPI Flash memory. We partition it to allocate dedicated storage for dynamic assets:

### Recommended Partition Scheme (4MB Flash):
```text
# Name,      Type, SubType, Offset,   Size,     Flags
nvs,         data, nvs,     0x9000,   0x5000,
otadata,     data, ota,     0xe000,   0x2000,
app0,        app,  ota_0,   0x10000,  0x180000,  # 1.5 MB Firmware
app1,        app,  ota_1,   0x190000, 0x180000,  # 1.5 MB OTA Update slot
spiffs,      data, spiffs,  0x310000, 0xF0000,   # ~960 KB LittleFS Asset Storage
```

### Storage Budget on LittleFS:
- Current Team Goal Horn (`/team_horn.mp3`): ~180 KB
- Current Team Round Logo (`/team_logo.bin`): ~115 KB
- Common Audio Sounds (`/siren.mp3`, `/buzzer.mp3`): ~150 KB
- **Total In-Use Flash**: **~445 KB** (Leaves > 500 KB free for future graphics).

> [!TIP]
> When switching teams, the firmware simply **overwrites** `/team_horn.mp3` and `/team_logo.bin`. The device only needs to store the *currently active team* in flash rather than all 32 teams simultaneously!

---

## 6. Audio Playback: LittleFS vs DFPlayer Pro

There are two primary ways to play the downloaded MP3 from storage:

1. **Dual Audio Architecture (Direct ESP32-S3 I2S Playback)**:
   - The ESP32-S3 has high processing power and can decode MP3 directly from `LittleFS` in software (using the `ESP32-audioI2S` library) and output to a MAX98357A I2S amplifier.
   - *Advantage*: The downloaded file plays immediately from flash with zero file transfer to another chip.

2. **DFPlayer Pro + LittleFS Hybrid**:
   - The DFPlayer Pro holds default factory sounds on its 128MB flash.
   - For custom downloaded team horns stored on the ESP32's `LittleFS`, the ESP32 can either stream audio over I2S, or the DFPlayer Pro can be updated via its USB-C port during initial build.

---

## 7. Web Configuration Portal Integration

In the web interface:
1. When the user picks a new team in the dropdown, an **"Update Team Assets"** button becomes active.
2. The user clicks **"Download Assets from GitHub"**.
3. The web page displays a live progress bar:
   - `[1/2] Downloading goal_horn.mp3 (192 KB)... 100%`
   - `[2/2] Downloading logo_240.bin (115 KB)... 100%`
   - `Sync Complete! Assets saved to device flash.`
4. The user can immediately click **"Test Horn"** or view the new logo on the round display!
