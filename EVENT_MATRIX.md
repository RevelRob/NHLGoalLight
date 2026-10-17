# In-Game Event & Alert Matrix

This document defines all supported in-game hockey events, their visual LED light patterns, audio cues, default durations, and web portal configuration toggles.

---

## 1. Event Behavior Matrix

| Event | Trigger Condition | Visual Light Pattern | Audio Cue | Duration | Web Toggleable? |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **🚨 Preferred Team Goal** | `watchedScore > lastWatchedScore` | High-speed rotating red beacon + police quad-strobe | Primary Team Goal Horn + Goal Song (`0001.mp3`) | 10–15 sec | Always Active |
| **🥅 Opponent Goal** | `opponentScore > lastOpponentScore` | Slow dim red/amber double-flash | Subtle buzzer or muted silence | 3 sec | Yes (Default: Off/Muted) |
| **⚡ Power Play (Man Adv)** | Situation code changes to advantage (e.g. `1451` or `1541`) | Pulsing team primary color with amber accent sweeps | Short organ charge or single siren blast | 5 sec | Yes (Default: On) |
| **🛡️ Penalty Kill (Shorthanded)**| Preferred team penalized | Gentle slow warning amber breath | None | 4 sec | Yes (Default: Off) |
| **🏒 Game Start (Puck Drop)** | State transitions from `PRE` to `LIVE` (P1 20:00) | Full team color fanfare sweep | Arena air-raid siren / buzzer (`0002.mp3`) | 6 sec | Yes (Default: On) |
| **⏸️ Period End / Intermission** | `clock.inIntermission == true` | Slow calm cyan/blue breathing | Short period horn (`0003.mp3`) | 4 sec | Yes (Default: On) |
| **🔥 Overtime Start** | Period 4 begins (regular season) | Intense alternating red/white rapid pulse | Double horn burst | 6 sec | Yes (Default: On) |
| **🏆 Preferred Team Win (FINAL)** | State = `FINAL`/`OFF` and preferred score > opp score | Victorious rainbow sweep or alternating team colors | Victory arena song / extended horn (`0004.mp3`) | 20 sec | Yes (Default: On) |
| **❌ Score Disallowed / Overturn** | `watchedScore < lastWatchedScore` | Quick double yellow flash (referee review sign) | Double low beep | 2 sec | Automatic |
| **📶 AP Configuration Mode** | Device boots into AP setup mode (`NHL-Goal-Light-Setup`)| Solid Amber / Slow Yellow Orbit | Silent | Infinite (until Wi-Fi saved)| System |
| **🧪 User Test Trigger** | Button double-press or web UI "Test Horn" button | Goal celebration light pattern | Full Goal Horn | 7–10 sec | Manual |

---

## 2. Visual Animation Pattern Specifications

### A. The "Classic Beacon" (Rotating Halogen Emulation)
- **Goal**: Recreate the look of a motorized mechanical parabolic reflector spinning inside an emergency dome using stationary NeoPixels.
- **Algorithm**:
  - A "hotspot" index $P$ travels continuously around the ring ($0$ to $N-1$).
  - Adjacent LEDs fall off with a parabolic or Gaussian gradient:
    $$\text{Intensity}(i) = \max\left(0, 1.0 - \frac{\text{distance}(i, P)^2}{\text{width}^2}\right)$$
  - Speed: 80–120 RPM (one full revolution every ~500ms–750ms).
  - Color: Pure Red (`#FF0000`) or primary team color.

### B. The "Arena Strobe"
- **Goal**: High-energy burst reminiscent of goal strobes installed behind NHL nets.
- **Algorithm**:
  - Quad-flash pattern: 4 rapid flashes of the left semicircle (50ms on / 40ms off), followed by 4 rapid flashes of the right semicircle.
  - Alternating pure white (`#FFFFFF`) and team red (`#FF0800`).

### C. The "Breathing Ambient Glow"
- **Goal**: Subtle status indicator during intermissions or idle evenings when games are live.
- **Algorithm**:
  - Sine wave brightness interpolation:
    $$V(t) = \text{MinBrightness} + (\text{MaxBrightness} - \text{MinBrightness}) \times \left(\frac{1 + \sin(2\pi \cdot t / T)}{2}\right)$$
  - Period $T \approx 3.0$ seconds.

---

## 3. Web UI Customization Toggles

All event actions can be fine-tuned through the onboard web configuration page:
```
[x] Enable Goal Celebrations (Always On)
[x] Play Audio on Goals
[x] Alert on Opponent Goals (Visual only, no horn)
[x] Alert on Power Play awarded
[ ] Alert on Penalty Kill
[x] Announce Period Starts
[x] Celebrate Game Win (FINAL)
```
