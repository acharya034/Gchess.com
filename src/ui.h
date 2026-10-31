// ui.h - the "phone OS": page manager, status toolbar, pull-down quick menu, touch gestures,
// toast, dialog, on-screen keyboard, numpad, list view. Everything runs in the UI task on CORE 0.
// External API (use these from pages):
//   uiGo(PG_xxx)  uiBack()  uiHome()  uiRedraw()          navigation
//   uiToast("text") / uiNotify(...)                         notification in the toolbar (tap = details)
//   uiDialog(title,msg,b1,b2,cb)                           cb(1|2)
//   uiKeyboard(title,initial,isPassword,cb)                cb(text, ok)
//   uiNumpad(title,digits,cb)                              cb(value, ok)  value = 16-bit passcode
//   Btn + uiDrawBtn()/uiHitBtns()                          simple buttons (also logs analytics)
//   ListView + uiListDraw()/uiListTouch()                  scrolling lists
//   uiQuickMenuOpen()  uiSleepScreen()  uiTap()
#pragma once
#include "gfx.h"
#include "debug.h"
#include "sound.h"
#include "analytics.h"
#include "power.h"
#include "events.h"
#include "leds.h"

#define TB_H 20
enum PageId : uint8_t {
    PG_INTRO,
    PG_LOCK,
    PG_HOME,
    PG_PLAY,
    PG_AI,
    PG_FRIEND_SETUP,
    PG_WAIT,
    PG_GAME,
    PG_PUZZLE,
    PG_HISTORY,
    PG_FRIENDS,
    PG_SETTINGS,
    PG_WIFI,
    PG_LINK,
    PG_BOARDTEST,
    PG_ABOUT,
    PG_ONGOING,
    PG_COUNT
};
enum TouchPhase : uint8_t {
    TP_DOWN,
    TP_MOVE,
    TP_UP
};
enum {
    PF_TOOLBAR = 1,
    PF_NOBACK = 2,
    PF_NOPUSH = 4
};  // NOBACK = hide back arrow, NOPUSH = never return to this page with Back
struct Page {
    const char* name;
    void (*enter)();
    void (*draw)();
    void (*touch)(TouchPhase, int, int);
    void (*tick)();
    void (*leave)();
    void (*event)(const UiEvt&);
    uint8_t flags;
    void (*partial)();
};
struct Btn {
    int16_t x, y, w, h;
    const char* label;
    uint8_t id;
    uint16_t bg, fg;
};

static Page gPages[PG_COUNT];
static PageId gCur = PG_INTRO, gStack[10];
static uint8_t gSp = 0;
static int8_t gNetState = 0, gRssi = 0;
static bool (*gGlobalEvt)(const UiEvt&) = nullptr;
static bool (*gLeaveGuard)(bool toHome) = nullptr;  // asked before leaving the game page (return true = I handle it)
static bool gScreenOff = false;
static bool gSwallow = false;  // ignore the rest of a touch that just closed a menu or woke the screen
static uint32_t gLastTouch = 0;

enum ModalType : uint8_t {
    MD_NONE,
    MD_QUICK,
    MD_DIALOG,
    MD_KEYBOARD,
    MD_NUMPAD,
    MD_NOTIF
};
static ModalType gModal = MD_NONE;

// Click sound.
inline void uiTap() {
    soundPlay(SND_TAP);
}

// Notifications live in the toolbar: kind 0 = info, 1 = problem, 2 = needs an answer (e.g. challenge).
struct Notif {
    bool active;
    uint8_t kind;
    char text[48];
    char detail[160];
    char b1[14], b2[14];
    uint32_t until;
    void (*cb)(int);
};
static Notif gNotif, gPend;  // gPend = notification that still waits for an answer (shows a red dot in the toolbar)
static Notif npn;
static int npH = 0;  // copy shown in the detail panel
// Tiny hash used to detect changes of a text.
inline uint32_t strHash(const char* s) {
    uint32_t h = 5381;
    while (*s) h = h * 33 + (uint8_t)*s++;
    return h;
}

// Shorten a text with ".." so it fits into maxw pixels.
inline void fitText(char* out, const char* in, int maxw) {
    strlcpy(out, in, 48);
    while (strlen(out) > 3 && tft.textWidth(out, &fonts::Font2) > maxw) out[strlen(out) - 1] = 0;
    if (strlen(out) < strlen(in)) {
        size_t l = strlen(out);
        out[l - 1] = '.';
        out[l - 2] = '.';
    }
}

inline void uiRedraw();
inline void uiGo(PageId id, bool push = true);

// ---------------- toolbar ----------------
static uint32_t tbLast = 0xFFFFFFFF;
// Top bar: back / logo, status lens, Wi-Fi, battery - or a notification. Repaints only when needed.
inline void drawToolbar(bool force = true) {
    const Page& p = gPages[gCur];
    bool back = gSp > 0 && !(p.flags & PF_NOBACK);
    int pct = batteryPercent();
    bool chg = isCharging();
    bool showN = gNotif.active && (gModal == MD_NONE || gModal == MD_QUICK);
    uint32_t sg = (gNetState + 1) | ((uint32_t)(gRssi / 5 + 30) << 4) | ((uint32_t)pct << 12) | ((uint32_t)chg << 20) | ((uint32_t)back << 21) | ((uint32_t)gCur << 22) | ((uint32_t)gSet.dark << 28) | ((uint32_t)gPend.active << 29);
    if (showN) sg ^= 0x40000000u ^ strHash(gNotif.text);
    if (!force && sg == tbLast) return;
    tbLast = sg;  // only repaint when something changed => no flicker
    bool s = sprBegin(0, 0, 240, TB_H, T.card);
    dRect(0, 0, 240, TB_H, T.card);
    if (showN) {
        // erase the toolbar, then show the notification
        uint16_t bg = gNotif.kind == 1 ? T.bad : gNotif.kind == 2 ? T.accent : T.card2, fg = gNotif.kind == 0 ? T.fg : (gNotif.kind == 1 ? GC_WHITE : RGB(10, 20, 20));
        dRect(0, 0, 240, TB_H, bg);
        dCircle(12, 10, 7, fg);
        gfxText(12, 10, gNotif.kind == 0 ? "i" : "!", 1, bg, TD_MC);
        char t[48];
        fitText(t, gNotif.text, 204);
        gfxText(26, 2, t, 1, fg);
        if (s) sprEnd();
        return;
    }
    if (back) {
        dTri(12, 4, 12, 16, 4, 10, T.fg);
        gfxText(18, 2, p.name, 1, T.fg);
    } else {
        logoSmall(11, 10, 8);
        gfxText(24, 2, GC_NAME, 1, T.accent);
    }
    uint16_t dot = gNetState == 3 ? T.ok : gNetState == 2 ? RGB(80, 160, 255) : gNetState == 1 ? T.warn : T.bad;  // camera-like status lens
    dCircle(120, 10, 8, RGB(10, 10, 14));
    dRing(120, 10, 8, T.card2);
    dCircle(120, 10, 4, dot);
    dCircle(118, 8, 1, GC_WHITE);
    if (gPend.active) dCircle(146, 10, 4, T.bad);  // waiting answer (tap to open)
    int bars = gNetState < 2 ? 0 : (gRssi > -55 ? 4 : gRssi > -65 ? 3 : gRssi > -75 ? 2 : 1);
    for (int i = 0; i < 4; i++) dRect(160 + i * 5, 15 - i * 3 - 1, 3, i * 3 + 3, i < bars ? T.fg : T.card2);
    uint16_t bc = chg ? T.ok : (pct < 20 ? T.bad : T.fg);
    D->drawRect(196 - gOx, 5 - gOy, 26, 11, bc);
    dRect(222, 8, 2, 5, bc);
    dRect(198, 7, (22 * pct) / 100, 7, bc);
    if (s) sprEnd();
}

// Draw toolbar and page again (no clearing).
inline void uiRedraw() {
    if (gPages[gCur].flags & PF_TOOLBAR) drawToolbar(true);
    if (gPages[gCur].draw) gPages[gCur].draw();
}

// Closing any menu/dialog: wipe the background first, then draw the page again (no leftovers of the menu)
inline void uiRepaint() {
    tft.fillScreen(T.bg);
    uiRedraw();
}

// Open a page. push = remember the current page for Back.
inline void uiGo(PageId id, bool push) {
    DBG(UI, "PAGE %s -> %s (push=%d, stack=%d)", gPages[gCur].name, gPages[id].name, (int)push, (int)gSp);
    if (gPages[gCur].leave) gPages[gCur].leave();
    if (push && gSp < 10 && !(gPages[gCur].flags & PF_NOPUSH)) gStack[gSp++] = gCur;
    gCur = id;
    analyticsPage(id);
    tft.fillScreen(T.bg);
    if (gPages[id].enter) gPages[id].enter();
    uiRedraw();
}

// Go back one page (asks first when leaving a running game).
inline void uiBack() {
    DBG(UI, "BACK pressed on %s", gPages[gCur].name);
    if (gLeaveGuard && gLeaveGuard(false)) return;
    if (gSp > 0) uiGo(gStack[--gSp], false);
}

// Go to the home page.
inline void uiHome() {
    DBG(UI, "HOME requested from %s", gPages[gCur].name);
    if (gLeaveGuard && gLeaveGuard(true)) return;
    gSp = 0;
    uiGo(PG_HOME, false);
}

// ---------------- notifications (replace the old pop-up pill) ----------------
//   uiNotify(text, detail, kind, cb, b1, b2, sound)   shows in the toolbar for ~4 s, plays a sound, tap = detail panel
//   uiToast(text)                                       shortcut (kind chosen from the wording)
inline void uiNotify(const char* text, const char* detail = nullptr, uint8_t kind = 0, void (*cb)(int) = nullptr, const char* b1 = "Close", const char* b2 = "", Snd snd = SND_NOTIFY) {
    Notif& n = gNotif;
    n.active = true;
    n.kind = kind;
    strlcpy(n.text, text, 48);
    strlcpy(n.detail, (detail && detail[0]) ? detail : text, 160);
    n.cb = cb;
    strlcpy(n.b1, b1, 14);
    strlcpy(n.b2, b2, 14);
    n.until = millis() + (kind == 2 ? 6000 : 3800);
    if (kind == 2) gPend = n;
    DBG(UI, "NOTIFY kind=%d text='%s' detail='%s'", (int)kind, n.text, n.detail);
    if (snd != SND_NONE) soundPlay(snd);
    if ((gPages[gCur].flags & PF_TOOLBAR) && (gModal == MD_NONE || gModal == MD_QUICK)) drawToolbar(true);
}

// Guess from the wording if a message is an error.
inline bool looksLikeProblem(const char* m) {
    static const char* k[] = {"fail", "Fail", "not ", "Not ", "reject", "Could", "expired", "lost", "Wrong", "denied", "error", "Error", "rejected", "no "};
    for (auto s : k) if (strstr(m, s)) return true;
    return false;
}

// Short message = notification.
inline void uiToast(const char* m, uint16_t ms = 0) {
    bool pr = looksLikeProblem(m);
    uiNotify(m, m, pr ? 1 : 0, nullptr, "Close", "", pr ? SND_WRONG : SND_NOTIFY);
}

// Remove the notification after its time.
inline void notifTick() {
    if (gNotif.active && gModal != MD_NOTIF && millis() > gNotif.until) {
        gNotif.active = false;
        if ((gPages[gCur].flags & PF_TOOLBAR) && (gModal == MD_NONE || gModal == MD_QUICK)) drawToolbar(true);
    }
}

// Split a text into lines for the detail panel.
inline int wrapLines(const char* s, int maxc, char out[][36], int maxl) {
    int n = 0;
    const char* p = s;
    while (*p && n < maxl) {
        while (*p == ' ') p++;
        int len = 0, last = -1;
        while (p[len] && p[len] != '\n' && len < maxc) {
            if (p[len] == ' ') last = len;
            len++;
        }
        if (p[len] && p[len] != '\n' && last > 0) len = last;
        memcpy(out[n], p, len);
        out[n][len] = 0;
        n++;
        p += len;
        if (*p == '\n') p++;
    }
    return n;
}

// Detail panel of a notification.
inline void notifPanelDraw() {
    char L[7][36];
    int n = wrapLines(npn.detail, 30, L, 7);
    npH = 62 + n * 15 + 44;
    tft.fillRoundRect(0, TB_H, 240, npH, 12, T.card);
    gfxText(14, TB_H + 12, npn.kind == 1 ? "Problem" : npn.kind == 2 ? "Needs your answer" : "Notice", 2, npn.kind == 1 ? T.bad : T.accent);
    for (int i = 0; i < n; i++) gfxText(14, TB_H + 50 + i * 15, L[i], 1, T.fg);
    int by = TB_H + npH - 38;
    if (npn.b2[0]) {
        gfxBtn(16, by, 100, 30, npn.b1, T.card2, T.fg);
        gfxBtn(124, by, 100, 30, npn.b2, T.accent, RGB(10, 20, 20));
    } else gfxBtn(70, by, 100, 30, npn.b1, T.card2, T.fg);
}

// Open the detail panel.
inline void notifOpen(const Notif& n) {
    npn = n;
    gNotif.active = false;
    gModal = MD_NOTIF;
    notifPanelDraw();
    drawToolbar(true);
}

// Close the panel; r = button pressed (0 = just closed).
inline void notifClose(int r) {
    DBG(UI, "NOTIFICATION panel closed, answer=%d", r);
    if (r && npn.kind == 2) gPend.active = false;
    gModal = MD_NONE;
    uiRepaint();
    if (r && npn.cb) npn.cb(r);
}

// Touch inside the detail panel.
inline void notifTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    int by = TB_H + npH - 38;
    if (y >= TB_H && y < TB_H + npH) {
        if (y >= by && y < by + 30) {
            int r = 0;
            if (npn.b2[0]) {
                if (x >= 16 && x < 116) r = 1;
                else if (x >= 124 && x < 224) r = 2;
            } else if (x >= 70 && x < 170) r = 1;
            if (r) {
                uiTap();
                notifClose(r);
            }
        } return;
    }
    notifClose(0);  // tap outside = just close
}

// ---------------- buttons / lists ----------------
// Draw a Btn.
inline void uiDrawBtn(const Btn& b) {
    gfxBtn(b.x, b.y, b.w, b.h, b.label, b.bg, b.fg);
}

// Which Btn was touched? Returns its id or -1.
inline int uiHitBtns(const Btn* b, int n, int x, int y) {
    for (int i = 0; i < n; i++) if (x >= b[i].x && x < b[i].x + b[i].w && y >= b[i].y && y < b[i].y + b[i].h) {
        DBG(UI, "BUTTON '%s' on %s", b[i].label, gPages[gCur].name);
        analyticsButton(gCur, b[i].label);
        uiTap();
        return b[i].id;
    }
    return -1;
}

struct ListView {
    int16_t x, y, w, h, rowH, n, scroll;
};
typedef void (*RowDraw)(int idx, int x, int y, int w, int h);
// Draw the visible rows of a scrolling list.
inline void uiListDraw(ListView& lv, RowDraw rd, bool clear = true) {
    if (clear) tft.fillRect(lv.x, lv.y, lv.w, lv.h, T.bg);
    int first = lv.scroll / lv.rowH;
    tft.setClipRect(lv.x, lv.y, lv.w, lv.h);
    for (int i = first; i < lv.n && i <= first + lv.h / lv.rowH + 1; i++) {
        int y = lv.y + i * lv.rowH - lv.scroll;
        bool s = sprBegin(lv.x, y, lv.w, lv.rowH, T.bg);
        rd(i, lv.x, y, lv.w, lv.rowH - 4);
        if (s) sprEnd();
    }
    int endY = max((int)lv.y, lv.y + lv.n * lv.rowH - lv.scroll);
    if (!clear && endY < lv.y + lv.h) tft.fillRect(lv.x, endY, lv.w, lv.y + lv.h - endY, T.bg);
    tft.clearClipRect();
}

// returns tapped row or -1 (-2 while scrolling)
inline int uiListTouch(ListView& lv, TouchPhase ph, int x, int y, RowDraw rd) {
    static int sy = 0, ss = 0;
    static bool scrolling = false, inside = false;
    if (ph == TP_DOWN) {
        inside = x >= lv.x && x < lv.x + lv.w && y >= lv.y && y < lv.y + lv.h;
        sy = y;
        ss = lv.scroll;
        scrolling = false;
        return -1;
    }
    if (!inside) return -1;
    if (ph == TP_MOVE) {
        if (abs(y - sy) > 8) scrolling = true;
        if (scrolling) {
            int maxS = max(0, lv.n * lv.rowH - lv.h);
            lv.scroll = constrain(ss + (sy - y), 0, maxS);
            uiListDraw(lv, rd, false);
            return -2;
        }
        return -1;
    }
    if (ph == TP_UP && !scrolling) {
        int i = (y - lv.y + lv.scroll) / lv.rowH;
        return (i >= 0 && i < lv.n) ? i : -1;
    }
    return -1;
}

// ---------------- dialog ----------------
static struct {
    char title[24], msg[96], b1[16], b2[16], b3[16];
    void (*cb)(int);
    bool outside, toQuick;
} dlg;
inline void qmDraw();  // defined below; used when a dialog that was opened from the quick menu is cancelled
// outside = tap outside the box cancels it.  toQuick = after cancelling, show the quick menu again (it was under the box).
inline void uiDialogOutsideCancels(bool b, bool toQuick = false) {
    dlg.outside = b;
    dlg.toQuick = toQuick;
}

// Draw the dialog box.
inline void dialogDraw() {
    bool three = dlg.b3[0];
    int h = three ? 196 : 140, y0 = three ? 60 : 86;
    tft.fillRoundRect(16, y0, 208, h, 12, T.card);
    tft.drawRoundRect(16, y0, 208, h, 12, T.card2);
    gfxText(120, y0 + 12, dlg.title, 2, T.fg, TD_TC);
    char m[96];
    strlcpy(m, dlg.msg, 96);
    int y = y0 + 46;
    for (char* l = strtok(m, "\n"); l && y < y0 + 90; l = strtok(nullptr, "\n"), y += 18) gfxText(120, y, l, 1, T.dim, TD_TC);
    if (three) {
        gfxBtn(32, y0 + 96, 176, 30, dlg.b1, T.accent, RGB(10, 20, 20));
        gfxBtn(32, y0 + 130, 176, 30, dlg.b2, T.bad, GC_WHITE);
        gfxBtn(32, y0 + 164, 176, 26, dlg.b3, T.card2, T.fg);
    } else if (dlg.b2[0]) {
        gfxBtn(28, 184, 86, 32, dlg.b1, T.card2, T.fg);
        gfxBtn(126, 184, 86, 32, dlg.b2, T.accent, RGB(10, 20, 20));
    } else gfxBtn(70, 184, 100, 32, dlg.b1, T.accent, RGB(10, 20, 20));
}

// Dialog with three buttons.
inline void uiDialog3(const char* title, const char* msg, const char* b1, const char* b2, const char* b3, void (*cb)(int)) {
    strlcpy(dlg.title, title, 24);
    strlcpy(dlg.msg, msg, 96);
    strlcpy(dlg.b1, b1, 16);
    strlcpy(dlg.b2, b2, 16);
    strlcpy(dlg.b3, b3, 16);
    dlg.cb = cb;
    dlg.outside = false;
    dlg.toQuick = false;
    DBG(UI, "DIALOG open '%s'", dlg.title);
    gModal = MD_DIALOG;
    dialogDraw();
}

// Dialog with one or two buttons.
inline void uiDialog(const char* title, const char* msg, const char* b1, const char* b2, void (*cb)(int)) {
    strlcpy(dlg.title, title, 24);
    strlcpy(dlg.msg, msg, 96);
    strlcpy(dlg.b1, b1, 16);
    strlcpy(dlg.b2, b2 ? b2 : "", 16);
    dlg.b3[0] = 0;
    dlg.cb = cb;
    dlg.outside = false;
    dlg.toQuick = false;
    DBG(UI, "DIALOG open '%s'", dlg.title);
    gModal = MD_DIALOG;
    dialogDraw();
}

// Touch on a dialog.
inline void dialogTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    int r = 0;
    if (dlg.b3[0]) {
        int y0 = 60;
        if (x >= 32 && x < 208) {
            if (y >= y0 + 96 && y < y0 + 126) r = 1;
            else if (y >= y0 + 130 && y < y0 + 160) r = 2;
            else if (y >= y0 + 164 && y < y0 + 190) r = 3;
        }
    } else if (y >= 184 && y <= 216) {
        if (dlg.b2[0]) {
            if (x >= 28 && x < 114) r = 1;
            else if (x >= 126 && x < 212) r = 2;
        } else if (x >= 70 && x < 170) r = 1;
    }
    if (!r) {
        int cy = dlg.b3[0] ? 60 : 86, ch = dlg.b3[0] ? 196 : 140;
        if (dlg.outside && (x < 16 || x > 224 || y < cy || y > cy + ch)) {
            // tap outside the box = cancel
            DBG(UI, "DIALOG '%s' cancelled by tap outside", dlg.title);
            if (dlg.toQuick) {
                gModal = MD_QUICK;
                qmDraw();
            }
            // keep the quick menu open
            else {
                gModal = MD_NONE;
                uiRepaint();
            }
        } return;
    }
    uiTap();
    DBG(UI, "DIALOG '%s' answer %d", dlg.title, r);
    gModal = MD_NONE;
    uiRepaint();
    if (dlg.cb) dlg.cb(r);
}

// ---------------- keyboard ----------------
static struct {
    char title[24];
    char buf[64];
    uint8_t len;
    bool pw, show;
    uint8_t layer;
    void (*cb)(const char*, bool);
} kb;
static const char* KB_L[3] = {"qwertyuiop", "asdfghjkl", "zxcvbnm"};
static const char* KB_U[3] = {"QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
static const char* KB_S[3] = {"1234567890", "-/:;()&@\"'", ".,?!#%+"};
#define KB_Y0 184
// Keyboard: text field.
inline void kbField() {
    tft.fillRoundRect(8, 62, 224, 34, 8, T.card);
    char tmp[66];
    int n = kb.len;
    if (kb.pw && !kb.show) {
        for (int i = 0; i < n; i++) tmp[i] = '*';
        tmp[n] = 0;
    } else strcpy(tmp, kb.buf);
    tft.setClipRect(12, 62, 216, 34);
    gfxText(14, 71, n > 20 ? tmp + n - 20 : tmp, 1, T.fg);
    tft.clearClipRect();
    if (kb.pw) gfxBtn(190, 100, 42, 24, kb.show ? "Hide" : "Show", T.card2, T.dim);
}

// Keyboard: draw one key.
inline void kbKey(int x, int y, int w, const char* l, uint16_t bg = 0) {
    tft.fillRoundRect(x + 1, y + 1, w - 2, 30, 5, bg ? bg : T.card2);
    gfxText(x + w / 2, y + 16, l, 1, T.fg, TD_MC);
}

// Keyboard: draw everything.
inline void kbDraw() {
    tft.fillScreen(T.bg);
    gfxText(120, 30, kb.title, 1, T.dim, TD_TC);
    kbField();
    const char** L = kb.layer == 0 ? KB_L : kb.layer == 1 ? KB_U : KB_S;
    char s[2] = {0, 0};
    for (int r = 0; r < 3; r++) {
        int n = strlen(L[r]), x0 = (r == 0) ? 0 : (r == 1) ? 12 : 36;
        for (int i = 0; i < n; i++) {
            s[0] = L[r][i];
            kbKey(x0 + i * 24, KB_Y0 + r * 34, 24, s);
        }
    }
    kbKey(0, KB_Y0 + 68, 36, kb.layer == 2 ? "abc" : "^", kb.layer == 1 ? T.accent : 0);
    kbKey(204, KB_Y0 + 68, 36, "<");
    kbKey(0, KB_Y0 + 102, 48, kb.layer == 2 ? "abc" : "123");
    kbKey(48, KB_Y0 + 102, 108, "space");
    kbKey(156, KB_Y0 + 102, 42, "esc", T.card2);
    kbKey(198, KB_Y0 + 102, 42, "OK", T.accent);
}

// Open the on-screen keyboard.
inline void uiKeyboard(const char* title, const char* initial, bool pw, void (*cb)(const char*, bool)) {
    strlcpy(kb.title, title, 24);
    strlcpy(kb.buf, initial ? initial : "", 64);
    kb.len = strlen(kb.buf);
    kb.pw = pw;
    kb.show = false;
    kb.layer = 0;
    kb.cb = cb;
    gModal = MD_KEYBOARD;
    kbDraw();
}

// Close the keyboard; ok = confirmed.
inline void kbClose(bool ok) {
    gModal = MD_NONE;
    tft.fillScreen(T.bg);
    uiRedraw();
    if (kb.cb) kb.cb(kb.buf, ok);
}

// Touch on the keyboard.
inline void kbTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    if (kb.pw && x >= 190 && y >= 100 && y < 124) {
        kb.show = !kb.show;
        kbField();
        return;
    }
    if (y < KB_Y0) return;
    int row = (y - KB_Y0) / 34;
    uiTap();
    if (row < 3) {
        const char** L = kb.layer == 0 ? KB_L : kb.layer == 1 ? KB_U : KB_S;
        int n = strlen(L[row]), x0 = (row == 0) ? 0 : (row == 1) ? 12 : 36;
        if (row == 2 && x < 36) {
            kb.layer = (kb.layer == 0) ? 1 : (kb.layer == 1 ? 0 : 0);
            kbDraw();
            return;
        }
        if (row == 2 && x >= 204) {
            if (kb.len) kb.buf[--kb.len] = 0;
            kbField();
            return;
        }
        int i = (x - x0) / 24;
        if (i >= 0 && i < n && kb.len < 62) {
            kb.buf[kb.len++] = L[row][i];
            kb.buf[kb.len] = 0;
            if (kb.layer == 1) {
                kb.layer = 0;
                kbDraw();
            } else kbField();
        }
    } else {
        if (x < 48) {
            kb.layer = kb.layer == 2 ? 0 : 2;
            kbDraw();
        } else if (x < 156) {
            if (kb.len < 62) {
                kb.buf[kb.len++] = ' ';
                kb.buf[kb.len] = 0;
                kbField();
            }
        } else if (x < 198) kbClose(false);
        else kbClose(true);
    }
}

// ---------------- numpad (4 digit passcode stored as uint16_t) ----------------
static struct {
    char title[28];
    uint8_t digits, len;
    uint16_t val;
    void (*cb)(uint16_t, bool);
} np;
// Numpad: the dots showing typed digits.
inline void npDots() {
    tft.fillRect(0, 78, 240, 24, T.bg);
    for (int i = 0; i < np.digits; i++) {
        int x = 120 + (i - np.digits / 2.0f + 0.5f) * 30;
        if (i < np.len) tft.fillCircle(x, 90, 8, T.accent);
        else tft.drawCircle(x, 90, 8, T.dim);
    }
}

// Numpad: draw everything.
inline void npDraw() {
    tft.fillScreen(T.bg);
    logoSmall(120, 34, 14);
    gfxText(120, 54, np.title, 1, T.dim, TD_TC);
    npDots();
    const char* k[12] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "C", "0", "<"};
    for (int i = 0; i < 12; i++) {
        int x = 8 + (i % 3) * 76, y = 114 + (i / 3) * 50;
        tft.fillRoundRect(x, y, 72, 44, 10, T.card);
        gfxText(x + 36, y + 22, k[i], 2, i == 9 || i == 11 ? T.dim : T.fg, TD_MC);
    }
}

// Open the numeric keypad (passcode).
inline void uiNumpad(const char* title, uint8_t digits, void (*cb)(uint16_t, bool)) {
    strlcpy(np.title, title, 28);
    np.digits = digits;
    np.len = 0;
    np.val = 0;
    np.cb = cb;
    gModal = MD_NUMPAD;
    npDraw();
}

// Touch on the numpad.
inline void npTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP || y < 114) return;
    int c = (x - 8) / 76, r = (y - 114) / 50;
    if (c < 0 || c > 2 || r > 3) return;
    int i = r * 3 + c;
    uiTap();
    if (i == 9) {
        np.len = 0;
        np.val = 0;
    } else if (i == 11) {
        if (np.len) {
            np.len--;
            np.val /= 10;
        }
    } else {
        int d = (i == 10) ? 0 : i + 1;
        if (np.len < np.digits) {
            np.val = np.val * 10 + d;
            np.len++;
        }
    }
    npDots();
    if (np.len == np.digits) {
        uint16_t v = np.val;
        delay(120);
        gModal = MD_NONE;
        tft.fillScreen(T.bg);
        uiRedraw();
        if (np.cb) np.cb(v, true);
    }
}

// ---------------- quick menu (pull down like a phone) ----------------
#define QM_H 214
static int qmDrag = -1;
// Screen off / on.  Going to sleep first closes every menu; waking up redraws the CURRENT page
// from scratch, so you never see an old quick menu or half drawn buttons.
inline void uiSleepScreen(bool off) {
    if (off) {
        DBG(UI, "screen SLEEP (closing menus)");
        gModal = MD_NONE;  // forget any open quick menu / dialog / keyboard
        gNotif.active = false;  // and the notification in the toolbar
        gScreenOff = true;
        tft.setBrightness(0);  // backlight off
        tft.sleep();
    } else {
        DBG(UI, "screen WAKE -> redraw %s", gPages[gCur].name);
        gScreenOff = false;
        tft.wakeup();
        tft.setBrightness(gSet.tftBright);
        tft.fillScreen(T.bg);  // clean background first
        uiRedraw();  // then the page of the moment
    }
}

// Quick menu: one slider row.
inline void qmSliderRow(int y, const char* label, int val, int vmin, int vmax) {
    gfxText(10, y + 2, label, 1, T.dim);
    gfxSlider(76, y, 150, val, vmin, vmax, T.accent);
}

// Is Wi-Fi switched on in the settings?
inline bool gWifiOnFlag() {
    return gSet.wifiOn;
}

// Switch Wi-Fi on / off.
inline void gWifiToggle() {
    gSet.wifiOn = !gSet.wifiOn;
    netCmd(NC_WIFI_ENABLE, "", "", gSet.wifiOn);
}

// Quick menu: draw the panel.
inline void qmDraw() {
    tft.fillRoundRect(0, TB_H, 240, QM_H, 12, T.card);
    qmSliderRow(TB_H + 12, "Screen", gSet.tftBright, 10, 255);
    qmSliderRow(TB_H + 44, "LEDs", gSet.ledBright, 5, 255);
    qmSliderRow(TB_H + 76, "Sound", gSet.volume, 0, 3);
    struct Tile {
        const char* l;
        bool on;
    } t[8] = {{"Wi-Fi", gWifiOnFlag()}, {"Dark", gSet.dark}, {"Scr 180", gSet.rotation == 2}, {"Sleep", false}, {"Home", false}, {"Back", false}, {"Brd 180", gSet.flipHw}, {"Power", false}};
    for (int i = 0; i < 8; i++) {
        int x = 6 + (i % 4) * 58, y = TB_H + 112 + (i / 4) * 48;
        tft.fillRoundRect(x, y, 54, 42, 8, t[i].on ? T.accent : T.card2);
        gfxText(x + 27, y + 21, t[i].l, 1, t[i].on ? RGB(10, 20, 20) : T.fg, TD_MC);
    }
    tft.fillRoundRect(100, TB_H + QM_H - 8, 40, 4, 2, T.dim);
}

// Open the quick menu.
inline void uiQuickMenuOpen() {
    DBG(UI, "QUICK MENU open");
    gModal = MD_QUICK;
    qmDrag = -1;
    qmDraw();
}

// Close the quick menu (background is cleared first).
inline void qmClose() {
    DBG(UI, "QUICK MENU close");
    gModal = MD_NONE;
    uiRepaint();
    storageSaveSettings();
}

static void powerDlgCb(int r);
// Touch on the quick menu: sliders, tiles, swipe up or tap outside to close.
inline void qmTouch(TouchPhase ph, int x, int y) {
    static int y0 = 0;
    if (ph == TP_DOWN) {
        y0 = y;
        qmDrag = -1;
        if (y > TB_H + QM_H) {
            qmClose();
            return;
        }
    }
    int rows[3] = {TB_H + 12, TB_H + 44, TB_H + 76};
    if (ph == TP_DOWN || ph == TP_MOVE) {
        if (ph == TP_DOWN) for (int i = 0; i < 3; i++) if (y >= rows[i] - 6 && y < rows[i] + 24) qmDrag = i;
        if (qmDrag >= 0 && qmDrag < 3) {
            float f = constrain((x - 76) / 150.0f, 0.0f, 1.0f);
            if (qmDrag == 0) gfxSetBrightness(10 + f * 245);
            else if (qmDrag == 1) ledSetBrightness(5 + f * 250);
            else {
                soundSetVolume((int)(f * 3 + 0.5f));
            }
            gfxSliderBox(76, rows[qmDrag], 150, qmDrag == 0 ? gSet.tftBright : qmDrag == 1 ? gSet.ledBright : gSet.volume, qmDrag == 0 ? 10 : qmDrag == 1 ? 5 : 0, qmDrag == 2 ? 3 : 255, T.card);
            if (ph == TP_MOVE) return;
        }
    }
    if (ph == TP_UP) {
        if (qmDrag >= 0) {
            if (qmDrag == 2) soundPlay(SND_TAP);
            qmDrag = -1;
            return;
        }
        if (y0 - y > 30) {
            qmClose();
            return;
        }
        // swipe up closes
        if (y >= TB_H + 112 && y < TB_H + 112 + 96) {
            int c = (x - 6) / 58, r = (y - TB_H - 112) / 48, i = r * 4 + c;
            if (c < 0 || c > 3 || i > 7) return;
            uiTap();
            analyticsButton(gCur, "quick_tile");
            switch (i) {
                case 0: gWifiToggle();
                qmDraw();
                break;
                case 1: gSet.dark = !gSet.dark;
                gfxApplyTheme();
                tft.fillScreen(T.bg);
                uiRedraw();
                qmDraw();
                break;
                case 2: gfxSetRotation(gSet.rotation == 0 ? 2 : 0);
                tft.fillScreen(T.bg);
                uiRedraw();
                qmDraw();
                break;
                case 3: gModal = MD_NONE;
                uiSleepScreen(true);
                break;
                case 4: gModal = MD_NONE;
                uiHome();
                break;
                case 5: gModal = MD_NONE;
                uiBack();
                break;
                case 6: gSet.flipHw = !gSet.flipHw;
                qmDraw();
                break;
                case 7: uiDialog("Power", "", "Restart", "Power off", powerDlgCb);
                uiDialogOutsideCancels(true, true);
                break;  // tap outside closes only this box
            }
        }
    }
}

// Answer of the power box: 1 restart, 2 power off.
static void powerDlgCb(int r) {
    if (r == 1) powerRestart();
    else if (r == 2) powerOff();
}

// ---------------- touch pump (called every ~15 ms) ----------------
static bool tDown = false, pullActive = false;
static int tx0, ty0, tx, ty;
// Decide who gets a touch: menus first, then toolbar gestures, then the page.
inline void routeTouch(TouchPhase ph, int x, int y) {
    gLastTouch = millis();
    // Touch log: DOWN / UP always, MOVE at most every 150 ms.
    static uint32_t lastMoveLog = 0, downAt = 0;
    if (ph == TP_DOWN) {
        downAt = millis();
        DBG(TOUCH, "DOWN x=%d y=%d  page=%s modal=%d%s", x, y, gPages[gCur].name, (int)gModal, gScreenOff ? " (screen off)" : "");
    } else if (ph == TP_UP) DBG(TOUCH, "UP   x=%d y=%d  held=%lums", x, y, (unsigned long)(millis() - downAt));
    else if (millis() - lastMoveLog > 150) {
        lastMoveLog = millis();
        DBG(TOUCH, "MOVE x=%d y=%d", x, y);
    }
    if (gScreenOff) {
        // the first touch only wakes the screen
        if (ph == TP_DOWN) {
            uiSleepScreen(false);
            gSwallow = true;
        } return;
    }
    if (gSwallow) {
        if (ph == TP_UP) {
            gSwallow = false;
            DBG(TOUCH, "(touch ignored: it closed/woke something)");
        } return;
    }
    // the touch that just closed a menu must NOT press what is under it
    ModalType before = gModal;
    switch (gModal) {
        case MD_QUICK: qmTouch(ph, x, y);
        break;
        case MD_DIALOG: dialogTouch(ph, x, y);
        break;
        case MD_KEYBOARD: kbTouch(ph, x, y);
        break;
        case MD_NUMPAD: npTouch(ph, x, y);
        break;
        case MD_NOTIF: notifTouch(ph, x, y);
        break;
        default: break;
    }
    if (before != MD_NONE) {
        if (gModal == MD_NONE && ph != TP_UP) gSwallow = true;
        return;
    }
    const Page& p = gPages[gCur];
    if (ph == TP_DOWN) pullActive = (p.flags & PF_TOOLBAR) && y < TB_H + 4;
    if (pullActive) {
        if (ph == TP_MOVE && y - ty0 > 26) {
            pullActive = false;
            uiQuickMenuOpen();
            return;
        }
        if (ph == TP_UP) {
            pullActive = false;
            if (abs(y - ty0) < 12) {
                if (gNotif.active) {
                    uiTap();
                    notifOpen(gNotif);
                }
                // tap a notification = full details
                else if (x < 60 && gSp > 0 && !(p.flags & PF_NOBACK)) {
                    uiTap();
                    uiBack();
                } else if (gPend.active && x > 132 && x < 160) {
                    uiTap();
                    notifOpen(gPend);
                }
                // red dot = pending challenge
                else if (x > 96 && x < 144) {
                    uiTap();
                    uiQuickMenuOpen();
                }
            }
        } return;
    }
    if (p.touch) p.touch(ph, x, y);
}

// Read the touch screen and call routeTouch with DOWN / MOVE / UP.
inline void uiTouchPump() {
    int x, y;
    bool t = tft.getTouch(&x, &y);
    if (t) {
        if (!tDown) {
            tDown = true;
            tx0 = x;
            ty0 = y;
            routeTouch(TP_DOWN, x, y);
        } else if (abs(x - tx) > 1 || abs(y - ty) > 1) routeTouch(TP_MOVE, x, y);
        tx = x;
        ty = y;
    } else if (tDown) {
        tDown = false;
        routeTouch(TP_UP, tx, ty);
    }
}

// ---------------- events from core 1 / hardware ----------------
static uint32_t tbNext = 0;
// Handle network events, power button events and notification timeout.
inline void uiPumpEvents() {
    UiEvt e;
    while (xQueueReceive(qUi, &e, 0) == pdTRUE) {
        if (e.t == UE_NET) {
            gNetState = e.a;
            gRssi = e.b;
            if (gPages[gCur].flags & PF_TOOLBAR && gModal == MD_NONE) drawToolbar(false);
            continue;
        }
        if (e.t == UE_TOAST) {
            bool pr = e.s2[0] || looksLikeProblem(e.s1);
            uiNotify(e.s1, e.s2[0] ? e.s2 : e.s1, pr ? 1 : 0, nullptr, "Close", "", pr ? SND_WRONG : SND_NOTIFY);
            continue;
        }
        if (gGlobalEvt && gGlobalEvt(e)) continue;
        if (gPages[gCur].event) gPages[gCur].event(e);
    }
    if (millis() > tbNext) {
        tbNext = millis() + 5000;
        if ((gPages[gCur].flags & PF_TOOLBAR) && gModal == MD_NONE && !gNotif.active) drawToolbar(false);
    }
    // battery refresh (only repaints if it changed)
    notifTick();
    PwrEvt pe = gPwrEvt;
    if (pe != PE_NONE) {
        gPwrEvt = PE_NONE;
        analyticsEvent("power_btn", pe);
        DBG(POWER, "power button event %d", (int)pe);
        if (pe == PE_SHORT) uiSleepScreen(!gScreenOff);
        else if (pe == PE_HOLD2) {
            if (gScreenOff) uiSleepScreen(false);
            uiToast("Release to restart - keep holding to power off", 4000);
        } else if (pe == PE_HOLD4) {
            uiToast("Release to power off", 4000);
        } else if (pe == PE_RESTART) {
            powerRestart();
        } else if (pe == PE_OFF) {
            powerOff();
        }
    }
}
