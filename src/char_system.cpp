#include "char_system.h"
#include "config.h"
#include <M5Cardputer.h>
#include <ArduinoJson.h>

static AnimatedGIF   s_gif;
static BuddyState    s_lastState   = (BuddyState)255;
static String        s_lastSpecies = "";
static bool          s_gifOpen     = false;
static unsigned long s_lastFrameMs = 0;
static int           s_frameDelay  = 200; // ms、GIF から取得
static File          s_gifFile;

// idle バリエーション
static String  s_idleVariants[16];
static int     s_idleCount  = 0;
static int     s_idleIndex  = 0;

// GIF フレームをキャッシュするサブスプライト
// → 毎フレーム必ず blit するのでちかちかなし
static LGFX_Sprite s_gifSprite;
static bool        s_gifSpriteReady = false;
static int         s_gifW = 96, s_gifH = 96;

// ── AnimatedGIF LittleFS コールバック ─────────────────────────────────────────
static void* gifFileOpen(const char* fname, int32_t* pSize) {
    s_gifFile = LittleFS.open(fname, "r");
    if (!s_gifFile) return nullptr;
    *pSize = s_gifFile.size();
    return (void*)&s_gifFile;
}
static void gifFileClose(void*) { s_gifFile.close(); }
static int32_t gifFileRead(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen) {
    int32_t n = ((File*)pFile->fHandle)->read(pBuf, iLen);
    pFile->iPos += n;
    return n;
}
static int32_t gifFileSeek(GIFFILE* pFile, int32_t iPos) {
    ((File*)pFile->fHandle)->seek(iPos);
    pFile->iPos = iPos;
    return iPos;
}

// ── GIF draw callback → サブスプライトに描画 ─────────────────────────────────
static void gifDraw(GIFDRAW* pDraw) {
    if (!s_gifSpriteReady) return;
    uint16_t* pal = (uint16_t*)pDraw->pPalette;
    uint8_t*  pix = pDraw->pPixels;
    int y = pDraw->iY + pDraw->y;
    for (int i = 0; i < pDraw->iWidth; i++) {
        uint8_t idx = pix[i];
        if (pDraw->ucHasTransparency && idx == pDraw->ucTransparent) continue;
        s_gifSprite.drawPixel(pDraw->iX + i, y, __builtin_bswap16(pal[idx]));
    }
}

// ── manifest.json パース ──────────────────────────────────────────────────────
static void loadManifest(const String& species) {
    s_idleCount = 0;
    String path = "/chars/" + species + "/manifest.json";
    File f = LittleFS.open(path, "r");
    if (!f) { s_idleVariants[0] = "idle.gif"; s_idleCount = 1; return; }
    JsonDocument doc;
    deserializeJson(doc, f);
    f.close();
    JsonVariantConst idle = doc["states"]["idle"];
    if (idle.is<JsonArrayConst>()) {
        for (JsonVariantConst v : idle.as<JsonArrayConst>()) {
            if (s_idleCount >= 16) break;
            s_idleVariants[s_idleCount++] = v.as<String>();
        }
    } else if (idle.is<const char*>()) {
        s_idleVariants[0] = idle.as<String>(); s_idleCount = 1;
    }
    if (s_idleCount == 0) { s_idleVariants[0] = "idle.gif"; s_idleCount = 1; }
    Serial.printf("[CHAR] %d idle variants\n", s_idleCount);
}

static const char* stateFile(BuddyState s) {
    switch (s) {
        case B_BUSY:      return "busy.gif";
        case B_ATTENTION: return "attention.gif";
        case B_CELEBRATE: return "celebrate.gif";
        case B_DIZZY:     return "dizzy.gif";
        case B_HEART:     return "heart.gif";
        case B_SLEEP:     return "sleep.gif";
        default:          return nullptr;
    }
}

static bool openGif(const String& species, BuddyState state) {
    if (s_gifOpen) { s_gif.close(); s_gifOpen = false; }

    String filename = (state == B_IDLE)
        ? s_idleVariants[s_idleIndex % s_idleCount]
        : String(stateFile(state));

    String path = "/chars/" + species + "/" + filename;
    if (!s_gif.open(path.c_str(), gifFileOpen, gifFileClose, gifFileRead, gifFileSeek, gifDraw)) {
        Serial.printf("[CHAR] open failed: %s\n", path.c_str());
        return false;
    }

    // サブスプライトをGIFサイズに合わせて作成
    s_gifW = s_gif.getCanvasWidth();
    s_gifH = s_gif.getCanvasHeight();
    if (s_gifSpriteReady) s_gifSprite.deleteSprite();
    s_gifSprite.setColorDepth(16);
    s_gifSprite.createSprite(s_gifW, s_gifH);
    s_gifSprite.fillScreen(0x0000);
    s_gifSpriteReady = true;

    s_gifOpen = true;
    Serial.printf("[CHAR] loaded: %s (%dx%d)\n", path.c_str(), s_gifW, s_gifH);
    return true;
}

// ── Public ────────────────────────────────────────────────────────────────────
void charSystemInit() {
    if (!LittleFS.begin(false))
        Serial.println("[CHAR] LittleFS mount failed — run pio uploadfs");
    s_gif.begin(GIF_PALETTE_RGB565_BE);
}

bool charSystemLoad(const String& species) {
    loadManifest(species);
    s_lastSpecies = species;
    s_idleIndex   = 0;
    return openGif(species, B_IDLE);
}

// フレームを進める（毎ループ呼ぶ）
void charSystemTick(BuddyState state, const String& species) {
    bool speciesChanged = (species != s_lastSpecies);
    bool stateChanged   = (state   != s_lastState);

    if (speciesChanged) { loadManifest(species); s_lastSpecies = species; s_idleIndex = 0; }
    if (stateChanged || speciesChanged) {
        s_lastState = state;
        if (state == B_IDLE) s_idleIndex = 0;
        openGif(species, state);
    }
    if (!s_gifOpen) return;

    unsigned long now = millis();
    if (now - s_lastFrameMs < (unsigned long)s_frameDelay) return;

    s_gifSprite.fillScreen(0x0000);
    int delayMs = 0;
    int result = s_gif.playFrame(false, &delayMs);
    s_lastFrameMs = now;
    if (delayMs > 0) s_frameDelay = delayMs;

    if (result == 0) {
        // GIF 終了
        if (state == B_IDLE && s_idleCount > 1) {
            s_idleIndex = (s_idleIndex + 1) % s_idleCount;
            openGif(species, B_IDLE);
        } else {
            s_gif.reset();
        }
    }
}

// キャンバスに現在フレームを貼り付け（毎フレーム呼ぶ）
void charSystemBlit(LGFX_Sprite* canvas) {
    if (!s_gifSpriteReady || !canvas) return;
    int x = (80 - s_gifW) / 2;
    int y = 14 + (107 - s_gifH) / 2; // top bar 下、hint bar 上でセンタリング
    s_gifSprite.pushSprite(canvas, x, y);
}
