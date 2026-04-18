# Roadmap

## Current Status

Working Cardputer port of the upstream BLE protocol and character display system.
Core features (BLE NUS, display, keyboard input, permission handling) are functional.

## Planned Extensions

### Bluetooth / Protocol
- Protocol compatibility notes with Claude Code desktop updates
- Multi-session handling improvements
- Bond management UX

### Keyboard UX
- Configurable key bindings
- Macro sequences for common approvals

### Character Pack System
- Clean slot definition for user-supplied character GIFs
- manifest validation at boot

### Hardware Extensions (M5Stack ecosystem)
- ENV IV unit — temperature/humidity/pressure → pass to Claude context
- GPS unit — location-aware context
- Camera unit (OV2640) — vision input
- ToF sensor — proximity detection as gesture input
- NFC/RFID — card-tap command trigger

### Power / Sleep
- Battery level reporting via BLE TX
- Deep sleep on BLE disconnect timeout

### Firmware Quality
- IMU shake detection (hardware present, unused)
- OTA update support

## Not Planned (upstream concerns)

Upstream ports are explicitly out of scope for the reference repo (`CONTRIBUTING.md`).
This repo is a standalone fork — no upstream PRs planned.
