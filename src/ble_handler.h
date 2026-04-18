#pragma once
#include <Arduino.h>
#include <NimBLEDevice.h>
#include "config.h"
#include "state_machine.h"

// ── Public API ───────────────────────────────────────────────────────────────
void  bleInit(const String& deviceName);
void  bleWrite(const String& json);        // send JSON to host
bool  bleConnected();
bool  blePoll(SessionData& out);           // parse pending RX; returns true if updated
void  bleClearBond();
String bleLocalName();
