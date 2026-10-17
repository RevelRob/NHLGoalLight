# Hardware Specifications & Wiring Guide

This document defines the hardware Bill of Materials (BOM), recommended microcontroller boards, pinouts, power requirements, and 3D printing enclosure guidelines for **Rob's NHL Goal Light**.

---

## 1. Bill of Materials (BOM)

### Core Electronics

| Component | Recommended Model | Alternative Options | Notes |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | **ESP32-S3 DevKit** or **Seeed XIAO ESP32-C3 / S3** | ESP32-WROOM-32, NodeMCU ESP32 | ESP32-S3 recommended if adding round display later |
| **LED Array** | **WS2812B NeoPixel Ring (16 or 24 LEDs)** | Dual ring (16 + 8), or high-density strip | 5V addressable RGB LEDs |
| **Audio Module** | **DFRobot DFPlayer Pro (DF1201S)** | MAX98357A I2S Amplifier, DFPlayer Mini | DFPlayer Pro has built-in 128MB USB-C flash storage |
| **Speaker** | **3W 4Ω or 8Ω miniature speaker (40mm)** | 28mm round speaker with acoustic chamber | Small form factor to fit inside printed base |
| **Input Button** | **Tactile momentary push button (6x6mm or 12x12mm)** | Arcade mini button, capacitive touch pin | Internal pull-up to GND |
| **Power Supply** | **5V 2A–2.4A USB-C Power Adapter & Cable** | 5V 3A supply if using dual LED rings | Peak draw during goal celebration is ~1.5A–2A |
| **Passive Components** | 1x 1000µF 6.3V/10V Electrolytic Capacitor, 1x 330Ω–470Ω Resistor | Logic level shifter (74HCT125) optional | Capacitor on 5V rail; resistor on NeoPixel DIN line |

---

## 2. Microcontroller: ESP32-S3 Super Mini

The **ESP32-S3 Super Mini** is the selected microcontroller for this build:
- **Form Factor**: Ultra-compact (~22.5 mm × 18 mm), easily fitting inside the base alongside the DFPlayer Pro and speaker.
- **CPU**: Dual-Core 32-bit Xtensa LX7 @ up to 240 MHz.
- **Storage**: 4MB / 8MB Flash with built-in USB-C interface.
- **Display Capability**: Hardware SPI with DMA support, ideal for powering the 240×240 GC9A01 round display at full 60 FPS in Phase 3.

---

## 3. Pinout & Wiring Specifications

### ESP32-S3 Super Mini Wiring Mapping

| Peripheral | Super Mini Pin | GPIO | Function / Notes |
| :--- | :--- | :--- | :--- |
| **WS2812B NeoPixel DIN** | `4` | `GPIO 4` | Connected via 330Ω resistor to Ring Data In |
| **Tactile Pushbutton** | `9` | `GPIO 9` | Active-low to GND (`INPUT_PULLUP`). Cycle vol / Test horn |
| **DFPlayer Pro RX** | `1` | `GPIO 1` | ESP32 HardwareSerial TX &rarr; DFPlayer Pro RX |
| **DFPlayer Pro TX** | `2` | `GPIO 2` | ESP32 HardwareSerial RX &larr; DFPlayer Pro TX |
| **[Phase 3] Round LCD SCL** | `12` | `GPIO 12` | Hardware SPI Clock (SCK / SCL) |
| **[Phase 3] Round LCD SDA** | `11` | `GPIO 11` | Hardware SPI Data (MOSI / SDA) |
| **[Phase 3] Round LCD CS** | `10` | `GPIO 10` | Chip Select (CS) |
| **[Phase 3] Round LCD DC** | `7` | `GPIO 7` | Data / Command register select |
| **[Phase 3] Round LCD RST** | `6` | `GPIO 6` | Display Hardware Reset |
| **[Phase 3] Round LCD BLK** | `3` | `GPIO 3` | Backlight PWM control (Brightness adjustment) |
| **Power Rails** | `5V` & `GND` | `5V` & `GND` | Common ground & 5V supply bus |

---

### Enclosure Layout: Front Face Stadium Scoreboard Display

The 3D printed base is designed to flush-mount the **1.28-inch round display (GC9A01)** onto the **front vertical face**:
```
           ╭─────────────────╮
           │   CAUTION CAGE  │
           │   ┌─────────┐   │
           │   │RED DOME │   │  <-- WS2812B NeoPixel Beacon / Strobe
           │   └─────────┘   │
      ╭────┴─────────────────┴────╮
      │   3D PRINTED BASE         │
      │       ╭─────────╮         │
      │       │  ROUND  │         │  <-- GC9A01 Round Display (Flush-Mounted)
      │       │ DISPLAY │         │      (Scoreboard / Clock / Headshots)
      │       ╰─────────╯         │
      │   [Vol/Test]    [USB-C]   │  <-- Button & Power Port
      ╰───────────────────────────╯
```

---

## 4. Power Architecture & Electrical Protection

```
                +5V USB-C In
                     │
      ┌──────────────┼───────────────────────────┐
      │              │                           │
   ┌──┴──┐        ┌──┴──┐                     ┌──┴──┐
   │ESP32│        │Audio│                     │LEDs │
   │ 5V  │        │DF1201S                    │ WS2812B (16-24x)
   └──┬──┘        └──┬──┘                     └──┬──┘
      │              │   [1000µF Cap across 5V/GND]
      │              │                           │
      └──────────────┴───────────────────────────┘
                     │
                    GND (Common Ground)
```

### Power Calculations (Peak Worst Case):
- **ESP32 Wi-Fi TX burst**: ~350mA peak @ 3.3V (internal LDO feeds off 5V &rarr; ~250mA @ 5V).
- **WS2812B 16-LED Ring**: 16 LEDs × 60mA (full white @ 100%) = 960mA peak. (In red beacon mode: ~350mA).
- **DFPlayer Pro + 3W Speaker**: ~500mA–700mA peak during loud horn bursts.
- **Total Peak Requirement**: **~1.8A to 2.0A**. A reliable 5V 2A USB wall block is required.

### Best Practices:
1. **Electrolytic Buffer Capacitor**: Place a 1000µF (or min 470µF) capacitor across 5V and GND near the LED ring to prevent brownouts when all LEDs turn on suddenly.
2. **Data Line Protection**: Place a 330Ω to 470Ω resistor between the ESP32 GPIO and the NeoPixel `DIN` pin to eliminate signal ringing and voltage spikes.
3. **Common Ground**: Always ensure the ESP32, LED ring, and audio board share a robust, low-impedance ground rail.

---

## 5. Audio Options & Speaker Architecture

### The Storage Question: DFPlayer vs ESP32 LittleFS

Where audio files are stored depends on which audio hardware path you choose:

1. **Option A: DFRobot DFPlayer Pro (DF1201S)**
   - **Where files live**: On the DFPlayer's own 128MB onboard flash chip.
   - **How files get there**: You plug the DFPlayer Pro's USB-C port into your computer like a USB thumb drive and drag `0001.mp3`, `0002.mp3`, etc. onto it.
   - **How it works**: The ESP32 does *not* stream audio; it just sends lightweight UART commands (`playFileNum(1)`).
   - **Cost**: ~$10–$14.

2. **Option B: MAX98357A I2S Amplifier (Recommended Cheap & Modern Alternative)**
   - **Where files live**: Directly inside the **ESP32-S3's onboard flash memory (`LittleFS`)**!
   - **How files get there**: The ESP32 downloads the team's MP3 file directly from GitHub over Wi-Fi and saves it to `/littlefs/team_horn.mp3`.
   - **How it works**: The ESP32-S3's dual-core processor decodes the MP3 file in software and streams digital audio directly via I2S to the MAX98357A amplifier.
   - **Cost**: **~$0.80 – $1.20 on AliExpress**!
   - **Form Factor**: Tiny breakout (~17 mm × 15 mm) that fits in the smallest base.

---

### Speaker Requirement (Required for Both Options)

Neither the DFPlayer Pro nor the MAX98357A has a built-in speaker—they are audio decoders/amplifiers. You **must connect a separate miniature speaker** to the amplifier's screw terminals or solder pads (`SPK+` / `SPK-`):

| Speaker Type | Diameter / Dimensions | Impedance & Power | Sound Quality | AliExpress Cost |
| :--- | :--- | :--- | :--- | :--- |
| **Enclosed Cavity Speaker** *(Recommended)* | ~30 mm × 20 mm × 10 mm | 4Ω 3W or 8Ω 2W | Best bass & resonance (sounds like a real horn) | ~$0.90 – $1.50 |
| **Round Mylar/Paper Speaker** | 28 mm, 36 mm, or 40 mm round | 4Ω 3W or 8Ω 2W | Good loudness, compact flat profile | ~$0.50 – $0.80 |

> [!TIP]
> Search AliExpress for: **"3W 4R speaker cavity"** or **"4 ohm 3W 28mm speaker"**. Enclosed cavity speakers produce significantly more volume and low-end punch inside a hollow 3D-printed plastic base.

---

### MAX98357A I2S Wiring to ESP32-S3 Super Mini:

| MAX98357A Pin | ESP32-S3 Super Mini Pin | Description |
| :--- | :--- | :--- |
| **VIN** | `5V` | 5V Power bus |
| **GND** | `GND` | Ground |
| **BCLK** | `GPIO 1` | I2S Bit Clock |
| **LRC** (WS) | `GPIO 2` | I2S Word Select / Left-Right Clock |
| **DIN** | `GPIO 8` | I2S Serial Data In |
| **SD_MODE** | Unconnected | Default stereo downmix to mono |
| **GAIN** | Unconnected (or to GND via 100k) | Default +9dB gain |
| **+ / -** | Speaker leads | Directly to 4Ω 3W speaker |

---

## 6. Physical 3D Printed Enclosure Notes

The classic caged goal light design consists of 4 printed sections:

1. **Base Housing**:
   - Hollow cylinder with internal mounting standoffs for the ESP32 board and audio amplifier.
   - Rear cutout for a USB-C breakout / panel-mount port.
   - Side or bottom cutout for the volume/test button.
   - Bottom acoustic vents or perforated speaker grille.
2. **Reflector / LED Mount Shelf**:
   - Internal horizontal ring platform to hold the 16/24 NeoPixel ring pointing upward into the dome.
   - Center hollow channel allowing wires (and future display ribbon cables) to pass up to the top.
3. **Dome (Diffuser)**:
   - Printed in translucent red PETG or clear red resin.
   - Bell/dome shape to scatter light smoothly from the NeoPixels.
4. **Protective Outer Cage**:
   - Wireframe cage styled after authentic arena goal lights.
   - Fastens to the base with M3 hex screws or snaps into place.
