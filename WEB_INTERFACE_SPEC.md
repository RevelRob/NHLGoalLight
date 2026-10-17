# Web Interface Specification & REST API

The ESP32 hosts a built-in web configuration portal accessible from any phone, tablet, or desktop browser on the local network (via `http://goallight.local` or direct IP). When no Wi-Fi credentials are saved, it acts as a SoftAP captive portal.

---

## 1. Network Modes & Discovery

### A. Setup / Access Point Mode (SoftAP)
- **SSID**: `NHL-Goal-Light-Setup` (Open or default password `goallight`)
- **IP Address**: `192.168.4.1`
- **Captive Portal**: Captive DNS redirect intercepts `captive.apple.com`, `connectivitycheck.gstatic.com`, etc., automatically presenting the setup page on smartphones.

### B. Normal Station Mode (STA)
- **Local IP**: Assigned via DHCP by home router.
- **mDNS Hostname**: `http://goallight.local/` (Resolves across Apple Bonjour, Windows 10/11, and Android mDNS).

---

## 2. UI/UX Design System

The web interface uses a responsive, mobile-first design styled with a modern dark arena aesthetic:
- **Background**: Sleek deep obsidian/charcoal (`#0e1117`) with subtle glassmorphism cards (`rgba(255, 255, 255, 0.05)` with `backdrop-filter: blur(12px)`).
- **Typography**: Clean system font stack (`-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif`).
- **Accent Lighting**: Dynamic neon glow reflecting the selected team's primary color.
- **Zero External Dependencies**: All CSS and JavaScript are self-contained in a single embedded, gzipped asset—no external CDNs needed. Works 100% offline in AP mode.

---

## 3. UI Wireframe & Component Layout

```
┌────────────────────────────────────────────────────────┐
│  🚨 ROB'S NHL GOAL LIGHT                 [● LIVE: 3-1] │
├────────────────────────────────────────────────────────┤
│                                                        │
│  ⭐ FAVORITE TEAM                                      │
│  ┌──────────────────────────────────────────────────┐  │
│  │ 🗽 New York Rangers (NYR)                     ▼  │  │
│  └──────────────────────────────────────────────────┘  │
│  Primary: [ ■ #0038A8 ]   Secondary: [ ■ #CE1126 ]     │
│                                                        │
│  💡 LIGHTING SETTINGS                                  │
│  Brightness: [═════════════════════════●════] 80%      │
│  Effect Style: (●) Rotating Beacon  ( ) Arena Strobe  │
│  Celebration Duration: [ 12 sec ]                      │
│                                                        │
│  🔊 SOUND SETTINGS                                     │
│  Volume Level: [═════════════════●══════════] 65%      │
│  [🔊 Test Goal Horn]    [💡 Test Light Only]           │
│                                                        │
│  ⚙️ EVENT NOTIFICATIONS                                │
│  ☑ Goal Horn & Lights                                  │
│  ☑ Alert on Opponent Goals                             │
│  ☑ Power Play Indicator                                │
│  ☑ Period Start & End Buzzer                           │
│  ☑ Victory Fanfare on Game Win                         │
│                                                        │
│  📶 WI-FI & SYSTEM                                     │
│  Network: Home_WiFi_5G (RSSI: -58 dBm)                 │
│  Free Heap: 198 KB   |   Uptime: 2d 14h 22m            │
│  [ Save Settings ]              [ Restart Device ]     │
└────────────────────────────────────────────────────────┘
```

---

## 4. Complete 32-Team Color & Abbreviation Database

The firmware includes a pre-indexed table of all 32 NHL teams with their official primary and secondary hex colors to automatically drive the NeoPixel animations:

| Abbreviation | Team Name | Primary Hex | Secondary Hex |
| :---: | :--- | :---: | :---: |
| **ANA** | Anaheim Ducks | `#F47A38` | `#B9975B` |
| **BOS** | Boston Bruins | `#FFB81C` | `#000000` |
| **BUF** | Buffalo Sabres | `#002654` | `#FCB514` |
| **CGY** | Calgary Flames | `#C8102E` | `#F1BE48` |
| **CAR** | Carolina Hurricanes | `#CC0000` | `#000000` |
| **CHI** | Chicago Blackhawks | `#CF0A2C` | `#000000` |
| **COL** | Colorado Avalanche | `#6F263D` | `#236192` |
| **CBJ** | Columbus Blue Jackets | `#002654` | `#CE1126` |
| **DAL** | Dallas Stars | `#006847` | `#8F8F8C` |
| **DET** | Detroit Red Wings | `#CE1126` | `#FFFFFF` |
| **EDM** | Edmonton Oilers | `#041E42` | `#FF4C00` |
| **FLA** | Florida Panthers | `#041E42` | `#C8102E` |
| **LAK** | Los Angeles Kings | `#111111` | `#A2AAAD` |
| **MIN** | Minnesota Wild | `#154734` | `#A6192E` |
| **MTL** | Montréal Canadiens | `#AF1E2D` | `#192168` |
| **NSH** | Nashville Predators | `#FFB81C` | `#041E42` |
| **NJD** | New Jersey Devils | `#CE1126` | `#000000` |
| **NYI** | New York Islanders | `#00539B` | `#F47920` |
| **NYR** | New York Rangers | `#0038A8` | `#CE1126` |
| **OTT** | Ottawa Senators | `#C8102E` | `#D69F3D` |
| **PHI** | Philadelphia Flyers | `#F74902` | `#000000` |
| **PIT** | Pittsburgh Penguins | `#000000` | `#FCB514` |
| **SJS** | San Jose Sharks | `#006D75` | `#EA7200` |
| **SEA** | Seattle Kraken | `#001628` | `#99D9D9` |
| **STL** | St. Louis Blues | `#002F87` | `#FCB514` |
| **TBL** | Tampa Bay Lightning | `#002868` | `#FFFFFF` |
| **TOR** | Toronto Maple Leafs | `#00205B` | `#FFFFFF` |
| **UTA** | Utah Hockey Club | `#71AFE5` | `#090909` |
| **VAN** | Vancouver Canucks | `#00205B` | `#00843D` |
| **VGK** | Vegas Golden Knights | `#B4975A` | `#333F48` |
| **WSH** | Washington Capitals | `#041E42` | `#C8102E` |
| **WPG** | Winnipeg Jets | `#041E42` | `#004C97` |

---

## 5. REST API Endpoints

The onboard web server exposes clean JSON endpoints:

### `GET /api/status`
Returns live system metrics and current game state.
```json
{
  "system": {
    "uptimeSeconds": 14205,
    "freeHeap": 198420,
    "wifiRSSI": -56,
    "ssid": "HomeNetwork"
  },
  "game": {
    "monitoredTeam": "NYR",
    "hasGameToday": true,
    "gameState": "LIVE",
    "homeTeam": "NYR",
    "homeScore": 3,
    "awayTeam": "BOS",
    "awayScore": 1,
    "period": 2,
    "timeRemaining": "08:14",
    "inIntermission": false
  }
}
```

### `GET /api/config`
Retrieves persistent device parameters stored in NVS.

### `POST /api/config`
Saves updated parameters to NVS.
```json
{
  "team": "NYR",
  "brightness": 200,
  "volume": 22,
  "hornDuration": 12,
  "effectStyle": 0,
  "alertOpponent": false,
  "alertPowerPlay": true
}
```

### `POST /api/test`
Triggers immediate test execution of light and/or horn.
```json
{
  "action": "test_goal",
  "audio": true,
  "lights": true
}
```

### `GET /api/scan`
Triggers an asynchronous Wi-Fi scan and returns available SSIDs with signal strengths.

### `POST /api/reboot`
Performs a graceful software restart (`ESP.restart()`).
