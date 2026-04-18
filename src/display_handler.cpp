#include "display_handler.h"
#include "config.h"
#include <M5Cardputer.h>

#define BUDDY_BG      0x0000
#define BUDDY_FG      0xFFFF
#define BUDDY_CYAN    0x07FF
#define BUDDY_GREEN   0x07E0
#define BUDDY_RED     0xF800
#define BUDDY_YELLOW  0xFFE0
#define BUDDY_PURPLE  0x801F
#define BUDDY_ORANGE  0xFD20

static auto& disp = M5Cardputer.Display;

// ── ダブルバッファ用スプライト ─────────────────────────────────────────────────
static LGFX_Sprite canvas(&disp);
static bool canvasReady = false;

static LGFX_Sprite* c = nullptr; // 描画先エイリアス

static void initCanvas() {
    if (canvasReady) return;
    canvas.setColorDepth(16);
    canvas.createSprite(DISP_W, DISP_H);
    c = &canvas;
    canvasReady = true;
}

void displayFlush() {
    canvas.pushSprite(0, 0);
}

LGFX_Sprite* displayGetCanvas() {
    initCanvas();
    return &canvas;
}

// ── ユーティリティ ────────────────────────────────────────────────────────────
static const char* stateIcon(BuddyState s) {
    switch (s) {
        case B_BUSY:      return ">>>";
        case B_ATTENTION: return "!!!";
        case B_CELEBRATE: return "***";
        case B_DIZZY:     return "~~~";
        case B_HEART:     return "<3 ";
        case B_IDLE:      return "---";
        default:          return "zzz";
    }
}
static uint16_t stateColor(BuddyState s) {
    switch (s) {
        case B_BUSY:      return BUDDY_CYAN;
        case B_ATTENTION: return BUDDY_RED;
        case B_CELEBRATE: return BUDDY_YELLOW;
        case B_DIZZY:     return BUDDY_PURPLE;
        case B_HEART:     return BUDDY_RED;
        case B_IDLE:      return BUDDY_GREEN;
        default:          return 0x4208;
    }
}
static const char* modeName(DeviceMode m) {
    switch (m) {
        case MODE_LIVE:   return "LIVE";
        case MODE_DEMO:   return "DEMO";
        case MODE_ASLEEP: return "AWAY";
    }
    return "????";
}

// ── Init ──────────────────────────────────────────────────────────────────────
void displayInit() {
    disp.setRotation(1);
    disp.fillScreen(BUDDY_BG);
    initCanvas();
}

// ── Splash ────────────────────────────────────────────────────────────────────
void displaySplash(const String& deviceName) {
    initCanvas();
    c->fillScreen(BUDDY_BG);
    c->setTextColor(BUDDY_CYAN);
    c->setTextSize(2);
    c->drawCenterString("Claude", DISP_CX, 30);
    c->setTextSize(1);
    c->setTextColor(BUDDY_FG);
    c->drawCenterString("Cardputer Buddy", DISP_CX, 60);
    c->setTextColor(0x8410);
    c->drawCenterString(deviceName.c_str(), DISP_CX, 80);
    c->drawCenterString("Waiting for BLE...", DISP_CX, 100);
    displayFlush();
}

// ── Home ─────────────────────────────────────────────────────────────────────
void displayHome(const SessionData& s, DeviceMode mode, BuddyState buddy, bool ble_connected) {
    initCanvas();
    c->fillScreen(BUDDY_BG);

    // Top bar
    uint16_t bar_color = ble_connected ? BUDDY_GREEN : 0x4208;
    c->fillRect(0, 0, DISP_W, 14, bar_color);
    c->setTextColor(BUDDY_BG, bar_color);
    c->setTextSize(1);
    c->drawString(modeName(mode), 4, 3);
    c->drawRightString(ble_connected ? "BLE" : "---", DISP_W - 4, 3);

    // Stats (right zone)
    char buf[48];
    c->setTextSize(1);
    c->setTextColor(BUDDY_FG);
    snprintf(buf, sizeof(buf), "Run:%d Wait:%d Tot:%d", s.running, s.waiting, s.total);
    c->drawString(buf, 90, 20);

    c->setTextColor(BUDDY_YELLOW);
    snprintf(buf, sizeof(buf), "Tok: %d", s.tokens_today);
    c->drawString(buf, 90, 36);

    c->setTextColor(0x8410);
    c->drawString(("@" + s.owner_name).c_str(), 90, 52);

    if (s.last_msg.length() > 0) {
        String msg = s.last_msg;
        if (msg.length() > 30) msg = msg.substring(0, 29) + "~";
        c->setTextColor(BUDDY_FG);
        c->drawString(msg.c_str(), 4, 88);
    }

    // Bottom hint bar
    if (!s.perm_id.isEmpty()) {
        c->fillRect(0, 118, DISP_W, 17, BUDDY_RED);
        c->setTextColor(BUDDY_FG, BUDDY_RED);
        String hint = "[O]once [D]deny  " + s.perm_tool;
        if (hint.length() > 32) hint = hint.substring(0, 32);
        c->drawCenterString(hint.c_str(), DISP_CX, 121);
    } else {
        c->fillRect(0, 118, DISP_W, 17, 0x1082);
        c->setTextColor(0x4208, 0x1082);
        c->drawCenterString("[O]once [D]deny [M]demo", DISP_CX, 121);
    }

    // flush は main.cpp で charSystemTick 後に呼ぶ
}

// ── Permission overlay ────────────────────────────────────────────────────────
void displayPermission(const SessionData& s) {
    initCanvas();
    c->fillScreen(BUDDY_BG);
    c->fillRect(0, 0, DISP_W, 20, BUDDY_RED);
    c->setTextColor(BUDDY_FG, BUDDY_RED);
    c->setTextSize(1);
    c->drawCenterString("PERMISSION REQUEST", DISP_CX, 6);

    c->setTextColor(BUDDY_YELLOW);
    c->drawCenterString(s.perm_tool.c_str(), DISP_CX, 32);

    if (s.perm_hint.length() > 0) {
        c->setTextColor(BUDDY_FG);
        String hint = s.perm_hint;
        if (hint.length() > 36) hint = hint.substring(0, 36);
        c->drawCenterString(hint.c_str(), DISP_CX, 54);
    }

    c->fillRect(0, 108, DISP_W, 27, 0x1082);
    c->setTextColor(BUDDY_GREEN, 0x1082);
    c->drawString("[O] Allow once", 10, 112);
    c->setTextColor(BUDDY_RED, 0x1082);
    c->drawRightString("[D] Deny", DISP_W - 10, 112);

    displayFlush();
}

// ── Utility ───────────────────────────────────────────────────────────────────
void displayMessage(const String& line1, const String& line2) {
    initCanvas();
    c->fillScreen(BUDDY_BG);
    c->setTextColor(BUDDY_FG);
    c->setTextSize(1);
    c->drawCenterString(line1.c_str(), DISP_CX, 50);
    if (line2.length() > 0) c->drawCenterString(line2.c_str(), DISP_CX, 70);
    displayFlush();
}

void displayOff() { disp.setBrightness(0); }
void displayOn()  { disp.setBrightness(100); }
