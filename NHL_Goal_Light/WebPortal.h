#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "Config.h"
#include "Teams.h"

const char INDEX_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no, viewport-fit=cover">
<title>NHL Goal Light</title>
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="apple-mobile-web-app-title" content="NHL Goal Light">
<link rel="apple-touch-icon" href="/apple-touch-icon.png">
<link rel="apple-touch-icon-precomposed" href="/apple-touch-icon-precomposed.png">
<link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png">
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
.tab-page.just-activated * {
pointer-events: none !important;
}
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
#ringCanvas, .event-canvas {
width: 220px;
height: 140px;
}
.last-play-box {
background: #11141a;
border: 1px solid var(--border);
border-radius: 12px;
padding: 12px 14px;
margin-bottom: 12px;
}
.last-play-title {
font-size: 0.72rem;
font-weight: 700;
color: var(--text-muted);
text-transform: uppercase;
letter-spacing: 0.6px;
margin-bottom: 4px;
display: flex;
justify-content: space-between;
align-items: center;
}
.last-play-text {
font-size: 0.95rem;
font-weight: 700;
color: var(--text);
line-height: 1.35;
}
.sync-clock-grid {
display: grid;
grid-template-columns: 1fr 1fr;
gap: 10px;
margin-bottom: 12px;
}
.sync-clock-card {
background: #11141a;
border: 1px solid var(--border);
border-radius: 10px;
padding: 12px 10px;
text-align: center;
}
.sync-clock-label {
font-size: 0.7rem;
color: var(--text-muted);
text-transform: uppercase;
letter-spacing: 0.5px;
font-weight: 700;
}
.sync-clock-time {
font-size: 1.45rem;
font-weight: 800;
color: var(--text);
margin: 4px 0 2px 0;
}
.sync-clock-sub {
font-size: 0.72rem;
color: var(--text-muted);
}
.btn-toggle-drawer {
background: rgba(255,255,255,0.04);
border: 1px solid var(--border);
border-radius: 6px;
padding: 6px 10px;
font-size: 0.75rem;
font-weight: 600;
color: var(--text-muted);
cursor: pointer;
display: inline-flex;
align-items: center;
justify-content: space-between;
width: 100%;
margin-top: 8px;
transition: all 0.2s;
}
.btn-toggle-drawer:hover {
background: var(--btn-bg);
color: var(--text);
border-color: rgba(255,255,255,0.15);
}
.btn-toggle-drawer.active {
background: rgba(230,57,70,0.12);
border-color: rgba(230,57,70,0.3);
color: var(--text);
}
.event-drawer {
margin-top: 8px;
animation: fadeInDrawer 0.25s ease-out;
}
@keyframes fadeInDrawer {
from { opacity: 0; transform: translateY(-4px); }
to { opacity: 1; transform: translateY(0); }
}
.drawer-caption {
font-size: 0.74rem;
color: var(--text-muted);
margin-top: 6px;
text-align: center;
}
.pinout-table {
width: 100%;
border-collapse: collapse;
font-size: 0.76rem;
margin-top: 8px;
}
.pinout-table th, .pinout-table td {
padding: 6px 8px;
border: 1px solid var(--border);
text-align: left;
}
.pinout-table th {
background: #11141a;
color: var(--text-muted);
text-transform: uppercase;
font-weight: 700;
font-size: 0.68rem;
}
.pinout-table td code {
color: var(--accent);
font-weight: 700;
}
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
[data-hw-hidden="true"] {
display: none !important;
}
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
<div id="tab-live" class="tab-page active">
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
<div class="card" id="cardLiveSync">
<h2>⏱️ Broadcast Synchronisation</h2>
<div class="last-play-box">
<div class="last-play-title">
<span>🏒 Last Official NHL Play</span>
<span style="font-size:0.68rem; color:#58a6ff; font-weight:600;">Broadcast Sync Reference</span>
</div>
<div class="last-play-text" id="liveLastPlay">⏳ Waiting for game action / puck drop...</div>
<div id="livePlayTiming" style="display:flex; justify-content:space-between; align-items:center; margin-top:8px; font-size:0.75rem; color:var(--text-muted);">
<span>📡 Detected: <strong id="livePlayDetectedTime" style="color:var(--text);">--:--:--</strong> (<span id="livePlayElapsedSec">0</span>s ago)</span>
<span id="livePlayFreshness" style="color:#58a6ff; font-weight:600;">● Live Cloud Feed</span>
</div>
<button type="button" id="btnLiveSyncToPlay" class="primary" style="margin-top:10px; width:100%; padding:9px 12px; font-size:0.85rem; font-weight:700; border-radius:8px; display:none;" onclick="syncToLastPlay()">
🎯 Sync Delay to This Play (<span id="livePlayDelayBtnTxt">0s</span>)
</button>
<div id="liveRecentPlays" style="margin-top:8px; display:flex; flex-direction:column; gap:5px;"></div>
<div class="helper-text" style="margin-top:6px;">When you see this play or whistle occur on your television, tap the button above to match your broadcast delay instantly!</div>
</div>
<div class="sync-clock-grid">
<div class="sync-clock-card">
<div class="sync-clock-label">Official NHL Clock</div>
<div class="sync-clock-time" id="liveOfficialClock">--:--</div>
<div class="sync-clock-sub" id="liveOfficialPeriod">Period --</div>
</div>
<div class="sync-clock-card" style="border-color:rgba(230,57,70,0.3); background:rgba(230,57,70,0.06);">
<div class="sync-clock-label" style="color:var(--accent);">Target TV Clock</div>
<div class="sync-clock-time" id="liveTargetTvClock" style="color:var(--accent);">--:--</div>
<div class="sync-clock-sub" id="liveDelayBadge">Delay: 0s Buffer</div>
</div>
</div>
<div class="form-group">
<label>Broadcast Delay Buffer: <strong id="liveDelayVal" style="color:var(--accent);">0s (Instantaneous)</strong></label>
<div class="slider-container">
<input type="range" id="liveDelaySlider" min="0" max="120" value="0" step="1">
</div>
</div>
<div class="btn-row" style="margin-top:8px; margin-bottom:8px;">
<button type="button" onclick="quickNudgeDelay(-5)">⏪ -5s</button>
<button type="button" onclick="quickNudgeDelay(-1)">◀️ -1s</button>
<button type="button" onclick="quickNudgeDelay(1)">▶️ +1s</button>
<button type="button" onclick="quickNudgeDelay(5)">⏩ +5s</button>
</div>
<div style="background:#11141a; border-radius:8px; border:1px solid var(--border); padding:10px 12px; margin-top:8px; margin-bottom:8px;">
<div style="font-size:0.8rem; font-weight:700; color:var(--text); margin-bottom:4px;">📺 Quick TV Scoreboard Matcher</div>
<div class="helper-text" style="margin-bottom:6px;">Type the clock from your TV screen:</div>
<div style="display:flex; gap:6px; align-items:center;">
<input type="number" id="liveTvMin" min="0" max="20" placeholder="MM" style="width:60px; text-align:center; font-size:1rem; font-weight:700; padding:6px; border-radius:6px; border:1px solid var(--border); background:#161b22; color:var(--text);">
<span style="font-weight:700; color:var(--text-muted);">:</span>
<input type="number" id="liveTvSec" min="0" max="59" placeholder="SS" style="width:60px; text-align:center; font-size:1rem; font-weight:700; padding:6px; border-radius:6px; border:1px solid var(--border); background:#161b22; color:var(--text);">
<button type="button" class="primary" style="padding:6px 14px; font-weight:700; border-radius:6px; flex:1;" onclick="applyTvScoreboardMatch('live')">🎯 Set Delay</button>
</div>
<div id="liveTvMatchResult" style="font-size:0.75rem; text-align:center; margin-top:4px; min-height:16px;"></div>
</div>
<button type="button" id="btnWhistleSyncLive" class="primary" style="margin-top:8px; margin-bottom:4px; width:100%; padding:10px 14px; font-size:0.88rem; font-weight:700; border-radius:8px; background:linear-gradient(135deg, #1f6feb 0%, #1158c7 100%); border:1px solid #388bfd; cursor:pointer;" onclick="syncToWhistleNow()">
🔔 I See a Whistle on TV Right Now! (<span class="whistleElapsedTxt">0s</span>)
</button>
<div class="helper-text" style="margin-bottom:10px; text-align:center;">One-tap sync: tap the second you see or hear a whistle on your TV!</div>
<label style="margin-top:8px;">Quick Broadcaster Presets:</label>
<div class="preset-row">
<button type="button" class="preset-btn" onclick="quickSetPreset(0)">⚡ Cable / OTA (0s)</button>
<button type="button" class="preset-btn" onclick="quickSetPreset(22)">🍁 Sportsnet+ (22s)</button>
<button type="button" class="preset-btn" onclick="quickSetPreset(28)">🏒 NHL.tv / ESPN+ (28s)</button>
<button type="button" class="preset-btn" onclick="quickSetPreset(45)">📺 YouTube TV (45s)</button>
</div>
<div class="helper-text" style="margin-top:8px;">⚡ Adjusting delay updates your goal light buffer immediately without saving or rebooting.</div>
</div>
<div class="card" id="cardUpcomingGame">
<h2>
<span>📅 Upcoming Game</span>
<div class="view-toggle">
<button type="button" class="view-toggle-btn active" id="btnViewSimple" onclick="setUpcomingView('simple')">Simple</button>
<button type="button" class="view-toggle-btn" id="btnViewDetailed" onclick="setUpcomingView('detailed')">Detailed</button>
</div>
</h2>
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
<div class="card">
<h2>⭐ Favourite Team</h2>
<div class="form-group">
<label for="teamSelect">Tracked NHL Franchise</label>
<select id="teamSelect"></select>
<div class="helper-text">Goal celebrations and ambient lighting automatically adapt to this team's official colours.</div>
</div>
</div>
<div class="card">
<h2>🎛️ Quick Levels</h2>
<div class="form-group" data-hw="lights">
<label>Light Brightness: <span id="brightVal">70%</span></label>
<div class="slider-container">
<input type="range" id="brightSlider" min="10" max="255" value="180">
</div>
</div>
<div class="form-group" data-hw="speaker">
<label>Speaker Volume: <span id="volVal">Level 3</span></label>
<div class="slider-container">
<input type="range" id="volSlider" min="0" max="5" value="3">
</div>
</div>
<div class="helper-text" style="margin-top:10px;">⚡ Levels update immediately. Adjusting volume plays an instant acoustic sound chirp.</div>
</div>
</div>
<div id="tab-lights" class="tab-page">
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
<div class="helper-text">Physical orientation and structure of your addressable LEDs.</div>
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
<div class="helper-text">Lighting style when no game event is actively firing. Selecting <strong>Dark / Off</strong> keeps all LEDs turned completely off during idle.</div>
</div>
</div>
<div class="card">
<h2>📐 Hardware Geometry & Idle Preview</h2>
<div class="ring-preview-container">
<canvas id="ringCanvas" width="220" height="140"></canvas>
<div id="previewCaption" style="font-size:0.75rem; color:var(--text-muted); margin-top:8px;">Hardware Geometry Preview</div>
</div>
<div class="helper-text" style="text-align:center; margin-top:8px;">Shows the active physical layout mapping and ambient idle behaviour.</div>
</div>
</div>
<div id="tab-events" class="tab-page">
<div class="helper-text" style="margin-bottom:4px;">Configure actions and visual effects for every game event. Tap any visualiser drawer to preview its celebration effect.</div>
<div class="card">
<h2>
<span>🚨 Favourite Team Goal</span>
<span style="font-size:0.72rem; color:var(--success); font-weight:700;">Always Active</span>
</h2>
<div class="form-group">
<label for="effectSelect">Celebration Light Pattern</label>
<select id="effectSelect">
<option value="0">🚨 Rotating Halogen Beacon (Classic Revolving Reflector)</option>
<option value="1">⚡ High-Speed Arena Strobe (Stadium Flash)</option>
<option value="2">🌊 Team Colour Breathe (Pulsing Unison Glow)</option>
<option value="3">🚔 Dual Siren Flasher (Emergency Wig-Wag)</option>
<option value="4">❤️ Heartbeat Pulse (Urgent Goal Surge)</option>
</select>
</div>
<div class="form-group">
<label class="checkbox-row" style="margin:0;">
<input type="checkbox" id="chkGoalClassicRed">
<div class="chk-text">
<span class="chk-title">🔴 Use Classic Stadium Red</span>
<span class="chk-desc">Illuminates in traditional red (typical stadium goal light colour) instead of primary franchise team colours.</span>
</div>
</label>
</div>
<div class="form-group">
<label>Celebration Duration: <span id="hornVal">12s</span></label>
<div class="slider-container">
<input type="range" id="hornSlider" min="5" max="30" value="12">
</div>
<div class="helper-text">Duration of light celebration and goal horn audio per goal.</div>
</div>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('goal')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-goal">▾</span>
</button>
<div class="event-drawer" id="drawer-goal" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-goal" width="220" height="140"></canvas>
<div class="drawer-caption" id="cap-goal">Live Goal Celebration</div>
</div>
</div>
</div>
<div class="card">
<h2>
<span>🥅 Opponent Goal Alert</span>
</h2>
<label class="checkbox-row">
<input type="checkbox" id="chkOpponent">
<div class="chk-text">
<span class="chk-title">Enable Opponent Goal Alert</span>
<span class="chk-desc">Triggers alert when the opposing team scores.</span>
</div>
</label>
<label class="checkbox-row" data-hw="speaker">
<input type="checkbox" id="chkOpponentSound" checked>
<div class="chk-text">
<span class="chk-title">🔊 Warning Audio Tone</span>
<span class="chk-desc">Play acoustic warning buzzer when opponent scores.</span>
</div>
</label>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('opponent')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-opponent">▾</span>
</button>
<div class="event-drawer" id="drawer-opponent" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-opponent" width="220" height="140"></canvas>
<div class="drawer-caption">Opponent Goal • Amber Warning Strobe</div>
</div>
</div>
</div>
<div class="card">
<h2>
<span>⚡ Power Play Awarded</span>
</h2>
<label class="checkbox-row">
<input type="checkbox" id="chkPowerPlay" checked>
<div class="chk-text">
<span class="chk-title">Enable Power Play Alert</span>
<span class="chk-desc">Vibrant gold chase animation when your team gets the man advantage.</span>
</div>
</label>
<label class="checkbox-row" data-hw="speaker">
<input type="checkbox" id="chkPowerPlaySound" checked>
<div class="chk-text">
<span class="chk-title">🔊 Arena Charge Chime</span>
<span class="chk-desc">Play uplifting stadium charge chime for power play.</span>
</div>
</label>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('powerplay')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-powerplay">▾</span>
</button>
<div class="event-drawer" id="drawer-powerplay" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-powerplay" width="220" height="140"></canvas>
<div class="drawer-caption">Power Play • Golden Man-Advantage Chase</div>
</div>
</div>
</div>
<div class="card">
<h2>
<span>🚨 Period End & Start Buzzer</span>
</h2>
<label class="checkbox-row">
<input type="checkbox" id="chkBuzzer" checked>
<div class="chk-text">
<span class="chk-title">Enable Period Buzzer Alert</span>
<span class="chk-desc">Flashes arena lighting at the conclusion of each period.</span>
</div>
</label>
<label class="checkbox-row" data-hw="speaker">
<input type="checkbox" id="chkBuzzerSound" checked>
<div class="chk-text">
<span class="chk-title">🔊 Authentic Arena Buzzer (120Hz/360Hz)</span>
<span class="chk-desc">Sounds authentic harsh arena horn at period end.</span>
</div>
</label>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('buzzer')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-buzzer">▾</span>
</button>
<div class="event-drawer" id="drawer-buzzer" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-buzzer" width="220" height="140"></canvas>
<div class="drawer-caption">Period Buzzer • Arena Strobe</div>
</div>
</div>
</div>
<div class="card" data-hw="lights">
<h2>
<span>☕ Relaxed Intermission Glow</span>
</h2>
<label class="checkbox-row">
<input type="checkbox" id="chkIntermission" checked>
<div class="chk-text">
<span class="chk-title">Enable Intermission Glow</span>
<span class="chk-desc">Transitions to calm ambient team colour lighting during the 18-minute break.</span>
</div>
</label>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('intermission')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-intermission">▾</span>
</button>
<div class="event-drawer" id="drawer-intermission" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-intermission" width="220" height="140"></canvas>
<div class="drawer-caption">Intermission • Calm Ambient Team Glow</div>
</div>
</div>
</div>
<div class="card">
<h2>
<span>🏆 Game Victory Fanfare</span>
</h2>
<label class="checkbox-row">
<input type="checkbox" id="chkWin" checked>
<div class="chk-text">
<span class="chk-title">Enable Victory Fanfare</span>
<span class="chk-desc">Rainbow light celebration when your favourite team wins!</span>
</div>
</label>
<label class="checkbox-row" data-hw="speaker">
<input type="checkbox" id="chkWinSound" checked>
<div class="chk-text">
<span class="chk-title">🔊 Victory Horn & Fanfare</span>
<span class="chk-desc">Play celebratory championship fanfare audio.</span>
</div>
</label>
<button type="button" class="btn-toggle-drawer" onclick="toggleEventDrawer('win')">
<span>👁️ Visualiser Preview</span>
<span id="arrow-win">▾</span>
</button>
<div class="event-drawer" id="drawer-win" style="display:none;">
<div class="ring-preview-container">
<canvas class="event-canvas" id="canvas-win" width="220" height="140"></canvas>
<div class="drawer-caption">Game Victory • Championship Rainbow Fanfare</div>
</div>
</div>
</div>
<div class="card">
<h2>🧪 Event Simulator Studio</h2>
<div class="helper-text" style="margin-bottom:14px;">Test any game event or celebration on your physical goal light in real time.</div>
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
</div>
<div id="tab-sync" class="tab-page">
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
<div class="card">
<h2>📺 Game Clock Matcher</h2>
<div class="helper-text" style="margin-bottom:12px;">Compare the live official NHL API game clock with your screen to dial in your synchronisation:</div>
<div class="sync-clock-grid" style="margin-bottom:12px;">
<div class="sync-clock-card">
<div class="sync-clock-label">Official NHL Clock</div>
<div class="sync-clock-time" id="syncNhlClock">--:--</div>
<div class="sync-clock-sub" id="syncNhlPeriod">Period --</div>
</div>
<div class="sync-clock-card" style="border-color:rgba(230,57,70,0.3); background:rgba(230,57,70,0.06);">
<div class="sync-clock-label" style="color:var(--accent);">Target TV Clock</div>
<div class="sync-clock-time" id="syncTargetClock" style="color:var(--accent);">--:--</div>
<div class="sync-clock-sub" id="syncDelayBadge">Delay: 0s Buffer</div>
</div>
</div>
<div class="btn-row" style="margin-bottom:12px;">
<button type="button" onclick="nudgeDelay(-5)">⏪ -5s</button>
<button type="button" onclick="nudgeDelay(-1)">◀️ -1s</button>
<button type="button" onclick="nudgeDelay(1)">▶️ +1s</button>
<button type="button" onclick="nudgeDelay(5)">⏩ +5s</button>
</div>
<div style="background:#11141a; border-radius:10px; border:1px solid var(--border); padding:14px; margin-bottom:12px;">
<div style="font-size:0.85rem; font-weight:700; color:var(--text); margin-bottom:6px;">🔢 Direct Scoreboard Clock Match</div>
<div class="helper-text" style="margin-bottom:10px;">Enter the exact time currently displayed on your TV scoreboard:</div>
<div style="display:flex; gap:8px; align-items:center; justify-content:center; margin-bottom:8px;">
<input type="number" id="tvMatchMinutes" min="0" max="20" placeholder="MM" style="width:72px; text-align:center; font-size:1.15rem; font-weight:800; padding:8px; border-radius:8px; border:1px solid var(--border); background:#161b22; color:var(--text);">
<span style="font-size:1.3rem; font-weight:800; color:var(--text-muted);">:</span>
<input type="number" id="tvMatchSeconds" min="0" max="59" placeholder="SS" style="width:72px; text-align:center; font-size:1.15rem; font-weight:800; padding:8px; border-radius:8px; border:1px solid var(--border); background:#161b22; color:var(--text);">
<button type="button" id="btnApplyTvMatch" class="primary" style="padding:9px 16px; font-weight:700; border-radius:8px;" onclick="applyTvScoreboardMatch('sync')">🎯 Match TV</button>
</div>
<div id="tvMatchResult" style="font-size:0.78rem; text-align:center; min-height:18px;"></div>
<button type="button" id="btnWhistleSyncTab4" class="primary" style="margin-top:10px; width:100%; padding:10px 14px; font-size:0.88rem; font-weight:700; border-radius:8px; background:linear-gradient(135deg, #1f6feb 0%, #1158c7 100%); border:1px solid #388bfd; cursor:pointer;" onclick="syncToWhistleNow()">
🔔 I See a Whistle on TV Right Now! (<span class="whistleElapsedTxt">0s</span>)
</button>
<div class="helper-text" style="margin-top:4px; text-align:center;">One-tap sync: tap the second you see or hear a whistle on your TV!</div>
</div>
<div class="last-play-box" style="margin-bottom:4px;">
<div class="last-play-title">
<span>🏒 Last Official NHL Play</span>
<span style="font-size:0.68rem; color:#58a6ff; font-weight:600;">Arena Whistle Anchor</span>
</div>
<div class="last-play-text" id="syncLastPlayText">⏳ Waiting for live game action / puck drop...</div>
<div style="display:flex; justify-content:space-between; align-items:center; margin-top:8px; font-size:0.75rem; color:var(--text-muted);">
<span>📡 Detected: <strong id="syncPlayDetectedTime" style="color:var(--text);">--:--:--</strong> (<span id="syncPlayElapsedSec">0</span>s ago)</span>
<span id="syncPlayStatusPill" style="color:#58a6ff; font-weight:600;">● Cloud Live</span>
</div>
<button type="button" id="btnSyncToPlay" class="primary" style="margin-top:10px; width:100%; padding:10px 14px; font-size:0.85rem; font-weight:700; border-radius:8px; display:none;" onclick="syncToLastPlay()">
🎯 Sync Delay to This Play (<span id="syncPlayDelayBtnTxt">0s</span>)
</button>
<div id="syncRecentPlays" style="margin-top:8px; display:flex; flex-direction:column; gap:5px;"></div>
</div>
</div>
<div class="card">
<h2>🌐 Arena & Device Time Diagnostics</h2>
<div class="helper-text" style="margin-bottom:12px;">Compares the NHL API cloud server timestamps with your Goal Light's internal real-time clock:</div>
<div style="display:grid; grid-template-columns:1fr 1fr; gap:10px; font-size:0.8rem;">
<div style="background:#11141a; padding:10px; border-radius:8px; border:1px solid var(--border);">
<div style="color:var(--text-muted); font-size:0.7rem; text-transform:uppercase;">Goal Light Clock (NTP)</div>
<div id="diagDeviceTime" style="font-weight:700; color:var(--text); margin-top:2px;">--:--:--</div>
</div>
<div style="background:#11141a; padding:10px; border-radius:8px; border:1px solid var(--border);">
<div style="color:var(--text-muted); font-size:0.7rem; text-transform:uppercase;">NHL Server Time (UTC)</div>
<div id="diagApiDate" style="font-weight:700; color:var(--text); margin-top:2px;">--:--:--</div>
</div>
<div style="background:#11141a; padding:10px; border-radius:8px; border:1px solid var(--border);">
<div style="color:var(--text-muted); font-size:0.7rem; text-transform:uppercase;">Cloudflare Cache Age</div>
<div id="diagCacheAge" style="font-weight:700; color:var(--text); margin-top:2px;">-- s</div>
</div>
<div style="background:#11141a; padding:10px; border-radius:8px; border:1px solid var(--border);">
<div style="color:var(--text-muted); font-size:0.7rem; text-transform:uppercase;">Last Play Latency</div>
<div id="diagPlayLag" style="font-weight:700; color:#58a6ff; margin-top:2px;">-- s ago</div>
</div>
<button type="button" class="btn-subtle" style="margin-top:12px; width:100%; padding:9px 12px; font-size:0.8rem; font-weight:600; border-radius:6px; background:#161b22; border:1px solid var(--border); color:var(--text); cursor:pointer;" onclick="copyLiveFeedDebug()">
📋 Copy Live Feed Data (JSON) to Clipboard
</button>
</div>
</div>
</div>
<div id="tab-system" class="tab-page">
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
<div class="card">
<h2>🏷️ Device Customisation</h2>
<div class="form-group">
<label for="deviceNameInput">Goal Light Name</label>
<input type="text" id="deviceNameInput" placeholder="NHL Goal Light" maxlength="32" autocomplete="off" autocorrect="off" autocapitalize="words" spellcheck="false">
<div class="helper-text">Custom name displayed across your web portal, home screen icon, and local mDNS address (<code id="lblMdnsUrl" style="color:var(--accent);">http://goallight.local</code>).</div>
</div>
</div>
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
<span class="chk-title">🔊 MAX98357A I2S Audio (Speaker)</span>
<span class="chk-desc">High-fidelity I2S DAC: synthesizes authentic horns, buzzers & fanfares (with or without MicroSD module).</span>
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
<div class="card">
<h2>📌 ESP32-S3 Super Mini Pinout Reference</h2>
<div class="helper-text" style="margin-bottom:10px;">Recommended wiring connections for your goal light components:</div>
<table class="pinout-table">
<thead>
<tr><th>Pin</th><th>Module / Function</th><th>Connection</th></tr>
</thead>
<tbody>
<tr><td><code>GPIO 1</code></td><td>MAX98357A I2S BCLK</td><td>Bit Clock</td></tr>
<tr><td><code>GPIO 2</code></td><td>MAX98357A I2S LRC</td><td>Word Select (LRC)</td></tr>
<tr><td><code>GPIO 3</code></td><td>MAX98357A I2S DIN</td><td>Audio Data In</td></tr>
<tr><td><code>GPIO 4</code></td><td>WS2812B NeoPixel DIN</td><td>LED Ring Signal</td></tr>
<tr><td><code>GPIO 8</code></td><td>Onboard Status LED</td><td>Wi-Fi / Activity</td></tr>
<tr><td><code>GPIO 9</code></td><td>Tactile Pushbutton</td><td>Manual Goal Trigger</td></tr>
<tr><td><code>GPIO 10-13</code></td><td>MicroSD SPI (Optional)</td><td>CS (10), MOSI (11), SCK (12), MISO (13)</td></tr>
<tr><td><code>5V / GND</code></td><td>Power Rail</td><td>MAX98357A Vin & LED Ring VDD</td></tr>
</tbody>
</table>
<div class="helper-text" style="margin-top:8px;">💡 <strong>Note:</strong> Real-time audio synthesis runs natively via I2S without needing a MicroSD card. Connecting an optional MicroSD module allows custom team WAV horn clips.</div>
</div>
<div class="card">
<h2>🔄 Firmware & Updates</h2>
<div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
<span style="font-size:0.85rem; color:var(--text-muted)">Current Version: <strong id="curVer" style="color:var(--text)">v1.1.0.7-beta</strong></span>
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
<div class="card">
<h2>🛠️ Diagnostics & Maintenance</h2>
<div style="font-size:0.82rem; color:var(--text-muted); display:flex; flex-direction:column; gap:6px; margin-bottom:12px;">
<div>Device IP: <strong id="diagIp" style="color:var(--text)">--</strong></div>
<div>Free Memory: <strong id="diagHeap" style="color:var(--text)">--</strong></div>
</div>
<button type="button" id="btnRestartDevice" style="background:rgba(230,57,70,0.15); border:1px solid var(--accent); color:#fff;">🔄 Reboot Goal Light</button>
</div>
</div>
</div>
<div class="fixed-save-bar">
<div class="save-inner">
<button type="button" id="btnSave" class="primary" style="padding:14px; font-size:0.95rem;">💾 Save & Apply Settings</button>
</div>
</div>
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
// Live Delay Controls (Live Tab & Sync Tab Synchronised)
const liveDelaySlider = document.getElementById('liveDelaySlider');
const liveDelayVal = document.getElementById('liveDelayVal');
const delaySlider = document.getElementById('delaySlider');
const delayVal = document.getElementById('delayVal');
let liveClockState = {
active: false,
period: 1,
officialSeconds: 0,
running: false,
inIntermission: false,
delaySec: 0,
stoppageSeconds: 0,
stoppageElapsed: 9999,
resumeElapsed: 9999,
lastSyncTime: Date.now()
};
function parseClockSeconds(str) {
if (!str || str === '--:--' || str.toLowerCase().indexOf('intermission') >= 0) return 0;
const parts = str.split(':');
if (parts.length === 2) {
const m = parseInt(parts[0]) || 0;
const s = parseInt(parts[1]) || 0;
return (m * 60) + s;
}
return 0;
}
function updateClientClocks() {
if (!liveClockState.active) return;
const now = Date.now();
const elapsedSinceSync = Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
const D = liveClockState.delaySec;

if (liveClockState.inIntermission) {
const offClockEl = document.getElementById('liveOfficialClock');
const syncNhlEl = document.getElementById('syncNhlClock');
const gameClockEl = document.getElementById('liveGameClock');
const targetEl = document.getElementById('liveTargetTvClock');
const syncTargetEl = document.getElementById('syncTargetClock');
const badgeEl = document.getElementById('liveDelayBadge');
const syncBadgeEl = document.getElementById('syncDelayBadge');
let intermText = 'Intermission';
let targetIntermText = 'Intermission';
let badgeDetail = '☕ Intermission in Progress';
if (liveClockState.officialSeconds > 0) {
const remSec = Math.max(0, liveClockState.officialSeconds - elapsedSinceSync);
const iM = Math.floor(remSec / 60);
const iS = remSec % 60;
const iStr = `${iM}:${iS < 10 ? '0' : ''}${iS}`;
intermText = `INT ${iStr}`;
const targetRemSec = remSec + D;
const tM = Math.floor(targetRemSec / 60);
const tS = targetRemSec % 60;
const tStr = `${tM}:${tS < 10 ? '0' : ''}${tS}`;
targetIntermText = `INT ${tStr}`;
badgeDetail = `☕ Intermission (${iStr} remaining)`;
if (gameClockEl) gameClockEl.textContent = (D > 0) ? `P${liveClockState.period} INT (${tStr})` : `P${liveClockState.period} INT (${iStr})`;
} else {
if (gameClockEl) gameClockEl.textContent = `P${liveClockState.period} Intermission`;
}
if (offClockEl) offClockEl.textContent = intermText;
if (syncNhlEl) syncNhlEl.textContent = intermText;
if (targetEl) targetEl.textContent = targetIntermText;
if (syncTargetEl) syncTargetEl.textContent = targetIntermText;
if (badgeEl) badgeEl.textContent = badgeDetail;
if (syncBadgeEl) syncBadgeEl.textContent = badgeDetail;
return;
}

// Compute current official clock
let curOfficialSec = liveClockState.officialSeconds;
if (liveClockState.running) {
curOfficialSec = Math.max(0, liveClockState.officialSeconds - elapsedSinceSync);
}
// Format Official Arena Clock: MM:SS
const offM = Math.floor(curOfficialSec / 60);
const offS = curOfficialSec % 60;
const offStr = `${offM}:${offS < 10 ? '0' : ''}${offS}`;
// Update Official Clock in DOM
const offClockEl = document.getElementById('liveOfficialClock');
if (offClockEl) offClockEl.textContent = offStr;
const syncNhlEl = document.getElementById('syncNhlClock');
if (syncNhlEl) syncNhlEl.textContent = offStr;

// VIDEO-BUFFER DELAY STATE MACHINE FOR TARGET TV CLOCK:
const stopElapsed = (typeof liveClockState.stoppageElapsed === 'number') ? (liveClockState.stoppageElapsed + elapsedSinceSync) : 9999;
const resumeElapsed = (typeof liveClockState.resumeElapsed === 'number') ? (liveClockState.resumeElapsed + elapsedSinceSync) : 9999;
const stopSec = (typeof liveClockState.stoppageSeconds === 'number' && liveClockState.stoppageSeconds >= 0) ? liveClockState.stoppageSeconds : curOfficialSec;

let targetSec = curOfficialSec;
let tvStatusSub = '';
if (D <= 0) {
targetSec = curOfficialSec;
tvStatusSub = '⚡ Delay: 0s (Instantaneous)';
} else if (!liveClockState.running) {
// Arena is currently STOPPED at whistle time stopSec
if (stopElapsed < D) {
// Whistle blew at arena, but play is STILL COUNTING DOWN on TV!
targetSec = stopSec + (D - stopElapsed);
tvStatusSub = `⏳ Whistle in ${D - stopElapsed}s on TV • Delay: ${D}s`;
} else {
// Whistle has reached TV! TV clock is frozen at whistle time
targetSec = stopSec;
tvStatusSub = `⏸️ TV Stopped by Whistle • Delay: ${D}s`;
}
} else {
// Arena is currently RUNNING
if (resumeElapsed < D) {
// Puck dropped at arena, but TV is STILL STOPPED in commercial / pre-faceoff!
targetSec = stopSec;
tvStatusSub = `⏳ Faceoff in ${D - resumeElapsed}s on TV • Delay: ${D}s`;
} else {
// Puck has dropped on TV! Active play on TV
targetSec = curOfficialSec + D;
tvStatusSub = `🟢 Active TV Play • Delay: ${D}s`;
}
}

const maxSec = (liveClockState.period > 3) ? 300 : 1200;
if (targetSec > maxSec) targetSec = maxSec;
if (targetSec < 0) targetSec = 0;

const tvM = Math.floor(targetSec / 60);
const tvS = targetSec % 60;
const tvStr = `${tvM}:${tvS < 10 ? '0' : ''}${tvS}`;

const targetEl = document.getElementById('liveTargetTvClock');
if (targetEl) targetEl.textContent = tvStr;
const syncTargetEl = document.getElementById('syncTargetClock');
if (syncTargetEl) syncTargetEl.textContent = tvStr;

// Main scoreboard pill: shows target TV clock when delay buffer D > 0
const gameClockEl = document.getElementById('liveGameClock');
if (gameClockEl) gameClockEl.textContent = (D > 0) ? `P${liveClockState.period} ${tvStr}` : `P${liveClockState.period} ${offStr}`;

const badgeEl = document.getElementById('liveDelayBadge');
if (badgeEl) badgeEl.textContent = tvStatusSub;
const syncBadgeEl = document.getElementById('syncDelayBadge');
if (syncBadgeEl) syncBadgeEl.textContent = tvStatusSub;

// Update live whistle sync button label
const whistleSecVal = Math.min(120, Math.max(0, stopElapsed));
document.querySelectorAll('.whistleElapsedTxt').forEach(el => {
el.textContent = `${whistleSecVal}s`;
});

// Update play elapsed timer and sync button labels
if (lastPlayTracker.detectedTime > 0) {
const playElapsed = Math.max(0, Math.floor((now - lastPlayTracker.detectedTime) / 1000));
const el1 = document.getElementById('livePlayElapsedSec');
if (el1) el1.textContent = playElapsed;
const btnTxt1 = document.getElementById('livePlayDelayBtnTxt');
if (btnTxt1) btnTxt1.textContent = playElapsed + 's';
const btn1 = document.getElementById('btnLiveSyncToPlay');
if (btn1) {
btn1.style.display = (playElapsed > 0 && playElapsed <= 120 && liveClockState.active) ? 'block' : 'none';
}
const el2 = document.getElementById('syncPlayElapsedSec');
if (el2) el2.textContent = playElapsed;
const btnTxt2 = document.getElementById('syncPlayDelayBtnTxt');
if (btnTxt2) btnTxt2.textContent = playElapsed + 's';
const btn2 = document.getElementById('btnSyncToPlay');
if (btn2) {
btn2.style.display = (playElapsed > 0 && playElapsed <= 120 && liveClockState.active) ? 'block' : 'none';
}
const diagLag = document.getElementById('diagPlayLag');
if (diagLag) diagLag.textContent = playElapsed + 's ago';
}
if (cachedRecentPlays.length > 0) {
document.querySelectorAll('.recent-elapsed').forEach(el => {
const idx = parseInt(el.getAttribute('data-idx'));
if (cachedRecentPlays[idx]) {
const elSec = (cachedRecentPlays[idx].elapsed || 0) + Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
el.textContent = elSec;
const btn = el.closest('.recent-play-row')?.querySelector('button');
if (btn) btn.textContent = `🎯 Sync (${elSec}s)`;
}
});
}
}
function showToast(msg) {
let t = document.getElementById('portalToast');
if (!t) {
t = document.createElement('div');
t.id = 'portalToast';
t.style.cssText = 'position:fixed; bottom:24px; left:50%; transform:translateX(-50%); background:#1f2937; border:1px solid #374151; color:#fff; padding:10px 18px; border-radius:30px; font-size:0.85rem; font-weight:700; z-index:99999; box-shadow:0 8px 24px rgba(0,0,0,0.5); transition:opacity 0.3s ease; opacity:0; pointer-events:none;';
document.body.appendChild(t);
}
t.textContent = msg;
t.style.opacity = '1';
clearTimeout(t._hideTimeout);
t._hideTimeout = setTimeout(() => { t.style.opacity = '0'; }, 3000);
}
let cachedRecentPlays = [];
function renderRecentPlaysTimeline(plays) {
if (!Array.isArray(plays)) return;
cachedRecentPlays = plays;
const containers = [document.getElementById('liveRecentPlays'), document.getElementById('syncRecentPlays')];
if (plays.length === 0) {
containers.forEach(c => { if (c) c.innerHTML = ''; });
return;
}
let html = '<div style="font-size:0.7rem; font-weight:700; color:var(--text-muted); text-transform:uppercase; margin-top:6px; margin-bottom:4px;">📜 Recent Arena Events Timeline:</div>';
plays.forEach((p, idx) => {
const esc = (p.desc || '').replace(/</g, '&lt;').replace(/>/g, '&gt;');
html += `<div class="recent-play-row" style="display:flex; justify-content:space-between; align-items:center; background:rgba(255,255,255,0.03); border:1px solid rgba(255,255,255,0.07); border-radius:6px; padding:5px 8px; font-size:0.75rem; gap:6px;">
<div style="flex:1; overflow:hidden; text-overflow:ellipsis; white-space:nowrap;">
<strong style="color:var(--text);">${esc}</strong>
<span style="color:var(--text-muted); font-size:0.68rem; display:block;">Detected <span class="recent-elapsed" data-idx="${idx}">${p.elapsed || 0}</span>s ago</span>
</div>
<button type="button" class="primary" style="padding:4px 8px; font-size:0.7rem; border-radius:4px; font-weight:700; white-space:nowrap;" onclick="syncToRecentPlay(${idx})">🎯 Sync (${p.elapsed || 0}s)</button>
</div>`;
});
containers.forEach(c => { if (c) c.innerHTML = html; });
}
window.syncToRecentPlay = function(idx) {
if (!cachedRecentPlays[idx]) return;
const p = cachedRecentPlays[idx];
const now = Date.now();
const elSec = (p.elapsed || 0) + Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
const targetDelay = Math.min(120, Math.max(0, elSec));
sendQuickDelay(targetDelay);
showToast(`🎯 Broadcast delay set to ${targetDelay}s!`);
};
let lastPlayTracker = {
text: '',
detectedTime: 0
};
window.syncToLastPlay = function() {
if (!lastPlayTracker.detectedTime) return;
const playElapsed = Math.max(0, Math.floor((Date.now() - lastPlayTracker.detectedTime) / 1000));
const targetDelay = Math.min(120, Math.max(0, playElapsed));
sendQuickDelay(targetDelay);
showToast(`🎯 Broadcast delay set to ${targetDelay}s (matches last arena event)!`);
};
window.syncToWhistleNow = function() {
const now = Date.now();
const elapsedSinceSync = Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
const stopElapsed = (typeof liveClockState.stoppageElapsed === 'number') ? (liveClockState.stoppageElapsed + elapsedSinceSync) : 9999;
if (liveClockState.running && liveClockState.resumeElapsed > 20) {
showToast('⚠️ Play is currently active at the arena. Wait for the next whistle on TV to sync!');
return;
}
const targetDelay = Math.min(120, Math.max(0, stopElapsed));
sendQuickDelay(targetDelay);
showToast(`🔔 Whistle matched! Broadcast delay set to ${targetDelay}s.`);
};
function copyToClipboard(text) {
if (navigator.clipboard && window.isSecureContext && navigator.clipboard.writeText) {
return navigator.clipboard.writeText(text);
}
return new Promise((resolve) => {
try {
const ta = document.createElement('textarea');
ta.value = text;
ta.style.position = 'fixed';
ta.style.left = '-9999px';
ta.style.top = '-9999px';
document.body.appendChild(ta);
ta.focus();
ta.select();
const success = document.execCommand('copy');
document.body.removeChild(ta);
if (success) {
resolve();
} else {
prompt('Copy live feed JSON manually (Ctrl+C / Cmd+C):', text);
resolve();
}
} catch (e) {
prompt('Copy live feed JSON manually (Ctrl+C / Cmd+C):', text);
resolve();
}
});
}
window.copyLiveFeedDebug = function() {
fetch('/api/status')
.then(r => r.json())
.then(data => {
const txt = JSON.stringify(data, null, 2);
copyToClipboard(txt)
.then(() => showToast('📋 Live feed JSON copied to clipboard!'))
.catch(() => showToast('📋 Feed output to console.'));
})
.catch(() => showToast('⚠️ Device did not respond. Check connection.'));
};
window.applyTvScoreboardMatch = function(source) {
const isLive = (source === 'live');
const minIn = document.getElementById(isLive ? 'liveTvMin' : 'tvMatchMinutes');
const secIn = document.getElementById(isLive ? 'liveTvSec' : 'tvMatchSeconds');
const resEl = document.getElementById(isLive ? 'liveTvMatchResult' : 'tvMatchResult');
if (!minIn || !secIn) return;
const m = parseInt(minIn.value);
const s = parseInt(secIn.value);
if (isNaN(m) || isNaN(s) || m < 0 || s < 0 || s > 59) {
if (resEl) resEl.innerHTML = '<span style="color:#f87171;">⚠️ Please enter valid MM (0-20) and SS (0-59).</span>';
return;
}
const tvTotalSec = (m * 60) + s;
const now = Date.now();
const elapsedSinceSync = Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
let curOfficial = liveClockState.officialSeconds;
if (liveClockState.running) {
curOfficial = Math.max(0, curOfficial - elapsedSinceSync);
}
const stopSec = (typeof liveClockState.stoppageSeconds === 'number' && liveClockState.stoppageSeconds >= 0) ? liveClockState.stoppageSeconds : curOfficial;
const stopElapsed = (typeof liveClockState.stoppageElapsed === 'number') ? (liveClockState.stoppageElapsed + elapsedSinceSync) : 9999;

let calculatedDelay = 0;
if (!liveClockState.running) {
if (tvTotalSec === stopSec) {
if (resEl) resEl.innerHTML = `<span style="color:#eab308;">ℹ️ Play is currently stopped at ${m}:${s < 10 ? '0' : ''}${s} on both arena and TV.<br>Tap <strong>"🔔 I See a Whistle on TV Right Now!"</strong> to sync immediately!</span>`;
return;
} else if (tvTotalSec > stopSec) {
// Play stopped at arena, but TV is still counting down towards the whistle
calculatedDelay = stopElapsed + (tvTotalSec - stopSec);
} else {
if (resEl) resEl.innerHTML = `<span style="color:#f87171;">⚠️ TV clock (${m}:${s < 10 ? '0' : ''}${s}) is ahead of arena stoppage (${Math.floor(stopSec/60)}:${stopSec%60 < 10 ? '0' : ''}${stopSec})!</span>`;
return;
}
} else {
calculatedDelay = tvTotalSec - curOfficial;
}

if (calculatedDelay < 0) {
if (resEl) resEl.innerHTML = `<span style="color:#f87171;">⚠️ TV clock is ahead of official arena clock! Broadcast feeds are delayed behind the arena.</span>`;
return;
}
if (calculatedDelay > 120) {
if (resEl) resEl.innerHTML = `<span style="color:#f87171;">⚠️ Calculated delay (${calculatedDelay}s) exceeds maximum 120s buffer. Please check period or time entered.</span>`;
return;
}

sendQuickDelay(calculatedDelay);
if (resEl) resEl.innerHTML = `<span style="color:#4ade80;">🎯 TV is ${calculatedDelay}s behind arena clock. Broadcast delay set to ${calculatedDelay}s!</span>`;
showToast(`🎯 Synchronised! Broadcast delay set to ${calculatedDelay}s.`);
};
let lastUserDelayChange = 0;
function updateDelayDisplays(v) {
const txt = (v === 0) ? '0s (Instantaneous)' : v + 's delay';
if (liveDelayVal) liveDelayVal.textContent = txt;
if (delayVal) delayVal.textContent = txt;
if (liveDelaySlider && parseInt(liveDelaySlider.value) !== v) liveDelaySlider.value = v;
if (delaySlider && parseInt(delaySlider.value) !== v) delaySlider.value = v;
liveClockState.delaySec = v;
updateClientClocks();
}
function sendQuickDelay(v) {
lastUserDelayChange = Date.now();
updateDelayDisplays(v);
fetch('/api/quick-set', {
method: 'POST',
headers: {'Content-Type': 'application/json'},
body: JSON.stringify({ broadcastDelay: v })
}).catch(console.error);
}
if (liveDelaySlider) {
liveDelaySlider.oninput = () => {
lastUserDelayChange = Date.now();
updateDelayDisplays(parseInt(liveDelaySlider.value) || 0);
};
liveDelaySlider.onchange = () => {
sendQuickDelay(parseInt(liveDelaySlider.value) || 0);
};
}
if (delaySlider) {
delaySlider.oninput = () => {
lastUserDelayChange = Date.now();
updateDelayDisplays(parseInt(delaySlider.value) || 0);
};
delaySlider.onchange = () => {
sendQuickDelay(parseInt(delaySlider.value) || 0);
};
}
window.quickSetPreset = (sec) => {
sendQuickDelay(sec);
};
window.setBroadcastPreset = (sec) => {
sendQuickDelay(sec);
};
window.quickNudgeDelay = (delta) => {
let cur = parseInt(liveDelaySlider ? liveDelaySlider.value : (delaySlider ? delaySlider.value : 0)) || 0;
let v = cur + delta;
if (v < 0) v = 0;
if (v > 120) v = 120;
sendQuickDelay(v);
};
window.nudgeDelay = (delta) => {
window.quickNudgeDelay(delta);
};
// Expandable Visualiser Drawers
const activeDrawers = {
goal: false,
opponent: false,
powerplay: false,
buzzer: false,
intermission: false,
win: false
};
window.toggleEventDrawer = function(eventName) {
const drawer = document.getElementById('drawer-' + eventName);
const arrow = document.getElementById('arrow-' + eventName);
if (!drawer) return;
const isOpen = (drawer.style.display !== 'none');
drawer.style.display = isOpen ? 'none' : 'block';
if (arrow) arrow.textContent = isOpen ? '▾' : '▴';
activeDrawers[eventName] = !isOpen;
};
function getTeamColourHex(abbrev) {
const t = TEAMS.find(item => item[0] === abbrev);
return t ? t[2] : '#e63946';
}
// Common Multi-Canvas LED Renderer Helper
function drawLedShape(ctx, w, h, nPixels, layout, isReverse, getLedState) {
ctx.clearRect(0, 0, w, h);
const cx = w / 2;
const cy = h / 2;
const ledRadius = Math.max(3, Math.min(7, Math.floor(120 / nPixels) + 2));
for (let i = 0; i < nPixels; i++) {
const mappedIdx = isReverse ? ((nPixels - 1) - i) : i;
let x = cx;
let y = cy;
if (layout === 0) { // Single Ring
const radius = 48;
const angle = (mappedIdx / nPixels) * (2 * Math.PI) - (Math.PI / 2);
x = cx + radius * Math.cos(angle);
y = cy + radius * Math.sin(angle);
} else if (layout === 1) { // Spiral Wrap Strip
const turns = 2.4;
const spiralAngle = (mappedIdx / nPixels) * (turns * 2 * Math.PI) - (Math.PI / 2);
const rx = 45;
x = cx + rx * Math.cos(spiralAngle);
y = (cy + 40) - (mappedIdx / Math.max(1, nPixels - 1)) * 80;
} else if (layout === 2) { // Parallel Columns
const half = Math.ceil(nPixels / 2);
const isFirstCol = (mappedIdx < half);
const colX = isFirstCol ? (cx - 32) : (cx + 32);
const colStep = isFirstCol ? mappedIdx : (nPixels - 1 - mappedIdx);
const colTotal = isFirstCol ? half : (nPixels - half);
x = colX;
y = (cy - 40) + (colStep / Math.max(1, colTotal - 1)) * 80;
} else { // Linear Strip
const barWidth = Math.min(180, nPixels * 12);
x = (cx - barWidth / 2) + (mappedIdx / Math.max(1, nPixels - 1)) * barWidth;
y = cy;
}
const state = getLedState(i, mappedIdx);
ctx.beginPath();
ctx.arc(x, y, ledRadius, 0, 2 * Math.PI);
ctx.fillStyle = state.fill;
ctx.globalAlpha = state.alpha;
ctx.fill();
ctx.globalAlpha = 1.0;
ctx.lineWidth = 1;
ctx.strokeStyle = 'rgba(255,255,255,0.2)';
ctx.stroke();
}
}
let previewStep = 0;
function renderAllVisualisers() {
const nPixels = Math.max(4, parseInt(document.getElementById('ledPixels').value) || 16);
const layout = parseInt(document.getElementById('ledLayout').value) || 0;
const isReverse = document.getElementById('chkLedReverse').checked;
const teamHex = getTeamColourHex(teamSelect.value);
const isClassicRed = document.getElementById('chkGoalClassicRed').checked;
const goalHex = isClassicRed ? '#e63946' : teamHex;
// 1. Hardware Geometry & Idle Preview on Lights Tab
const ringCanvas = document.getElementById('ringCanvas');
if (ringCanvas) {
const ringCtx = ringCanvas.getContext('2d');
const idleEffRaw = parseInt(document.getElementById('idleEffect').value);
const idleEff = isNaN(idleEffRaw) ? 1 : idleEffRaw;
drawLedShape(ringCtx, ringCanvas.width, ringCanvas.height, nPixels, layout, isReverse, (i) => {
let alpha = 0.15;
let fill = teamHex;
if (idleEff === 0) { // Dark / Off
alpha = 0.04;
fill = '#333333';
} else if (idleEff === 1) { // Breathe
alpha = 0.12 + 0.65 * (Math.sin(previewStep * 0.05) + 1.0) / 2.0;
fill = teamHex;
} else if (idleEff === 2) { // Solid Accent
alpha = 0.65;
fill = teamHex;
} else if (idleEff === 3) { // Candle / Ember
const f1 = Math.sin(previewStep * 0.18 + i * 2.1);
const f2 = Math.sin(previewStep * 0.35 + i * 4.7);
alpha = 0.2 + 0.65 * ((f1 + f2 + 2) / 4);
fill = '#ff7700';
} else if (idleEff === 4) { // Orbit Sweep
const head = Math.floor(previewStep / 5) % nPixels;
const dist = Math.min(Math.abs(i - head), nPixels - Math.abs(i - head));
alpha = (dist === 0) ? 0.9 : ((dist === 1) ? 0.38 : 0.06);
fill = teamHex;
}
return { fill, alpha };
});
const cap = document.getElementById('previewCaption');
if (cap && previewStep % 30 === 0) {
const layoutNames = ['Single Ring', 'Spiral Wrap', 'Parallel Columns', 'Linear Strip'];
const idleNames = ['Dark / Off', 'Team Colour Breathe', 'Solid Team Accent', 'Candle / Ember Flicker', 'Orbit Sweep'];
cap.textContent = `${layoutNames[layout] || 'Ring'} (${nPixels} LEDs) • ${idleNames[idleEff] || 'Idle'}`;
}
}
// 2. Goal Celebration Drawer
if (activeDrawers.goal) {
const c = document.getElementById('canvas-goal');
if (c) {
const ctx = c.getContext('2d');
const effect = parseInt(document.getElementById('effectSelect').value) || 0;
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, (i) => {
let alpha = 0.15;
let fill = goalHex;
if (effect === 0) { // Rotating Beacon
const head = Math.floor(previewStep / 3) % nPixels;
const dist = Math.min(Math.abs(i - head), nPixels - Math.abs(i - head));
alpha = (dist === 0) ? 1.0 : ((dist === 1) ? 0.65 : ((dist === 2) ? 0.25 : 0.05));
fill = goalHex;
} else if (effect === 1) { // Arena Strobe
const flash = (previewStep % 4 < 2);
alpha = flash ? 1.0 : 0.05;
fill = (previewStep % 8 < 4) ? '#ffffff' : goalHex;
} else if (effect === 2) { // Breathe
alpha = 0.15 + 0.85 * (Math.sin(previewStep * 0.12) + 1.0) / 2.0;
fill = goalHex;
} else if (effect === 3) { // Dual Siren
const half = Math.floor(nPixels / 2);
const isLeft = (i < half);
const sideA = ((previewStep % 16) < 8);
const flashOn = (previewStep % 4 < 2);
if (flashOn && ((sideA && isLeft) || (!sideA && !isLeft))) {
alpha = 1.0;
fill = isLeft ? goalHex : '#ffffff';
} else {
alpha = 0.08;
fill = isLeft ? goalHex : '#ffffff';
}
} else if (effect === 4) { // Heartbeat
const sub = previewStep % 24;
alpha = (sub < 3 || (sub >= 6 && sub < 9)) ? 1.0 : 0.08;
fill = goalHex;
}
return { fill, alpha };
});
const capG = document.getElementById('cap-goal');
if (capG && previewStep % 30 === 0) {
const celebNames = ['Rotating Halogen Beacon', 'Arena Strobe', 'Colour Breathe', 'Dual Siren', 'Heartbeat Pulse'];
const colName = isClassicRed ? 'Classic Red' : 'Team Colours';
capG.textContent = `${celebNames[effect] || 'Celebration'} in ${colName}`;
}
}
}
// 3. Opponent Goal Drawer
if (activeDrawers.opponent) {
const c = document.getElementById('canvas-opponent');
if (c) {
const ctx = c.getContext('2d');
const flash = (previewStep % 8 < 4);
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, () => ({
fill: '#ff9800',
alpha: flash ? 0.95 : 0.12
}));
}
}
// 4. Power Play Drawer
if (activeDrawers.powerplay) {
const c = document.getElementById('canvas-powerplay');
if (c) {
const ctx = c.getContext('2d');
const ppHead = Math.floor(previewStep / 2) % nPixels;
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, (i) => {
const dist = Math.min(Math.abs(i - ppHead), nPixels - Math.abs(i - ppHead));
const alpha = (dist === 0) ? 1.0 : ((dist === 1) ? 0.7 : ((dist === 2) ? 0.35 : 0.08));
return { fill: '#ffd700', alpha };
});
}
}
// 5. Period Buzzer Drawer
if (activeDrawers.buzzer) {
const c = document.getElementById('canvas-buzzer');
if (c) {
const ctx = c.getContext('2d');
const buzzFlash = (previewStep % 3 === 0);
const col = (previewStep % 6 < 3) ? '#ff2222' : '#ffffff';
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, () => ({
fill: col,
alpha: buzzFlash ? 1.0 : 0.06
}));
}
}
// 6. Intermission Drawer
if (activeDrawers.intermission) {
const c = document.getElementById('canvas-intermission');
if (c) {
const ctx = c.getContext('2d');
const alpha = 0.15 + 0.65 * (Math.sin(previewStep * 0.04) + 1.0) / 2.0;
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, () => ({
fill: teamHex,
alpha
}));
}
}
// 7. Victory Fanfare Drawer
if (activeDrawers.win) {
const c = document.getElementById('canvas-win');
if (c) {
const ctx = c.getContext('2d');
drawLedShape(ctx, c.width, c.height, nPixels, layout, isReverse, (i) => {
const hue = (previewStep * 5 + (i * 360 / nPixels)) % 360;
return {
fill: 'hsl(' + hue + ', 100%, 55%)',
alpha: 0.95
};
});
}
}
previewStep++;
requestAnimationFrame(renderAllVisualisers);
}
requestAnimationFrame(renderAllVisualisers);
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
if (cfg.goalClassicRed !== undefined) document.getElementById('chkGoalClassicRed').checked = !!cfg.goalClassicRed;
document.getElementById('chkOpponent').checked = !!cfg.alertOpponent;
if (cfg.alertOpponentSound !== undefined) document.getElementById('chkOpponentSound').checked = !!cfg.alertOpponentSound;
document.getElementById('chkPowerPlay').checked = !!cfg.alertPowerPlay;
if (cfg.alertPowerPlaySound !== undefined) document.getElementById('chkPowerPlaySound').checked = !!cfg.alertPowerPlaySound;
document.getElementById('chkBuzzer').checked = !!cfg.alertBuzzer;
if (cfg.alertBuzzerSound !== undefined) document.getElementById('chkBuzzerSound').checked = !!cfg.alertBuzzerSound;
if (cfg.alertIntermission !== undefined) document.getElementById('chkIntermission').checked = !!cfg.alertIntermission;
document.getElementById('chkWin').checked = !!cfg.alertWin;
if (cfg.alertWinSound !== undefined) document.getElementById('chkWinSound').checked = !!cfg.alertWinSound;
if (cfg.broadcastDelay !== undefined) {
updateDelayDisplays(cfg.broadcastDelay);
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
// Track Last Play & arrival timing
const playText = s.game.lastPlay || '';
if (playText && playText !== lastPlayTracker.text) {
lastPlayTracker.text = playText;
const elapsedSec = (typeof s.game.lastPlayElapsed === 'number') ? s.game.lastPlayElapsed : 0;
lastPlayTracker.detectedTime = Date.now() - (elapsedSec * 1000);
const d = new Date(lastPlayTracker.detectedTime);
const timeStr = d.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit', second: '2-digit' });
const dt1 = document.getElementById('livePlayDetectedTime');
if (dt1) dt1.textContent = timeStr;
const dt2 = document.getElementById('syncPlayDetectedTime');
if (dt2) dt2.textContent = timeStr;
}
const lastPlayEl = document.getElementById('liveLastPlay');
if (lastPlayEl) lastPlayEl.textContent = playText || '⏳ Waiting for live game action / puck drop...';
const syncPlayEl = document.getElementById('syncLastPlayText');
if (syncPlayEl) syncPlayEl.textContent = playText || '⏳ Waiting for live game action / puck drop...';
const offPerEl = document.getElementById('liveOfficialPeriod');
if (offPerEl) offPerEl.textContent = `Period ${s.game.period}`;
const syncNhlPer = document.getElementById('syncNhlPeriod');
if (syncNhlPer) syncNhlPer.textContent = `Period ${s.game.period}`;
if (s.game && s.game.recentPlays) renderRecentPlaysTimeline(s.game.recentPlays);
// Real-Time Clock & Cloud Diagnostics
if (s.game.espEpoch) {
const d = new Date(s.game.espEpoch * 1000);
const diagDev = document.getElementById('diagDeviceTime');
if (diagDev) diagDev.textContent = d.toLocaleTimeString();
}
if (s.game.apiDate) {
const diagApi = document.getElementById('diagApiDate');
if (diagApi) {
try {
const ad = new Date(s.game.apiDate);
diagApi.textContent = ad.toLocaleTimeString();
} catch(e) {
diagApi.textContent = s.game.apiDate;
}
}
}
if (typeof s.game.apiAge !== 'undefined') {
const diagAge = document.getElementById('diagCacheAge');
if (diagAge) diagAge.textContent = s.game.apiAge + 's';
}
// Synchronise client clock timeline engine with ESP32 status
liveClockState.active = true;
const newPeriod = s.game.period || 1;
const periodChanged = (newPeriod !== liveClockState.period);
liveClockState.period = newPeriod;
const newOfficialSec = (typeof s.game.secondsRemaining !== 'undefined' && s.game.secondsRemaining >= 0) ? s.game.secondsRemaining : parseClockSeconds(s.game.timeRemaining);
const isRunning = !!s.game.running;
const wasRunning = liveClockState.running;
const isIntermission = !!s.game.inIntermission;
const wasIntermission = !!liveClockState.inIntermission;
liveClockState.inIntermission = isIntermission;
liveClockState.running = isRunning;
liveClockState.stoppageElapsed = (typeof s.game.stoppageElapsed !== 'undefined') ? s.game.stoppageElapsed : 9999;
liveClockState.resumeElapsed = (typeof s.game.resumeElapsed !== 'undefined') ? s.game.resumeElapsed : 9999;
liveClockState.stoppageSeconds = (typeof s.game.stoppageSeconds !== 'undefined') ? s.game.stoppageSeconds : newOfficialSec;

const now = Date.now();
const elapsedSinceSync = Math.max(0, Math.floor((now - liveClockState.lastSyncTime) / 1000));
const currentBrowserSec = wasRunning ? Math.max(0, liveClockState.officialSeconds - elapsedSinceSync) : liveClockState.officialSeconds;

if (periodChanged || (isIntermission !== wasIntermission)) {
  // Period transition or entered/exited intermission: snap anchor
  liveClockState.officialSeconds = newOfficialSec;
  liveClockState.lastSyncTime = now;
} else if (isIntermission) {
  // During intermission: local timer ticks down smoothly 1 sec/sec
  // Strictly monotonic descending: never jump backwards into the past!
  const currentIntermSec = Math.max(0, liveClockState.officialSeconds - elapsedSinceSync);
  if (newOfficialSec < currentIntermSec && (currentIntermSec - newOfficialSec > 5)) {
    liveClockState.officialSeconds = newOfficialSec;
    liveClockState.lastSyncTime = now;
  }
} else if (!isRunning) {
  // Arena is stopped by whistle: stay firmly locked at stoppage seconds
  const stopSec = (typeof s.game.stoppageSeconds === 'number' && s.game.stoppageSeconds >= 0) ? s.game.stoppageSeconds : newOfficialSec;
  liveClockState.officialSeconds = stopSec;
  liveClockState.lastSyncTime = now;
} else {
  // Arena is running
  if (!wasRunning) {
    // Just resumed (puck drop): snap anchor to start of play
    liveClockState.officialSeconds = newOfficialSec;
    liveClockState.lastSyncTime = now;
  } else {
    // Continuing play: maintain local 1.0s/sec countdown!
    // Strictly monotonic descending: NEVER jump backwards into the past!
    // Stale cached CDN responses (newOfficialSec >= currentBrowserSec) are ignored
    if (newOfficialSec < currentBrowserSec && (currentBrowserSec - newOfficialSec > 5)) {
      liveClockState.officialSeconds = newOfficialSec;
      liveClockState.lastSyncTime = now;
    }
  }
}
// Sync delay settings from hardware if user isn't actively adjusting them
const userInteracting = (document.activeElement === liveDelaySlider || document.activeElement === delaySlider);
if (!userInteracting && (Date.now() - lastUserDelayChange > 3000)) {
if (s.game && typeof s.game.broadcastDelay === 'number') {
updateDelayDisplays(s.game.broadcastDelay);
}
} else {
liveClockState.delaySec = parseInt(liveDelaySlider ? liveDelaySlider.value : (delaySlider ? delaySlider.value : 0)) || 0;
}
updateClientClocks();
const interm = !!s.game.inIntermission;
document.getElementById('liveIntermissionNote').style.display = interm ? 'inline' : 'none';
document.getElementById('liveStatusSubtext').textContent = interm ? 'Intermission in progress.' : (s.game.running ? 'Live play in progress.' : 'Play stopped by whistle.');
if (cardLive) cardLive.style.display = 'block';
} else if (s.system && s.system.connected) {
liveClockState.active = false;
liveClockState.running = false;
badge.className = 'badge';
badge.textContent = `Online (${s.system.team})`;
document.getElementById('liveGameClock').textContent = 'No Live Game';
document.getElementById('syncNhlClock').textContent = '--:--';
document.getElementById('syncNhlPeriod').textContent = 'No Live Game Right Now';
const lastPlayEl = document.getElementById('liveLastPlay');
if (lastPlayEl) lastPlayEl.textContent = 'No game currently active.';
const offClockEl = document.getElementById('liveOfficialClock');
if (offClockEl) offClockEl.textContent = '--:--';
const offPerEl = document.getElementById('liveOfficialPeriod');
if (offPerEl) offPerEl.textContent = 'No Live Game';
const targetEl = document.getElementById('liveTargetTvClock');
if (targetEl) targetEl.textContent = '--:--';
document.getElementById('liveIntermissionNote').style.display = 'none';
document.getElementById('liveStatusSubtext').textContent = 'No game currently in progress.';
} else {
liveClockState.active = false;
liveClockState.running = false;
badge.className = 'badge';
badge.textContent = 'AP Setup Mode (192.168.4.1)';
document.getElementById('liveGameClock').textContent = 'Setup Mode';
const lastPlayEl = document.getElementById('liveLastPlay');
if (lastPlayEl) lastPlayEl.textContent = 'Connect device to home Wi-Fi.';
const offClockEl = document.getElementById('liveOfficialClock');
if (offClockEl) offClockEl.textContent = '--:--';
const offPerEl = document.getElementById('liveOfficialPeriod');
if (offPerEl) offPerEl.textContent = 'Setup Mode';
const targetEl = document.getElementById('liveTargetTvClock');
if (targetEl) targetEl.textContent = '--:--';
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
setInterval(updateClientClocks, 1000);
setInterval(refreshStatus, 2500);
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
const idleVal = parseInt(document.getElementById('idleEffect').value);
const data = {
deviceName: document.getElementById('deviceNameInput').value.trim() || 'NHL Goal Light',
team: teamSelect.value,
hwLights: chkHwLights.checked,
hwSpeaker: chkHwSpeaker.checked,
hwDisplay: chkHwDisplay.checked,
numLeds: parseInt(document.getElementById('ledPixels').value) || 16,
ledLayout: parseInt(document.getElementById('ledLayout').value) || 0,
ledReverse: document.getElementById('chkLedReverse').checked,
idleEffect: isNaN(idleVal) ? 1 : idleVal,
brightness: parseInt(brightSlider.value),
volume: parseInt(volSlider.value),
hornDuration: parseInt(hornSlider.value),
effectStyle: parseInt(document.getElementById('effectSelect').value),
goalClassicRed: document.getElementById('chkGoalClassicRed').checked,
alertOpponent: document.getElementById('chkOpponent').checked,
alertOpponentSound: document.getElementById('chkOpponentSound').checked,
alertPowerPlay: document.getElementById('chkPowerPlay').checked,
alertPowerPlaySound: document.getElementById('chkPowerPlaySound').checked,
alertBuzzer: document.getElementById('chkBuzzer').checked,
alertBuzzerSound: document.getElementById('chkBuzzerSound').checked,
alertIntermission: document.getElementById('chkIntermission').checked,
alertWin: document.getElementById('chkWin').checked,
alertWinSound: document.getElementById('chkWinSound').checked,
broadcastDelay: parseInt(liveDelaySlider ? liveDelaySlider.value : delaySlider.value) || 0,
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
let highest = null;
for (const r of data) {
const tag = r.tag_name || r.name || '';
if (!highest || compareVersions(tag, highest.tag_name || highest.name || '') > 0) {
highest = r;
}
}
rel = highest || data[0] || null;
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
`<span id="cloudStatusText" style="font-size:0.82rem; color:var(--text-muted);">Connecting to firmware server...</span><br><br>` +
`<div style="background:var(--border); border-radius:6px; height:12px; overflow:hidden;"><div id="cloudBar" style="background:var(--accent); height:100%; width:0%; transition:width 0.3s ease;"></div></div>`;
// Use direct CDN URL for reliable high speed download without redirects
const directUrl = 'https://revelrob.github.io/NHLGoalLight/builds/firmware.bin';
fetch('/api/ota-cloud', {
method: 'POST',
headers: { 'Content-Type': 'application/json' },
body: JSON.stringify({ url: directUrl })
}).then(res => res.json()).then(data => {
if (!data.success) {
throw new Error(data.message || 'Failed to start cloud update');
}
// Start real-time progress polling
const pollInterval = setInterval(() => {
fetch('/api/ota-status')
.then(r => r.json())
.then(st => {
const cb = document.getElementById('cloudBar');
const stText = document.getElementById('cloudStatusText');
if (cb) cb.style.width = st.percent + '%';
if (stText) {
if (st.totalBytes > 0) {
const kbWritten = Math.round(st.bytesWritten / 1024);
const kbTotal = Math.round(st.totalBytes / 1024);
stText.textContent = `${st.message} (${st.percent}%, ${kbWritten} KB / ${kbTotal} KB)`;
} else {
stText.textContent = st.message || 'Updating...';
}
}
if (st.success) {
clearInterval(pollInterval);
if (cb) cb.style.background = 'var(--success)';
if (stText) stText.innerHTML = '<strong>✅ Flash verified! Rebooting Goal Light...</strong>';
setTimeout(() => window.location.reload(), 6000);
} else if (st.failed) {
clearInterval(pollInterval);
if (cb) cb.style.background = '#e63946';
if (stText) stText.innerHTML = `<span style="color:#e63946;">❌ Update failed: ${st.message}</span>`;
btn1Click.disabled = false;
}
})
.catch(() => {
// Device may have rebooted after flash success
setTimeout(() => window.location.reload(), 4000);
});
}, 600);
}).catch(err => {
alert('Cloud OTA error: ' + err);
btn1Click.disabled = false;
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
if (file.name.toLowerCase().includes('merged')) {
alert('❌ Error: You selected merged.bin (4MB full flash image).\n\nFor over-the-air updates, please select firmware.bin (1.28MB).');
return;
}
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
let resp = {};
try { resp = JSON.parse(xhr.responseText); } catch(e){}
if (xhr.status === 200 && resp.success) {
otaText.textContent = '✅ Update complete! Rebooting device in 6s...';
otaBar.style.background = 'var(--success)';
setTimeout(() => window.location.reload(), 6000);
} else {
otaText.textContent = '❌ Update failed: ' + (resp.error || xhr.responseText || 'Flash error');
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
</html>)rawliteral";
