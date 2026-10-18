#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "Config.h"
#include "Teams.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
  <title>NHL Goal Light</title>

  <!-- iOS Safari Add to Home Screen (Standalone App) -->
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <meta name="apple-mobile-web-app-title" content="NHL Goal Light">
  <link rel="apple-touch-icon" href="/apple-touch-icon.png">
  <link rel="apple-touch-icon-precomposed" href="/apple-touch-icon-precomposed.png">
  <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png">

  <!-- Android Chrome / PWA Standalone -->
  <meta name="mobile-web-app-capable" content="yes">
  <meta name="application-name" content="NHL Goal Light">
  <meta name="theme-color" content="#0d1117">
  <link rel="manifest" href="/manifest.json">
  <link rel="icon" type="image/png" sizes="192x192" href="/icon.png">
  <link rel="icon" type="image/png" sizes="32x32" href="/icon.png">
  <link rel="shortcut icon" href="/icon.png">

  <style>
    :root {
      --bg: #0d1117;
      --card-bg: rgba(22, 27, 34, 0.88);
      --card-inner: #161b22;
      --border: rgba(255, 255, 255, 0.12);
      --text: #f0f6fc;
      --text-muted: #8b949e;
      --accent: #e63946;
      --accent-glow: rgba(230, 57, 70, 0.35);
      --btn-bg: #21262d;
      --btn-hover: #30363d;
      --success: #2ea043;
      --gold: #d4a373;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
      display: flex;
      justify-content: center;
      padding: max(12px, env(safe-area-inset-top)) max(12px, env(safe-area-inset-right)) max(84px, env(safe-area-inset-bottom)) max(12px, env(safe-area-inset-left));
      min-height: 100vh;
      -webkit-tap-highlight-color: transparent;
      user-select: none;
    }
    .container {
      width: 100%;
      max-width: 520px;
      display: flex;
      flex-direction: column;
      gap: 14px;
    }

    /* Header */
    .header {
      text-align: center;
      padding: 6px 0 2px 0;
    }
    .header h1 {
      font-size: 1.5rem;
      font-weight: 800;
      letter-spacing: -0.5px;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
    }
    .badge {
      display: inline-block;
      font-size: 0.75rem;
      font-weight: 600;
      padding: 4px 10px;
      border-radius: 12px;
      background: var(--border);
      color: var(--text-muted);
      margin-top: 6px;
      transition: all 0.3s ease;
    }
    .badge.live { background: #e63946; color: #fff; box-shadow: 0 0 12px var(--accent-glow); }

    /* Navigation Bar */
    .nav-bar {
      display: flex;
      background: var(--card-inner);
      border: 1px solid var(--border);
      border-radius: 14px;
      padding: 4px;
      gap: 4px;
      box-shadow: 0 4px 16px rgba(0,0,0,0.3);
      position: sticky;
      top: max(8px, env(safe-area-inset-top));
      z-index: 100;
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
    }
    .nav-tab {
      flex: 1;
      padding: 9px 4px;
      font-size: 0.78rem;
      font-weight: 700;
      text-align: center;
      background: transparent;
      border: none;
      color: var(--text-muted);
      border-radius: 10px;
      cursor: pointer;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 3px;
      transition: all 0.2s ease;
      touch-action: manipulation;
      -webkit-user-select: none;
      user-select: none;
    }
    .nav-tab .tab-icon { font-size: 1.05rem; pointer-events: none; }
    .nav-tab.active {
      background: var(--btn-bg);
      color: var(--text);
      box-shadow: 0 2px 8px rgba(0,0,0,0.4);
      border: 1px solid rgba(255,255,255,0.08);
    }

    .tab-page {
      display: none;
      flex-direction: column;
      gap: 14px;
    }
    .tab-page.active {
      display: flex;
    }
    /* Touch ghost-click shield to prevent touchscreen pass-through during tab transitions */
    .tab-page.just-activated * {
      pointer-events: none !important;
    }

    /* Cards */
    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 18px;
      backdrop-filter: blur(10px);
      -webkit-backdrop-filter: blur(10px);
      box-shadow: 0 8px 24px rgba(0,0,0,0.3);
    }
    .card h2 {
      font-size: 0.92rem;
      font-weight: 700;
      margin-bottom: 14px;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.8px;
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .form-group {
      margin-bottom: 14px;
    }
    .form-group:last-child { margin-bottom: 0; }
    label {
      display: flex;
      justify-content: space-between;
      font-size: 0.85rem;
      font-weight: 600;
      margin-bottom: 6px;
    }
    .helper-text {
      font-size: 0.76rem;
      color: var(--text-muted);
      margin-top: 5px;
      line-height: 1.35;
    }

    select, input[type="text"], input[type="password"], input[type="number"] {
      width: 100%;
      background: #11141a;
      border: 1px solid var(--border);
      color: var(--text);
      padding: 10px 12px;
      border-radius: 8px;
      font-size: 0.92rem;
      outline: none;
      transition: border-color 0.2s;
    }
    select:focus, input:focus { border-color: var(--accent); }

    .slider-container {
      display: flex;
      align-items: center;
      gap: 12px;
    }
    input[type="range"] {
      flex: 1;
      accent-color: var(--accent);
      cursor: pointer;
      height: 6px;
    }

    .btn-row {
      display: flex;
      gap: 10px;
    }
    button {
      flex: 1;
      padding: 12px;
      border-radius: 8px;
      border: 1px solid var(--border);
      background: var(--btn-bg);
      color: var(--text);
      font-size: 0.88rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s ease;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 6px;
    }
    button:hover { background: var(--btn-hover); }
    button.primary {
      background: var(--accent);
      border-color: transparent;
      color: #fff;
      font-weight: 700;
    }
    button.primary:hover { opacity: 0.9; }

    /* Checkbox & Switch Rows */
    .checkbox-row {
      display: flex;
      align-items: flex-start;
      gap: 12px;
      margin-bottom: 12px;
      font-size: 0.88rem;
      cursor: pointer;
      padding: 8px 10px;
      background: rgba(255,255,255,0.03);
      border-radius: 8px;
      border: 1px solid rgba(255,255,255,0.04);
      transition: background 0.2s;
    }
    .checkbox-row:last-child { margin-bottom: 0; }
    .checkbox-row:hover { background: rgba(255,255,255,0.06); }
    .checkbox-row input {
      accent-color: var(--accent);
      width: 18px;
      height: 18px;
      margin-top: 2px;
      flex: none;
    }
    .chk-text {
      display: flex;
      flex-direction: column;
      gap: 2px;
    }
    .chk-title { font-weight: 600; }
    .chk-desc { font-size: 0.75rem; color: var(--text-muted); line-height: 1.3; }

    /* Live Scoreboard Match Card */
    .score-card {
      display: flex;
      align-items: center;
      justify-content: space-around;
      background: linear-gradient(135deg, rgba(255,255,255,0.04), rgba(0,0,0,0.3));
      border: 1px solid var(--border);
      border-radius: 14px;
      padding: 16px 8px;
      margin-bottom: 8px;
      text-align: center;
    }
    .team-box {
      flex: 1;
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 4px;
    }
    .team-abbrev {
      font-size: 1.25rem;
      font-weight: 800;
      letter-spacing: 0.5px;
    }
    .team-score {
      font-size: 2.2rem;
      font-weight: 900;
      line-height: 1;
      color: var(--text);
    }
    .score-vs {
      font-size: 0.8rem;
      font-weight: 700;
      color: var(--text-muted);
      display: flex;
      flex-direction: column;
      align-items: center;
      gap: 4px;
    }
    .game-period-pill {
      font-size: 0.72rem;
      font-weight: 700;
      padding: 2px 8px;
      border-radius: 8px;
      background: var(--btn-bg);
      border: 1px solid var(--border);
    }

    /* Preset Pill Buttons */
    .preset-row {
      display: flex;
      gap: 6px;
      flex-wrap: wrap;
      margin-top: 8px;
    }
    .preset-btn {
      flex: 1;
      min-width: 90px;
      padding: 7px 10px;
      font-size: 0.76rem;
      background: #11141a;
      border: 1px solid var(--border);
      border-radius: 6px;
      cursor: pointer;
      text-align: center;
    }
    .preset-btn:hover { background: var(--btn-hover); }

    /* View Toggle Segmented Buttons */
    .view-toggle {
      display: inline-flex;
      background: var(--card-inner);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 2px;
      gap: 2px;
    }
    .view-toggle-btn {
      padding: 3px 8px;
      font-size: 0.72rem;
      font-weight: 700;
      background: transparent;
      border: none;
      color: var(--text-muted);
      border-radius: 6px;
      cursor: pointer;
      transition: all 0.2s;
    }
    .view-toggle-btn.active {
      background: var(--btn-hover);
      color: var(--text);
      box-shadow: 0 1px 4px rgba(0,0,0,0.4);
    }

    /* Virtual LED Preview */
    .ring-preview-container {
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      padding: 16px 0;
      background: #11141a;
      border-radius: 12px;
      border: 1px solid var(--border);
      margin-top: 10px;
    }
    #ringCanvas {
      width: 220px;
      height: 160px;
    }

    /* Fixed Bottom Save Bar */
    .fixed-save-bar {
      position: fixed;
      bottom: 0;
      left: 0;
      width: 100%;
      background: rgba(13, 17, 23, 0.94);
      border-top: 1px solid var(--border);
      padding: 12px 16px max(12px, env(safe-area-inset-bottom));
      display: flex;
      justify-content: center;
      z-index: 99;
      backdrop-filter: blur(12px);
      -webkit-backdrop-filter: blur(12px);
    }
    .fixed-save-bar .save-inner {
      width: 100%;
      max-width: 520px;
      display: flex;
      gap: 10px;
    }

    /* Hidden hardware elements */
    [data-hw-hidden="true"] {
      display: none !important;
    }

    /* Modal Overlay */
    .modal-overlay {
      display: none;
      position: fixed;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(13,17,23,0.95);
      z-index: 9999;
      backdrop-filter: blur(10px);
      -webkit-backdrop-filter: blur(10px);
      align-items: center;
      justify-content: center;
      padding: 20px;
      box-sizing: border-box;
    }
    .modal-content {
      background: #161b22;
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 24px;
      max-width: 400px;
      width: 100%;
      text-align: center;
      box-shadow: 0 12px 36px rgba(0,0,0,0.6);
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1 id="titleDeviceName">🚨 NHL Goal Light</h1>
      <div id="statusBadge" class="badge">Connecting...</div>
    </div>

    <!-- 5-Tab Navigation Bar -->
    <nav class="nav-bar">
      <button type="button" class="nav-tab active" data-tab="live">
        <span class="tab-icon">🚨</span>
        <span>Live</span>
      </button>
      <button type="button" class="nav-tab" data-tab="lights" data-hw="lights">
        <span class="tab-icon">💡</span>
        <span>Lights</span>
      </button>
      <button type="button" class="nav-tab" data-tab="events">
        <span class="tab-icon">⚡</span>
        <span>Events</span>
      </button>
      <button type="button" class="nav-tab" data-tab="sync">
        <span class="tab-icon">📺</span>
        <span>Sync</span>
      </button>
      <button type="button" class="nav-tab" data-tab="system">
        <span class="tab-icon">⚙️</span>
        <span>System</span>
      </button>
    </nav>

    <!-- ===================================================== -->
    <!-- TAB 1: LIVE (DASHBOARD & PRIMARY CONTROLS)            -->
    <!-- ===================================================== -->
    <div id="tab-live" class="tab-page active">
      
      <!-- Live Game Matchup Card -->
      <div class="card" id="cardLiveGame">
        <h2>🏒 Game Status</h2>
        <div class="score-card">
          <div class="team-box">
            <span class="team-abbrev" id="liveAwayTeam">---</span>
            <span class="team-score" id="liveAwayScore">-</span>
          </div>
          <div class="score-vs">
            <span id="liveGameClock" class="game-period-pill">No Live Game</span>
            <span id="liveIntermissionNote" style="font-size:0.7rem; color:var(--accent); font-weight:700; display:none;">INTERMISSION</span>
          </div>
          <div class="team-box">
            <span class="team-abbrev" id="liveHomeTeam">---</span>
            <span class="team-score" id="liveHomeScore">-</span>
          </div>
        </div>
        <div class="helper-text" id="liveStatusSubtext" style="text-align:center;">Synchronised with NHL live scores.</div>
      </div>

      <!-- Upcoming Game Card (Simple & Detailed Views) -->
      <div class="card" id="cardUpcomingGame">
        <h2>
          <span>📅 Upcoming Game</span>
          <div class="view-toggle">
            <button type="button" class="view-toggle-btn active" id="btnViewSimple" onclick="setUpcomingView('simple')">Simple</button>
            <button type="button" class="view-toggle-btn" id="btnViewDetailed" onclick="setUpcomingView('detailed')">Detailed</button>
          </div>
        </h2>

        <!-- Simple View -->
        <div id="upcomingSimpleView">
          <div class="score-card" style="padding:14px 10px; margin-bottom:8px;">
            <div class="team-box">
              <span class="team-abbrev" id="upSimpleAway">---</span>
              <span style="font-size:0.72rem; color:var(--text-muted);" id="upSimpleAwayLabel">Away</span>
            </div>
            <div class="score-vs">
              <span style="font-size:0.75rem; font-weight:800; color:var(--accent);" id="upSimpleMatchupPill">VS</span>
              <span style="font-size:0.88rem; font-weight:800; color:var(--text); margin-top:2px;" id="upSimpleTime">--:--</span>
              <span style="font-size:0.72rem; color:var(--text-muted);" id="upSimpleDate">Checking Schedule...</span>
            </div>
            <div class="team-box">
              <span class="team-abbrev" id="upSimpleHome">---</span>
              <span style="font-size:0.72rem; color:var(--text-muted);" id="upSimpleHomeLabel">Home</span>
            </div>
          </div>
          <div style="display:flex; justify-content:space-between; align-items:center; font-size:0.78rem; color:var(--text-muted);">
            <span id="upSimpleVenue">📍 Venue loading...</span>
            <span id="upSimpleBroadcasts" style="color:#58a6ff; font-weight:600;">📺 Broadcasts pending</span>
          </div>
        </div>

        <!-- Detailed View -->
        <div id="upcomingDetailedView" style="display:none;">
          <div style="background:#11141a; border-radius:12px; border:1px solid var(--border); padding:14px; margin-bottom:10px;">
            <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
              <span id="upDetMatchupTitle" style="font-size:1.05rem; font-weight:800; color:var(--text);">--- vs ---</span>
              <span id="upDetHomeAwayBadge" style="font-size:0.72rem; font-weight:700; padding:3px 8px; border-radius:6px; background:rgba(230,57,70,0.15); color:var(--accent); border:1px solid rgba(230,57,70,0.3);">Home Game</span>
            </div>
            <div style="display:grid; grid-template-columns:1fr 1fr; gap:12px; font-size:0.8rem;">
              <div>
                <div style="color:var(--text-muted); font-size:0.72rem; text-transform:uppercase;">📅 Puck Drop Date</div>
                <div id="upDetDate" style="font-weight:700; color:var(--text); margin-top:2px;">--</div>
              </div>
              <div>
                <div style="color:var(--text-muted); font-size:0.72rem; text-transform:uppercase;">⏰ Local Start Time</div>
                <div id="upDetTime" style="font-weight:700; color:var(--text); margin-top:2px;">--</div>
              </div>
              <div style="grid-column: span 2;">
                <div style="color:var(--text-muted); font-size:0.72rem; text-transform:uppercase;">🏟️ Arena & Location</div>
                <div id="upDetVenue" style="font-weight:600; color:var(--text); margin-top:2px;">--</div>
              </div>
              <div style="grid-column: span 2;">
                <div style="color:var(--text-muted); font-size:0.72rem; text-transform:uppercase;">📺 Television Broadcasts</div>
                <div id="upDetBroadcasts" style="font-weight:600; color:#58a6ff; margin-top:2px;">--</div>
              </div>
              <div style="grid-column: span 2;">
                <div style="color:var(--text-muted); font-size:0.72rem; text-transform:uppercase;">📻 Radio & Audio Coverage</div>
                <div id="upDetRadio" style="font-weight:600; color:var(--gold); margin-top:2px;">NHL GameCenter Live Audio</div>
              </div>
            </div>
          </div>
          <div style="text-align:center;">
            <a id="upDetNhlLink" href="https://www.nhl.com" target="_blank" style="font-size:0.76rem; color:#58a6ff; text-decoration:none;">View Matchup Preview on NHL.com →</a>
          </div>
        </div>
      </div>

      <!-- Favourite Team Selector -->
      <div class="card">
        <h2>⭐ Favourite Team</h2>
        <div class="form-group">
          <label for="teamSelect">Tracked NHL Franchise</label>
          <select id="teamSelect"></select>
          <div class="helper-text">Goal celebrations and ambient lighting automatically adapt to this team's official colours.</div>
        </div>
      </div>

      <!-- Quick Levels (Live Updating without Save) -->
      <div class="card">
        <h2>🎛️ Quick Levels</h2>
        
        <!-- Brightness Slider (Guarded by Lights Hardware) -->
        <div class="form-group" data-hw="lights">
          <label>Light Brightness: <span id="brightVal">70%</span></label>
          <div class="slider-container">
            <input type="range" id="brightSlider" min="10" max="255" value="180">
          </div>
        </div>

        <!-- Volume Slider (Guarded by Speaker Hardware) -->
        <div class="form-group" data-hw="speaker">
          <label>Speaker Volume: <span id="volVal">Level 3</span></label>
          <div class="slider-container">
            <input type="range" id="volSlider" min="0" max="5" value="3">
          </div>
        </div>

        <div class="helper-text" style="margin-top:10px;">⚡ Levels update immediately. Adjusting volume plays an instant acoustic sound chirp.</div>
      </div>
    </div>

    <!-- ===================================================== -->
    <!-- TAB 2: LIGHTS (LED GEOMETRY & CELEBRATION FX)         -->
    <!-- ===================================================== -->
    <div id="tab-lights" class="tab-page">
      
      <!-- LED Hardware & Geometry -->
      <div class="card">
        <h2>⭕ Hardware & Geometry</h2>
        <div class="form-group">
          <label for="ledPixels">Connected Pixel Count</label>
          <input type="number" id="ledPixels" min="4" max="300" value="16">
          <div class="helper-text">Number of addressable WS2812B RGB LEDs attached (e.g. 16 or 24 for NeoPixel rings).</div>
        </div>
        <div class="form-group">
          <label for="ledLayout">Arrangement Geometry</label>
          <select id="ledLayout">
            <option value="0">⭕ Single Ring (16/24 NeoPixel Ring)</option>
            <option value="1">🌪️ Spiral Wrap Strip (Cylindrical Beacon)</option>
            <option value="2">⏸️ Parallel Columns</option>
            <option value="3">📏 Linear Strip / Bar</option>
          </select>
        </div>
        <div class="form-group">
          <label class="checkbox-row" style="margin:0;">
            <input type="checkbox" id="chkLedReverse">
            <div class="chk-text">
              <span class="chk-title">Invert Rotation Direction</span>
              <span class="chk-desc">Reverses clockwise vs counter-clockwise beacon sweep.</span>
            </div>
          </label>
        </div>
      </div>

      <!-- Celebration Lighting Effects -->
      <div class="card">
        <h2>✨ Goal Celebration Effects</h2>
        <div class="form-group">
          <label for="effectSelect">Celebration Light Pattern</label>
          <select id="effectSelect">
            <option value="0">🚨 Rotating Halogen Beacon (Classic Revolving Reflector)</option>
            <option value="1">⚡ High-Speed Arena Strobe (Stadium Flash)</option>
            <option value="2">🌊 Team Colour Breathe (Pulsing Unison Glow)</option>
            <option value="3">🚔 Dual Siren Flasher (Emergency Wig-Wag)</option>
            <option value="4">❤️ Heartbeat Pulse (Urgent Goal Surge)</option>
          </select>
          <div class="helper-text">Each effect generates distinct, high-impact stadium celebrations tailored to your ring.</div>
        </div>
        <div class="form-group">
          <label>Celebration Duration: <span id="hornVal">12s</span></label>
          <div class="slider-container">
            <input type="range" id="hornSlider" min="5" max="30" value="12">
          </div>
          <div class="helper-text">Length of light celebration and goal horn playback per goal.</div>
        </div>

        <!-- Virtual LED Ring Live Preview -->
        <div class="form-group">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:6px;">
            <label style="margin:0;">Live LED Visualiser Preview</label>
            <div class="view-toggle">
              <button type="button" class="view-toggle-btn active" id="btnPrevCeleb" onclick="setVisualiserMode('celebration')">🚨 Celebration</button>
              <button type="button" class="view-toggle-btn" id="btnPrevIdle" onclick="setVisualiserMode('idle')">🌙 Idle Mode</button>
            </div>
          </div>
          <div class="ring-preview-container">
            <canvas id="ringCanvas" width="220" height="160"></canvas>
            <div id="previewCaption" style="font-size:0.75rem; color:var(--text-muted); margin-top:8px;">Live animation in team colour</div>
          </div>
        </div>
      </div>

      <!-- Idle Lighting Behaviour -->
      <div class="card">
        <h2>🌙 Idle Lighting Behaviour</h2>
        <div class="form-group">
          <label for="idleEffect">Between Games / Idle Mode</label>
          <select id="idleEffect">
            <option value="0">🌑 Dark / Off</option>
            <option value="1">🌬️ Team Colour Breathe (Subtle Ambient Glow)</option>
            <option value="2">💡 Solid Team Colour Accent</option>
            <option value="3">🔥 Candle / Ember Flicker</option>
            <option value="4">💫 Orbit Sweep (Gentle Comet)</option>
          </select>
          <div class="helper-text">Lighting style when no game event is actively firing.</div>
        </div>
      </div>
    </div>

    <!-- ===================================================== -->
    <!-- TAB 3: EVENTS (GAME EVENT ACTION MATRIX & SIMULATOR)  -->
    <!-- ===================================================== -->
    <div id="tab-events" class="tab-page">
      
      <!-- Event Simulator Studio -->
      <div class="card">
        <h2>🧪 Event Simulator Studio</h2>
        <div class="helper-text" style="margin-bottom:14px;">Test any game event or goal celebration in real time with custom output selection.</div>

        <div class="form-group">
          <label for="simEventSelect">Simulated Game Event</label>
          <select id="simEventSelect">
            <option value="goal">🚨 Favourite Team Goal</option>
            <option value="opponent">🥅 Opponent Goal Alert</option>
            <option value="powerplay">⚡ Power Play Awarded</option>
            <option value="buzzer">🚨 Period End & Start Buzzer</option>
            <option value="intermission">☕ Relaxed Intermission Glow</option>
            <option value="win">🏆 Game Victory Fanfare</option>
          </select>
        </div>

        <div class="form-group" id="simEffectGroup">
          <label for="simEffectSelect">Celebration Light Effect Override</label>
          <select id="simEffectSelect">
            <option value="0">🚨 Rotating Halogen Beacon</option>
            <option value="1">⚡ High-Speed Arena Strobe</option>
            <option value="2">🌊 Team Colour Breathe</option>
            <option value="3">🚔 Dual Siren Flasher</option>
            <option value="4">❤️ Heartbeat Pulse</option>
          </select>
        </div>

        <div class="form-group">
          <label style="margin-bottom:8px;">Target Hardware Outputs</label>
          <div style="display:flex; gap:10px; flex-wrap:wrap;">
            <label class="checkbox-row" style="flex:1; min-width:110px; margin:0;" data-hw="lights">
              <input type="checkbox" id="chkSimLights" checked>
              <div class="chk-text"><span class="chk-title">💡 Lights</span></div>
            </label>
            <label class="checkbox-row" style="flex:1; min-width:110px; margin:0;" data-hw="speaker">
              <input type="checkbox" id="chkSimHorn" checked>
              <div class="chk-text"><span class="chk-title">🔊 Sound</span></div>
            </label>
            <label class="checkbox-row" style="flex:1; min-width:110px; margin:0;" data-hw="display">
              <input type="checkbox" id="chkSimDisplay" checked>
              <div class="chk-text"><span class="chk-title">📺 Display</span></div>
            </label>
          </div>
        </div>

        <div class="btn-row" style="margin-top:14px;">
          <button type="button" id="btnRunSimTest" class="primary">▶️ Trigger Event Test</button>
          <button type="button" id="btnStopSimTest" style="background:rgba(230,57,70,0.15); border-color:var(--accent); color:#fff; flex:0.7;">⏹️ Stop / Idle</button>
        </div>
      </div>

      <!-- Event Trigger Matrix -->
      <div class="card">
        <h2>⚡ Event Trigger Matrix</h2>
        <div class="helper-text" style="margin-bottom:14px;">Select which game milestones trigger goal light alerts, sounds, and lighting effects during live games.</div>

        <label class="checkbox-row">
          <input type="checkbox" id="chkEventGoal" checked disabled>
          <div class="chk-text">
            <span class="chk-title">🚨 Favourite Team Goal</span>
            <span class="chk-desc">Full celebration fanfare, goal horn sound, and primary lighting effect.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkOpponent">
          <div class="chk-text">
            <span class="chk-title">🥅 Opponent Goal Alert</span>
            <span class="chk-desc">Subtle amber warning pulse when the opposing team scores.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkPowerPlay" checked>
          <div class="chk-text">
            <span class="chk-title">⚡ Power Play Awarded</span>
            <span class="chk-desc">Vibrant gold chase animation when your team gets the man advantage.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkBuzzer" checked>
          <div class="chk-text">
            <span class="chk-title">🚨 Period End & Start Buzzer</span>
            <span class="chk-desc">Sounds the arena horn at the conclusion of each period.</span>
          </div>
        </label>

        <label class="checkbox-row" data-hw="lights">
          <input type="checkbox" id="chkIntermission" checked>
          <div class="chk-text">
            <span class="chk-title">☕ Relaxed Intermission Glow</span>
            <span class="chk-desc">Transitions to calm ambient team colour lighting during the 18-minute break.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkWin" checked>
          <div class="chk-text">
            <span class="chk-title">🏆 Game Victory Fanfare</span>
            <span class="chk-desc">Extended victory horn and rainbow light celebration when your team wins!</span>
          </div>
        </label>
      </div>
    </div>

    <!-- ===================================================== -->
    <!-- TAB 4: SYNC (TV BROADCAST SYNCHRONISATION)            -->
    <!-- ===================================================== -->
    <div id="tab-sync" class="tab-page">
      
      <!-- TV Broadcast Delay Buffer -->
      <div class="card">
        <h2>⏱️ TV Broadcast Delay</h2>
        <div class="helper-text" style="margin-bottom:14px;">Hold celebrations in buffer so your goal light never triggers before the puck crosses the line on your TV feed!</div>
        
        <div class="form-group">
          <label>Broadcast Delay: <strong id="delayVal" style="color:var(--accent);">0s (Instantaneous)</strong></label>
          <div class="slider-container">
            <input type="range" id="delaySlider" min="0" max="120" value="0" step="1">
          </div>
        </div>

        <label style="margin-top:12px;">Quick Broadcaster Presets:</label>
        <div class="preset-row">
          <button type="button" class="preset-btn" onclick="setBroadcastPreset(0)">⚡ Cable / OTA (0s)</button>
          <button type="button" class="preset-btn" onclick="setBroadcastPreset(22)">🍁 Sportsnet+ (22s)</button>
          <button type="button" class="preset-btn" onclick="setBroadcastPreset(28)">🏒 NHL.tv / ESPN+ (28s)</button>
          <button type="button" class="preset-btn" onclick="setBroadcastPreset(45)">📺 YouTube TV (45s)</button>
        </div>
      </div>

      <!-- Live Game Clock Synchroniser -->
      <div class="card">
        <h2>📺 Game Clock Matcher</h2>
        <div class="helper-text" style="margin-bottom:12px;">Compare the live official NHL API game clock with your screen to dial in your synchronisation:</div>

        <div style="background:#11141a; border-radius:10px; border:1px solid var(--border); padding:14px; text-align:center; margin-bottom:12px;">
          <div style="font-size:0.75rem; color:var(--text-muted); text-transform:uppercase; letter-spacing:0.5px;">Official NHL Cloud Clock</div>
          <div id="syncNhlClock" style="font-size:1.6rem; font-weight:800; color:var(--text); margin:4px 0;">--:--</div>
          <div id="syncNhlPeriod" style="font-size:0.8rem; color:var(--text-muted);">Period: Not In Progress</div>
        </div>

        <div class="btn-row" style="margin-bottom:8px;">
          <button type="button" onclick="nudgeDelay(-5)">⏪ -5s</button>
          <button type="button" onclick="nudgeDelay(-1)">◀️ -1s</button>
          <button type="button" onclick="nudgeDelay(1)">▶️ +1s</button>
          <button type="button" onclick="nudgeDelay(5)">⏩ +5s</button>
        </div>

        <button type="button" id="btnSyncWhistle" style="width:100%; padding:11px; background:var(--btn-bg); border:1px solid var(--border); border-radius:8px; font-weight:600; cursor:pointer;">
          🎯 Sync on TV Whistle / Clock Match
        </button>
      </div>
    </div>

    <!-- ===================================================== -->
    <!-- TAB 5: SYSTEM (HARDWARE, WI-FI & DEVICE PREFS)        -->
    <!-- ===================================================== -->
    <div id="tab-system" class="tab-page">
      
      <!-- Wi-Fi Network Setup (Card 1: Top priority in AP mode) -->
      <div class="card" id="wifiCard">
        <h2>📶 Wi-Fi Settings</h2>
        <div class="form-group">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:6px;">
            <label style="margin:0;">Select Nearby Network</label>
            <button type="button" id="btnRescan" style="flex:none; padding:4px 10px; font-size:0.75rem; border-radius:6px; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); cursor:pointer;">🔄 Rescan</button>
          </div>
          <select id="wifiSelect">
            <option value="" disabled selected>🔍 Scanning for Wi-Fi networks...</option>
          </select>
        </div>

        <div class="form-group" id="manualSsidGroup">
          <label for="wifiSsid">Wi-Fi Network Name (SSID)</label>
          <input type="text" id="wifiSsid" placeholder="Choose above or enter SSID manually" autocomplete="off" autocorrect="off" autocapitalize="none" spellcheck="false">
          <div class="helper-text">Select your network from the dropdown above or type your SSID directly.</div>
        </div>

        <div class="form-group">
          <label for="wifiPass">Wi-Fi Password</label>
          <div style="position:relative; display:flex; align-items:center;">
            <input type="password" id="wifiPass" placeholder="Enter Wi-Fi Password" autocomplete="new-password" style="padding-right:42px;">
            <button type="button" id="btnTogglePass" style="position:absolute; right:4px; background:none; border:none; color:var(--text-muted); cursor:pointer; font-size:1.1rem; padding:4px 8px; flex:none;">👁️</button>
          </div>
        </div>

        <div class="form-group" style="margin-top:14px; border-top:1px solid var(--border); padding-top:12px;">
          <label for="apPass">Hotspot Password (Optional)</label>
          <div style="position:relative; display:flex; align-items:center;">
            <input type="password" id="apPass" placeholder="Leave blank for open network (min 8 chars)" autocomplete="new-password" style="padding-right:42px;">
            <button type="button" id="btnToggleApPass" style="position:absolute; right:4px; background:none; border:none; color:var(--text-muted); cursor:pointer; font-size:1.1rem; padding:4px 8px; flex:none;">👁️</button>
          </div>
          <div class="helper-text">Security password for <strong>NHL-Goal-Light-Setup</strong> Wi-Fi. Leave blank for open access.</div>
        </div>
      </div>

      <!-- Device Customisation (Card 2: Below Wi-Fi setup) -->
      <div class="card">
        <h2>🏷️ Device Customisation</h2>
        <div class="form-group">
          <label for="deviceNameInput">Goal Light Name</label>
          <input type="text" id="deviceNameInput" placeholder="NHL Goal Light" maxlength="32" autocomplete="off" autocorrect="off" autocapitalize="words" spellcheck="false">
          <div class="helper-text">Custom name displayed across your web portal, home screen icon, and local mDNS address (<code id="lblMdnsUrl" style="color:var(--accent);">http://goallight.local</code>).</div>
        </div>
      </div>

      <!-- Connected Hardware Modules (Card 3) -->
      <div class="card">
        <h2>🔌 Attached Hardware Modules</h2>
        <div class="helper-text" style="margin-bottom:12px;">Select the components physically attached. Unselected hardware options will be hidden automatically across all pages.</div>

        <label class="checkbox-row">
          <input type="checkbox" id="chkHwLights" checked>
          <div class="chk-text">
            <span class="chk-title">💡 Addressable RGB LEDs (WS2812B)</span>
            <span class="chk-desc">Goal celebration beacon, team colour breathing, and ambient ring lighting.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkHwSpeaker" checked>
          <div class="chk-text">
            <span class="chk-title">🔊 DFPlayer Mini Audio (Speaker)</span>
            <span class="chk-desc">Authentic goal horns, period end sirens, and championship fanfares.</span>
          </div>
        </label>

        <label class="checkbox-row">
          <input type="checkbox" id="chkHwDisplay">
          <div class="chk-text">
            <span class="chk-title">📺 GC9A01 Circular LCD Display</span>
            <span class="chk-desc">Phase 3 round puck screen showing live scores and team logos.</span>
          </div>
        </label>
      </div>

      <!-- Firmware & Updates -->
      <div class="card">
        <h2>🔄 Firmware & Updates</h2>
        <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
          <span style="font-size:0.85rem; color:var(--text-muted)">Current Version: <strong id="curVer" style="color:var(--text)">v1.1.0.2</strong></span>
          <button type="button" id="btnCheckUpdate" style="padding:6px 12px; font-size:0.8rem; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:8px; cursor:pointer;">🔍 Check GitHub</button>
        </div>
        <label class="checkbox-row" style="margin-bottom:12px;">
          <input type="checkbox" id="chkIncludeBeta">
          <div class="chk-text">
            <span class="chk-title" style="font-size:0.8rem;">Include Beta / Pre-release Channels</span>
            <span class="chk-desc" style="font-size:0.72rem;">Check for early preview builds and developer test versions.</span>
          </div>
        </label>
        <div id="updateInfo" style="display:none; padding:10px; border-radius:8px; background:rgba(46,160,67,0.15); border:1px solid var(--success); margin-bottom:12px; font-size:0.82rem; line-height:1.4;"></div>
        
        <div class="form-group" style="margin-bottom:8px;">
          <label for="otaFile">Manual OTA Firmware Upload (.bin)</label>
          <input type="file" id="otaFile" accept=".bin" style="font-size:0.8rem; padding:8px 0; color:var(--text-muted);">
        </div>
        <button type="button" id="btnOtaUpload" style="width:100%; padding:10px; background:#1f6feb; border:none; border-radius:8px; color:white; font-weight:600; cursor:pointer; display:none;">⚡ Install Firmware Update</button>
        <div id="otaProgress" style="display:none; margin-top:8px;">
          <div style="background:var(--border); border-radius:6px; height:12px; overflow:hidden;">
            <div id="otaBar" style="background:#1f6feb; height:100%; width:0%; transition:width 0.2s;"></div>
          </div>
          <div id="otaText" style="font-size:0.75rem; color:var(--text-muted); text-align:center; margin-top:4px;">Uploading: 0%</div>
        </div>
      </div>

      <!-- Smart Home & Home Assistant Integration (Card 4) -->
      <div class="card">
        <h2>🏡 Home Assistant Integration</h2>
        <div class="helper-text" style="margin-bottom:12px;">Integrate your NHL Goal Light with Home Assistant via MQTT Discovery or direct REST entity control.</div>

        <label class="checkbox-row">
          <input type="checkbox" id="chkHaEnabled">
          <div class="chk-text">
            <span class="chk-title">Enable Home Assistant Discovery</span>
            <span class="chk-desc">Broadcasts MQTT auto-discovery (light entities, live game score sensor, and goal horn trigger).</span>
          </div>
        </label>

        <div id="haSettingsGroup" style="display:none; margin-top:12px;">
          <div class="form-group">
            <label for="haBroker">MQTT Broker Host / IP</label>
            <input type="text" id="haBroker" placeholder="homeassistant.local or 192.168.1.100" autocomplete="off">
          </div>
          <div class="form-group">
            <label for="haUser">MQTT Username (Optional)</label>
            <input type="text" id="haUser" placeholder="homeassistant" autocomplete="off">
          </div>
          <div class="form-group">
            <label for="haPass">MQTT Password (Optional)</label>
            <input type="password" id="haPass" placeholder="Broker password" autocomplete="new-password">
          </div>
        </div>
      </div>

      <!-- Open Source & Community Transparency (Card 5) -->
      <div class="card" style="border-color:rgba(255,255,255,0.12); background:rgba(255,255,255,0.02);">
        <h2>✨ Open Source & Community</h2>
        <div style="font-size:0.82rem; color:var(--text-muted); line-height:1.5; margin-bottom:12px;">
          🏒 <strong>100% Free & Open Source</strong>: Built for hockey fans everywhere. 3D printable models, schematics, and code are freely available for non-commercial personal use.<br><br>
          🤖 <strong>Proudly Vibe Coded</strong>: This firmware and web portal are built with modern AI-assisted pair programming ("vibe coding"). All source code is completely public for community review, audits, and collaborative pull requests.<br><br>
          ☕ <strong>Support Development</strong>: Enjoying your goal light? Voluntary donations help support ongoing maintenance and new features.
        </div>
        <div style="display:flex; gap:8px; flex-wrap:wrap;">
          <a href="https://github.com/RevelRob/NHLGoalLight" target="_blank" style="padding:6px 12px; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:8px; text-decoration:none; font-size:0.8rem; font-weight:600;">⭐ Star on GitHub</a>
          <a href="https://revelrob.github.io/NHLGoalLight/" target="_blank" style="padding:6px 12px; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:8px; text-decoration:none; font-size:0.8rem; font-weight:600;">📖 Project Guide</a>
        </div>
      </div>

      <!-- System Diagnostics & Maintenance (Card 6) -->
      <div class="card">
        <h2>🛠️ Diagnostics & Maintenance</h2>
        <div style="font-size:0.82rem; color:var(--text-muted); display:flex; flex-direction:column; gap:6px; margin-bottom:12px;">
          <div>Device IP: <strong id="diagIp" style="color:var(--text)">--</strong></div>
          <div>Free Memory: <strong id="diagHeap" style="color:var(--text)">--</strong></div>
        </div>
        <button type="button" id="btnRestartDevice" style="background:rgba(230,57,70,0.15); border:1px solid var(--accent); color:#fff;">🔄 Reboot Goal Light</button>
      </div>
    </div>

  </div> <!-- /container -->

  <!-- Fixed Bottom Save Bar -->
  <div class="fixed-save-bar">
    <div class="save-inner">
      <button type="button" id="btnSave" class="primary" style="padding:14px; font-size:0.95rem;">💾 Save & Apply Settings</button>
    </div>
  </div>

  <!-- Save & Reboot Modal Overlay -->
  <div id="saveModal" class="modal-overlay">
    <div class="modal-content">
      <div id="modalIcon" style="font-size:3rem; margin-bottom:12px;">🚨</div>
      <h3 id="modalTitle" style="font-size:1.2rem; font-weight:700; margin-bottom:8px; color:var(--text);">Connecting to Home Wi-Fi</h3>
      <p id="modalBody" style="font-size:0.88rem; color:var(--text-muted); line-height:1.5; margin-bottom:16px;">Applying your configuration...</p>
      <div id="modalProgress" style="background:var(--border); border-radius:6px; height:8px; overflow:hidden; margin-bottom:16px;">
        <div id="modalBar" style="background:var(--accent); height:100%; width:0%; transition:width 0.4s ease;"></div>
      </div>
      <div id="modalInstructions" style="display:none; background:rgba(46,160,67,0.12); border:1px solid var(--success); border-radius:8px; padding:12px; margin-bottom:16px; font-size:0.82rem; text-align:left; line-height:1.4;">
        <strong>Next Steps:</strong><br>
        1. Connect your phone back to your home Wi-Fi.<br>
        2. Open <a href="http://goallight.local" target="_blank" style="color:#58a6ff; font-weight:600;">http://goallight.local</a> in your browser.
      </div>
      <button id="modalBtn" type="button" style="display:none; width:100%; padding:12px; border-radius:8px; background:var(--accent); color:#fff; border:none; font-weight:600; cursor:pointer;" onclick="window.location.href='http://goallight.local'">Open goallight.local</button>
    </div>
  </div>

  <script>
    // 32 NHL Teams with Primary Hex Colours
    const TEAMS = [
      ["ANA", "Anaheim Ducks", "#F47A38"], ["BOS", "Boston Bruins", "#FFB81C"], ["BUF", "Buffalo Sabres", "#002654"],
      ["CGY", "Calgary Flames", "#C8102E"], ["CAR", "Carolina Hurricanes", "#CC0000"], ["CHI", "Chicago Blackhawks", "#CF0A2C"],
      ["COL", "Colorado Avalanche", "#6F263D"], ["CBJ", "Columbus Blue Jackets", "#002654"], ["DAL", "Dallas Stars", "#006847"],
      ["DET", "Detroit Red Wings", "#CE1126"], ["EDM", "Edmonton Oilers", "#041E42"], ["FLA", "Florida Panthers", "#041E42"],
      ["LAK", "Los Angeles Kings", "#111111"], ["MIN", "Minnesota Wild", "#154734"], ["MTL", "Montreal Canadiens", "#AF1E2D"],
      ["NSH", "Nashville Predators", "#FFB81C"], ["NJD", "New Jersey Devils", "#CE1126"], ["NYI", "New York Islanders", "#00539B"],
      ["NYR", "New York Rangers", "#0038A8"], ["OTT", "Ottawa Senators", "#DA1A32"], ["PHI", "Philadelphia Flyers", "#F74902"],
      ["PIT", "Pittsburgh Penguins", "#FCB514"], ["SJS", "San Jose Sharks", "#006D75"], ["SEA", "Seattle Kraken", "#001628"],
      ["STL", "St. Louis Blues", "#002F87"], ["TBL", "Tampa Bay Lightning", "#002868"], ["TOR", "Toronto Maple Leafs", "#00205B"],
      ["UTA", "Utah Hockey Club", "#79CAEC"], ["VAN", "Vancouver Canucks", "#00205B"], ["VGK", "Vegas Golden Knights", "#B4975A"],
      ["WSH", "Washington Capitals", "#041E42"], ["WPG", "Winnipeg Jets", "#041E42"]
    ];

    // Populate Team Select
    const teamSelect = document.getElementById('teamSelect');
    TEAMS.forEach(([abbrev, name]) => {
      const opt = document.createElement('option');
      opt.value = abbrev;
      opt.textContent = `${name} (${abbrev})`;
      teamSelect.appendChild(opt);
    });

    // Tab Navigation Switcher
    const navTabs = document.querySelectorAll('.nav-tab');
    const tabPages = document.querySelectorAll('.tab-page');
    let userNavigated = false;

    function switchTab(targetTab) {
      if (document.activeElement && typeof document.activeElement.blur === 'function') {
        document.activeElement.blur();
      }
      navTabs.forEach(t => {
        if (t.getAttribute('data-tab') === targetTab) {
          t.classList.add('active');
        } else {
          t.classList.remove('active');
        }
      });
      tabPages.forEach(p => {
        if (p.id === 'tab-' + targetTab) {
          p.classList.add('active');
          // Shield tab from touchscreen synthetic click bleed-through
          p.classList.add('just-activated');
          setTimeout(() => p.classList.remove('just-activated'), 300);
        } else {
          p.classList.remove('active');
          p.classList.remove('just-activated');
        }
      });
    }

    navTabs.forEach(tab => {
      tab.addEventListener('click', (e) => {
        e.preventDefault();
        userNavigated = true;
        const targetTab = tab.getAttribute('data-tab');
        switchTab(targetTab);
      });
    });

    // Detect if loaded in AP Setup Mode or inside Captive Network Assistant (CNA) browser
    const currentHost = window.location.hostname.toLowerCase();
    const isLikelyCaptivePortal = (
      currentHost === '192.168.4.1' ||
      currentHost.includes('apple') ||
      currentHost.includes('captive') ||
      currentHost.includes('gstatic') ||
      currentHost.includes('google') ||
      currentHost.includes('msftconnecttest') ||
      currentHost.includes('hotspot')
    );

    if (isLikelyCaptivePortal) {
      switchTab('system');
    }

    window.addEventListener('DOMContentLoaded', () => {
      if (document.activeElement && typeof document.activeElement.blur === 'function') {
        document.activeElement.blur();
      }
    });

    // Dynamic Hardware Enablement Engine
    const chkHwLights = document.getElementById('chkHwLights');
    const chkHwSpeaker = document.getElementById('chkHwSpeaker');
    const chkHwDisplay = document.getElementById('chkHwDisplay');

    function updateHardwareVisibility() {
      const lightsOn = chkHwLights.checked;
      const speakerOn = chkHwSpeaker.checked;
      const displayOn = chkHwDisplay.checked;

      document.querySelectorAll('[data-hw="lights"]').forEach(el => {
        el.setAttribute('data-hw-hidden', !lightsOn);
      });
      document.querySelectorAll('[data-hw="speaker"]').forEach(el => {
        el.setAttribute('data-hw-hidden', !speakerOn);
      });
      document.querySelectorAll('[data-hw="display"]').forEach(el => {
        el.setAttribute('data-hw-hidden', !displayOn);
      });
    }

    chkHwLights.onchange = updateHardwareVisibility;
    chkHwSpeaker.onchange = updateHardwareVisibility;
    chkHwDisplay.onchange = updateHardwareVisibility;

    // Sliders
    const brightSlider = document.getElementById('brightSlider');
    const brightVal = document.getElementById('brightVal');
    brightSlider.oninput = () => brightVal.textContent = Math.round((brightSlider.value / 255) * 100) + '%';
    brightSlider.onchange = () => {
      fetch('/api/quick-set', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ brightness: parseInt(brightSlider.value) })
      }).catch(console.error);
    };

    const volSlider = document.getElementById('volSlider');
    const volVal = document.getElementById('volVal');
    volSlider.oninput = () => volVal.textContent = volSlider.value == 0 ? 'Mute' : 'Level ' + volSlider.value;
    volSlider.onchange = () => {
      fetch('/api/quick-set', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ volume: parseInt(volSlider.value), chirp: true })
      }).catch(console.error);
    };

    const hornSlider = document.getElementById('hornSlider');
    const hornVal = document.getElementById('hornVal');
    hornSlider.oninput = () => hornVal.textContent = hornSlider.value + 's';

    const delaySlider = document.getElementById('delaySlider');
    const delayVal = document.getElementById('delayVal');
    delaySlider.oninput = () => {
      const v = parseInt(delaySlider.value);
      delayVal.textContent = v === 0 ? '0s (Instantaneous)' : v + 's delay';
    };

    window.setBroadcastPreset = (sec) => {
      delaySlider.value = sec;
      delaySlider.oninput();
    };

    window.nudgeDelay = (delta) => {
      let v = parseInt(delaySlider.value) + delta;
      if (v < 0) v = 0;
      if (v > 120) v = 120;
      delaySlider.value = v;
      delaySlider.oninput();
    };

    // Virtual LED Preview Renderer (Responsive to Geometry & Celebration/Idle Modes)
    const ringCanvas = document.getElementById('ringCanvas');
    const ringCtx = ringCanvas.getContext('2d');
    let previewStep = 0;
    let visualiserMode = 'celebration'; // 'celebration' or 'idle'

    window.setVisualiserMode = function(mode) {
      visualiserMode = mode;
      const btnC = document.getElementById('btnPrevCeleb');
      const btnI = document.getElementById('btnPrevIdle');
      if (btnC) btnC.classList.toggle('active', mode === 'celebration');
      if (btnI) btnI.classList.toggle('active', mode === 'idle');
    };

    function getTeamColourHex(abbrev) {
      const t = TEAMS.find(item => item[0] === abbrev);
      return t ? t[2] : '#e63946';
    }

    function renderVirtualRing() {
      const nPixels = Math.max(4, parseInt(document.getElementById('ledPixels').value) || 16);
      const layout = parseInt(document.getElementById('ledLayout').value) || 0;
      const effect = parseInt(document.getElementById('effectSelect').value) || 0;
      const idleEff = parseInt(document.getElementById('idleEffect').value) || 0;
      const teamHex = getTeamColourHex(teamSelect.value);
      const isReverse = document.getElementById('chkLedReverse').checked;

      const w = ringCanvas.width;
      const h = ringCanvas.height;
      ringCtx.clearRect(0, 0, w, h);

      const cx = w / 2;
      const cy = h / 2;
      const ledRadius = Math.max(3, Math.min(8, Math.floor(130 / nPixels) + 2));

      for (let i = 0; i < nPixels; i++) {
        const mappedIdx = isReverse ? ((nPixels - 1) - i) : i;
        let x = cx;
        let y = cy;

        // 1. Calculate (x, y) based on arrangement geometry
        if (layout === 0) { // Single Ring
          const radius = 58;
          const angle = (mappedIdx / nPixels) * (2 * Math.PI) - (Math.PI / 2);
          x = cx + radius * Math.cos(angle);
          y = cy + radius * Math.sin(angle);
        } else if (layout === 1) { // Spiral Wrap Strip (Cylindrical Beacon)
          const turns = 2.4;
          const spiralAngle = (mappedIdx / nPixels) * (turns * 2 * Math.PI) - (Math.PI / 2);
          const rx = 52;
          x = cx + rx * Math.cos(spiralAngle);
          y = (cy + 48) - (mappedIdx / Math.max(1, nPixels - 1)) * 96;
        } else if (layout === 2) { // Parallel Columns
          const half = Math.ceil(nPixels / 2);
          const isFirstCol = (mappedIdx < half);
          const colX = isFirstCol ? (cx - 38) : (cx + 38);
          const colStep = isFirstCol ? mappedIdx : (nPixels - 1 - mappedIdx);
          const colTotal = isFirstCol ? half : (nPixels - half);
          x = colX;
          y = (cy - 48) + (colStep / Math.max(1, colTotal - 1)) * 96;
        } else { // 3: Linear Strip / Bar
          const barWidth = Math.min(180, nPixels * 14);
          x = (cx - barWidth / 2) + (mappedIdx / Math.max(1, nPixels - 1)) * barWidth;
          y = cy;
        }

        let alpha = 0.15;
        let fill = teamHex;

        // 2. Calculate animation state
        if (visualiserMode === 'celebration') {
          if (effect === 0) { // Rotating Halogen Beacon (Fixed continuous modulo wrap)
            const head = Math.floor(previewStep / 3) % nPixels;
            const rawDiff = Math.abs(i - head);
            const dist = Math.min(rawDiff, nPixels - rawDiff);
            if (dist === 0) alpha = 1.0;
            else if (dist === 1) alpha = 0.65;
            else if (dist === 2) alpha = 0.25;
            else alpha = 0.05;
            fill = teamHex;
          } else if (effect === 1) { // Arena Strobe
            const flash = (previewStep % 4 < 2);
            alpha = flash ? 1.0 : 0.05;
            fill = (previewStep % 8 < 4) ? '#ffffff' : teamHex;
          } else if (effect === 2) { // Breathe
            alpha = 0.15 + 0.85 * (Math.sin(previewStep * 0.12) + 1.0) / 2.0;
            fill = teamHex;
          } else if (effect === 3) { // Dual Siren
            const half = Math.floor(nPixels / 2);
            const isLeft = (i < half);
            const sideA = ((previewStep % 16) < 8);
            const flashOn = (previewStep % 4 < 2);
            if (flashOn && ((sideA && isLeft) || (!sideA && !isLeft))) {
              alpha = 1.0;
              fill = isLeft ? teamHex : '#ffffff';
            } else {
              alpha = 0.08;
              fill = isLeft ? teamHex : '#ffffff';
            }
          } else if (effect === 4) { // Heartbeat
            const sub = previewStep % 24;
            alpha = (sub === 0 || sub === 1 || sub === 2 || sub === 6 || sub === 7 || sub === 8) ? 1.0 : 0.08;
            fill = teamHex;
          }
        } else {
          // Idle Mode Effects
          if (idleEff === 0) { // Off
            alpha = 0.04;
            fill = '#444444';
          } else if (idleEff === 1) { // Breathe
            alpha = 0.12 + 0.65 * (Math.sin(previewStep * 0.05) + 1.0) / 2.0;
            fill = teamHex;
          } else if (idleEff === 2) { // Solid Accent
            alpha = 0.65;
            fill = teamHex;
          } else if (idleEff === 3) { // Candle / Ember Flicker
            const f1 = Math.sin(previewStep * 0.18 + i * 2.1);
            const f2 = Math.sin(previewStep * 0.35 + i * 4.7);
            alpha = 0.2 + 0.65 * ((f1 + f2 + 2) / 4);
            fill = '#ff7700';
          } else if (idleEff === 4) { // Orbit Sweep
            const orbitHead = Math.floor(previewStep / 5) % nPixels;
            const rawDiff = Math.abs(i - orbitHead);
            const dist = Math.min(rawDiff, nPixels - rawDiff);
            if (dist === 0) alpha = 0.9;
            else if (dist === 1) alpha = 0.38;
            else alpha = 0.06;
            fill = teamHex;
          }
        }

        ringCtx.beginPath();
        ringCtx.arc(x, y, ledRadius, 0, 2 * Math.PI);
        ringCtx.fillStyle = fill;
        ringCtx.globalAlpha = alpha;
        ringCtx.fill();
        ringCtx.globalAlpha = 1.0;

        // Subtle outline
        ringCtx.lineWidth = 1;
        ringCtx.strokeStyle = 'rgba(255,255,255,0.2)';
        ringCtx.stroke();
      }

      // Update caption
      const cap = document.getElementById('previewCaption');
      if (cap && previewStep % 30 === 0) {
        const layoutNames = ['Single Ring', 'Spiral Wrap', 'Parallel Columns', 'Linear Strip'];
        const celebNames = ['Halogen Beacon', 'Arena Strobe', 'Team Breathe', 'Dual Siren', 'Heartbeat Pulse'];
        const idleNames = ['Dark (Off)', 'Team Breathe', 'Solid Accent', 'Ember Flicker', 'Orbit Sweep'];
        const activeName = (visualiserMode === 'celebration') ? celebNames[effect] : idleNames[idleEff];
        cap.textContent = `${activeName} • ${layoutNames[layout] || 'Ring'}`;
      }

      previewStep++;
      requestAnimationFrame(renderVirtualRing);
    }
    requestAnimationFrame(renderVirtualRing);

    // Wi-Fi Scanning & Password Controls
    const wifiSelect = document.getElementById('wifiSelect');
    const wifiSsid = document.getElementById('wifiSsid');
    const manualSsidGroup = document.getElementById('manualSsidGroup');
    const btnRescan = document.getElementById('btnRescan');
    const wifiPass = document.getElementById('wifiPass');
    const btnTogglePass = document.getElementById('btnTogglePass');
    const apPass = document.getElementById('apPass');
    const btnToggleApPass = document.getElementById('btnToggleApPass');

    let savedSsid = '';
    let scanPollTimer = null;

    btnTogglePass.onclick = () => {
      const isPass = wifiPass.type === 'password';
      wifiPass.type = isPass ? 'text' : 'password';
      btnTogglePass.textContent = isPass ? '🔒' : '👁️';
    };

    btnToggleApPass.onclick = () => {
      const isPass = apPass.type === 'password';
      apPass.type = isPass ? 'text' : 'password';
      btnToggleApPass.textContent = isPass ? '🔒' : '👁️';
    };

    wifiSelect.onchange = () => {
      if (wifiSelect.value && wifiSelect.value !== '__manual__') {
        wifiSsid.value = wifiSelect.value;
      }
    };

    function renderNetworks(networks) {
      const map = new Map();
      networks.forEach(n => {
        if (!n.ssid) return;
        if (!map.has(n.ssid) || map.get(n.ssid).rssi < n.rssi) {
          map.set(n.ssid, n);
        }
      });

      const unique = Array.from(map.values()).sort((a, b) => b.rssi - a.rssi);

      wifiSelect.innerHTML = '';
      const defaultOpt = document.createElement('option');
      defaultOpt.value = '';
      defaultOpt.disabled = true;
      defaultOpt.textContent = '-- Select your Wi-Fi network --';
      wifiSelect.appendChild(defaultOpt);

      let foundSaved = false;
      unique.forEach(net => {
        const opt = document.createElement('option');
        opt.value = net.ssid;
        const icon = net.rssi >= -60 ? '📶' : (net.rssi >= -75 ? '🛜' : '📡');
        const lock = net.secure ? '🔒' : '🔓';
        opt.textContent = `${icon} ${net.ssid} (${net.rssi} dBm, ${lock})`;
        if (savedSsid && net.ssid === savedSsid) {
          opt.selected = true;
          foundSaved = true;
          wifiSsid.value = net.ssid;
        }
        wifiSelect.appendChild(opt);
      });

      if (savedSsid && !foundSaved) {
        const savedOpt = document.createElement('option');
        savedOpt.value = savedSsid;
        savedOpt.selected = true;
        savedOpt.textContent = `📶 ${savedSsid} (Saved)`;
        wifiSelect.appendChild(savedOpt);
        wifiSsid.value = savedSsid;
      }

      const manualOpt = document.createElement('option');
      manualOpt.value = '__manual__';
      manualOpt.textContent = '✍️ Enter SSID Manually...';
      wifiSelect.appendChild(manualOpt);

      if (!savedSsid && !foundSaved) {
        defaultOpt.selected = true;
      }
    }

    function fetchScan(retryCount = 0) {
      if (scanPollTimer) clearTimeout(scanPollTimer);
      fetch('/api/scan')
        .then(r => r.json())
        .then(data => {
          if (data && data.scanning) {
            btnRescan.textContent = '⏳ Scanning...';
            btnRescan.disabled = true;
            if (retryCount < 8) {
              scanPollTimer = setTimeout(() => fetchScan(retryCount + 1), 1500);
            } else {
              btnRescan.textContent = '🔄 Rescan';
              btnRescan.disabled = false;
            }
          } else if (Array.isArray(data)) {
            btnRescan.textContent = '🔄 Rescan';
            btnRescan.disabled = false;
            renderNetworks(data);
          }
        })
        .catch(() => {
          btnRescan.textContent = '🔄 Rescan';
          btnRescan.disabled = false;
        });
    }

    btnRescan.onclick = () => {
      btnRescan.textContent = '⏳ Scanning...';
      btnRescan.disabled = true;
      wifiSelect.innerHTML = '<option value="" disabled selected>🔍 Scanning for Wi-Fi networks...</option>';
      fetch('/api/scan', { method: 'POST' })
        .then(() => {
          scanPollTimer = setTimeout(() => fetchScan(0), 1000);
        })
        .catch(() => fetchScan(0));
    };

    // Load initial config from ESP32
    fetch('/api/config')
      .then(r => r.json())
      .then(cfg => {
        if (cfg.deviceName) {
          document.getElementById('deviceNameInput').value = cfg.deviceName;
          document.getElementById('titleDeviceName').textContent = '🚨 ' + cfg.deviceName;
        }
        if (cfg.team) teamSelect.value = cfg.team;
        if (cfg.hwLights !== undefined) chkHwLights.checked = !!cfg.hwLights;
        if (cfg.hwSpeaker !== undefined) chkHwSpeaker.checked = !!cfg.hwSpeaker;
        if (cfg.hwDisplay !== undefined) chkHwDisplay.checked = !!cfg.hwDisplay;
        updateHardwareVisibility();

        if (cfg.numLeds) document.getElementById('ledPixels').value = cfg.numLeds;
        if (cfg.ledLayout !== undefined) document.getElementById('ledLayout').value = cfg.ledLayout;
        if (cfg.ledReverse !== undefined) document.getElementById('chkLedReverse').checked = !!cfg.ledReverse;
        if (cfg.idleEffect !== undefined) document.getElementById('idleEffect').value = cfg.idleEffect;

        if (cfg.brightness) {
          brightSlider.value = cfg.brightness;
          brightVal.textContent = Math.round((cfg.brightness / 255) * 100) + '%';
        }
        if (cfg.volume !== undefined) {
          volSlider.value = cfg.volume;
          volVal.textContent = cfg.volume == 0 ? 'Mute' : 'Level ' + cfg.volume;
        }
        if (cfg.hornDuration) {
          hornSlider.value = cfg.hornDuration;
          hornVal.textContent = cfg.hornDuration + 's';
        }
        if (cfg.effectStyle !== undefined) document.getElementById('effectSelect').value = cfg.effectStyle;

        document.getElementById('chkOpponent').checked = !!cfg.alertOpponent;
        document.getElementById('chkPowerPlay').checked = !!cfg.alertPowerPlay;
        document.getElementById('chkBuzzer').checked = !!cfg.alertBuzzer;
        if (cfg.alertIntermission !== undefined) document.getElementById('chkIntermission').checked = !!cfg.alertIntermission;
        document.getElementById('chkWin').checked = !!cfg.alertWin;

        if (cfg.broadcastDelay !== undefined) {
          delaySlider.value = cfg.broadcastDelay;
          delaySlider.oninput();
        }

        if (cfg.ssid) {
          savedSsid = cfg.ssid;
          wifiSsid.value = cfg.ssid;
        }
        if (cfg.apPass) apPass.value = cfg.apPass;

        fetchScan();
      }).catch(console.error);

    let deviceMacHostname = '';

    function deriveHostname(name, fallback) {
      if (!name || !name.trim()) return fallback || 'goallight.local';
      const trimmed = name.trim();
      // Match "NHL Goal Light X" or "Goal Light X"
      const nhlMatch = trimmed.match(/^(?:NHL\s+Goal\s+Light|Goal\s+Light)(?:\s*[-_ ]*\s*(.*))?$/i);
      if (nhlMatch) {
        const suffix = nhlMatch[1] ? nhlMatch[1].trim() : '';
        if (suffix) {
          const slug = suffix.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
          return 'goallight-' + slug + '.local';
        } else {
          // Exactly "NHL Goal Light" -> return fallback with MAC if available
          return fallback || 'goallight.local';
        }
      }
      // Generic custom name (e.g. "Man Cave" -> "man-cave.local")
      const slug = trimmed.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
      return (slug || 'goallight') + '.local';
    }

    // Upcoming Game View Switcher
    window.setUpcomingView = function(view) {
      const isSimple = (view === 'simple');
      const btnS = document.getElementById('btnViewSimple');
      const btnD = document.getElementById('btnViewDetailed');
      const simV = document.getElementById('upcomingSimpleView');
      const detV = document.getElementById('upcomingDetailedView');
      if (btnS) btnS.classList.toggle('active', isSimple);
      if (btnD) btnD.classList.toggle('active', !isSimple);
      if (simV) simV.style.display = isSimple ? 'block' : 'none';
      if (detV) detV.style.display = isSimple ? 'none' : 'block';
    };

    // Live Game & Status Polling
    function refreshStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(s => {
          const badge = document.getElementById('statusBadge');
          if (s.system && s.system.deviceName) {
            document.getElementById('titleDeviceName').textContent = '🚨 ' + s.system.deviceName;
          }

          // If in AP Setup Mode and user has not intentionally navigated to another tab, stay on system tab
          if (!userNavigated && s.system && s.system.isApMode) {
            switchTab('system');
          }

          const cardLive = document.getElementById('cardLiveGame');
          const isLiveGame = (s.game && s.game.state === 'LIVE');

          if (isLiveGame) {
            badge.className = 'badge live';
            badge.textContent = `● LIVE: ${s.game.awayAbbrev} ${s.game.awayScore} - ${s.game.homeScore} ${s.game.homeAbbrev}`;
            document.getElementById('liveAwayTeam').textContent = s.game.awayAbbrev || 'AWY';
            document.getElementById('liveAwayScore').textContent = s.game.awayScore;
            document.getElementById('liveHomeTeam').textContent = s.game.homeAbbrev || 'HME';
            document.getElementById('liveHomeScore').textContent = s.game.homeScore;

            const clockText = s.game.timeRemaining ? `P${s.game.period} ${s.game.timeRemaining}` : `Period ${s.game.period}`;
            document.getElementById('liveGameClock').textContent = clockText;
            document.getElementById('syncNhlClock').textContent = s.game.timeRemaining || 'In Progress';
            document.getElementById('syncNhlPeriod').textContent = `Period ${s.game.period}`;

            const interm = !!s.game.inIntermission;
            document.getElementById('liveIntermissionNote').style.display = interm ? 'inline' : 'none';
            document.getElementById('liveStatusSubtext').textContent = interm ? 'Intermission in progress.' : 'Live game active.';
            if (cardLive) cardLive.style.display = 'block';
          } else if (s.system && s.system.connected) {
            badge.className = 'badge';
            badge.textContent = `Online (${s.system.team})`;
            document.getElementById('liveGameClock').textContent = 'No Live Game';
            document.getElementById('syncNhlClock').textContent = '--:--';
            document.getElementById('syncNhlPeriod').textContent = 'No Live Game Right Now';
            document.getElementById('liveIntermissionNote').style.display = 'none';
            document.getElementById('liveStatusSubtext').textContent = 'No game currently in progress.';
          } else {
            badge.className = 'badge';
            badge.textContent = 'AP Setup Mode (192.168.4.1)';
            document.getElementById('liveGameClock').textContent = 'Setup Mode';
            document.getElementById('liveStatusSubtext').textContent = 'Configure home Wi-Fi to start.';
          }

          // Upcoming Game Details (Simple & Detailed Views)
          if (s.upcoming && s.upcoming.valid) {
            const up = s.upcoming;
            const isHome = !!up.isHome;
            const myTeam = s.system ? s.system.team : 'TOR';
            const opp = up.opponent || 'OPP';
            const oppName = up.opponentName || opp;
            const homeAbbrev = isHome ? myTeam : opp;
            const awayAbbrev = isHome ? opp : myTeam;

            let dateStr = up.date || '';
            let timeStr = '';
            if (up.timeUTC) {
              try {
                const d = new Date(up.timeUTC);
                dateStr = d.toLocaleDateString(undefined, { weekday: 'short', month: 'short', day: 'numeric' });
                timeStr = d.toLocaleTimeString(undefined, { hour: 'numeric', minute: '2-digit' });
              } catch (e) {
                timeStr = up.timeUTC;
              }
            }

            document.getElementById('upSimpleAway').textContent = awayAbbrev;
            document.getElementById('upSimpleHome').textContent = homeAbbrev;
            document.getElementById('upSimpleAwayLabel').textContent = isHome ? `Away (${opp})` : `${myTeam} (Away)`;
            document.getElementById('upSimpleHomeLabel').textContent = isHome ? `${myTeam} (Home)` : `Home (${opp})`;
            document.getElementById('upSimpleDate').textContent = dateStr;
            document.getElementById('upSimpleTime').textContent = timeStr || 'Time TBA';
            document.getElementById('upSimpleVenue').textContent = '📍 ' + (up.venue || 'Arena TBD');
            document.getElementById('upSimpleBroadcasts').textContent = '📺 ' + (up.broadcasts || 'TV TBA');

            // Detailed view fields
            document.getElementById('upDetMatchupTitle').textContent = `${awayAbbrev} @ ${homeAbbrev} (${oppName})`;
            const badgeEl = document.getElementById('upDetHomeAwayBadge');
            if (badgeEl) {
              badgeEl.textContent = isHome ? '🏠 Home Game' : '✈️ Road Game';
              badgeEl.style.background = isHome ? 'rgba(46,160,67,0.15)' : 'rgba(230,57,70,0.15)';
              badgeEl.style.borderColor = isHome ? 'var(--success)' : 'var(--accent)';
              badgeEl.style.color = isHome ? 'var(--success)' : 'var(--accent)';
            }
            document.getElementById('upDetDate').textContent = dateStr || 'TBD';
            document.getElementById('upDetTime').textContent = timeStr ? `${timeStr} (Local Time)` : 'Time TBA';
            document.getElementById('upDetVenue').textContent = up.venue || 'Arena to be announced';
            document.getElementById('upDetBroadcasts').textContent = up.broadcasts || 'Broadcasters pending';
            document.getElementById('upDetRadio').textContent = `${myTeam} Audio Network & NHL App`;
            if (up.gameId) {
              document.getElementById('upDetNhlLink').href = `https://www.nhl.com/gamecenter/${up.gameId}`;
            }
          } else {
            document.getElementById('upSimpleDate').textContent = 'No Games Scheduled This Week';
            document.getElementById('upSimpleTime').textContent = 'Off';
            document.getElementById('upSimpleVenue').textContent = '📍 Off-season or bye week';
            document.getElementById('upSimpleBroadcasts').textContent = 'NHL.com';
            document.getElementById('upDetDate').textContent = 'No upcoming games scheduled';
            document.getElementById('upDetVenue').textContent = 'Enjoy the off-days!';
            document.getElementById('upDetBroadcasts').textContent = 'Check NHL.com for updates';
          }

          if (s.system) {
            if (s.system.version) document.getElementById('curVer').textContent = 'v' + s.system.version;
            if (s.system.freeHeap) document.getElementById('diagHeap').textContent = `${Math.round(s.system.freeHeap / 1024)} KB`;
            document.getElementById('diagIp').textContent = window.location.hostname;
            if (s.system.hostname) {
              deviceMacHostname = s.system.hostname;
              const lbl = document.getElementById('lblMdnsUrl');
              const devInp = document.getElementById('deviceNameInput');
              // Only update if user is not actively editing the name field
              if (lbl && devInp && document.activeElement !== devInp) {
                lbl.textContent = 'http://' + deriveHostname(devInp.value, deviceMacHostname);
              }
            }
          }
        }).catch(() => {});
    }
    setInterval(refreshStatus, 4000);
    refreshStatus();

    // Live update mDNS label as user types device name
    const devInput = document.getElementById('deviceNameInput');
    if (devInput) {
      devInput.oninput = function() {
        const lbl = document.getElementById('lblMdnsUrl');
        if (lbl) lbl.textContent = 'http://' + deriveHostname(this.value, deviceMacHostname || 'goallight.local');
      };
    }

    // Event Simulator Studio (Dedicated Testing Controls)
    const simEvent = document.getElementById('simEventSelect');
    const simEffectGroup = document.getElementById('simEffectGroup');
    const simEffect = document.getElementById('simEffectSelect');

    if (simEvent && simEffectGroup) {
      simEvent.onchange = () => {
        simEffectGroup.style.display = (simEvent.value === 'goal') ? 'block' : 'none';
      };
    }

    const btnRunSim = document.getElementById('btnRunSimTest');
    if (btnRunSim) {
      btnRunSim.onclick = () => {
        const ev = simEvent ? simEvent.value : 'goal';
        const effectVal = simEffect ? (parseInt(simEffect.value) || 0) : 0;
        const lights = document.getElementById('chkSimLights') ? document.getElementById('chkSimLights').checked : true;
        const horn = document.getElementById('chkSimHorn') ? document.getElementById('chkSimHorn').checked : true;
        const display = document.getElementById('chkSimDisplay') ? document.getElementById('chkSimDisplay').checked : true;

        fetch('/api/test', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({
            event: ev,
            effect: effectVal,
            light: lights,
            horn: horn,
            display: display
          })
        }).catch(console.error);
      };
    }

    const btnStopSim = document.getElementById('btnStopSimTest');
    if (btnStopSim) {
      btnStopSim.onclick = () => {
        fetch('/api/test', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ event: 'stop' })
        }).catch(console.error);
      };
    }

    // Save & Reboot Flow
    const saveModal = document.getElementById('saveModal');
    const modalIcon = document.getElementById('modalIcon');
    const modalTitle = document.getElementById('modalTitle');
    const modalBody = document.getElementById('modalBody');
    const modalBar = document.getElementById('modalBar');
    const modalInstructions = document.getElementById('modalInstructions');
    const modalBtn = document.getElementById('modalBtn');

    document.getElementById('btnSave').onclick = () => {
      if (document.activeElement && typeof document.activeElement.blur === 'function') {
        document.activeElement.blur();
      }

      let targetSsid = wifiSsid.value.trim();
      if (!targetSsid && wifiSelect.value && wifiSelect.value !== '__manual__') {
        targetSsid = wifiSelect.value;
        wifiSsid.value = targetSsid;
      }

      const isApModeActive = (window.location.hostname === '192.168.4.1');
      if (isApModeActive && !targetSsid) {
        alert('⚠️ Please select or enter your Wi-Fi Network Name (SSID) before saving!');
        switchTab('system');
        return;
      }

      const data = {
        deviceName: document.getElementById('deviceNameInput').value.trim() || 'NHL Goal Light',
        team: teamSelect.value,
        hwLights: chkHwLights.checked,
        hwSpeaker: chkHwSpeaker.checked,
        hwDisplay: chkHwDisplay.checked,
        numLeds: parseInt(document.getElementById('ledPixels').value) || 16,
        ledLayout: parseInt(document.getElementById('ledLayout').value) || 0,
        ledReverse: document.getElementById('chkLedReverse').checked,
        idleEffect: parseInt(document.getElementById('idleEffect').value) || 1,
        brightness: parseInt(brightSlider.value),
        volume: parseInt(volSlider.value),
        hornDuration: parseInt(hornSlider.value),
        effectStyle: parseInt(document.getElementById('effectSelect').value),
        alertOpponent: document.getElementById('chkOpponent').checked,
        alertPowerPlay: document.getElementById('chkPowerPlay').checked,
        alertBuzzer: document.getElementById('chkBuzzer').checked,
        alertIntermission: document.getElementById('chkIntermission').checked,
        alertWin: document.getElementById('chkWin').checked,
        broadcastDelay: parseInt(delaySlider.value) || 0,
        ssid: targetSsid,
        pass: wifiPass.value,
        apPass: apPass.value
      };

      saveModal.style.display = 'flex';
      modalBar.style.width = '20%';
      modalTitle.textContent = 'Saving Settings...';
      modalBody.textContent = 'Writing configuration to flash memory...';
      modalInstructions.style.display = 'none';
      modalBtn.style.display = 'none';

      fetch('/api/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(data)
      })
      .then(r => r.json().catch(() => ({ success: true, restart: false })))
      .then(res => {
        if (res.restart) {
          modalIcon.textContent = '🚨';
          modalTitle.textContent = 'Connecting to Wi-Fi!';
          modalBody.innerHTML = `Connecting to <strong>${targetSsid}</strong>... The Goal Light is rebooting to join your home network.`;
          modalInstructions.style.display = 'block';

          let pct = 20;
          const interval = setInterval(() => {
            pct += 8;
            if (pct >= 100) {
              pct = 100;
              clearInterval(interval);
              modalBtn.style.display = 'block';
            }
            modalBar.style.width = pct + '%';
          }, 800);
        } else {
          modalIcon.textContent = '✅';
          modalTitle.textContent = 'Settings Applied!';
          modalBody.textContent = 'Your configuration has been updated successfully.';
          modalBar.style.width = '100%';
          setTimeout(() => {
            saveModal.style.display = 'none';
          }, 1400);
        }
      })
      .catch(() => {
        modalIcon.textContent = '✅';
        modalTitle.textContent = 'Settings Saved';
        modalBody.innerHTML = `Settings written. The device is applying changes.`;
        modalInstructions.style.display = 'block';
        modalBar.style.width = '100%';
        modalBtn.style.display = 'block';
      });
    };

    // Firmware & GitHub Updates
    const btnCheckUpdate = document.getElementById('btnCheckUpdate');
    const updateInfo = document.getElementById('updateInfo');
    const otaFile = document.getElementById('otaFile');
    const btnOtaUpload = document.getElementById('btnOtaUpload');
    const otaProgress = document.getElementById('otaProgress');
    const otaBar = document.getElementById('otaBar');
    const otaText = document.getElementById('otaText');

    // Helper: Parse semver / quad-ver strings like "v1.1.0.1" into [1, 1, 0, 1]
    function parseSemVer(vStr) {
      if (!vStr) return [0, 0, 0, 0];
      const cleaned = vStr.replace(/^v/i, '').trim();
      const parts = cleaned.split(/[\.-]/).map(p => parseInt(p, 10) || 0);
      while (parts.length < 4) parts.push(0);
      return parts;
    }

    function compareVersions(vA, vB) {
      const pA = parseSemVer(vA);
      const pB = parseSemVer(vB);
      for (let i = 0; i < Math.max(pA.length, pB.length); i++) {
        const a = pA[i] || 0;
        const b = pB[i] || 0;
        if (a > b) return 1;
        if (a < b) return -1;
      }
      return 0;
    }

    btnCheckUpdate.onclick = () => {
      btnCheckUpdate.textContent = 'Checking...';
      const includeBeta = document.getElementById('chkIncludeBeta') ? document.getElementById('chkIncludeBeta').checked : false;

      const checkPagesManifest = () => {
        return fetch('https://revelrob.github.io/NHLGoalLight/manifest.json?t=' + Date.now())
          .then(r => {
            if (!r.ok) throw new Error('No manifest found');
            return r.json();
          })
          .then(man => {
            return {
              tag_name: 'v' + man.version,
              body: 'Latest release from NHL Goal Light update pipeline.',
              html_url: 'https://revelrob.github.io/NHLGoalLight/',
              assets: [{
                name: 'firmware.bin',
                browser_download_url: 'https://revelrob.github.io/NHLGoalLight/builds/firmware.bin'
              }]
            };
          });
      };

      const githubUrl = includeBeta
        ? 'https://api.github.com/repos/RevelRob/NHLGoalLight/releases'
        : 'https://api.github.com/repos/RevelRob/NHLGoalLight/releases/latest';

      fetch(githubUrl)
        .then(r => {
          if (!r.ok) return checkPagesManifest();
          return r.json();
        })
        .then(data => {
          let rel = data;
          if (Array.isArray(data)) {
            rel = data[0] || null;
          }
          if (!rel || (!rel.tag_name && !rel.name)) return checkPagesManifest();
          return rel;
        })
        .catch(() => checkPagesManifest())
        .then(rel => {
          btnCheckUpdate.textContent = '🔍 Check GitHub';
          const latestTag = rel.tag_name || rel.name || '';
          const currentTag = document.getElementById('curVer').textContent.trim();
          updateInfo.style.display = 'block';

          const cmp = compareVersions(latestTag, currentTag);

          if (cmp > 0 && latestTag) {
            updateInfo.style.borderColor = '#e63946';
            updateInfo.style.background = 'rgba(230,57,70,0.15)';
            let binAsset = rel.assets && rel.assets.find(a => a.name.endsWith('.bin'));
            let assetUrl = binAsset ? binAsset.browser_download_url : 'https://revelrob.github.io/NHLGoalLight/builds/firmware.bin';
            const isBetaRel = rel.prerelease || latestTag.toLowerCase().includes('beta') || latestTag.toLowerCase().includes('rc');

            updateInfo.innerHTML = `<strong>✨ ${isBetaRel ? '🧪 Beta Update' : 'New Update'} Available: ${latestTag}</strong><br>` +
              `Release notes: ${rel.body ? rel.body.substring(0, 150) : 'Performance improvements and fixes'}<br><br>` +
              `<button type="button" id="btn1ClickUpdate" style="width:100%; padding:10px 14px; background:var(--accent); color:#fff; border:none; border-radius:8px; font-weight:700; font-size:0.88rem; cursor:pointer; margin-bottom:8px; display:flex; align-items:center; justify-content:center; gap:6px;">⚡ 1-Click Direct Install (${latestTag})</button>` +
              `<span style="font-size:0.75rem; color:var(--text-muted); display:block; margin-bottom:10px;">The Goal Light will download directly over Wi-Fi and flash itself. No files saved to your phone!</span>` +
              `<details style="font-size:0.78rem; color:var(--text-muted);"><summary style="cursor:pointer; color:#58a6ff;">Manual File Download Options</summary><div style="margin-top:8px;">` +
              `<a href="${assetUrl}" target="_blank" style="display:inline-block; padding:5px 10px; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:6px; text-decoration:none; font-size:0.75rem; margin-right:8px;">📥 Download ${binAsset ? binAsset.name : 'firmware.bin'}</a>` +
              `<a href="${rel.html_url || 'https://github.com/RevelRob/NHLGoalLight'}" target="_blank" style="color:#58a6ff; text-decoration:underline; font-size:0.75rem;">View on GitHub</a>` +
              `</div></details>`;

            const btn1Click = document.getElementById('btn1ClickUpdate');
            if (btn1Click) {
              btn1Click.onclick = () => {
                if (!confirm(`Install ${latestTag} directly on the Goal Light? It will reboot after installation.`)) return;
                btn1Click.disabled = true;
                updateInfo.innerHTML = `<strong>⚡ Installing ${latestTag} directly from Cloud...</strong><br>` +
                  `<span style="font-size:0.82rem; color:var(--text-muted);">Downloading and writing to flash memory. Please wait about 15 seconds...</span><br><br>` +
                  `<div style="background:var(--border); border-radius:6px; height:10px; overflow:hidden;"><div id="cloudBar" style="background:var(--accent); height:100%; width:0%; transition:width 15s linear;"></div></div>`;
                setTimeout(() => {
                  const cb = document.getElementById('cloudBar');
                  if (cb) cb.style.width = '100%';
                }, 100);

                fetch('/api/ota-cloud', {
                  method: 'POST',
                  headers: { 'Content-Type': 'application/json' },
                  body: JSON.stringify({ url: assetUrl })
                }).then(() => {
                  let count = 16;
                  const t = setInterval(() => {
                    count--;
                    if (count <= 0) {
                      clearInterval(t);
                      window.location.reload();
                    }
                  }, 1000);
                }).catch(err => {
                  alert('Cloud OTA error: ' + err);
                });
              };
            }
          } else {
            updateInfo.style.borderColor = 'var(--success)';
            updateInfo.style.background = 'rgba(46,160,67,0.15)';
            if (cmp < 0) {
              updateInfo.innerHTML = `<strong>✅ Firmware Up to Date!</strong><br><span style="font-size:0.8rem; color:var(--text-muted);">Installed build (${currentTag}) is newer than latest public release (${latestTag}).</span>`;
            } else {
              updateInfo.innerHTML = `<strong>✅ Firmware Up to Date (${currentTag})</strong><br><span style="font-size:0.8rem; color:var(--text-muted);">You are running the latest version.</span>`;
            }
          }
        })
        .catch(err => {
          btnCheckUpdate.textContent = '🔍 Check GitHub';
          updateInfo.style.display = 'block';
          updateInfo.style.borderColor = 'var(--border)';
          updateInfo.style.background = 'rgba(255,255,255,0.05)';
          updateInfo.innerHTML = `No releases published yet on GitHub. Check <a href="https://github.com/RevelRob/NHLGoalLight" target="_blank" style="color:#58a6ff;">RevelRob/NHLGoalLight</a>.`;
        });
    };

    otaFile.onchange = () => {
      btnOtaUpload.style.display = otaFile.files.length > 0 ? 'block' : 'none';
    };

    btnOtaUpload.onclick = () => {
      const file = otaFile.files[0];
      if (!file) return;
      if (!confirm(`Flash ${file.name}? The goal light will reboot after installation.`)) return;

      const formData = new FormData();
      formData.append('update', file);
      const xhr = new XMLHttpRequest();
      xhr.open('POST', '/api/update', true);

      otaProgress.style.display = 'block';
      btnOtaUpload.disabled = true;

      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const pct = Math.round((e.loaded / e.total) * 100);
          otaBar.style.width = pct + '%';
          otaText.textContent = `Installing: ${pct}%`;
        }
      };

      xhr.onload = () => {
        if (xhr.status === 200) {
          otaText.textContent = '✅ Update complete! Rebooting device in 10s...';
          otaBar.style.background = 'var(--success)';
          setTimeout(() => window.location.reload(), 10000);
        } else {
          otaText.textContent = '❌ Update failed: ' + xhr.responseText;
          otaBar.style.background = '#e63946';
          btnOtaUpload.disabled = false;
        }
      };

      xhr.onerror = () => {
        otaText.textContent = '❌ Upload error or connection lost.';
        btnOtaUpload.disabled = false;
      };

      xhr.send(formData);
    };

    // Diagnostics: Reboot Button
    document.getElementById('btnRestartDevice').onclick = () => {
      if (confirm('Reboot the Goal Light now?')) {
        fetch('/api/config', {
          method: 'POST',
          headers: {'Content-Type': 'application/json'},
          body: JSON.stringify({ ssid: savedSsid }) // triggers reboot flag
        }).then(() => {
          alert('Goal Light is rebooting...');
          setTimeout(() => window.location.reload(), 4000);
        });
      }
    };
  </script>
</body>
</html>
)rawliteral";
