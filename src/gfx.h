// gfx.h - display driver (LovyanGFX), theme, logos, basic drawing helpers.
// External API:
//   gfxBegin()  gfxSetRotation(0|2)  gfxSetBrightness(10..255)
//   theme colours: T.bg T.card T.fg T.dim T.accent T.ok T.warn T.bad   (T changes with dark mode, call gfxApplyTheme())
//   logoSmall(cx,cy,r)   round icon for toolbar / lists
//   logoMedium(cx,cy)    ~64px mascot for splash / lock
//   logoFull(cx,y)       mascot + "Gchess" + one-line statement
//   gfxText(x,y,str,size,colour,datum)   gfxBtn(...)  gfxCard(...)  gfxSlider(...)
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "gchess_config.h"
// fallbacks in case an old config is picked up
#ifndef PIN_TFT_SCLK
#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23
#define PIN_TFT_MISO 19
#define PIN_TFT_CS   15
#define PIN_TFT_DC   17
#define PIN_TFT_RST  -1
#define PIN_TFT_BL    5
#define PIN_TOUCH_CS 21
#endif
#define GC_BLACK 0x0000
#define GC_WHITE 0xFFFF
using TD = lgfx::textdatum::textdatum_t;
static constexpr TD TD_TL = lgfx::textdatum::top_left, TD_TC = lgfx::textdatum::top_center, TD_TR = lgfx::textdatum::top_right, TD_MC = lgfx::textdatum::middle_center;
#include "storage.h"
#include "debug.h"

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ILI9341 _panel;
    lgfx::Bus_SPI _bus;
    lgfx::Light_PWM _light;
    lgfx::Touch_XPT2046 _touch;
    public: LGFX() {
        {
            auto c = _bus.config();
            c.spi_host = SPI3_HOST;
            c.spi_mode = 0;
            c.freq_write = 40000000;
            c.freq_read = 16000000;
            c.spi_3wire = false;
            c.use_lock = true;
            c.dma_channel = SPI_DMA_CH_AUTO;
            c.pin_sclk = PIN_TFT_SCLK;
            c.pin_mosi = PIN_TFT_MOSI;
            c.pin_miso = PIN_TFT_MISO;
            c.pin_dc = PIN_TFT_DC;
            _bus.config(c);
            _panel.setBus(&_bus);
        }
        {
            auto c = _panel.config();
            c.pin_cs = PIN_TFT_CS;
            c.pin_rst = PIN_TFT_RST;
            c.pin_busy = -1;
            c.panel_width = 240;
            c.panel_height = 320;
            c.memory_width = 240;
            c.memory_height = 320;
            c.offset_x = 0;
            c.offset_y = 0;
            c.offset_rotation = 0;
            c.readable = true;
            c.invert = false;
            c.rgb_order = false;
            c.dlen_16bit = false;
            c.bus_shared = true;  // <- if colours look wrong flip invert / rgb_order
            _panel.config(c);
        }
        {
            auto c = _light.config();
            c.pin_bl = PIN_TFT_BL;
            c.invert = false;
            c.freq = 20000;
            c.pwm_channel = 7;
            _light.config(c);
            _panel.setLight(&_light);
        }
        {
            auto c = _touch.config();
            c.x_min = 300;
            c.x_max = 3900;
            c.y_min = 300;
            c.y_max = 3800;
            c.pin_int = -1;
            c.bus_shared = true;
            c.offset_rotation = 0;
            c.spi_host = SPI3_HOST;
            c.freq = 1000000;
            c.pin_sclk = PIN_TFT_SCLK;
            c.pin_mosi = PIN_TFT_MOSI;
            c.pin_miso = PIN_TFT_MISO;
            c.pin_cs = PIN_TOUCH_CS;
            _touch.config(c);
            _panel.setTouch(&_touch);
        }
        setPanel(&_panel);
    }
};
static LGFX tft;
// ---- small-sprite drawing: every button/square/strip is drawn into a tiny RAM buffer and pushed once => no flicker ----
#define POOL_W 232
#define POOL_H 52
static LGFX_Sprite gSpr(&tft);
static uint16_t* gPool = nullptr;  // allocated from the heap in gfxBegin() (not static RAM)
static lgfx::LGFXBase* D = &tft;  // current drawing target (tft or the sprite)
static int gOx = 0, gOy = 0;  // screen position of the sprite's top-left
// Start drawing into the small RAM buffer instead of the screen (prevents flicker).
inline bool sprBegin(int x, int y, int w, int h, uint16_t bg) {
    if (!gPool || D != &tft || w <= 0 || h <= 0 || w * h > POOL_W * POOL_H) return false;  // nested or too big: draw directly
    gSpr.setBuffer(gPool, w, h);
    gSpr.fillSprite(bg);
    D = &gSpr;
    gOx = x;
    gOy = y;
    return true;
}

// Send the small buffer to the screen in one go.
inline void sprEnd() {
    gSpr.pushSprite(&tft, gOx, gOy);
    D = &tft;
    gOx = gOy = 0;
}

// Filled rectangle (works on the screen or inside the small buffer).
inline void dRect(int x, int y, int w, int h, uint16_t c) {
    D->fillRect(x - gOx, y - gOy, w, h, c);
}

// Filled rounded rectangle.
inline void dRRect(int x, int y, int w, int h, int r, uint16_t c) {
    D->fillRoundRect(x - gOx, y - gOy, w, h, r, c);
}

// Outline of a rounded rectangle.
inline void dRRectO(int x, int y, int w, int h, int r, uint16_t c) {
    D->drawRoundRect(x - gOx, y - gOy, w, h, r, c);
}

// Filled circle.
inline void dCircle(int x, int y, int r, uint16_t c) {
    D->fillCircle(x - gOx, y - gOy, r, c);
}

// Circle outline.
inline void dRing(int x, int y, int r, uint16_t c) {
    D->drawCircle(x - gOx, y - gOy, r, c);
}

// Filled triangle.
inline void dTri(int a, int b, int c_, int d_, int e, int f, uint16_t col) {
    D->fillTriangle(a - gOx, b - gOy, c_ - gOx, d_ - gOy, e - gOx, f - gOy, col);
}

// Line.
inline void dLine(int a, int b, int c_, int d_, uint16_t col) {
    D->drawLine(a - gOx, b - gOy, c_ - gOx, d_ - gOy, col);
}

// Arc (used for the smile of the logo).
inline void dArc(int x, int y, int r0, int r1, float a0, float a1, uint16_t c) {
    D->drawArc(x - gOx, y - gOy, r0, r1, a0, a1, c);
}

#define RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))
struct Theme {
    uint16_t bg, card, card2, fg, dim, accent, ok, warn, bad, sqLight, sqDark;
};
static Theme T;
// Choose the colour set for dark or light mode.
inline void gfxApplyTheme() {
    if (gSet.dark) T = {RGB(16,18,27), RGB(30,34,48), RGB(42,47,64), RGB(236,239,246), RGB(140,149,168), RGB(100,210,170), RGB(90,220,120), RGB(255,196,70), RGB(255,92,92), RGB(235,236,208), RGB(115,149,82)};
    else T = {RGB(244,246,251), RGB(255,255,255), RGB(226,231,242), RGB(24,28,40), RGB(110,118,136), RGB(24,150,112), RGB(40,170,80), RGB(230,150,20), RGB(220,60,60), RGB(240,217,181), RGB(181,136,99)};
}

// Set the backlight brightness (10..255).
inline void gfxSetBrightness(uint8_t b) {
    gSet.tftBright = b;
    tft.setBrightness(b);
}

// Rotate the screen (0 or 2 = upside down).
inline void gfxSetRotation(uint8_t r) {
    gSet.rotation = (r == 2) ? 2 : 0;
    tft.setRotation(gSet.rotation);
}

// Touch calibration: measured once (4 corner touches), saved in NVS ("tcal"), reused on every boot.
inline void gfxCalibrate() {
    uint8_t rot = gSet.rotation;
    tft.setRotation(0);
    tft.fillScreen(GC_BLACK);
    tft.setFont(&fonts::Font2);
    tft.setTextColor(GC_WHITE);
    tft.setTextDatum(TD_MC);
    tft.drawString("Touch the arrow at each corner", 120, 160);
    tft.drawString("(use a stylus or fingertip tip)", 120, 180);
    uint16_t cal[8] = {0};
    tft.calibrateTouch(cal, (uint16_t)0xFFFF, (uint16_t)0x0000, 30);
    gPrefs.putBytes("tcal", cal, sizeof(cal));
    tft.setTouchCalibrate(cal);
    DBG(TOUCH, "touch calibration saved: %u %u %u %u %u %u %u %u", cal[0], cal[1], cal[2], cal[3], cal[4], cal[5], cal[6], cal[7]);
    tft.setRotation(rot);
    tft.fillScreen(GC_BLACK);
}

// Start the display, load the touch calibration (or ask for it on the first boot).
inline void gfxBegin() {
    DBG(SYS, "display init...");
    tft.init();
    tft.setBrightness(gSet.tftBright);
    tft.setRotation(gSet.rotation);
    tft.fillScreen(GC_BLACK);
    uint16_t cal[8];
    if (gPrefs.isKey("tcal") && gPrefs.getBytes("tcal", cal, sizeof(cal)) == sizeof(cal)) {
        tft.setTouchCalibrate(cal);
        DBG(TOUCH, "touch calibration loaded");
    } else gfxCalibrate();  // first boot only
    gPool = (uint16_t*)malloc(POOL_W * POOL_H * sizeof(uint16_t));
    gSpr.setColorDepth(16);
    gfxApplyTheme();
}

// ---- helpers ----
// Draw text. size 1 = small font, 2 = big font. datum = which point of the text sits at x,y.
inline void gfxText(int x, int y, const char* s, uint8_t size, uint16_t col, TD datum = TD_TL) {
    D->setFont(size >= 2 ? (const lgfx::IFont*)&fonts::Font4 : (const lgfx::IFont*)&fonts::Font2);
    D->setTextSize(1);
    D->setTextDatum(datum);
    D->setTextColor(col);
    D->drawString(s, x - gOx, y - gOy);
}

// Draw a rounded card background.
inline void gfxCard(int x, int y, int w, int h, uint16_t col) {
    dRRect(x, y, w, h, 8, col);
}

// Draw a button with a centred label.
inline void gfxBtn(int x, int y, int w, int h, const char* label, uint16_t bg, uint16_t fg) {
    bool s = sprBegin(x, y, w, h, T.bg);
    dRRect(x, y, w, h, 8, bg);
    gfxText(x + w / 2, y + h / 2, label, 1, fg, TD_MC);
    if (s) sprEnd();
}

// Draw a slider bar with a knob.
inline void gfxSlider(int x, int y, int w, int val, int vmin, int vmax, uint16_t col) {
    dRRect(x, y + 6, w, 6, 3, T.card2);
    int fw = (int)((long)(val - vmin) * w / (vmax - vmin));
    dRRect(x, y + 6, max(fw, 6), 6, 3, col);
    dCircle(x + fw, y + 9, 9, T.fg);
}

// Draw a slider flicker-free (inside a small buffer).
inline void gfxSliderBox(int x, int y, int w, int val, int vmin, int vmax, uint16_t bg) {
    // flicker-free slider
    bool s = sprBegin(x - 12, y - 4, w + 24, 26, bg);
    gfxSlider(x, y, w, val, vmin, vmax, T.accent);
    if (s) sprEnd();
}

// ---- Chess pieces (vector, 30x30 cell). t: 1 pawn 2 knight 3 bishop 4 rook 5 queen 6 king ----
// The outline shapes of one chess piece. g = extra pixels (used to draw the black outline).
inline void pieceShapes(int x, int y, int t, uint16_t c, int g) {
    switch (t) {
        case 1: dCircle(x + 15, y + 9, 4 + g, c);
        dTri(x + 15, y + 11 - g, x + 9 - g, y + 22 + g, x + 21 + g, y + 22 + g, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
        case 2: dTri(x + 10 - g, y + 23 + g, x + 23 + g, y + 23 + g, x + 21 + g, y + 11 - g, c);
        dTri(x + 21 + g, y + 11 - g, x + 12 - g, y + 7 - g, x + 17, y + 20 + g, c);
        dTri(x + 12 - g, y + 7 - g, x + 6 - g, y + 17 + g, x + 17, y + 20 + g, c);
        dTri(x + 13 - g, y + 3 - g, x + 18 + g, y + 6 - g, x + 12 - g, y + 9, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
        case 3: dCircle(x + 15, y + 11, 4 + g, c);
        dCircle(x + 15, y + 5, 2 + g, c);
        dTri(x + 15, y + 12 - g, x + 9 - g, y + 22 + g, x + 21 + g, y + 22 + g, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
        case 4: for (int k = 0; k < 3; k++) dRect(x + 8 + k * 5 - g, y + 5 - g, 4 + 2 * g, 5 + 2 * g, c);
        dRect(x + 9 - g, y + 9 - g, 12 + 2 * g, 13 + 2 * g, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
        case 5: for (int k = 0; k < 5; k++) dCircle(x + 7 + k * 4, y + 6, 2 + g, c);
        dTri(x + 7 - g, y + 7, x + 23 + g, y + 7, x + 19 + g, y + 22 + g, c);
        dTri(x + 7 - g, y + 7, x + 19, y + 22 + g, x + 11 - g, y + 22 + g, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
        case 6: dRect(x + 14 - g, y + 1 - g, 3 + 2 * g, 9 + 2 * g, c);
        dRect(x + 11 - g, y + 4 - g, 9 + 2 * g, 3 + 2 * g, c);
        dTri(x + 15, y + 10 - g, x + 8 - g, y + 22 + g, x + 22 + g, y + 22 + g, c);
        dRRect(x + 9 - g, y + 10 - g, 13 + 2 * g, 4 + 2 * g, 2, c);
        dRRect(x + 7 - g, y + 21 - g, 16 + 2 * g, 5 + 2 * g, 2, c);
        break;
    }
}

// Draw a chess piece in a 30x30 square (positive = white, negative = black).
inline void gfxPiece(int x, int y, int8_t pc) {
    // pc > 0 white, < 0 black. (x,y) = top-left of the 30x30 square
    bool w = pc > 0;
    int t = abs(pc);
    uint16_t fill = w ? RGB(250, 250, 250) : RGB(58, 62, 80), line = w ? RGB(25, 25, 35) : RGB(8, 8, 12);
    pieceShapes(x, y, t, line, 1);
    pieceShapes(x, y, t, fill, 0);
    if (t == 2) dCircle(x + 14, y + 11, 1, line);
    if (t == 3) {
        dLine(x + 13, y + 10, x + 17, y + 8, line);
        dLine(x + 13, y + 11, x + 17, y + 9, line);
    }
}

// ---- Logos: a cute pawn with rosy cheeks on a round badge ----
// Draw the cute pawn logo scaled to radius r.
inline void drawPawnLogo(int cx, int cy, int r, bool withBadge = true) {
    uint16_t badge = RGB(60, 190, 160), white = RGB(255, 255, 255), shade = RGB(205, 225, 235), ink = RGB(30, 40, 60), cheek = RGB(255, 140, 160);
    if (withBadge) {
        dCircle(cx, cy, r, badge);
        dRing(cx, cy, r, shade);
    }
    int hr = r * 26 / 100, hy = cy - r * 36 / 100;
    dTri(cx - r * 30 / 100, cy + r * 42 / 100, cx + r * 30 / 100, cy + r * 42 / 100, cx, hy + hr / 2, white);
    dRRect(cx - r * 17 / 100, hy + hr - 2, r * 34 / 100, r * 12 / 100 + 2, 2, white);
    dRRect(cx - r * 44 / 100, cy + r * 40 / 100, r * 88 / 100, r * 17 / 100, r * 8 / 100, white);
    dCircle(cx, hy, hr, white);
    if (r >= 14) {
        int ey = hy - hr / 8, ex = hr * 40 / 100, er = max(1, hr / 7);
        dCircle(cx - ex, ey, er, ink);
        dCircle(cx + ex, ey, er, ink);
        if (r >= 24) {
            dCircle(cx - ex - er * 2, ey + hr / 3, max(1, hr / 6), cheek);
            dCircle(cx + ex + er * 2, ey + hr / 3, max(1, hr / 6), cheek);
        }
        dArc(cx, ey + hr / 4, hr / 4 + 1, hr / 4, 20, 160, ink);
    }
}

// Small round logo (toolbar, lists).
inline void logoSmall(int cx, int cy, int r = 10) {
    drawPawnLogo(cx, cy, r);
}

// Medium logo (splash, goodbye screen).
inline void logoMedium(int cx, int cy) {
    drawPawnLogo(cx, cy, 32);
}

// Big logo with name and tagline.
inline void logoFull(int cx, int y) {
    drawPawnLogo(cx, y + 44, 44);
    gfxText(cx, y + 106, GC_NAME, 2, T.accent, TD_TC);
    gfxText(cx, y + 136, GC_TAGLINE, 1, T.dim, TD_TC);
}
