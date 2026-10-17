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
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>NHL Goal Light</title>
  <link rel="icon" type="image/svg+xml" href="data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'%3E%3Ccircle cx='50' cy='50' r='46' fill='%23111' stroke='%23333' stroke-width='4'/%3E%3Ccircle cx='50' cy='50' r='38' fill='%23d90429'/%3E%3Cpath d='M25 50h50M50 25v50' stroke='%23fff' stroke-width='8' stroke-linecap='round'/%3E%3Ccircle cx='50' cy='50' r='18' fill='%23fff'/%3E%3Ccircle cx='50' cy='50' r='10' fill='%23d90429'/%3E%3C/svg%3E">
  <style>
    :root {
      --bg: #0d1117;
      --card-bg: rgba(22, 27, 34, 0.85);
      --border: rgba(255, 255, 255, 0.1);
      --text: #f0f6fc;
      --text-muted: #8b949e;
      --accent: #e63946;
      --accent-glow: rgba(230, 57, 70, 0.35);
      --btn-bg: #21262d;
      --btn-hover: #30363d;
      --success: #2ea043;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
      background: var(--bg);
      color: var(--text);
      display: flex;
      justify-content: center;
      padding: 16px;
      min-height: 100vh;
    }
    .container {
      width: 100%;
      max-width: 480px;
      display: flex;
      flex-direction: column;
      gap: 16px;
    }
    .header {
      text-align: center;
      padding: 12px 0;
    }
    .header h1 {
      font-size: 1.6rem;
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
      padding: 3px 8px;
      border-radius: 12px;
      background: var(--border);
      color: var(--text-muted);
      margin-top: 6px;
    }
    .badge.live { background: #e63946; color: #fff; }
    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 16px;
      padding: 18px;
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 24px rgba(0,0,0,0.3);
    }
    .card h2 {
      font-size: 1rem;
      font-weight: 600;
      margin-bottom: 14px;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.8px;
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .form-group {
      margin-bottom: 14px;
    }
    .form-group:last-child { margin-bottom: 0; }
    label {
      display: flex;
      justify-content: space-between;
      font-size: 0.85rem;
      font-weight: 500;
      margin-bottom: 6px;
    }
    select, input[type="text"], input[type="password"] {
      width: 100%;
      background: #161b22;
      border: 1px solid var(--border);
      color: var(--text);
      padding: 10px 12px;
      border-radius: 8px;
      font-size: 0.95rem;
      outline: none;
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
      font-size: 0.9rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s ease;
    }
    button:hover { background: var(--btn-hover); }
    button.primary {
      background: var(--accent);
      border-color: transparent;
      color: #fff;
    }
    button.primary:hover { opacity: 0.9; }
    .checkbox-row {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-bottom: 10px;
      font-size: 0.9rem;
      cursor: pointer;
    }
    .checkbox-row input {
      accent-color: var(--accent);
      width: 18px;
      height: 18px;
    }
    .status-toast {
      padding: 10px;
      border-radius: 8px;
      font-size: 0.85rem;
      text-align: center;
      display: none;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>🚨 NHL Goal Light</h1>
      <div id="statusBadge" class="badge">Connecting...</div>
    </div>

    <!-- Favorite Team -->
    <div class="card">
      <h2>⭐ Favorite Team</h2>
      <div class="form-group">
        <label for="teamSelect">Selected Team</label>
        <select id="teamSelect"></select>
      </div>
    </div>

    <!-- Lighting Controls -->
    <div class="card">
      <h2>💡 Lighting Controls</h2>
      <div class="form-group">
        <label>Brightness: <span id="brightVal">70%</span></label>
        <div class="slider-container">
          <input type="range" id="brightSlider" min="10" max="255" value="180">
        </div>
      </div>
      <div class="form-group">
        <label for="effectSelect">Celebration Effect</label>
        <select id="effectSelect">
          <option value="0">Rotating Halogen Beacon</option>
          <option value="1">High-Speed Arena Strobe</option>
          <option value="2">Team Color Breath</option>
        </select>
      </div>
      <div class="form-group">
        <label>Horn Duration: <span id="hornVal">12s</span></label>
        <div class="slider-container">
          <input type="range" id="hornSlider" min="5" max="30" value="12">
        </div>
      </div>
    </div>

    <!-- Sound Controls -->
    <div class="card">
      <h2>🔊 Sound Controls</h2>
      <div class="form-group">
        <label>Volume: <span id="volVal">Level 3</span></label>
        <div class="slider-container">
          <input type="range" id="volSlider" min="0" max="5" value="3">
        </div>
      </div>
      <div class="btn-row">
        <button id="btnTestHorn" class="primary">🚨 Test Goal Horn</button>
        <button id="btnTestLight">💡 Test Light Only</button>
      </div>
    </div>

    <!-- Event Toggles -->
    <div class="card">
      <h2>⚙️ Event Alerts</h2>
      <label class="checkbox-row">
        <input type="checkbox" id="chkOpponent">
        <span>Alert on Opponent Goals</span>
      </label>
      <label class="checkbox-row">
        <input type="checkbox" id="chkPowerPlay" checked>
        <span>Alert on Power Play Awarded</span>
      </label>
      <label class="checkbox-row">
        <input type="checkbox" id="chkBuzzer" checked>
        <span>Period Start & End Buzzers</span>
      </label>
      <label class="checkbox-row">
        <input type="checkbox" id="chkWin" checked>
        <span>Game Win Victory Fanfare</span>
      </label>
    </div>

    <!-- Wi-Fi Setup -->
    <div class="card">
      <h2>📶 Wi-Fi Settings</h2>
      <div class="form-group">
        <label for="wifiSsid">Wi-Fi Network (SSID)</label>
        <input type="text" id="wifiSsid" placeholder="Your Wi-Fi SSID">
      </div>
      <div class="form-group">
        <label for="wifiPass">Wi-Fi Password</label>
        <input type="password" id="wifiPass" placeholder="Wi-Fi Password">
      </div>
    </div>

    <!-- Firmware & Updates -->
    <div class="card">
      <h2>🔄 Firmware & Updates</h2>
      <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
        <span style="font-size:0.85rem; color:var(--text-muted)">Current Version: <strong id="curVer" style="color:var(--text)">v1.0.0</strong></span>
        <button id="btnCheckUpdate" style="padding:6px 12px; font-size:0.8rem; background:var(--btn-bg); border:1px solid var(--border); color:var(--text); border-radius:8px; cursor:pointer;">🔍 Check GitHub</button>
      </div>
      <div id="updateInfo" style="display:none; padding:10px; border-radius:8px; background:rgba(46,160,67,0.15); border:1px solid var(--success); margin-bottom:12px; font-size:0.82rem; line-height:1.4;"></div>
      
      <div class="form-group" style="margin-bottom:8px;">
        <label for="otaFile">Manual OTA Firmware Upload (.bin)</label>
        <input type="file" id="otaFile" accept=".bin" style="font-size:0.8rem; padding:8px 0; color:var(--text-muted);">
      </div>
      <button id="btnOtaUpload" style="width:100%; padding:10px; background:#1f6feb; border:none; border-radius:8px; color:white; font-weight:600; cursor:pointer; display:none;">⚡ Install Firmware Update</button>
      <div id="otaProgress" style="display:none; margin-top:8px;">
        <div style="background:var(--border); border-radius:6px; height:12px; overflow:hidden;">
          <div id="otaBar" style="background:#1f6feb; height:100%; width:0%; transition:width 0.2s;"></div>
        </div>
        <div id="otaText" style="font-size:0.75rem; color:var(--text-muted); text-align:center; margin-top:4px;">Uploading: 0%</div>
      </div>
    </div>

    <button id="btnSave" class="primary" style="padding: 16px; font-size: 1rem;">💾 Save & Apply Settings</button>
    <div id="statusToast" class="status-toast"></div>
  </div>

  <script>
    const TEAMS = [
      ["ANA", "Anaheim Ducks"], ["BOS", "Boston Bruins"], ["BUF", "Buffalo Sabres"],
      ["CGY", "Calgary Flames"], ["CAR", "Carolina Hurricanes"], ["CHI", "Chicago Blackhawks"],
      ["COL", "Colorado Avalanche"], ["CBJ", "Columbus Blue Jackets"], ["DAL", "Dallas Stars"],
      ["DET", "Detroit Red Wings"], ["EDM", "Edmonton Oilers"], ["FLA", "Florida Panthers"],
      ["LAK", "Los Angeles Kings"], ["MIN", "Minnesota Wild"], ["MTL", "Montreal Canadiens"],
      ["NSH", "Nashville Predators"], ["NJD", "New Jersey Devils"], ["NYI", "New York Islanders"],
      ["NYR", "New York Rangers"], ["OTT", "Ottawa Senators"], ["PHI", "Philadelphia Flyers"],
      ["PIT", "Pittsburgh Penguins"], ["SJS", "San Jose Sharks"], ["SEA", "Seattle Kraken"],
      ["STL", "St. Louis Blues"], ["TBL", "Tampa Bay Lightning"], ["TOR", "Toronto Maple Leafs"],
      ["UTA", "Utah Hockey Club"], ["VAN", "Vancouver Canucks"], ["VGK", "Vegas Golden Knights"],
      ["WSH", "Washington Capitals"], ["WPG", "Winnipeg Jets"]
    ];

    const teamSelect = document.getElementById('teamSelect');
    TEAMS.forEach(([abbrev, name]) => {
      const opt = document.createElement('option');
      opt.value = abbrev;
      opt.textContent = `${name} (${abbrev})`;
      teamSelect.appendChild(opt);
    });

    const brightSlider = document.getElementById('brightSlider');
    const brightVal = document.getElementById('brightVal');
    brightSlider.oninput = () => brightVal.textContent = Math.round((brightSlider.value / 255) * 100) + '%';

    const volSlider = document.getElementById('volSlider');
    const volVal = document.getElementById('volVal');
    volSlider.oninput = () => volVal.textContent = volSlider.value == 0 ? 'Mute' : 'Level ' + volSlider.value;

    const hornSlider = document.getElementById('hornSlider');
    const hornVal = document.getElementById('hornVal');
    hornSlider.oninput = () => hornVal.textContent = hornSlider.value + 's';

    // Fetch initial config
    fetch('/api/config')
      .then(r => r.json())
      .then(cfg => {
        if (cfg.team) teamSelect.value = cfg.team;
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
        document.getElementById('chkWin').checked = !!cfg.alertWin;
        if (cfg.ssid) document.getElementById('wifiSsid').value = cfg.ssid;
      }).catch(console.error);

    // Fetch status
    function refreshStatus() {
      fetch('/api/status')
        .then(r => r.json())
        .then(s => {
          const badge = document.getElementById('statusBadge');
          if (s.game && s.game.state === 'LIVE') {
            badge.className = 'badge live';
            badge.textContent = `● LIVE: ${s.game.awayAbbrev} ${s.game.awayScore} - ${s.game.homeScore} ${s.game.homeAbbrev}`;
          } else if (s.system && s.system.connected) {
            badge.className = 'badge';
            badge.textContent = `Online (${s.system.team})`;
          } else {
            badge.className = 'badge';
            badge.textContent = 'AP Setup Mode';
          }
          if (s.system && s.system.version) {
            document.getElementById('curVer').textContent = s.system.version;
          }
        }).catch(() => {});
    }
    setInterval(refreshStatus, 4000);
    refreshStatus();

    // Test buttons
    document.getElementById('btnTestHorn').onclick = () => {
      fetch('/api/test', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({horn: true, light: true}) });
    };
    document.getElementById('btnTestLight').onclick = () => {
      fetch('/api/test', { method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({horn: false, light: true}) });
    };

    // Save button
    document.getElementById('btnSave').onclick = () => {
      const data = {
        team: teamSelect.value,
        brightness: parseInt(brightSlider.value),
        volume: parseInt(volSlider.value),
        hornDuration: parseInt(hornSlider.value),
        effectStyle: parseInt(document.getElementById('effectSelect').value),
        alertOpponent: document.getElementById('chkOpponent').checked,
        alertPowerPlay: document.getElementById('chkPowerPlay').checked,
        alertBuzzer: document.getElementById('chkBuzzer').checked,
        alertWin: document.getElementById('chkWin').checked,
        ssid: document.getElementById('wifiSsid').value,
        pass: document.getElementById('wifiPass').value
      };
      fetch('/api/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify(data)
      }).then(r => r.text()).then(msg => {
        const t = document.getElementById('statusToast');
        t.style.display = 'block';
        t.style.background = '#2ea043';
        t.textContent = 'Settings saved successfully!';
        setTimeout(() => t.style.display = 'none', 3000);
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
      fetch('https://api.github.com/repos/RevelRob/NHLGoalLight/releases/latest')
        .then(r => {
          if (!r.ok) throw new Error('No releases found');
          return r.json();
        })
        .then(rel => {
          btnCheckUpdate.textContent = '🔍 Check GitHub';
          const latestTag = rel.tag_name || rel.name;
          const currentTag = document.getElementById('curVer').textContent;
          updateInfo.style.display = 'block';
          if (latestTag !== currentTag && latestTag) {
            updateInfo.style.borderColor = '#e63946';
            updateInfo.style.background = 'rgba(230,57,70,0.15)';
            updateInfo.innerHTML = `<strong>✨ Update Available: ${latestTag}</strong><br>` +
              `Release notes: ${rel.body ? rel.body.substring(0, 100) : 'New improvements'}<br>` +
              `<a href="${rel.html_url}" target="_blank" style="color:#58a6ff; text-decoration:underline; display:inline-block; margin-top:6px;">View on GitHub / Download .bin</a>`;
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
  </script>
</body>
</html>
)rawliteral";
