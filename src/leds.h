// leds.h - 64 square LEDs (+1 sacrificial level shifter at index 0), snake wired.
// Square numbering everywhere: a1=0 ... h8=63.
// External API:
//   ledsBegin()  ledsTick()
//   ledSetBrightness(0..255)
//   ledsClear()                              all squares off (hint layer)
//   ledSq(sq, CRGB)                          persistent colour on a square
//   ledsShowLastMove(from,to)                blue from / green to
//   ledsShowOptions(from, destMask, capMask) white-blue origin, green dots, red captures
//   ledsFlash(sq, CRGB, ms)                  temporary overlay (wrong move etc.)
//   ledsFlashMask(mask, CRGB, ms)
//   ledsRainbowTest()
//   hwToSq(row,col) / sqToHw(sq,&row,&col)   physical <-> chess square, honours gSet.flipHw
#pragma once
#include <FastLED.h>
#include "gchess_config.h"
#include "storage.h"

static CRGB leds[LED_COUNT];
static CRGB ledLayer[64];
static CRGB ledOverlay[64];
static uint32_t ledOverlayUntil[64];
static bool ledDirty = true;
static uint32_t ledLastShow = 0;

// row 0 = top row as seen by the user (rank 8 when white sits at the bottom)
inline int hwToSq(int row, int col) {
    return gSet.flipHw ? (row * 8 + (7 - col)) : ((7 - row) * 8 + col);
}

// Chess square -> board position (row, column).
inline void sqToHw(int sq, int* row, int* col) {
    int f = sq & 7, r = sq >> 3;
    if (gSet.flipHw) {
        *row = r;
        *col = 7 - f;
    } else {
        *row = 7 - r;
        *col = f;
    }
}

// Chess square -> number of the LED in the snake-wired strip.
inline int sqToLed(int sq) {
    int row, col;
    sqToHw(sq, &row, &col);
    return 1 + row * 8 + ((row & 1) ? (7 - col) : col);
}

// Set the LED brightness (5..255).
inline void ledSetBrightness(uint8_t b) {
    gSet.ledBright = b;
    FastLED.setBrightness(b);
    ledDirty = true;
}

// Start the LED strip with a power limit.
inline void ledsBegin() {
    FastLED.addLeds<WS2812B, PIN_LED_DATA, GRB>(leds, LED_COUNT);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_MA);
    FastLED.setBrightness(gSet.ledBright);
    memset(ledLayer, 0, sizeof(ledLayer));
    FastLED.clear(true);
    leds[0] = CRGB::Black;
}

// Switch all square LEDs off (hint layer).
inline void ledsClear() {
    memset(ledLayer, 0, sizeof(ledLayer));
    ledDirty = true;
}

// (LED changes are not logged: too frequent)
inline void ledSq(int sq, CRGB c) {
    if (sq >= 0 && sq < 64) {
        ledLayer[sq] = c;
        ledDirty = true;
    }
}

// Blink one square for a short time (on top of the hints).
inline void ledsFlash(int sq, CRGB c, uint16_t ms) {
    if (sq >= 0 && sq < 64) {
        ledOverlay[sq] = c;
        ledOverlayUntil[sq] = millis() + ms;
        ledDirty = true;
    }
}

// Blink several squares.
inline void ledsFlashMask(uint64_t m, CRGB c, uint16_t ms) {
    for (int i = 0; i < 64; i++) if (m >> i & 1) ledsFlash(i, c, ms);
}

// Blue = square the piece left, green = square it moved to.
inline void ledsShowLastMove(int from, int to) {
    if (from >= 0) ledSq(from, CRGB(0, 70, 255));
    if (to >= 0) ledSq(to, CRGB(0, 255, 60));
}

// Selected piece blue, free squares green, captures red.
inline void ledsShowOptions(int from, uint64_t dest, uint64_t caps) {
    ledsClear();
    ledSq(from, CRGB(0, 70, 255));
    for (int i = 0; i < 64; i++) if (dest >> i & 1) ledSq(i, (caps >> i & 1) ? CRGB(255, 20, 0) : CRGB(0, 255, 60));
}

// Colour every LED differently (wiring test).
inline void ledsRainbowTest() {
    for (int sq = 0; sq < 64; sq++) ledLayer[sq] = CHSV(sq * 4, 255, 255);
    ledDirty = true;
}

// call every ~10 ms; show() only when something changed (keeps the CPU free)
inline void ledsTick() {
    uint32_t now = millis();
    for (int i = 0; i < 64; i++) if (ledOverlayUntil[i] && now > ledOverlayUntil[i]) {
        ledOverlayUntil[i] = 0;
        ledDirty = true;
    }
    if (!ledDirty || now - ledLastShow < 16) return;
    ledDirty = false;
    ledLastShow = now;
    for (int sq = 0; sq < 64; sq++) {
        CRGB c = ledOverlayUntil[sq] ? ledOverlay[sq] : ledLayer[sq];
        if (ledOverlayUntil[sq] && ((now / 120) & 1)) c = CRGB::Black;  // blink overlays
        leds[sqToLed(sq)] = c;
    }
    FastLED.show();
}
