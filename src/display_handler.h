#pragma once
#include <M5Cardputer.h>
#include "state_machine.h"

void displayInit();
void displaySplash(const String& deviceName);
void displayHome(const SessionData& s, DeviceMode mode, BuddyState buddy, bool ble_connected);
void displayPermission(const SessionData& s);
void displayMessage(const String& line1, const String& line2 = "");
void displayFlush();       // スプライトを画面に転送
void displayOff();
void displayOn();

LGFX_Sprite* displayGetCanvas(); // GIF描画用キャンバス取得
