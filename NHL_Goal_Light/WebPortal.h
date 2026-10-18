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
  <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Ccircle cx='50' cy='50' r='46' fill='%23111' stroke='%23333' stroke-width='4'/%3E%3Ccircle cx='50' cy='50' r='38' fill='%23d90429'/%3E%3Cpath d='M25 50h50M50 25v50' stroke='%23fff' stroke-width='8' stroke-linecap='round'/%3E%3Ccircle cx='50' cy='50' r='18' fill='%23fff'/%3E%3Ccircle cx='50' cy='50' r='10' fill='%23d90429'/%3E%3C/svg%3E">

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

    /* Tab Page Container */
    .tab-page {
      display: none;
      flex-direction: column;
      gap: 14px;
    }
    .tab-page.active {
      display: flex;
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

    /* Virtual LED Ring Preview */
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
      width: 160px;
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
      <div class="card">
        <h2>🏒 Game Status</h2>
        <div class="score-card">
          <div class="team-box">
            <span class="team-abbrev" id="liveAwayTeam">---</span>
            <span class="team-score" id="liveAwayScore">-</span>
          </div>
          <div class="score-vs">
            <span id="liveGameClock" class="game-period-pill">No Game</span>
            <span id="liveIntermissionNote" style="font-size:0.7rem; color:var(--accent); font-weight:700; display:none;">INTERMISSION</span>
          </div>
          <div class="team-box">
            <span class="team-abbrev" id="liveHomeTeam">---</span>
            <span class="team-score" id="liveHomeScore">-</span>
          </div>
        </div>
        <div class="helper-text" id="liveStatusSubtext" style="text-align:center;">Synchronised with NHL live scores.</div>
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

      <!-- Quick Controls -->
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

        <!-- Quick Celebration Tests -->
        <div class="btn-row" style="margin-top:14px;">
          <button type="button" id="btnTestCelebration" class="primary">🚨 Test Goal Celebration</button>
        </div>
        <div class="btn-row" style="margin-top:8px;">
          <button type="button" id="btnTestLightsOnly" data-hw="lights">💡 Lights Only</button>
          <button type="button" id="btnTestHornOnly" data-hw="speaker">🔊 Horn Only</button>
        </div>
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
          <label>Live Ring Visualiser Preview</label>
          <div class="ring-preview-container">
            <canvas id="ringCanvas" width="160" height="160"></canvas>
            <div style="font-size:0.75rem; color:var(--text-muted); margin-top:8px;">Live animation in team colour</div>
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
          </select>
          <div class="helper-text">Lighting style when no game event is actively firing.</div>
        </div>
      </div>
    </div>

    <!-- ===================================================== -->
    <!-- TAB 3: EVENTS (GAME EVENT ACTION MATRIX)              -->
    <!-- ===================================================== -->
    <div id="tab-events" class="tab-page">
      <div class="card">
        <h2>⚡ Event Trigger Matrix</h2>
        <div class="helper-text" style="margin-bottom:14px;">Select which game milestones trigger goal light alerts, sounds, and lighting effects.</div>

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
          <span style="font-size:0.85rem; color:var(--text-muted)">Current Version: <strong id="curVer" style="color:var(--text)">v1.1.0.1</strong></span>
          <button type="button" id="btnCheckUpdate" style="padding:6px 12px; font-size:0.8rem; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:8px; cursor:pointer;">🔍 Check GitHub</button>
        </div>
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

      <!-- System Diagnostics & Maintenance -->
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
        } else {
          p.classList.remove('active');
        }
      });
    }

    navTabs.forEach(tab => {
      tab.addEventListener('click', (e) => {
        e.preventDefault();
        const targetTab = tab.getAttribute('data-tab');
        switchTab(targetTab);
      });
    });

    // If loaded in AP Setup Mode (192.168.4.1), default immediately to Settings/System tab
    if (window.location.hostname === '192.168.4.1') {
      switchTab('system');
    }

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

    const volSlider = document.getElementById('volSlider');
    const volVal = document.getElementById('volVal');
    volSlider.oninput = () => volVal.textContent = volSlider.value == 0 ? 'Mute' : 'Level ' + volSlider.value;

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

    // Virtual LED Ring Preview Renderer
    const ringCanvas = document.getElementById('ringCanvas');
    const ringCtx = ringCanvas.getContext('2d');
    let previewStep = 0;

    function getTeamColourHex(abbrev) {
      const t = TEAMS.find(item => item[0] === abbrev);
      return t ? t[2] : '#e63946';
    }

    function renderVirtualRing() {
      const nPixels = parseInt(document.getElementById('ledPixels').value) || 16;
      const effect = parseInt(document.getElementById('effectSelect').value) || 0;
      const teamHex = getTeamColourHex(teamSelect.value);
      const isReverse = document.getElementById('chkLedReverse').checked;

      const w = ringCanvas.width;
      const h = ringCanvas.height;
      ringCtx.clearRect(0, 0, w, h);

      const cx = w / 2;
      const cy = h / 2;
      const radius = 62;
      const ledRadius = Math.max(3, Math.min(8, 90 / nPixels));

      for (let i = 0; i < nPixels; i++) {
        const mappedIdx = isReverse ? ((nPixels - 1) - i) : i;
        const angle = (mappedIdx / nPixels) * (2 * Math.PI) - (Math.PI / 2);
        const x = cx + radius * Math.cos(angle);
        const y = cy + radius * Math.sin(angle);

        let alpha = 0.15;
        let fill = teamHex;

        if (effect === 0) { // Halogen Beacon
          const dist = Math.min(Math.abs(i - previewStep), Math.abs(i - previewStep + nPixels), Math.abs(i - previewStep - nPixels));
          if (dist === 0) alpha = 1.0;
          else if (dist === 1) alpha = 0.55;
          else if (dist === 2) alpha = 0.2;
          else alpha = 0.05;
        } else if (effect === 1) { // Arena Strobe
          alpha = (previewStep % 2 === 0) ? 1.0 : 0.05;
          fill = (previewStep % 4 === 0) ? '#ffffff' : teamHex;
        } else if (effect === 2) { // Breathe
          alpha = (Math.sin(previewStep * 0.15) + 1.0) / 2.0 * 0.9 + 0.1;
        } else if (effect === 3) { // Dual Siren
          const half = Math.floor(nPixels / 2);
          const isLeft = (i < half);
          const sideA = ((previewStep % 12) < 6);
          const flashOn = (previewStep % 2 === 0);
          if (flashOn && ((sideA && isLeft) || (!sideA && !isLeft))) {
            alpha = 1.0;
            fill = isLeft ? teamHex : '#ffffff';
          } else {
            alpha = 0.08;
          }
        } else if (effect === 4) { // Heartbeat
          const sub = previewStep % 16;
          alpha = (sub === 0 || sub === 1 || sub === 4 || sub === 5) ? 1.0 : 0.1;
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
      } else if (wifiSelect.value === '__manual__') {
        wifiSsid.focus();
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

    // Live Game & Status Polling
    function refreshStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(s => {
          const badge = document.getElementById('statusBadge');
          if (s.system && s.system.deviceName) {
            document.getElementById('titleDeviceName').textContent = '🚨 ' + s.system.deviceName;
          }

          if (s.game && s.game.state === 'LIVE') {
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
          } else if (s.system && s.system.connected) {
            badge.className = 'badge';
            badge.textContent = `Online (${s.system.team})`;
            document.getElementById('liveGameClock').textContent = 'Final / Off';
            document.getElementById('syncNhlClock').textContent = '--:--';
            document.getElementById('syncNhlPeriod').textContent = 'No Live Game Right Now';
          } else {
            badge.className = 'badge';
            badge.textContent = 'AP Setup Mode (192.168.4.1)';
            document.getElementById('liveGameClock').textContent = 'Setup Mode';
          }

          if (s.system) {
            if (s.system.version) document.getElementById('curVer').textContent = 'v' + s.system.version;
            if (s.system.freeHeap) document.getElementById('diagHeap').textContent = `${Math.round(s.system.freeHeap / 1024)} KB`;
            document.getElementById('diagIp').textContent = window.location.hostname;
            if (s.system.hostname) {
              const lbl = document.getElementById('lblMdnsUrl');
              if (lbl) lbl.textContent = 'http://' + s.system.hostname;
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
        const val = this.value.trim();
        const slug = val ? val.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '') : 'goallight';
        const lbl = document.getElementById('lblMdnsUrl');
        if (lbl) lbl.textContent = 'http://' + (slug || 'goallight') + '.local';
      };
    }

    // Test Celebration Buttons (transmits active effect immediately)
    document.getElementById('btnTestCelebration').onclick = () => {
      const effectVal = parseInt(document.getElementById('effectSelect').value) || 0;
      fetch('/api/test', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({horn: true, light: true, effect: effectVal})
      });
    };
    document.getElementById('btnTestLightsOnly').onclick = () => {
      const effectVal = parseInt(document.getElementById('effectSelect').value) || 0;
      fetch('/api/test', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({horn: false, light: true, effect: effectVal})
      });
    };
    document.getElementById('btnTestHornOnly').onclick = () => {
      fetch('/api/test', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({horn: true, light: false})
      });
    };

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
        wifiSsid.focus();
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

    btnCheckUpdate.onclick = () => {
      btnCheckUpdate.textContent = 'Checking...';

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

      fetch('https://api.github.com/repos/RevelRob/NHLGoalLight/releases/latest')
        .then(r => {
          if (!r.ok) return checkPagesManifest();
          return r.json();
        })
        .catch(() => checkPagesManifest())
        .then(rel => {
          btnCheckUpdate.textContent = '🔍 Check GitHub';
          const latestTag = rel.tag_name || rel.name;
          const currentTag = document.getElementById('curVer').textContent;
          updateInfo.style.display = 'block';
          if (latestTag !== currentTag && latestTag) {
            updateInfo.style.borderColor = '#e63946';
            updateInfo.style.background = 'rgba(230,57,70,0.15)';
            let binAsset = rel.assets && rel.assets.find(a => a.name.endsWith('.bin'));
            let assetUrl = binAsset ? binAsset.browser_download_url : rel.html_url;
            updateInfo.innerHTML = `<strong>✨ Update Available: ${latestTag}</strong><br>` +
              `Release notes: ${rel.body ? rel.body.substring(0, 150) : 'Performance improvements and fixes'}<br><br>` +
              `<button type="button" id="btn1ClickUpdate" style="width:100%; padding:10px 14px; background:var(--accent); color:#fff; border:none; border-radius:8px; font-weight:700; font-size:0.88rem; cursor:pointer; margin-bottom:8px; display:flex; align-items:center; justify-content:center; gap:6px;">⚡ 1-Click Direct Install (${latestTag})</button>` +
              `<span style="font-size:0.75rem; color:var(--text-muted); display:block; margin-bottom:10px;">The Goal Light will download directly over Wi-Fi and flash itself. No files saved to your phone!</span>` +
              `<details style="font-size:0.78rem; color:var(--text-muted);"><summary style="cursor:pointer; color:#58a6ff;">Manual File Download Options</summary><div style="margin-top:8px;">` +
              `<a href="${assetUrl}" target="_blank" style="display:inline-block; padding:5px 10px; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:6px; text-decoration:none; font-size:0.75rem; margin-right:8px;">📥 Download ${binAsset ? binAsset.name : 'firmware.bin'}</a>` +
              `<a href="${rel.html_url}" target="_blank" style="color:#58a6ff; text-decoration:underline; font-size:0.75rem;">View on GitHub</a>` +
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
            updateInfo.innerHTML = `<strong>✅ Firmware Up to Date (${currentTag})</strong>`;
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
