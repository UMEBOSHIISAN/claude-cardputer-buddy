#include <M5Cardputer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "state_machine.h"
#include "ble_handler.h"
#include "display_handler.h"
#include "char_system.h"

// ── State ─────────────────────────────────────────────────────────────────────
static SessionData  g_session;
static DeviceMode   g_mode      = MODE_DEMO;
static BuddyState   g_buddy     = B_SLEEP;
static String       g_species   = "claude";
static bool         g_dispOn    = true;

static unsigned long g_lastHbMs   = 0;  // last heartbeat received
static unsigned long g_lastDrawMs = 0;
static unsigned long g_demoMs     = 0;
static int           g_demoScene  = 0;

// ── Demo scenario ─────────────────────────────────────────────────────────────
static void advanceDemoScene() {
    g_demoScene = (g_demoScene + 1) % DEMO_SCENE_COUNT;
    switch (g_demoScene) {
        case 0: g_buddy = B_IDLE;      g_session.last_msg = "Waiting...";        break;
        case 1: g_buddy = B_BUSY;      g_session.last_msg = "Running tool...";   break;
        case 2: g_buddy = B_ATTENTION; g_session.last_msg = "Permission needed"; break;
        case 3: g_buddy = B_CELEBRATE; g_session.last_msg = "Task complete!";    break;
        case 4: g_buddy = B_HEART;     g_session.last_msg = "Connected!";        break;
    }
}

// ── Permission response sender ────────────────────────────────────────────────
static void sendPermission(const String& decision) {
    if (g_session.perm_id.isEmpty()) return;
    JsonDocument doc;
    doc["cmd"]      = "permission";
    doc["id"]       = g_session.perm_id;
    doc["decision"] = decision;
    String out;
    serializeJson(doc, out);
    bleWrite(out);
    g_session.perm_id   = "";
    g_session.perm_tool = "";
    g_session.perm_hint = "";
    Serial.printf("[PERM] sent %s\n", decision.c_str());
}

// ── Status response ───────────────────────────────────────────────────────────
static void sendStatus() {
    JsonDocument doc;
    doc["uptime_ms"]    = millis();
    doc["free_heap"]    = ESP.getFreeHeap();
    doc["battery_mv"]   = (int)(M5Cardputer.Power.getBatteryVoltage());
    doc["battery_pct"]  = M5Cardputer.Power.getBatteryLevel();
    String out;
    serializeJson(doc, out);
    bleWrite(out);
}

// ── Keyboard handler ──────────────────────────────────────────────────────────
static void handleKeyboard() {
    M5Cardputer.update();
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;

    auto keys = M5Cardputer.Keyboard.keysState();
    for (char c : keys.word) {
        switch (tolower(c)) {
            case KEY_PERMIT_ONCE:
                if (!g_session.perm_id.isEmpty()) sendPermission("once");
                break;
            case KEY_PERMIT_DENY:
                if (!g_session.perm_id.isEmpty()) sendPermission("deny");
                break;
            case KEY_MODE_DEMO:
                g_mode = MODE_DEMO;
                Serial.println("[KEY] forced DEMO mode");
                break;
            case 's':
                sendStatus();
                break;
            case 'u':
                bleClearBond();
                displayMessage("Bond cleared", "Restart to re-pair");
                delay(2000);
                break;
            default: break;
        }
    }
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    Serial.begin(115200);
    Serial.println("\n[BOOT] Claude Cardputer Buddy");

    displayInit();

    // Build device name: "Claude-XXYY" from last 2 bytes of MAC
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_BT);
    char devName[16];
    snprintf(devName, sizeof(devName), "Claude-%02X%02X", mac[4], mac[5]);

    displaySplash(devName);

    bleInit(devName);
    charSystemInit();

    g_demoMs = millis();
    advanceDemoScene();

    Serial.printf("[BOOT] ready as %s\n", devName);
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    // 1. Keyboard input
    handleKeyboard();

    // 2. BLE poll
    bool updated = blePoll(g_session);
    if (updated) {
        g_lastHbMs = now;
        // wake display if sleeping
        if (!g_dispOn) { displayOn(); g_dispOn = true; }
    }

    // 3. Mode transition
    bool ble_ok = bleConnected();
    if (g_mode != MODE_DEMO) {
        if (!ble_ok) {
            g_mode = MODE_ASLEEP;
        } else if (now - g_lastHbMs < IDLE_TIMEOUT_MS) {
            g_mode = MODE_LIVE;
        } else {
            g_mode = MODE_ASLEEP;
        }
    } else {
        // heartbeat 受信で初めて DEMO を抜ける
        if (ble_ok && now - g_lastHbMs < IDLE_TIMEOUT_MS) g_mode = MODE_LIVE;
    }

    // 4. Demo scene advance
    if (g_mode == MODE_DEMO && now - g_demoMs > DEMO_SCENE_MS) {
        g_demoMs = now;
        advanceDemoScene();
    }

    // 5. Derive buddy state
    BuddyState nextBuddy = deriveBuddyState(g_session, g_mode);
    if (g_mode == MODE_DEMO) nextBuddy = g_buddy; // demo overrides
    g_buddy = nextBuddy;

    // 6. Display idle timeout (skip if USB powered — always on)
    bool usb_powered = M5Cardputer.Power.isCharging();
    if (!usb_powered && g_dispOn && now - g_lastHbMs > IDLE_TIMEOUT_MS) {
        displayOff();
        g_dispOn = false;
    } else if (!g_dispOn && updated) {
        displayOn();
        g_dispOn = true;
    }

    // 7. Render at ~LOOP_FPS
    unsigned long frameMs = 1000 / LOOP_FPS;
    if (now - g_lastDrawMs >= frameMs) {
        g_lastDrawMs = now;

        if (g_dispOn) {
            if (!g_session.perm_id.isEmpty() && g_mode == MODE_LIVE) {
                displayPermission(g_session);
                displayFlush();
            } else {
                // 1. フレーム進行（サブスプライト更新）
                charSystemTick(g_buddy, g_species);
                // 2. UI をキャンバスに描く
                displayHome(g_session, g_mode, g_buddy, ble_ok);
                // 3. GIF をキャンバスに貼り付け（毎フレーム必ず）
                charSystemBlit(displayGetCanvas());
                // 4. まとめて転送
                displayFlush();
            }
        }
    }

    // 8. Yield
    delay(1);
}
