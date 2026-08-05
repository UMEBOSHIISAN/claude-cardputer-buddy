# Claude Cardputer Buddy

An independent Cardputer-focused adaptation of Anthropic's [`claude-desktop-buddy`](https://github.com/anthropics/claude-desktop-buddy), redesigned for the M5Stack Cardputer's built-in QWERTY keyboard and device-specific UX.

This project preserves the upstream BLE desk buddy concept while adapting the firmware, input model, and interaction flow for Cardputer hardware.

Character / artwork assets are not included by default unless their license is explicitly confirmed. See `THIRD_PARTY_ASSETS.md`.

## Attribution

This repository is an independent adaptation inspired by Anthropic's
[`claude-desktop-buddy`](https://github.com/anthropics/claude-desktop-buddy),
reworked for M5Stack Cardputer-specific hardware, input handling, and firmware behavior.

| | Upstream | This repo |
|---|---|---|
| Scope | Reference / example | Cardputer-targeted port |
| License | MIT | MIT (code only — see below) |
| Relationship | Original | Independent adaptation |

**Code:** MIT License — see `LICENSE`  
**Third-party assets:** See `THIRD_PARTY_ASSETS.md`

## What changed from the upstream reference

| Item | Upstream (M5StickC Plus) | This repo (Cardputer) |
|------|--------------------------|----------------------|
| MCU | ESP32 | ESP32-S3 |
| BLE | ESP32 built-in BLE | NimBLE-Arduino |
| Display | 135×240 | 240×135 (same size, rotated) |
| Power | AXP192 | M5Cardputer Power API |
| Input | 2 buttons | QWERTY keyboard |
| IMU | Accelerometer (shake) | Not required by this firmware |

## BLE Protocol (same as upstream)

- Service: Nordic UART Service `6e400001-...`
- RX: `6e400002-...` (host → device)
- TX: `6e400003-...` (device → host, notify)
- Format: UTF-8 JSON, one object per line

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `o` | Permission — Allow once |
| `d` | Permission — Deny |
| `m` | Force demo mode |
| `s` | Send status JSON to host |
| `u` | Clear BLE bond info |

## Character Assets

Place GIFs at `data/chars/<species>/<state>.gif` and flash with LittleFS.

```
data/chars/
  default/
    idle.gif
    busy.gif
    attention.gif
    celebrate.gif
    sleep.gif
    dizzy.gif
    heart.gif
```

Note: Third-party character assets are not bundled unless their license is explicitly confirmed.
See `THIRD_PARTY_ASSETS.md` for details.

## Build & Flash

Requirements: M5Stack Cardputer, USB-C data cable, and
[PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).

```bash
# Clone and enter the repository
git clone https://github.com/UMEBOSHIISAN/claude-cardputer-buddy.git
cd claude-cardputer-buddy

# Compile + flash firmware
pio run --target upload

# Flash LittleFS (character GIFs)
pio run --target uploadfs

# Serial monitor
pio device monitor
```

## Directory Structure

```
claude-cardputer-buddy/
├── LICENSE
├── THIRD_PARTY_ASSETS.md
├── platformio.ini
├── docs/
│   └── ROADMAP.md
├── src/
│   ├── main.cpp
│   ├── config.h
│   ├── state_machine.h
│   ├── ble_handler.h/cpp
│   ├── display_handler.h/cpp
│   └── char_system.h/cpp
└── data/
    └── chars/
```

## Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for planned extensions including M5Stack ecosystem modules
(GPS, camera, ENV sensor, NFC/RFID).
