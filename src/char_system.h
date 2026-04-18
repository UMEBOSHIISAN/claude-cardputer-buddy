#pragma once
#include <Arduino.h>
#include <AnimatedGIF.h>
#include <LittleFS.h>
#include <M5Cardputer.h>
#include "state_machine.h"
#include "display_handler.h"

// Manages animated GIF characters stored on LittleFS.
// Directory layout: /chars/<species>/<state>.gif
// e.g. /chars/capybara/busy.gif

void charSystemInit();
void charSystemTick(BuddyState state, const String& species = "default");
void charSystemBlit(LGFX_Sprite* canvas);
bool charSystemLoad(const String& species);
