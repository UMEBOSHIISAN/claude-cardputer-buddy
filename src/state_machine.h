#pragma once
#include <Arduino.h>

// ── Device operating mode ────────────────────────────────────────────────────
enum DeviceMode : uint8_t {
    MODE_DEMO,    // no host connected — cycles demo scenarios
    MODE_LIVE,    // JSON heartbeats arriving within timeout
    MODE_ASLEEP   // was connected, timed out
};

// ── Character emotional state ────────────────────────────────────────────────
enum BuddyState : uint8_t {
    B_SLEEP,
    B_IDLE,
    B_BUSY,
    B_ATTENTION,
    B_CELEBRATE,
    B_DIZZY,
    B_HEART
};

// ── Live session data (populated from BLE JSON) ──────────────────────────────
struct SessionData {
    int  total        = 0;
    int  running      = 0;
    int  waiting      = 0;
    int  tokens_today = 0;

    String owner_name = "Claude";
    String device_name = "";
    String last_msg   = "";

    // pending permission prompt
    String perm_id   = "";
    String perm_tool = "";
    String perm_hint = "";

    unsigned long last_update_ms = 0;
};

// ── Derive buddy state from session data ─────────────────────────────────────
inline BuddyState deriveBuddyState(const SessionData& s, DeviceMode mode) {
    if (mode == MODE_ASLEEP || mode == MODE_DEMO) return B_SLEEP;
    if (!s.perm_id.isEmpty())                     return B_ATTENTION;
    if (s.running > 0)                            return B_BUSY;
    if (s.waiting > 0)                            return B_IDLE;
    if (s.tokens_today > 0)                       return B_IDLE;
    return B_SLEEP;
}
