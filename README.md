# SELT Scoreboard Controller

A Wi-Fi-controlled electronic scoreboard controller built with the **ESP32** microcontroller. Designed for the Public Sports Complex of Lavras (SELT), it allows real-time scoreboard management for sports events directly from a smartphone or browser — no app installation required.

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [System Architecture](#system-architecture)
- [Hardware](#hardware)
  - [Components](#components)
  - [GPIO Pinout](#gpio-pinout)
  - [ESP32 External Antenna Mod](#esp32-external-antenna-mod)
  - [Prototype](#prototype)
- [Software](#software)
  - [Dependencies](#dependencies)
  - [Project Structure](#project-structure)
- [Setup & Flashing](#setup--flashing)
- [Usage](#usage)
- [REST API Reference](#rest-api-reference)
- [Web Interface](#web-interface)
- [Author](#author)

---

## Overview

The controller runs a lightweight **async HTTP server** directly on the ESP32, which acts as a Wi-Fi access point. Users connect to the ESP32's network and access the control panel through any browser. Each button press on the interface sends an HTTP request to the ESP32, which then outputs precise **digital pulses** on GPIO pins to drive the physical scoreboard hardware.

---

## Features

- Add **1, 2, or 3 points** for each team with a single tap
- Track **fouls/sets** per team (up to 19)
- Increment **period/set counter** (up to 10)
- Toggle the **scoreboard timer** (start/pause)
- **Inline score editing** — manually set any value without resetting the board
- **Reset points** with a short press; **full reset** (points + fouls + period) with a 2-second long press (prevents accidental resets)
- Responsive web UI that works on **mobile and desktop**
- **No app required** — runs entirely in the browser
- State is kept in sync via JSON responses on every action

---

## System Architecture

```
┌──────────────────────────┐
│  Browser (any device)    │  ← Control panel UI (HTML/CSS/JS)
└────────────┬─────────────┘
             │  HTTP requests over Wi-Fi
┌────────────▼─────────────┐
│  ESP32 (Access Point)    │  ← Async web server (C++ / Arduino)
│  IP: 192.168.1.1         │
│  mDNS: placar.local      │
└────────────┬─────────────┘
             │  Digital GPIO pulses
┌────────────▼─────────────┐
│  Physical Scoreboard     │  ← Electronic scoreboard hardware
└──────────────────────────┘
```

The ESP32 operates as a standalone **Wi-Fi access point**, so no external router or internet connection is needed. Files (fonts, icons) are served from **LittleFS**, the ESP32's onboard flash filesystem.

---

## Hardware

### Components

| Component | Quantity | Description |
|---|---|---|
| ESP32 DevKit | 1 | Main microcontroller (Wi-Fi + dual-core) |
| BC546 NPN transistor | 6 | One per GPIO output — acts as a switch to drive the scoreboard signals |
| 100 Ω resistor | 6 | Collector resistors — one per transistor |
| 4.7 kΩ resistor | 6 | Base resistors — one per transistor, limits base current from ESP32 GPIO |
| Perfboard | 1 | For soldering the final prototype |
| SMA connector + external antenna | 1 | For extended Wi-Fi range (see mod below) |
| Jumper wires / solder | — | Connections to the scoreboard |

### Transistor Switch Circuit

Each of the 6 GPIO outputs drives a **BC546 NPN transistor** configured as a switch. This decouples the ESP32's 3.3 V logic from the scoreboard's own circuitry, protecting the microcontroller from voltage or current spikes.

```
ESP32 GPIO ──[ 4.7 kΩ ]──► Base (BC546)
                             │
                          Collector ──[ 100 Ω ]──► Scoreboard input
                             │
                          Emitter ──► GND
```

- The **4.7 kΩ base resistor** limits the current sourced by the ESP32 GPIO pin (~0.7 mA), well within the pin's 12 mA maximum.
- The **100 Ω collector resistor** provides additional current limiting on the output side.
- When the GPIO goes HIGH (3.3 V), the transistor saturates and pulls the scoreboard input LOW through the collector. Adjust the logic polarity in firmware if the scoreboard expects active-HIGH pulses.

### GPIO Pinout

| Signal | GPIO Pin | Description |
|---|---|---|
| Team A — Points | 4 | Pulse to increment Team A score |
| Team B — Points | 16 | Pulse to increment Team B score |
| Team A — Fouls | 17 | Pulse to increment Team A fouls |
| Team B — Fouls | 5 | Pulse to increment Team B fouls |
| Timer | 18 | Toggle start/pause on the scoreboard timer |
| Period | 19 | Pulse to advance the period/set counter |

**Pulse timing:**
- Normal pulse: `180 ms` HIGH + `180 ms` LOW per pulse
- Reset pulse: `1800 ms` HIGH (held by both score pins simultaneously)

### ESP32 External Antenna Mod

The stock ESP32 DevKit uses a small **PCB trace antenna**, which can have limited range in larger venues. To improve signal strength and reliability, the onboard antenna was bypassed and replaced with an **external antenna connected via an SMA connector**.

This modification follows the procedure shown in this video:  
[**How to add an external antenna to ESP32**](https://www.youtube.com/watch?v=-8hxk81H6QI)

The mod involves moving a tiny **0-ohm resistor** (or solder bridge) on the ESP32 module from the PCB antenna path to the U.FL/IPEX pad, then connecting an external antenna through an SMA pigtail.

> **Note:** This modification requires soldering skills and care with SMD components. Proceed at your own risk.

### Prototype

> _Photos of the final prototype soldered on the perfboard will be added here._

<!-- Add your photos below -->
<!-- ![Front view](docs/images/prototype-front.jpg) -->
<!-- ![Side view](docs/images/prototype-side.jpg) -->
<!-- ![Antenna mod close-up](docs/images/antenna-mod.jpg) -->

---

## Software

### Dependencies

This is an **Arduino/ESP32** project. Install the following before opening the `.ino` file:

1. **Arduino IDE** (or PlatformIO)
2. **ESP32 board support** — install via Arduino Board Manager:  
   URL: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. **ESPAsyncWebServer** library by me-no-dev:  
   [github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
4. **AsyncTCP** (dependency of ESPAsyncWebServer):  
   [github.com/me-no-dev/AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

The following libraries are **built into the ESP32 Arduino core** and require no separate installation:
- `WiFi.h`
- `ESPmDNS.h`
- `LittleFS.h`
- `FS.h`

### Project Structure

```
selt-scoreboard-controller/
├── html-css/                        # Standalone UI files (for development/preview)
│   ├── index.html
│   └── styles.css
├── placar-selt/
│   └── controller-code/
│       ├── controller-code.ino      # Main ESP32 firmware
│       └── data/                    # Files uploaded to LittleFS flash storage
│           ├── sevenSegment.ttf     # Seven-segment display font
│           └── github.svg           # GitHub icon used in the footer
├── fonts/
│   └── sevenSegment.ttf
└── README.md
```

> The `html-css/` folder contains a standalone version of the interface for easier development and preview in a regular browser. The production version is embedded as a raw string inside `controller-code.ino`.

---

## Setup & Flashing

### 1. Clone the repository

```bash
git clone https://github.com/TiagoH1ro/selt-scoreboard-controller.git
cd selt-scoreboard-controller
```

### 2. Open the firmware

Open `placar-selt/controller-code/controller-code.ino` in Arduino IDE.

### 3. Configure board settings

In Arduino IDE, select:
- **Board:** `ESP32 Dev Module`
- **Partition Scheme:** `Default 4MB with spiffs` (or any scheme with at least 1 MB for app + 1 MB for filesystem)
- **Upload Speed:** `115200`

### 4. Upload the filesystem (LittleFS)

The font and icon files must be uploaded separately to the ESP32's flash:

1. Install the **Arduino LittleFS upload plugin** for your IDE version
2. Place all files from `placar-selt/controller-code/data/` in the `data/` folder (already done)
3. Use **Tools → ESP32 LittleFS Data Upload** to flash the filesystem

### 5. Flash the firmware

Click **Upload** in Arduino IDE. After flashing, open the Serial Monitor at `115200` baud to confirm the server starts successfully.

---

## Usage

1. On your device (phone, tablet, or laptop), go to **Wi-Fi settings**
2. Connect to the network: **`Controlador Placar - SELT`**  
   Password: `set-up-your-password`
3. Open a browser and navigate to:
   - `http://192.168.1.1` — direct IP access
   - `http://placar.local` — mDNS (may not work on all Android devices)
4. The scoreboard control panel will load. Use the buttons to control the physical scoreboard.

---

## REST API Reference

All endpoints use `HTTP GET` unless otherwise noted. Responses are `application/json`.

### State response format

```json
{
  "teamAScore": 0,
  "teamBScore": 0,
  "teamAFaults": 0,
  "teamBFaults": 0,
  "currentPeriod": 1
}
```

### Endpoints

| Method | Endpoint | Description |
|---|---|---|
| GET | `/start` | Returns the current scoreboard state |
| GET | `/teamA/add1` | Add 1 point to Team A |
| GET | `/teamA/add2` | Add 2 points to Team A |
| GET | `/teamA/add3` | Add 3 points to Team A |
| GET | `/teamB/add1` | Add 1 point to Team B |
| GET | `/teamB/add2` | Add 2 points to Team B |
| GET | `/teamB/add3` | Add 3 points to Team B |
| GET | `/teamA/fault` | Add 1 foul to Team A |
| GET | `/teamB/fault` | Add 1 foul to Team B |
| GET | `/timer` | Toggle the scoreboard timer |
| GET | `/period` | Advance the period/set counter |
| GET | `/resetPts` | Reset points only (keeps fouls and period) |
| GET | `/resetAll` | Full reset — points, fouls, and period |
| POST | `/updateScore` | Set all values at once (used by the edit mode) |

#### POST `/updateScore` — body parameters (form-urlencoded)

| Parameter | Type | Description |
|---|---|---|
| `teamAScore` | integer | New score for Team A |
| `teamBScore` | integer | New score for Team B |
| `teamAFaults` | integer | New foul count for Team A |
| `teamBFaults` | integer | New foul count for Team B |
| `currentPeriod` | integer | New period/set number |

---

## Web Interface

The control panel is a responsive single-page app served directly by the ESP32.

**Scoreboard display panel:**
- Team A and B scores (large seven-segment style digits)
- Fouls/sets per team (top corners)
- Current period (center top)
- Timer display with pulsing animation

**Controls:**
- `+1`, `+2`, `+3` — point buttons per team
- `FALTA` — foul button per team
- `PERÍODO` — advance the period
- `CRONÔMETRO` — toggle the timer
- `RESETAR PONTOS` — short press resets points; **hold 2 seconds** for a full reset
- `⚙️` edit button — enables inline editing of all values simultaneously

---

## Author

Made by **Tiago Oliveira** — [github.com/TiagoH1ro](https://github.com/TiagoH1ro)
