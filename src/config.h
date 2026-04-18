#pragma once

// ── BLE / Nordic UART Service (unchanged from original) ─────────────────────
#define NUS_SERVICE_UUID  "6e400001-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_RX_UUID       "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
#define NUS_TX_UUID       "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
#define BLE_MTU_REQUEST   517
#define BLE_CHUNK_DELAY   4      // ms between chunks
#define BLE_RX_BUF_SIZE   2048
#define MAX_MSG_BYTES     4096

// ── Display (M5Cardputer: 240×135 landscape) ────────────────────────────────
#define DISP_W            240
#define DISP_H            135
#define DISP_CX           120
#define DISP_CY           67

// ── Timing ───────────────────────────────────────────────────────────────────
#define LOOP_FPS          60
#define CHAR_FPS          2
#define IDLE_TIMEOUT_MS   30000
#define HB_INTERVAL_MS    10000   // heartbeat re-draw interval

// ── Keyboard shortcuts (Cardputer advantage: physical keys) ──────────────────
#define KEY_PERMIT_ONCE   'o'    // approve permission once
#define KEY_PERMIT_DENY   'd'    // deny permission
#define KEY_MODE_DEMO     'm'    // force demo mode (debug)

// ── Demo cycle ───────────────────────────────────────────────────────────────
#define DEMO_SCENE_MS     8000   // ms per demo scenario
#define DEMO_SCENE_COUNT  5
