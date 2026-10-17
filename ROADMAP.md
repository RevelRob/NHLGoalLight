# Project Roadmap & Future Enhancements

This document outlines the phased development roadmap for **Rob's NHL Goal Light**, detailing immediate milestones, the planned round display integration, and smart home connectivity.

---

## 🗺️ Roadmap Overview

```mermaid
timeline
    title Project Evolution
    Phase 1 : Core Goal Light : Web Config Portal : Adaptive NHL API Polling : WS2812B NeoPixel Beacon : DFPlayer Audio Horn
    Phase 2 : Multi-Event Engine : Power Play / Situation Code Alerts : Period Horns : Victory Celebration : Night Mode
    Phase 3 : Round LCD Display (GC9A01) : Team Logos & Matchup Screen : Live Game Clock : Scorer & Assist Graphics
    Phase 4 : Smart Home Ecosystem : Home Assistant MQTT Integration : Sync with Living Room Hue Lights : Mobile Push Alerts
```

---

## 📍 Phase 1: Core Goal Light & Web Portal (Current Milestone)

- [x] Architectural design, hardware specifications, and API documentation.
- [ ] ESP32 core firmware setup (Arduino / PlatformIO).
- [ ] Wi-Fi Manager with captive portal fallback (`NHL-Goal-Light-Setup` AP mode).
- [ ] Embedded web configuration portal (`http://goallight.local`):
  - 32-team selector with automated team colors.
  - Brightness and volume sliders.
  - Effect style selector (Beacon, Arena Strobe, Team Breath).
  - Web-based test buttons (Test Horn & Test Lights).
- [ ] Adaptive NHL API polling client (`/club-schedule` & `/boxscore`).
- [ ] Non-blocking WS2812B animation engine.
- [ ] DFRobot DFPlayer Pro (DF1201S) hardware serial audio driver.
- [ ] Physical button debouncer (Single press = Volume cycle, Double press = Test goal horn, Long press = Force AP mode).

---

## 📍 Phase 2: Enhanced Game Intelligence & Multi-Event Audio

- [ ] **Remote GitHub Asset Sync Engine**: Connects to GitHub asset repository to dynamically download team-specific goal horns (`.mp3`) and 240×240 circular logos (`.bin`) into `LittleFS` on team change. See [REMOTE_ASSETS_SYNC.md](file:///Users/robtesta/Documents/Arduino/NHL%20Goal%20Light%2010.2026/Rob%20NHL%20Goal%20Light/REMOTE_ASSETS_SYNC.md).
- [ ] **Power Play Detection**: Parse NHL API 4-digit `situationCode` (e.g. `1451`) to trigger amber power-play strobes.
- [ ] **Period Horns & Intermission Mode**: Distinct audio buzzers on period start/end; calm cyan breathing effect during intermissions.
- [ ] **Victory Fanfare**: Detect `FINAL`/`OFF` state; if preferred team won, trigger an extended 30-second light celebration with arena victory music.
- [ ] **Disallowed Goal Protection**: Handle score rollbacks silently if a goal is overturned upon video review.
- [ ] **Do Not Disturb / Night Mode**: Automated scheduled dimming or silent mode for late West Coast games.
- [ ] **Multi-Team Support**: Option to track a secondary team with distinct color alerts.

---

## 📍 Phase 3: Round LCD Display Integration (User Feature Request) 📺

Integration of a circular IPS LCD flush-mounted on the front face of the 3D-printed base (like a stadium scoreboard).

### Hardware Candidates
1. **Waveshare 1.28-inch Round LCD (GC9A01)**:
   - Resolution: 240×240 pixels, 65K RGB colors.
   - Interface: 4-wire SPI + DMA.
   - Form factor: Compact circular module flush-mounted on the base front face.
2. **All-In-One ESP32-S3 Round Display Modules** (e.g. Waveshare ESP32-S3-Touch-LCD-1.28).

### Graphics Architecture
- **Framework**: LVGL 9 (Light and Versatile Graphics Library) or `TFT_eSPI` with SPI DMA rendering.
- **Dynamic Assets**: Stream 16-bit RGB565 raw team logos (`logo_240.bin`) directly from `LittleFS` into the display buffer.

### Planned Display Screens:

| Screen Mode | Visual Contents | Trigger Condition |
| :--- | :--- | :--- |
| **Idle / Off-Day** | Team Logo + Digital Clock + Next Game Countdown ("vs BOS - Tomorrow 7:00 PM") | No game currently active |
| **Pregame Warmup** | Matchup card (Away Logo vs Home Logo), Arena name, Puck drop timer | Within 60 min of start |
| **Live In-Game** | Live scoreboard: Team logos, current score (`NYR 3 - 1 BOS`), period indicator (`P2`), and game clock countdown (`08:42`) | Game `LIVE` |
| **Goal Celebration!** | Flashing "🚨 GOAL! 🚨" banner with animated spinning puck, followed by player headshot, name, and season goal count | Preferred team scores |
| **Power Play** | Glowing "POWER PLAY" banner with 2:00 countdown clock | Man advantage awarded |
| **Intermission** | Intermission countdown timer + Shots On Goal (SOG) comparison | Between periods |
| **Final / Win** | "FINAL - RANGERS WIN!" celebratory screen with animated stars | Game ends in victory |

---

## 📍 Phase 4: Smart Home & Browser Web Flasher

- [ ] **Zero-Code Web Flasher (GitHub Pages)**:
  - 1-click browser flasher using **ESP Web Tools** via Web Serial API.
  - Flash firmware and configure Wi-Fi directly in Chrome/Edge without installing Arduino IDE or PlatformIO. See [WEB_FLASHER_GUIDE.md](file:///Users/robtesta/Documents/Arduino/NHL%20Goal%20Light%2010.2026/Rob%20NHL%20Goal%20Light/WEB_FLASHER_GUIDE.md).
- [ ] **Home Assistant Integration**:
  - Auto-discovery via MQTT (`homeassistant/light/goal_light/config`).
  - Native Home Assistant entities:
    - `light.goal_light`: Power, brightness, color, and effect controls.
    - `sensor.nhl_game_state`: Current score, period, clock, and opponent.
    - `button.test_goal_horn`: Trigger physical horn remotely.
    - `select.preferred_team`: Change team from Home Assistant dashboards.
- [ ] **Whole-House Goal Sync**: Broadcast MQTT or webhook events so room lights (Philips Hue, LIFX, WLED) flash red in sync with the goal light.
