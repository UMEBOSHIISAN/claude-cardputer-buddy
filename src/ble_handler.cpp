#include "ble_handler.h"
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

// ── Internal state ────────────────────────────────────────────────────────────
static NimBLEServer*         s_server  = nullptr;
static NimBLECharacteristic* s_tx      = nullptr;
static bool                  s_connected = false;
static String                s_localName;

// Ring-buffer for incoming BLE data
static char   s_rxBuf[BLE_RX_BUF_SIZE];
static int    s_rxHead = 0;
static int    s_rxTail = 0;

static inline int rxAvail() { return (s_rxHead - s_rxTail + BLE_RX_BUF_SIZE) % BLE_RX_BUF_SIZE; }

static void rxPush(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        int next = (s_rxHead + 1) % BLE_RX_BUF_SIZE;
        if (next == s_rxTail) return; // overflow: drop
        s_rxBuf[s_rxHead] = (char)data[i];
        s_rxHead = next;
    }
}

static bool rxReadLine(String& out) {
    while (s_rxTail != s_rxHead) {
        char c = s_rxBuf[s_rxTail];
        s_rxTail = (s_rxTail + 1) % BLE_RX_BUF_SIZE;
        if (c == '\n') { return true; }
        out += c;
    }
    return false;
}

// ── Server callbacks ──────────────────────────────────────────────────────────
class ServerCB : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer*) override    { s_connected = true;  }
    void onDisconnect(NimBLEServer*) override {
        s_connected = false;
        NimBLEDevice::startAdvertising();
    }
};

// ── RX characteristic callback ────────────────────────────────────────────────
class RxCB : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* c) override {
        NimBLEAttValue val = c->getValue();
        rxPush(val.data(), val.length());
    }
};

// ── Init ──────────────────────────────────────────────────────────────────────
void bleInit(const String& deviceName) {
    s_localName = deviceName;
    NimBLEDevice::init(deviceName.c_str());
    NimBLEDevice::setMTU(BLE_MTU_REQUEST);

    s_server = NimBLEDevice::createServer();
    s_server->setCallbacks(new ServerCB());

    NimBLEService* svc = s_server->createService(NUS_SERVICE_UUID);

    // TX: notify (device → host)
    s_tx = svc->createCharacteristic(
        NUS_TX_UUID,
        NIMBLE_PROPERTY::NOTIFY
    );

    // RX: write (host → device)
    NimBLECharacteristic* rx = svc->createCharacteristic(
        NUS_RX_UUID,
        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
    );
    rx->setCallbacks(new RxCB());

    svc->start();

    NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
    pAdv->addServiceUUID(NUS_SERVICE_UUID);
    pAdv->setName(deviceName.c_str());
    pAdv->setScanResponse(true);
    NimBLEDevice::startAdvertising();
    Serial.printf("[BLE] advertising as \"%s\"\n", deviceName.c_str());
}

// ── Write (chunked by MTU) ────────────────────────────────────────────────────
void bleWrite(const String& json) {
    if (!s_connected || !s_tx) return;
    String payload = json + "\n";
    size_t mtu = NimBLEDevice::getMTU() - 3;
    size_t offset = 0;
    while (offset < payload.length()) {
        size_t chunk = min(mtu, payload.length() - offset);
        s_tx->setValue((const uint8_t*)payload.c_str() + offset, chunk);
        s_tx->notify();
        offset += chunk;
        delay(BLE_CHUNK_DELAY);
    }
    Serial.println("[BLE TX] " + json);
}

bool bleConnected() { return s_connected; }

String bleLocalName() { return s_localName; }

void bleClearBond() {
    NimBLEDevice::deleteAllBonds();
    Serial.println("[BLE] bonds cleared");
}

// ── Poll: parse one JSON line from RX buffer ──────────────────────────────────
static void applyJson(const JsonDocument& doc, SessionData& s) {
    // heartbeat snapshot
    if (doc["total"].is<int>())        s.total        = doc["total"];
    if (doc["running"].is<int>())      s.running      = doc["running"];
    if (doc["waiting"].is<int>())      s.waiting      = doc["waiting"];
    if (doc["tokens_today"].is<int>()) s.tokens_today = doc["tokens_today"];
    if (doc["msg"].is<const char*>())  s.last_msg     = doc["msg"].as<String>();

    // permission prompt
    if (doc["prompt"].is<JsonObjectConst>()) {
        JsonObjectConst p = doc["prompt"];
        s.perm_id   = p["id"]   | "";
        s.perm_tool = p["tool"] | "";
        s.perm_hint = p["hint"] | "";
    }

    // meta commands
    const char* cmd = doc["cmd"] | "";
    if (strcmp(cmd, "owner") == 0) s.owner_name = doc["name"] | s.owner_name;
    if (strcmp(cmd, "name")  == 0) s.device_name = doc["name"] | s.device_name;

    // time sync
    if (doc["time"].is<JsonArrayConst>()) {
        JsonArrayConst t = doc["time"];
        time_t epoch = t[0];
        struct timeval tv = { epoch, 0 };
        settimeofday(&tv, nullptr);
        Serial.printf("[BLE] time synced: %ld\n", (long)epoch);
    }

    s.last_update_ms = millis();
}

bool blePoll(SessionData& out) {
    // also drain Serial for USB tethering (same protocol)
    while (Serial.available()) {
        char c = Serial.read();
        uint8_t uc = (uint8_t)c;
        rxPush(&uc, 1);
    }

    String line;
    if (!rxReadLine(line)) return false;
    Serial.println("[BLE RX] " + line.substring(0, 80)); // debug
    if (line.length() > MAX_MSG_BYTES) {
        Serial.println("[BLE] msg too large, dropped");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, line);
    if (err) {
        Serial.printf("[BLE] JSON err: %s\n", err.c_str());
        return false;
    }

    // respond to status query
    if (doc["cmd"].is<const char*>() && strcmp(doc["cmd"], "status") == 0) {
        JsonDocument resp;
        resp["uptime_ms"] = millis();
        resp["free_heap"] = ESP.getFreeHeap();
        String out_str;
        serializeJson(resp, out_str);
        bleWrite(out_str);
        return false;
    }

    if (doc["cmd"].is<const char*>() && strcmp(doc["cmd"], "unpair") == 0) {
        bleClearBond();
        return false;
    }

    applyJson(doc, out);
    return true;
}
