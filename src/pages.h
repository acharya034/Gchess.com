// pages.h - every screen of the "smartphone". Registered in pagesBegin(). Runs on CORE 0 (UI task).
#pragma once
#include <qrcode.h>
#include "ui.h"
#include "game.h"
#include "net.h"
#include "debug.h"

struct TC {
    uint16_t limit;
    uint8_t inc;
    const char* name;
};
static const TC TCS[] = {{180, 0, "3+0"}, {300, 3, "5+3"}, {600, 0, "10+0"}, {900, 10, "15+10"}, {1800, 0, "30+0"}};
static uint8_t gTc = 2, gAiLevel = 3, gColorSel = 0;
static bool gRated = false;
static int8_t gActiveAcc = -1;
static char gFriend[30];
#define gWhiteBottom (G.whiteBottom)
static int8_t gView = 2;  // which game slot the game page shows (0,1 online, 2 scratch)
static uint32_t gExpectGame = 0;  // set when THE USER starts a game => only then we jump to the game page
static void (*gPendingStart)() = nullptr;
static bool gPendingOnline = false;
static int puzReqMode = 0;
// Show this game slot on the game page.
inline void viewGame(int s) {
    gView = s;
    gGp = &GS[s];
    gGp->slot = s;
}

static int8_t ogSlots[3];
static uint8_t ogN = 0;
// Find games that are loaded in slots.
static void ogScan() {
    ogN = 0;
    for (int i = 0; i < 3; i++) if (GS[i].mode != GM_NONE && GS[i].mode != GM_REVIEW && !GS[i].over) ogSlots[ogN++] = i;
}

// Rows of the "Ongoing" page: games loaded in a live slot + the lightweight list of every other running game (any number, no RAM per game)
struct OgRow {
    int8_t slot;
    bool white, myTurn;
    char id[14];
    char opp[30];
    char t1[34];
    char t2[28];
};
static OgRow ogRows[32];
static uint8_t ogRowN = 0;
static bool gOpenedFromList = false;
// Build the rows of the Ongoing page: loaded games first, then the light list.
static void ogBuild() {
    ogScan();
    ogRowN = 0;
    for (int k = 0; k < ogN && ogRowN < 32; k++) {
        Game& g = GS[ogSlots[k]];
        OgRow& r = ogRows[ogRowN++];
        memset(&r, 0, sizeof(r));
        r.slot = ogSlots[k];
        if (g.mode == GM_ONLINE) {
            NetGame& n = gNetG[g.slot & 1];
            strlcpy(r.opp, g.myWhite ? n.black : n.white, 30);
            snprintf(r.t1, 34, "vs %s", r.opp);
            r.myTurn = g.pos.white == g.myWhite;
            strlcpy(r.id, n.id, 14);
        } else if (g.mode == GM_LOCAL) {
            strcpy(r.t1, "Pass & play");
            r.myTurn = true;
        } else {
            snprintf(r.t1, 34, "Puzzle %s", g.puzId);
            r.myTurn = true;
        }
        snprintf(r.t2, 28, "%s - %s", r.myTurn ? "your move" : "waiting", g.physical ? "on board" : "display");
    }
    for (int i = 0; i < gList[LK_ONGOING].n && ogRowN < 32; i++) {
        ListItem& it = gList[LK_ONGOING].it[i];
        bool loaded = false;
        for (int s = 0; s < 2; s++) if (gNetG[s].active && !strcmp(gNetG[s].id, it.id)) loaded = true;
        if (loaded) continue;
        OgRow& r = ogRows[ogRowN++];
        memset(&r, 0, sizeof(r));
        r.slot = -1;
        r.white = it.val & 1;
        r.myTurn = it.val & 2;
        strlcpy(r.id, it.id, 14);
        strlcpy(r.opp, it.main, 30);
        snprintf(r.t1, 34, "vs %s", it.main);
        strlcpy(r.t2, it.sub, 28);
    }
}

// How many online games are loaded now (max 2).
static int onlineLive() {
    int n = 0;
    for (int i = 0; i < 2; i++) n += gNetG[i].active;
    return n;
}

// The unfinished game that owns the physical board, or nothing.
static Game* physLive() {
    for (int i = 0; i < 3; i++) if (GS[i].physical && GS[i].mode != GM_NONE && GS[i].mode != GM_REVIEW && !GS[i].over) return &GS[i];
    return nullptr;
}

// Resign / abort a game (or end a local one).
static void leaveGame(Game* g) {
    if (g->mode == GM_ONLINE) netCmd(g->nHist >= 2 ? NC_RESIGN : NC_ABORT, "", gNetG[g->slot & 1].id);
    else g->mode = GM_NONE;
    g->physical = false;
    ledsClear();
}

// Answer of the "Game in progress" question.
static void ogParkSlot(int s);   // defined in the Ongoing section below

// Which loaded online game (slot 0/1) is running, if any.
static int liveSlot() {
    for (int i = 0; i < 2; i++) if (gNetG[i].active) return i;
    return 0;
}

// Answer of the "Game in progress" question.
static void guardCb(int r) {
    DBG(GAME, "new-game guard: user chose %d (1=keep/park old game, 2=leave old game, 3=cancel)", r);
    Game* pg = physLive();
    if (r == 1) {
        if (pg && pg->mode == GM_ONLINE && gPendingOnline) ogParkSlot(pg->slot);   // online -> online: park (stops its live link, it keeps running on Lichess)
        else if (pg) { pg->physical = false; pg->syncing = false; }                // otherwise it simply keeps running on the display
    } else if (r == 2) {
        if (pg) leaveGame(pg);
    } else return;
    if (gPendingStart) gPendingStart();
}

// Every "start a new game" goes through here: asks what to do with the game that is still on the board.
static void guardNewGame(void (*fn)(), bool online) {
    Game* pg = physLive();
    // Only MAX_LIVE_GAMES online games can be live (memory). If that game is not the one on the board we cannot help.
    if (online && onlineLive() >= MAX_LIVE_GAMES && (!pg || pg->mode != GM_ONLINE)) {
        uiToast("A game is already live - park it in Ongoing first");
        soundPlay(SND_WRONG);
        return;
    }
    gPendingStart = fn;
    gPendingOnline = online;
    DBG(GAME, "start new game requested (online=%d), game on board: %s", (int)online, pg ? "yes -> asking" : "none");
    if (!pg) {
        fn();
        return;
    }
    if (pg->mode == GM_ONLINE && online) uiDialog3("Game in progress", "Park it (it keeps running on\nLichess) or resign it?", "Park old game", "Resign old game", "Cancel", guardCb);
    else uiDialog3("Game in progress", "Continue the old game on the\ndisplay only, or leave it?", "Display only", "Leave old game", "Cancel", guardCb);
}

static char gOpp[32];
static int gOppRating = 0;
#define BTN(x, y, w, h, l, id, bg, fg) Btn{x, y, w, h, l, id, bg, fg}
static const uint16_t INK = RGB(10, 20, 20);

// ---------- session ----------
// Account unlocked: tell the network task and go to the home page.
inline void sessionLogin(int i) {
    gActiveAcc = i;
    gSet.lastAcc = i;
    netCmd(NC_LOGIN, gAcc[i].user, gAcc[i].token);
    strlcpy(gMyUser, gAcc[i].user, 24);
    analyticsEvent("login");
    gSp = 0;
    uiGo(PG_HOME, false);
}

// ---------- INTRO ----------
static uint8_t introStage = 0xFF;
static uint32_t introT0;
// Intro page: play the start melody.
static void introEnter() {
    introT0 = millis();
    introStage = 0xFF;
    soundPlay(SND_POWER_ON);
}

// Intro page: logo.
static void introDraw() {
    logoFull(120, 40);
}

// Intro page: progress text and bar.
static void introStatus(uint8_t st, const char* msg) {
    tft.fillRect(0, 230, 240, 60, T.bg);
    gfxText(120, 236, msg, 1, T.dim, TD_TC);
    tft.drawRoundRect(30, 262, 180, 8, 4, T.card2);
    tft.fillRoundRect(30, 262, 45 * st, 8, 4, T.accent);
}

// Intro page: steps of the start-up, then go to lock or link page.
static void introTick() {
    uint32_t t = millis() - introT0;
    uint8_t st = t < 600 ? 0 : t < 1200 ? 1 : 2;
    bool wifiWanted = gSet.wifiOn && wifiFind("") < 0 && (gWifi[0].used || gWifi[1].used || gWifi[2].used || gWifi[3].used || gWifi[4].used || gWifi[5].used || gWifi[6].used || gWifi[7].used);
    if (t >= 1200) {
        if (wifiWanted && gNetState < 2 && t < 9000) st = 2;
        else st = 3;
    }
    if (st != introStage) {
        introStage = st;
        const char* m[] = {"Starting hardware...", "Loading your settings...", "Connecting to Wi-Fi...", "Ready!"};
        introStatus(st == 3 ? 4 : st, m[st]);
    }
    if (st == 3 && t > 1800) {
        gSp = 0;
        gStack[0] = PG_INTRO;
        uiGo(accCount() ? PG_LOCK : PG_LINK, false);
    }
}

// ---------- LOCK ----------
static int lockAcc = -1;
static uint16_t lockTmp = 0;
// Lock page: list of accounts.
static void lockDraw() {
    logoSmall(120, 40, 18);
    gfxText(120, 66, "Who's playing?", 2, T.fg, TD_TC);
    int y = 100;
    for (int i = 0; i < MAX_ACC; i++) if (gAcc[i].used) {
        gfxCard(12, y, 216, 46, T.card);
        tft.fillCircle(38, y + 23, 16, T.accent);
        char c[2] = {(char)toupper(gAcc[i].user[0]), 0};
        gfxText(38, y + 24, c, 2, INK, TD_MC);
        gfxText(64, y + 6, gAcc[i].user, 2, T.fg);
        gfxText(64, y + 28, gAcc[i].pass ? "Tap to unlock" : "Tap to set passcode", 1, T.dim);
        y += 52;
    }
    if (accCount() < MAX_ACC) gfxBtn(12, 276, 216, 34, "+ Add Lichess account", T.card2, T.accent);
}

static void lockCb(uint16_t v, bool ok);
// Second entry of a new passcode.
static void lockCreate2(uint16_t v, bool ok) {
    if (v == lockTmp) {
        gAcc[lockAcc].pass = v + 1;
        storageSaveAccounts();
        sessionLogin(lockAcc);
    } else {
        soundPlay(SND_WRONG);
        uiToast("Codes differ - try again");
        uiNumpad("Create 4-digit passcode", 4, [](uint16_t a, bool) {
            lockTmp = a;
            uiNumpad("Repeat passcode", 4, lockCreate2);
        });
    }
}

// Passcode typed on the lock page.
static void lockCb(uint16_t v, bool ok) {
    if (!gAcc[lockAcc].pass) {
        lockTmp = v;
        uiNumpad("Repeat passcode", 4, lockCreate2);
        return;
    }
    if (gAcc[lockAcc].pass == v + 1) sessionLogin(lockAcc);
    else {
        soundPlay(SND_WRONG);
        analyticsEvent("bad_pass");
        uiToast("Wrong passcode");
    }
}

// Lock page: tap an account or "Add account".
static void lockTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    int yy = 100;
    for (int i = 0; i < MAX_ACC; i++) if (gAcc[i].used) {
        if (y >= yy && y < yy + 46) {
            uiTap();
            analyticsButton(PG_LOCK, "account");
            lockAcc = i;
            char t[40];
            snprintf(t, 40, gAcc[i].pass ? "Passcode for %s" : "Create passcode for %s", gAcc[i].user);
            uiNumpad(t, 4, lockCb);
            return;
        }
        yy += 52;
    }
    if (y >= 276 && accCount() < MAX_ACC) {
        uiTap();
        analyticsButton(PG_LOCK, "add_account");
        uiGo(PG_LINK);
    }
}

// ---------- LINK (QR) ----------
static bool linkShown = false;
// Draw a QR code.
static void drawQR(const char* text, int x, int y, int scale) {
    QRCode q;
    static uint8_t buf[600];
    qrcode_initText(&q, buf, 7, ECC_LOW, text);
    tft.fillRect(x - 6, y - 6, q.size * scale + 12, q.size * scale + 12, GC_WHITE);
    for (int j = 0; j < q.size; j++) for (int i = 0; i < q.size; i++) if (qrcode_getModule(&q, i, j)) tft.fillRect(x + i * scale, y + j * scale, scale, scale, GC_BLACK);
}

// Link page: start the receiver when Wi-Fi is available.
static void linkEnter() {
    linkShown = false;
    if (gNetState >= 2) netCmd(NC_LINK_START);
}

// Link page: QR code.
static void linkDraw() {
    gfxText(120, 28, "Link your Lichess account", 1, T.fg, TD_TC);
    if (gNetState < 2) {
        gfxText(120, 100, "Connect to Wi-Fi first", 1, T.dim, TD_TC);
        gfxBtn(50, 130, 140, 36, "Wi-Fi setup", T.accent, INK);
        return;
    }
    if (linkShown) {
        drawQR(gLinkUrl, 30, 52, 4);
        gfxText(120, 262, "Scan with your phone", 1, T.dim, TD_TC);
        gfxText(120, 282, "Phone must be on the same Wi-Fi", 1, T.accent, TD_TC);
    } else gfxText(120, 140, "Preparing...", 1, T.dim, TD_TC);
}

// Link page: button to Wi-Fi setup.
static void linkTouch(TouchPhase ph, int x, int y) {
    if (ph == TP_UP && gNetState < 2 && y > 130 && y < 166) {
        uiTap();
        uiGo(PG_WIFI);
    }
}

// Link page: close the pairing window.
static void linkLeave() {
    netCmd(NC_LINK_STOP);
}

// Link page: QR ready / account linked.
static void linkEvent(const UiEvt& e) {
    if (e.t == UE_LINK_URL) {
        linkShown = true;
        uiRedraw();
    } else if (e.t == UE_LINK_DONE) {
        soundPlay(SND_SUCCESS);
        analyticsEvent("account_linked");
        lockAcc = e.a;
        char m[48];
        snprintf(m, 48, "Linked as %s", e.s1);
        uiToast(m);
        gSp = 0;
        gStack[0] = PG_INTRO;
        uiGo(PG_LOCK, false);
    }
}

// ---------- HOME ----------
struct Tile {
    const char* t;
    const char* s;
    uint16_t c;
    PageId pg;
};
static const Tile HOME_TILES[6] = {{"Play", "AI, friends, online", RGB(60, 190, 160), PG_PLAY}, {"Puzzles", "Daily & offline", RGB(250, 170, 60), PG_PUZZLE}, {"History", "Review your games", RGB(120, 140, 250), PG_HISTORY}, {"Friends", "Search & challenge", RGB(240, 110, 150), PG_FRIENDS}, {"Ongoing", "Resume your games", RGB(100, 200, 120), PG_ONGOING}, {"Settings", "Wi-Fi, display...", RGB(150, 160, 180), PG_SETTINGS}};
// Home page: profile card and 6 tiles.
static void homeDraw() {
    gfxCard(8, 26, 224, 44, T.card);
    tft.fillCircle(32, 48, 14, T.accent);
    char c[2] = {(char)toupper(gMyUser[0] ? gMyUser[0] : '?'), 0};
    gfxText(32, 49, c, 2, INK, TD_MC);
    gfxText(54, 30, gMyUser, 2, T.fg);
    char r[24];
    if (gMyRating) snprintf(r, 24, "Rating %d", gMyRating);
    else strcpy(r, gNetState == 3 ? "Online" : "Offline");
    gfxText(54, 52, r, 1, T.dim);
    for (int i = 0; i < 6; i++) {
        int x = 8 + (i % 2) * 114, y = 80 + (i / 2) * 76;
        tft.fillRoundRect(x, y, 110, 70, 12, HOME_TILES[i].c);
        gfxText(x + 10, y + 12, HOME_TILES[i].t, 2, INK);
        gfxText(x + 10, y + 44, HOME_TILES[i].s, 1, INK);
    }
    ogBuild();
    if (ogRowN) {
        char c[4];
        snprintf(c, 4, "%d", ogRowN);
        tft.fillCircle(106, 244, 11, T.bad);
        gfxText(106, 244, c, 1, GC_WHITE, TD_MC);
    }
}

static void localStart();
static void doStartAi();
static void doStartFriend();
static void doStartSeek();
static void doStartLocal();
static void doAccept();
static void doPuzzleReq();
// Home page: tile pressed.
static void homeTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP || y < 80) return;
    int i = ((y - 80) / 76) * 2 + (x > 118);
    if (i > 5 || (y - 80) % 76 > 70) return;
    uiTap();
    analyticsButton(PG_HOME, HOME_TILES[i].t);
    uiGo(HOME_TILES[i].pg);
}

// ---------- PLAY ----------
// Play page.
static void playDraw() {
    gfxText(12, 28, "Time", 1, T.dim);
    gfxBtn(60, 24, 70, 28, TCS[gTc].name, T.card2, T.fg);
    gfxBtn(138, 24, 90, 28, gRated ? "Rated" : "Casual", T.card2, gRated ? T.accent : T.dim);
    const char* l[4] = {"Play vs Computer", "Challenge a friend", "Quick match (online)", "Pass & play (this board)"};
    for (int i = 0; i < 4; i++) gfxBtn(12, 66 + i * 54, 216, 46, l[i], T.card, T.fg);
}

// Play page: time control, rated switch, game mode buttons.
static void playTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    if (y < 56) {
        uiTap();
        if (x < 134) gTc = (gTc + 1) % 5;
        else gRated = !gRated;
        analyticsButton(PG_PLAY, "tc_rated");
        playDraw();
        return;
    }
    int i = (y - 66) / 54;
    if (i < 0 || i > 3) return;
    uiTap();
    analyticsButton(PG_PLAY, "mode");
    if (i == 0) uiGo(PG_AI);
    else if (i == 1) uiGo(PG_FRIENDS);
    else if (i == 2) guardNewGame(doStartSeek, true);
    else localStart();
}

// ---------- AI setup ----------
// Computer page.
static void aiDraw() {
    gfxText(120, 28, "Engine: Stockfish (Lichess)", 1, T.dim, TD_TC);
    gfxText(120, 56, "Level", 1, T.dim, TD_TC);
    gfxBtn(20, 76, 56, 50, "-", T.card2, T.fg);
    gfxBtn(164, 76, 56, 50, "+", T.card2, T.fg);
    char s[4];
    snprintf(s, 4, "%d", gAiLevel);
    gfxText(120, 100, s, 2, T.accent, TD_MC);
    const char* c[3] = {"Random", "White", "Black"};
    for (int i = 0; i < 3; i++) gfxBtn(8 + i * 76, 140, 72, 36, c[i], gColorSel == i ? T.accent : T.card2, gColorSel == i ? INK : T.fg);
    gfxBtn(60, 190, 120, 34, TCS[gTc].name, T.card2, T.fg);
    gfxBtn(30, 244, 180, 46, "Start game", T.accent, INK);
}

// Computer page: level, colour, time, start.
static void aiTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    uiTap();
    if (y > 76 && y < 126) {
        if (x < 80 && gAiLevel > 1) gAiLevel--;
        else if (x > 160 && gAiLevel < 8) gAiLevel++;
    } else if (y > 140 && y < 176) gColorSel = constrain(x / 80, 0, 2);
    else if (y > 190 && y < 224) gTc = (gTc + 1) % 5;
    else if (y > 244) {
        guardNewGame(doStartAi, true);
        return;
    }
    aiDraw();
}

// ---------- FRIEND SETUP ----------
// Friend challenge page.
static void fsDraw() {
    gfxText(120, 34, "Challenge", 1, T.dim, TD_TC);
    gfxText(120, 54, gFriend, 2, T.fg, TD_TC);
    const char* c[3] = {"Random", "White", "Black"};
    for (int i = 0; i < 3; i++) gfxBtn(8 + i * 76, 110, 72, 36, c[i], gColorSel == i ? T.accent : T.card2, gColorSel == i ? INK : T.fg);
    gfxBtn(20, 160, 90, 34, TCS[gTc].name, T.card2, T.fg);
    gfxBtn(130, 160, 90, 34, gRated ? "Rated" : "Casual", T.card2, T.fg);
    gfxBtn(30, 230, 180, 46, "Send challenge", T.accent, INK);
}

// Friend challenge page: colour, time, send.
static void fsTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    uiTap();
    if (y > 110 && y < 146) gColorSel = constrain(x / 80, 0, 2);
    else if (y > 160 && y < 194) {
        if (x < 120) gTc = (gTc + 1) % 5;
        else gRated = !gRated;
    } else if (y > 230) {
        guardNewGame(doStartFriend, true);
        return;
    }
    fsDraw();
}

// ---------- WAIT ----------
static uint8_t waitDots;
// Waiting page.
static void waitDraw() {
    logoMedium(120, 100);
    gfxText(120, 160, "Waiting for the game to start", 1, T.fg, TD_TC);
    gfxBtn(70, 250, 100, 36, "Cancel", T.card2, T.bad);
}

// Waiting page: animated dots.
static void waitTick() {
    static uint32_t t;
    if (millis() - t > 400) {
        t = millis();
        waitDots = (waitDots + 1) % 4;
        tft.fillRect(90, 186, 60, 14, T.bg);
        for (int i = 0; i < 3; i++) tft.fillCircle(100 + i * 20, 192, i < waitDots ? 5 : 3, i < waitDots ? T.accent : T.dim);
    }
}

// Waiting page: Cancel = same as Back.
static void waitTouch(TouchPhase ph, int x, int y) {
    if (ph == TP_UP && y > 250) {
        uiTap();
        gExpectGame = 0;
        netCmd(NC_CANCEL_SEEK);
        uiBack();
    }
}

// cancel = same as Back, and no auto-jump into a game later

// ======================= GAME PAGE =======================
#define BY0 38
#define SQP 30
static uint32_t sqHash[64];
static uint64_t wrongMask = 0;
static uint32_t wrongUntil = 0;
static bool promoShown = false, sheetOpen = false;
static uint32_t lastClockSec = 0;
static char gMovesBuf[2400];
static char stripCache[2][80];
static bool leaveHome = false, leaveBypass = false, offerDlg = false;
static int8_t offerSlot = 0;

// Network data of the game that is shown.
inline NetGame& gnet() {
    return G.mode == GM_REVIEW ? gNetX : gNetG[G.slot & 1];
}

// Chess square -> screen position.
inline void sqXY(int sq, int* x, int* y) {
    int f = sq & 7, r = sq >> 3;
    *x = (gWhiteBottom ? f : 7 - f) * SQP;
    *y = BY0 + (gWhiteBottom ? 7 - r : r) * SQP;
}

// Draw one board square (only when it changed).
inline void drawSq(int sq, bool force = false) {
    uint32_t flags = (sq == G.lastFrom) | (sq == G.lastTo) << 1 | (sq == G.selFrom) << 2 | (uint32_t)(G.selDest >> sq & 1) << 3 | (uint32_t)(G.selCap >> sq & 1) << 4 | (uint32_t)(wrongMask >> sq & 1) << 5;
    uint32_t h = (G.pos.b[sq] + 8) | flags << 8 | 1 << 20;
    if (!force && sqHash[sq] == h) return;
    sqHash[sq] = h;  // unchanged squares are never redrawn
    int x, y;
    sqXY(sq, &x, &y);
    int f = sq & 7, r = sq >> 3;
    uint16_t c = ((f + r) & 1) ? T.sqLight : T.sqDark;
    if (flags & 1) c = RGB(95, 140, 235);
    if (flags & 2) c = RGB(105, 205, 115);
    if (flags & 4) c = RGB(70, 125, 240);
    if (flags & 32) c = RGB(225, 60, 60);
    bool s = sprBegin(x, y, SQP, SQP, c);
    dRect(x, y, SQP, SQP, c);
    if (G.pos.b[sq]) gfxPiece(x, y, G.pos.b[sq]);
    if (flags & 8) {
        if (flags & 16) {
            dRing(x + 15, y + 15, 14, RGB(230, 50, 40));
            dRing(x + 15, y + 15, 13, RGB(230, 50, 40));
        } else if (!G.pos.b[sq]) dCircle(x + 15, y + 15, 5, RGB(30, 190, 80));
        else dRing(x + 15, y + 15, 14, RGB(30, 190, 80));
    }
    if (s) sprEnd();
}

// Draw the whole board.
inline void drawBoardAll() {
    for (int i = 0; i < 64; i++) drawSq(i, true);
}

// Milliseconds -> "mm:ss".
inline void fmtClock(uint32_t ms, char* o) {
    uint32_t s = ms / 1000;
    if (s >= 3600) sprintf(o, "%u:%02u:%02u", s / 3600, (s / 60) % 60, s % 60);
    else sprintf(o, "%02u:%02u", s / 60, s % 60);
}

// Player strip above / below the board: name, rating, clock.
inline void drawStrip(bool top, bool force = false) {
    bool white = top ? !gWhiteBottom : gWhiteBottom;
    int y = top ? TB_H : BY0 + 8 * SQP;
    bool turn = G.pos.white == white && !G.over;
    char name[40] = "", clk[12] = "";
    if (G.mode == GM_ONLINE || G.mode == GM_REVIEW) {
        NetGame& n = gnet();
        xSemaphoreTake(gNetMux, portMAX_DELAY);
        snprintf(name, 40, "%s (%d)", white ? n.white : n.black, white ? n.wRating : n.bRating);
        uint32_t t = white ? n.wtime : n.btime;
        if (G.mode == GM_ONLINE && n.active && turn && G.nHist >= 2) {
            uint32_t el = millis() - n.stamp;
            t = t > el ? t - el : 0;
        }
        if (G.mode == GM_ONLINE) fmtClock(t, clk);
        xSemaphoreGive(gNetMux);
    } else if (G.mode == GM_PUZZLE) {
        if (top) snprintf(name, 40, "Puzzle %s  (%d)", G.puzId, G.puzRating);
        else strcpy(name, G.over ? "Done" : "Find the best move");
    } else strcpy(name, white ? "White" : "Black");
    char sig[80];
    snprintf(sig, 80, "%d|%s|%s", turn, name, clk);
    if (!force && !strcmp(sig, stripCache[top])) return;
    strlcpy(stripCache[top], sig, 80);  // repaint only on change
    uint16_t bg = turn ? T.card2 : T.card;
    bool s = sprBegin(0, y, 240, 18, bg);
    dRect(0, y, 240, 18, bg);
    if (turn) dCircle(8, y + 9, 3, T.ok);
    gfxText(16, y + 1, name, 1, T.fg);
    if (clk[0]) gfxText(234, y + 1, clk, 1, T.fg, TD_TR);
    if (s) sprEnd();
}

// The 4 buttons under the board (depend on the game mode).
static void gameButtonsDraw() {
    const char* l[4];
    uint16_t fg[4] = {T.fg, T.fg, T.fg, T.fg};
    switch (G.mode) {
        case GM_ONLINE: l[0] = !G.physical ? "Take board" : G.touchAllowed ? "Touch:ON" : "Touch:OFF";
        fg[0] = (!G.physical || G.touchAllowed) ? T.accent : T.dim;
        l[1] = "Draw";
        l[2] = "Takeback";
        l[3] = "...";
        break;
        case GM_LOCAL: l[0] = "New";
        l[1] = "Flip";
        l[2] = "Undo";
        l[3] = "Exit";
        break;
        case GM_PUZZLE: l[0] = "Hint";
        l[1] = "Skip";
        l[2] = "Flip";
        l[3] = "Exit";
        break;
        default: l[0] = "|<";
        l[1] = "<";
        l[2] = ">";
        l[3] = "Exit";
    }
    for (int i = 0; i < 4; i++) gfxBtn(2 + i * 60, 297, 56, 22, l[i], T.card, fg[i]);
}

// Repair only the button area (used after a notification).
static void gamePartial() {
    tft.fillRect(0, 292, 240, 28, T.bg);
    gameButtonsDraw();
}

// after a toast disappears: no full redraw
static void promoDraw() {
    tft.fillRoundRect(10, BY0 + 90, 220, 60, 10, T.card);
    const int8_t pcs[4] = {cr::Q, cr::R, cr::B, cr::N};
    gfxText(120, BY0 + 94, "Promote to", 1, T.dim, TD_TC);
    for (int i = 0; i < 4; i++) {
        int x = 20 + i * 52;
        tft.fillRoundRect(x, BY0 + 108, 46, 36, 6, T.sqLight);
        gfxPiece(x + 8, BY0 + 108, G.pos.white ? pcs[i] : -pcs[i]);
    }
}

// The "..." menu of an online game.
static void sheetDraw() {
    tft.fillRoundRect(30, BY0 + 10, 180, 206, 12, T.card);
    const char* l[5] = {"Resign", "Abort game", "Flip display", "Rotate-board mode", "Close"};
    for (int i = 0; i < 5; i++) gfxBtn(40, BY0 + 20 + i * 36, 160, 30, l[i], i == 0 ? T.bad : T.card2, i == 0 ? GC_WHITE : T.fg);
}

// Draw the complete game page.
static void gameDraw() {
    memset(sqHash, 0, sizeof(sqHash));
    stripCache[0][0] = stripCache[1][0] = 0;
    drawBoardAll();
    drawStrip(true, true);
    drawStrip(false, true);
    gameButtonsDraw();
    if (promoShown) promoDraw();
    if (sheetOpen) sheetDraw();
}

// Game page opened.
static void gameEnter() {
    promoShown = sheetOpen = false;
    wrongMask = 0;
    gGp = &GS[gView];
}

// Game page closed (the game itself keeps running).
static void gameLeave() {
}

// the game (and the LEDs) keep running when you leave the page
static void gameFinishDlg(int r) {
    if (r == 1) {
        G.over = true;
        uiHome();
    }
}

// Show the result of a finished game.
static void showResult(const char* msg, bool win) {
    G.over = true;
    soundPlay(win ? SND_SUCCESS : SND_FAIL);
    analyticsEvent("game_end", win);
    if (G.physical) {
        G.physical = false;
        ledsClear();
    }
    bool viewing = gCur == PG_GAME && gGp == &GS[gView];
    if (viewing) {
        drawStrip(true);
        drawStrip(false);
        if (G.mode != GM_REVIEW) uiDialog("Game over", msg, "Home", "Stay", gameFinishDlg);
    } else uiToast(msg, 3500);
}

// The user finished a move: send it to Lichess / check the puzzle / check game end.
static void onUserMove() {
    {
        char u[8];
        cr::toUci(G.lastSubmitted, u);
        DBG(GAME, "USER MOVE %s in slot%d mode=%d", u, (int)G.slot, (int)G.mode);
    }
    analyticsEvent("my_move");
    if (G.mode == GM_ONLINE) {
        char u[8];
        cr::toUci(G.lastSubmitted, u);
        netCmd(NC_MOVE, u, gNetG[G.slot & 1].id);
    } else if (G.mode == GM_PUZZLE) {
        PzRes r = gamePuzzleCheck();
        if (r == PZ_SOLVED) {
            analyticsEvent("puzzle_solved");
            showResult("Puzzle solved!", true);
        } else if (r == PZ_WRONG) {
            soundPlay(SND_WRONG);
            uiToast("Not the best move - put it back");
            gamePuzzleRewind();
        }
    } else if (G.mode == GM_LOCAL && G.over) showResult(G.result, true);
}

// Runs ALWAYS (every page): the game that owns the physical board keeps reading the sensors.
static void physTick() {
    int idx = -1;
    for (int i = 0; i < 3; i++) if (GS[i].physical && GS[i].mode != GM_NONE && GS[i].mode != GM_REVIEW) idx = i;
    if (idx < 0) return;
    GameScope sc(idx);
    bool viewing = gCur == PG_GAME && idx == gView;
    GEvt ev = gameSensorTick(boardOccupancy());
    if (ev == GE_MOVE) onUserMove();
    else if (ev == GE_NEED_PROMO) {
        if (viewing) {
            promoShown = true;
            promoDraw();
        } else {
            gameSubmit(G.pFrom, G.pTo, cr::Q);
            onUserMove();
            uiToast("Promoted to queen");
        }
    } else if (ev == GE_WRONG) {
        analyticsEvent("wrong_move");
        wrongMask = boardOccupancy() & ~G.expected;
        wrongUntil = millis() + 2500;
        uiToast("Wrong move - put it back");
    }
    if (viewing && promoShown && !G.promoPending) {
        promoShown = false;
        gameDraw();
    }
}

// Game page: redraw changed squares and clocks.
static void gameTick() {
    // page tick: drawing only
    if (G.mode == GM_NONE) return;
    if (wrongMask && (millis() > wrongUntil || !G.wrongActive)) wrongMask = 0;
    for (int i = 0; i < 64; i++) drawSq(i);
    uint32_t s = millis() / 500;
    if (s != lastClockSec && G.mode == GM_ONLINE) {
        lastClockSec = s;
        drawStrip(true);
        drawStrip(false);
    }
}

// ----- starting games -----
// Start a pass & play game on this board.
static void doStartLocal() {
    {
        GameScope sc(2);
        gameReset(GM_LOCAL, nullptr, true);
        G.touchAllowed = true;
        G.physical = false;
    }
    gameClaimBoard(2);
    viewGame(2);
    analyticsEvent("start_local");
    uiGo(PG_GAME);
}

// Pass & play button (asks first if another game is on the board).
static void localStart() {
    guardNewGame(doStartLocal, false);
}

// Create a game against Stockfish.
static void doStartAi() {
    if (gNetState < 3) {
        uiToast("Not online");
        soundPlay(SND_WRONG);
        return;
    } gExpectGame = millis() + 90000;
    const char* col[3] = {"random", "white", "black"};
    analyticsEvent("start_ai", gAiLevel);
    netCmd(NC_AI, col[gColorSel], "", gAiLevel, TCS[gTc].limit, TCS[gTc].inc);
    uiGo(PG_WAIT);
}

// Send a challenge to a friend.
static void doStartFriend() {
    if (gNetState < 3) {
        uiToast("Not online");
        soundPlay(SND_WRONG);
        return;
    } gExpectGame = millis() + 90000;
    const char* col[3] = {"random", "white", "black"};
    analyticsEvent("challenge_friend");
    netCmd(NC_CHALLENGE, gFriend, col[gColorSel], TCS[gTc].limit, TCS[gTc].inc, gRated);
    uiGo(PG_WAIT);
}

// Look for an opponent online.
static void doStartSeek() {
    if (gNetState < 3) {
        uiToast("Not online");
        soundPlay(SND_WRONG);
        return;
    } gExpectGame = millis() + 120000;
    netCmd(NC_SEEK, "", "", TCS[gTc].limit / 60, TCS[gTc].inc, gRated);
    uiGo(PG_WAIT);
}

static char chId[16];
// Accept the challenge.
static void doAccept() {
    gExpectGame = millis() + 30000;
    netCmd(NC_ACCEPT, chId);
}

// Ask the network task for a puzzle.
static void doPuzzleReq() {
    netCmd(NC_PUZ_DAILY, "", "", puzReqMode);
}

// ----- leaving the game page -----
// Really leave the game page.
static void doLeave() {
    leaveBypass = true;
    if (leaveHome) uiHome();
    else uiBack();
    leaveBypass = false;
}

// Answer of the "Leave this game?" box.
static void leaveCb(int r) {
    if (r == 1) doLeave();
    else if (r == 2) {
        leaveGame(&G);
        doLeave();
    }
}

// 3 = stay
static bool leaveGuard(bool toHome) {
    DBG(UI, "leave-guard asked (toHome=%d) on %s", (int)toHome, gPages[gCur].name);
    if (leaveBypass || gCur != PG_GAME) return false;
    if (G.mode == GM_ONLINE && !G.over && gNetG[G.slot & 1].active) {
        leaveHome = toHome;
        uiDialog3("Leave this game?", G.physical ? "Keep playing on the board\nwhile you use the display?" : "Keep it running in the\nbackground?", G.physical ? "Keep on board" : "Keep playing", G.nHist >= 2 ? "Resign" : "Abort game", "Stay here", leaveCb);
        return true;
    }
    return false;
}

// Answer of the resign question.
static void resignCb(int r) {
    if (r == 2) {
        analyticsEvent("resign");
        netCmd(NC_RESIGN, "", gNetG[G.slot & 1].id);
    }
}

// Answer of "New game?".
static void localNewCb(int r) {
    if (r == 2) {
        gameReset(GM_LOCAL, nullptr, true);
        gameDraw();
    }
}

// Answer to the opponent draw offer.
static void drawOfferCb(int r) {
    offerDlg = false;
    netCmd(NC_DRAW, r == 2 ? "yes" : "no", gNetG[offerSlot].id);
}

// Answer to the opponent takeback request.
static void takebackCb(int r) {
    netCmd(NC_TAKEBACK, r == 2 ? "yes" : "no", gNetG[offerSlot].id);
}

// Game page: touch on board, promotion chooser, menu or buttons.
static void gameTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    if (promoShown) {
        if (y > BY0 + 108 && y < BY0 + 144 && x >= 20 && x < 228) {
            int i = (x - 20) / 52;
            const int8_t pcs[4] = {cr::Q, cr::R, cr::B, cr::N};
            uiTap();
            promoShown = false;
            G.promoPending = false;
            G.promoByTouch = false;
            gameSubmit(G.pFrom, G.pTo, pcs[i]);
            gameDraw();
            onUserMove();
        } else {
            promoShown = false;
            G.promoPending = false;
            G.promoByTouch = false;
            gameDraw();
        } return;
    }
    if (sheetOpen) {
        int i = (y - BY0 - 20) / 36;
        sheetOpen = false;
        if (y >= BY0 + 20 && i >= 0 && i < 5) {
            uiTap();
            if (i == 0) {
                gameDraw();
                uiDialog("Resign?", "You will lose this game.", "No", "Resign", resignCb);
                return;
            } else if (i == 1) netCmd(NC_ABORT, "", gNetG[G.slot & 1].id);
            else if (i == 2) G.whiteBottom = !G.whiteBottom;
            else if (i == 3) {
                gSet.flipHw = !gSet.flipHw;
                uiToast(gSet.flipHw ? "Board rotated mode ON" : "Board rotated mode OFF");
            }
        }
        gameDraw();
        return;
    }
    if (y >= BY0 && y < BY0 + 8 * SQP) {
        if (!G.touchAllowed && G.mode != GM_REVIEW) {
            uiToast("Touch play is OFF (Touch:OFF)");
            return;
        }
        int col = x / SQP, row = (y - BY0) / SQP;
        int f = G.whiteBottom ? col : 7 - col, r = G.whiteBottom ? 7 - row : row;
        GEvt e = gameTouchSquare(r * 8 + f);
        if (e == GE_MOVE) onUserMove();
        else if (e == GE_NEED_PROMO) {
            promoShown = true;
            promoDraw();
        } return;
    }
    if (y < 296) return;
    int b = x / 60;
    if (b > 3) return;
    uiTap();
    analyticsButton(PG_GAME, "game_btn");
    const char* gid = gNetG[G.slot & 1].id;
    switch (G.mode) {
        case GM_ONLINE: if (b == 0) {
            if (G.physical) {
                G.touchAllowed = !G.touchAllowed;
                uiToast(G.touchAllowed ? "Touch play allowed" : "Touch play blocked");
            } else {
                gameClaimBoard(G.slot);
                uiToast("This game now uses the board");
            }
            gameButtonsDraw();
        } else if (b == 1) {
            netCmd(NC_DRAW, "yes", gid);
            uiToast("Draw offered");
        } else if (b == 2) {
            netCmd(NC_TAKEBACK, "yes", gid);
            uiToast("Takeback requested");
        } else {
            sheetOpen = true;
            sheetDraw();
        } break;
        case GM_LOCAL: if (b == 0) uiDialog("New game?", "Reset to the start position.", "No", "Yes", localNewCb);
        else if (b == 1) {
            G.whiteBottom = !G.whiteBottom;
            gameDraw();
        } else if (b == 2) {
            if (G.nHist) {
                static cr::Move save[512];
                int n = G.nHist - 1;
                memcpy(save, G.hist, sizeof(cr::Move) * n);
                gameReset(GM_LOCAL, nullptr, true);
                for (int i = 0; i < n; i++) gameApply(save[i], true);
                gameDraw();
            }
        } else uiHome();
        break;
        case GM_PUZZLE: if (b == 0 && G.puzIdx < G.puzN) {
            cr::Move m;
            if (cr::fromUci(G.pos, G.puzSol[G.puzIdx], &m)) {
                gLedsClear();
                gLedSq(m.from, CRGB(0, 70, 255));
                uiToast("Hint: move the blue piece");
            }
        } else if (b == 1) netCmd(NC_PUZ_DAILY, "", "", gNetState >= 3 ? 1 : 2);
        else if (b == 2) {
            G.whiteBottom = !G.whiteBottom;
            gameDraw();
        } else if (b == 3) uiHome();
        break;
        default: if (b == 0) gameRevGoto(0);
        else if (b == 1) gameRevGoto(G.revIdx - 1);
        else if (b == 2) gameRevGoto(G.revIdx + 1);
        else uiBack();
        break;
    }
}

// server -> game (any slot, any page)
static void onGameUpdate(int slot, bool full) {
    DBG(GAME, "server update for slot%d (full=%d)", slot, (int)full);
    GameScope sc(slot);
    NetGame& n = gNetG[slot];
    static char fen[96];
    bool iw, d1, t1;
    xSemaphoreTake(gNetMux, portMAX_DELAY);
    strlcpy(gMovesBuf, n.moves, sizeof(gMovesBuf));
    strlcpy(fen, n.initialFen, 96);
    iw = n.iAmWhite;
    d1 = iw ? n.bdraw : n.wdraw;
    t1 = iw ? n.btake : n.wtake;
    xSemaphoreGive(gNetMux);
    if (full && G.mode != GM_ONLINE) gameReset(GM_ONLINE, fen, iw);   // a reconnect (after a pause) must not wipe the game
    if (G.mode != GM_ONLINE) return;
    G.myWhite = iw;
    int applied = gameApplyMovesStr(gMovesBuf);
    bool viewing = gCur == PG_GAME && slot == gView;
    if (d1 && !offerDlg && gModal == MD_NONE) {
        offerDlg = true;
        offerSlot = slot;
        uiDialog("Draw offer", "Your opponent offers a draw.", "Decline", "Accept", drawOfferCb);
    } else if (t1 && gModal == MD_NONE) {
        offerSlot = slot;
        uiDialog("Takeback", "Opponent asks to take back a move.", "Decline", "Allow", takebackCb);
    }
    if (viewing) {
        for (int i = 0; i < 64; i++) drawSq(i);
        drawStrip(true);
        drawStrip(false);
    } else if (applied > 0 && G.pos.white == G.myWhite) uiToast("Opponent moved - your turn");
}

// The server says the game is over.
static void onGameEnd(int slot, const char* status, const char* winner) {
    DBG(GAME, "game over slot%d status=%s winner=%s", slot, status, winner);
    GameScope sc(slot);
    if (G.mode != GM_ONLINE) return;
    bool iw = gNetG[slot].iAmWhite;
    char m[40];
    const char* why = !strcmp(status, "mate") ? "checkmate" : !strcmp(status, "resign") ? "resignation" : !strcmp(status, "outoftime") || !strcmp(status, "timeout") ? "time" : !strcmp(status, "aborted") ? "aborted" : !strcmp(status, "stalemate") ? "stalemate" : status;
    if (!winner[0]) {
        snprintf(m, 40, "Draw / %s", why);
        showResult(m, false);
        return;
    }
    bool win = (!strcmp(winner, "white")) == iw;
    snprintf(m, 40, "You %s - %s", win ? "won" : "lost", why);
    showResult(m, win);
}

// ---------- ONGOING GAMES ----------
static ListView ogv;
static OgRow gOpenRow;
// One row of the Ongoing page.
static void ogRowDraw(int i, int x, int y, int w, int h) {
    OgRow& r = ogRows[i];
    gfxCard(x, y, w, h, T.card);
    gfxText(x + 10, y + 5, r.t1, 1, T.fg);
    gfxText(x + 10, y + 24, r.t2, 1, r.myTurn ? T.accent : T.dim);
    if (r.slot >= 0) {
        Game& g = GS[r.slot];
        if (!g.physical) gfxBtn(x + w - 86, y + h - 28, 40, 22, "Board", T.card2, T.accent);
        if (g.mode == GM_ONLINE) gfxBtn(x + w - 42, y + h - 28, 36, 22, "Park", T.card2, T.warn);
    } else gfxText(x + w - 10, y + 24, "open >", 1, T.dim, TD_TR);
}

// Ongoing page opened: refresh the list.
static void ogEnter() {
    ogBuild();
    ogv = {4, 24, 232, 294, 54, (int16_t)ogRowN, 0};
    if (gNetState >= 3) netCmd(NC_ONGOING);
}

// Ongoing page.
static void ogDraw() {
    ogBuild();
    ogv.n = ogRowN;
    tft.fillRect(0, 22, 240, 298, T.bg);
    if (!ogRowN) {
        gfxText(120, 120, "No ongoing games", 2, T.dim, TD_TC);
        gfxText(120, 160, "Start one from Play", 1, T.dim, TD_TC);
        return;
    }
    uiListDraw(ogv, ogRowDraw, false);
}

// Ongoing page: list arrived.
static void ogEvent(const UiEvt& e) {
    if (e.t == UE_LIST && e.a == LK_ONGOING) {
        ogv.scroll = 0;
        ogDraw();
    }
}

// Free a live slot; the game keeps running on Lichess.
static void ogParkSlot(int s) {
    netCmd(NC_PARK_GAME, "", "", s);
    if (GS[s].physical) {
        GS[s].physical = false;
        ledsClear();
    }
    GS[s].mode = GM_NONE;
    gNetG[s].active = false;
}

// game keeps running on Lichess
static void openRemote(const OgRow& r) {
    gOpenedFromList = true;
    gExpectGame = millis() + 25000;
    netCmd(NC_OPEN_GAME, r.id, r.opp, r.white);
    uiToast("Opening game...");
}

// Which game to park when both slots are busy.
static void parkChooseCb(int r) {
    DBG(GAME, "park dialog answer %d", r);
    if (r == 2) {
        ogParkSlot(liveSlot());
        openRemote(gOpenRow);
    }
}

// Only one game can be live: ask before parking it to open another one.
static void askPark(const OgRow& r) {
    gOpenRow = r;
    uiDialog("Park current game?", "It keeps running on Lichess\nand stays in this list.", "Cancel", "Park & open", parkChooseCb);
}

// Ongoing page: open, take the board or park.
static void ogTouch(TouchPhase ph, int x, int y) {
    int i = uiListTouch(ogv, ph, x, y, ogRowDraw);
    if (i < 0) return;
    DBG(UI, "ongoing list: tapped row %d", i);
    OgRow r = ogRows[i];
    uiTap();
    analyticsButton(PG_ONGOING, "open");
    if (r.slot >= 0) {
        Game& g = GS[r.slot];
        int bx = ogv.x + ogv.w;
        if (x >= bx - 42 && g.mode == GM_ONLINE) {
            ogParkSlot(r.slot);
            uiToast("Game parked - still running on Lichess");
            ogDraw();
            return;
        }
        if (x >= bx - 86 && x < bx - 44 && !g.physical) {
            gameClaimBoard(r.slot);
            uiToast("This game now uses the board");
            ogDraw();
            return;
        }
        viewGame(r.slot);
        uiGo(PG_GAME);
        return;
    }
    if (gNetState < 3) {
        uiToast("Not online");
        return;
    }
    if (onlineLive() >= MAX_LIVE_GAMES) askPark(r);
    else openRemote(r);
}

// ---------- PUZZLE menu ----------
// Puzzle menu.
static void puzzleDraw() {
    gfxBtn(12, 30, 216, 50, "Daily puzzle", T.card, T.fg);
    gfxBtn(12, 90, 216, 50, "Next puzzle (my rating)", T.card, T.fg);
    char s[40];
    snprintf(s, 40, "Offline pack (%d saved)", puzzleStoreCount());
    gfxBtn(12, 150, 216, 50, s, T.card, T.fg);
    gfxBtn(12, 210, 216, 40, "Download 10 more", T.card2, T.accent);
    gfxText(120, 264, "Puzzles near your rating are kept offline.", 1, T.dim, TD_TC);
}

// Puzzle menu buttons.
static void puzzleTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    int i = y < 85 ? 0 : y < 145 ? 1 : y < 205 ? 2 : y < 255 ? 3 : -1;
    if (i < 0) return;
    uiTap();
    analyticsButton(PG_PUZZLE, "puz");
    if (i == 3) {
        if (gNetState < 3) uiToast("Not online");
        else {
            netCmd(NC_PUZ_BATCH, "", "", 10);
            uiToast("Downloading...");
        } return;
    }
    if (i == 0 && gNetState < 3) {
        uiToast("Daily puzzle needs Wi-Fi");
        return;
    } puzReqMode = (i == 0) ? 0 : (i == 1 && gNetState >= 3) ? 1 : 2;
    guardNewGame(doPuzzleReq, false);
}

// ---------- LIST PAGES (history / friends / wifi) ----------
static ListKind lpKind;
static ListView lpv;
static bool lpSearching = false;
// One row of a list page (friends, history, Wi-Fi).
static void rowDraw(int i, int x, int y, int w, int h) {
    ListItem& it = gList[lpKind].it[i];
    gfxCard(x, y, w, h, T.card);
    gfxText(x + 10, y + 3, it.main, 1, T.fg);
    gfxText(x + 10, y + 21, it.sub, 1, strstr(it.sub, "online") || strstr(it.sub, "saved") ? T.accent : T.dim);
}

// Prepare a list view.
static void listSetup(ListKind k, int y0) {
    lpKind = k;
    lpv = {4, (int16_t)y0, 232, (int16_t)(318 - y0), 46, 0, 0};
    lpv.n = gList[k].n;
}

// Draw a list page.
static void listDraw() {
    uiListDraw(lpv, rowDraw);
    if (!lpv.n) gfxText(120, 150, "Loading...", 1, T.dim, TD_TC);
}

// History page opened.
static void historyEnter() {
    gList[LK_HISTORY].n = 0;
    listSetup(LK_HISTORY, 24);
    if (gNetState >= 3) netCmd(NC_HISTORY);
}

// Friends page opened.
static void friendsEnter() {
    gList[LK_FRIENDS].n = 0;
    lpSearching = false;
    listSetup(LK_FRIENDS, 58);
    if (gNetState >= 3) netCmd(NC_FRIENDS);
}

// Friends page.
static void friendsDraw() {
    gfxBtn(8, 24, 224, 30, "Search players...", T.card2, T.accent);
    listDraw();
}

// Search text typed.
static void searchCb(const char* t, bool ok) {
    if (ok && t[0]) {
        netCmd(NC_SEARCH, t);
        lpSearching = true;
        listSetup(LK_SEARCH, 58);
    }
}

// Friends page: search or pick a friend.
static void friendsTouch(TouchPhase ph, int x, int y) {
    if (ph == TP_UP && y < 56 && y > 24) {
        uiTap();
        uiKeyboard("Search Lichess username", "", false, searchCb);
        return;
    }
    int i = uiListTouch(lpv, ph, x, y, rowDraw);
    if (i >= 0) {
        uiTap();
        analyticsButton(PG_FRIENDS, "friend");
        strlcpy(gFriend, gList[lpKind].it[i].main, 30);
        uiGo(PG_FRIEND_SETUP);
    }
}

// History page: pick a game to review.
static void historyTouch(TouchPhase ph, int x, int y) {
    int i = uiListTouch(lpv, ph, x, y, rowDraw);
    if (i >= 0) {
        uiTap();
        analyticsButton(PG_HISTORY, "game");
        netCmd(NC_GAME_EXPORT, gList[LK_HISTORY].it[i].id);
        uiToast("Loading game...");
    }
}

// A list arrived from the network.
static void listEvent(const UiEvt& e) {
    if (e.t == UE_LIST && e.a == lpKind) {
        lpv.n = e.b;
        lpv.scroll = 0;
        listDraw();
    }
}

// ---------- WIFI ----------
static char wifiPick[34];
// Wi-Fi page opened: scan.
static void wifiEnter() {
    gList[LK_WIFI].n = 0;
    listSetup(LK_WIFI, 58);
    netCmd(NC_WIFI_SCAN);
}

// Wi-Fi page.
static void wifiDraw() {
    gfxBtn(8, 24, 108, 30, "Rescan", T.card2, T.accent);
    gfxBtn(124, 24, 108, 30, "Forget all", T.card2, T.bad);
    listDraw();
}

// Wi-Fi password typed.
static void wifiPassCb(const char* t, bool ok) {
    if (ok) {
        netCmd(NC_WIFI_CONNECT, wifiPick, t);
        uiToast("Connecting...");
    }
}

// Answer of "Forget Wi-Fi".
static void wifiForgetCb(int r) {
    if (r == 2) {
        memset(gWifi, 0, sizeof(gWifi));
        storageSaveWifi();
        uiToast("Saved networks removed");
    }
}

// Wi-Fi page: rescan, forget or pick a network.
static void wifiTouch(TouchPhase ph, int x, int y) {
    if (ph == TP_UP && y > 24 && y < 54) {
        uiTap();
        if (x < 120) {
            netCmd(NC_WIFI_SCAN);
            gList[LK_WIFI].n = 0;
            lpv.n = 0;
            listDraw();
        } else uiDialog("Forget Wi-Fi", "Remove all saved networks?", "No", "Yes", wifiForgetCb);
        return;
    }
    int i = uiListTouch(lpv, ph, x, y, rowDraw);
    if (i < 0) return;
    uiTap();
    ListItem& it = gList[LK_WIFI].it[i];
    strlcpy(wifiPick, it.main, 34);
    if (it.id[0] == 'o' || strstr(it.sub, "saved")) {
        netCmd(NC_WIFI_CONNECT, it.main, "");
        uiToast("Connecting...");
    } else uiKeyboard(it.main, "", true, wifiPassCb);
}

// ---------- SETTINGS ----------
// Settings page.
static void settingsDraw() {
    gfxText(10, 28, "Screen", 1, T.dim);
    gfxSlider(76, 26, 150, gSet.tftBright, 10, 255, T.accent);
    gfxText(10, 60, "LEDs", 1, T.dim);
    gfxSlider(76, 58, 150, gSet.ledBright, 5, 255, T.accent);
    gfxBtn(8, 92, 72, 32, gSet.dark ? "Dark" : "Light", T.card2, T.fg);
    gfxBtn(84, 92, 72, 32, gSet.volume ? "Sound" : "Muted", T.card2, T.fg);
    gfxBtn(160, 92, 72, 32, gSet.rotation ? "Flipped" : "Normal", T.card2, T.fg);
    const char* r[6] = {"Wi-Fi networks", "Link another account", "Board & sensor test", "Lock / switch account", "Calibrate touch", "About Gchess"};
    for (int i = 0; i < 6; i++) gfxBtn(8, 130 + i * 31, 224, 28, r[i], T.card, T.fg);
}

// Answer of "Lock board".
static void lockSwitchCb(int r) {
    if (r == 2) {
        netCmd(NC_LOGOUT);
        gActiveAcc = -1;
        gSp = 0;
        gStack[0] = PG_INTRO;
        storageSaveAll();
        uiGo(PG_LOCK, false);
    }
}

// Answer of "Calibrate touch".
static void calibCb(int r) {
    if (r == 2) {
        gfxCalibrate();
        uiRedraw();
        uiToast("Touch calibrated & saved");
    }
}

static bool setDrag = false;
static int setRow = 0;
// Settings page: sliders, switches and rows.
static void settingsTouch(TouchPhase ph, int x, int y) {
    if (ph == TP_DOWN) {
        setDrag = false;
        if (y > 22 && y < 82) {
            setDrag = true;
            setRow = y < 52 ? 0 : 1;
        }
    }
    if (setDrag && (ph == TP_DOWN || ph == TP_MOVE)) {
        float f = constrain((x - 76) / 150.0f, 0.0f, 1.0f);
        if (setRow == 0) gfxSetBrightness(10 + f * 245);
        else ledSetBrightness(5 + f * 250);
        gfxSliderBox(76, 26 + setRow * 32, 150, setRow ? gSet.ledBright : gSet.tftBright, setRow ? 5 : 10, 255, T.bg);
        return;
    }
    if (ph != TP_UP) return;
    if (setDrag) {
        setDrag = false;
        storageSaveSettings();
        return;
    }
    if (y > 92 && y < 124) {
        uiTap();
        if (x < 80) {
            gSet.dark = !gSet.dark;
            gfxApplyTheme();
            tft.fillScreen(T.bg);
            uiRedraw();
            return;
        } else if (x < 158) gSet.volume = (gSet.volume + 1) & 3;
        else {
            gfxSetRotation(gSet.rotation ? 0 : 2);
            tft.fillScreen(T.bg);
            uiRedraw();
            return;
        }
        storageSaveSettings();
        settingsDraw();
        return;
    }
    int i = (y - 130) / 31;
    if (y < 130 || i > 5) return;
    uiTap();
    analyticsButton(PG_SETTINGS, "row");
    const PageId p[6] = {PG_WIFI, PG_LINK, PG_BOARDTEST, PG_LOCK, PG_SETTINGS, PG_ABOUT};
    if (i == 3) uiDialog("Lock board", "Log out and return to the lock screen?", "No", "Lock", lockSwitchCb);
    else if (i == 4) uiDialog("Calibrate touch", "Touch 4 corner arrows.\nThe result is saved.", "Cancel", "Start", calibCb);
    else uiGo(p[i]);
}

// ---------- BOARD TEST ----------
static uint64_t btLast = ~0ULL;
// Board test page.
static void btDraw() {
    btLast = ~0ULL;
    gfxText(120, 26, "Place a magnet on each square", 1, T.dim, TD_TC);
    gfxBtn(8, 276, 108, 34, "Rainbow LEDs", T.card2, T.fg);
    gfxBtn(124, 276, 108, 34, "LEDs off", T.card2, T.fg);
}

// Board test page: show which sensors see a magnet.
static void btTick() {
    uint64_t o = boardOccupancy();
    if (o == btLast) return;
    uint64_t d = o ^ btLast;
    btLast = o;
    for (int sq = 0; sq < 64; sq++) if (d >> sq & 1) {
        int f = sq & 7, r = sq >> 3, x = 8 + f * 28, y = 44 + (7 - r) * 28;
        tft.fillRect(x, y, 27, 27, (o >> sq & 1) ? T.ok : (((f + r) & 1) ? T.card2 : T.card));
        char n[3];
        cr::sqName(sq, n);
        gfxText(x + 13, y + 14, n, 1, T.dim, TD_MC);
    }
}

// Board test page: tap a square to light its LED.
static void btTouch(TouchPhase ph, int x, int y) {
    if (ph != TP_UP) return;
    if (y > 276) {
        uiTap();
        if (x < 120) ledsRainbowTest();
        else ledsClear();
        return;
    }
    if (y >= 44 && y < 268 && x >= 8) {
        int f = (x - 8) / 28, r = 7 - (y - 44) / 28;
        if (f < 8 && r >= 0) {
            ledsClear();
            ledSq(r * 8 + f, CRGB(255, 255, 255));
        }
    }
}

// ---------- ABOUT ----------
// About page.
static void aboutDraw() {
    logoFull(120, 30);
    char s[64];
    snprintf(s, 64, "v%s   free heap %uK", GC_VERSION, ESP.getFreeHeap() / 1024);
    gfxText(120, 200, s, 1, T.dim, TD_TC);
    snprintf(s, 64, "Sessions %u   uptime %us", gAn.sessions, millis() / 1000);
    gfxText(120, 220, s, 1, T.dim, TD_TC);
    gfxText(120, 250, "Made with love for real chess.", 1, T.accent, TD_TC);
}

// ---------- global events ----------
// Answer of the challenge notification (1 decline, 2 accept).
static void challengeCb(int r) {
    if (r == 2) guardNewGame(doAccept, true);
    else netCmd(NC_DECLINE, chId);
}

// r: 1 decline, 2 accept
static bool globalEvt(const UiEvt& e) {
    DBG(UI, "global event %s handled by the UI task", UE_NAMES[e.t]);
    switch (e.t) {
        case UE_GAME_START: {
            int slot = e.a >> 1;
            bool white = e.a & 1;
            bool mine = gExpectGame && millis() < gExpectGame;
            bool claim = (mine && !gOpenedFromList) || !physLive();
            strlcpy(gOpp, e.s1, 32);
            gOppRating = e.b;
            analyticsEvent("game_start");
            {
                GameScope sc(slot);
                gameReset(GM_ONLINE, nullptr, white);
                G.touchAllowed = true;
                G.physical = false;
            }
            if (claim) gameClaimBoard(slot);
            if (mine) {
                gExpectGame = 0;
                gOpenedFromList = false;
                viewGame(slot);
                gModal = MD_NONE;
                gSp = 0;
                gStack[gSp++] = PG_HOME;
                uiGo(PG_GAME, false);
                if (!white) uiToast("You are Black: turn the board or use Rotate mode", 3500);
            } else {
                char m[48];
                snprintf(m, 48, "Game vs %s is running (Home > Ongoing)", e.s1);
                uiToast(m, 3000);
                soundPlay(SND_NOTIFY);
            }
            // never jump pages on our own
            return true;
        }
        case UE_GAME_UPDATE: onGameUpdate(e.b, e.a);
        return true;
        case UE_GAME_END: onGameEnd(e.a, e.s1, e.s2);
        return true;
        case UE_CHALLENGE: {
            strlcpy(chId, e.s1, 16);
            char d[160], t[48];
            snprintf(d, 160, "%s\n\nAccept to start the game now, or decline.", e.s2);
            snprintf(t, 48, "Challenge: %s", e.s2);
            uiNotify(t, d, 2, challengeCb, "Decline", "Accept", SND_NOTIFY);
            return true;
        }
        case UE_LOGIN: if (e.a < 0) {
            uiToast("Token expired - link again");
            soundPlay(SND_WRONG);
        } else if (e.a == 1) {
            gMyRating = e.b;
            if (gCur == PG_HOME) uiRedraw();
        }
        return true;
        case UE_PUZZLE: {
            xSemaphoreTake(gNetMux, portMAX_DELAY);
            NetPuzzle pz = gPuz;
            xSemaphoreGive(gNetMux);
            {
                GameScope sc(2);
                gamePuzzleStart(pz.id, pz.rating, pz.fen, pz.sol);
                G.touchAllowed = true;
                G.physical = false;
            }
            gameClaimBoard(2);
            viewGame(2);
            analyticsEvent("puzzle_start");
            if (gCur == PG_GAME) gameDraw();
            else uiGo(PG_GAME);
            return true;
        }
        case UE_EXPORT: {
            xSemaphoreTake(gNetMux, portMAX_DELAY);
            strlcpy(gMovesBuf, gNetX.moves, sizeof(gMovesBuf));
            xSemaphoreGive(gNetMux);
            bool ok;
            {
                GameScope sc(2);
                G.physical = false;
                ok = gameLoadSan(gMovesBuf);
                if (ok) {
                    gameRevGoto(0);
                    G.whiteBottom = true;
                    G.touchAllowed = true;
                }
            }
            if (ok) {
                viewGame(2);
                uiGo(PG_GAME);
            } else uiToast("Could not read game");
            return true;
        }
        default: return false;
    }
}

// Register all pages: name, enter, draw, touch, tick, leave, event, flags.
inline void pagesBegin() {
    gGlobalEvt = globalEvt;
    gPages[PG_INTRO] = {"Gchess", introEnter, introDraw, nullptr, introTick, nullptr, nullptr, PF_NOBACK | PF_NOPUSH};
    gPages[PG_LOCK] = {"Lock", nullptr, lockDraw, lockTouch, nullptr, nullptr, nullptr, PF_TOOLBAR | PF_NOBACK | PF_NOPUSH};
    gPages[PG_LINK] = {"Link account", linkEnter, linkDraw, linkTouch, nullptr, linkLeave, linkEvent, PF_TOOLBAR};
    gPages[PG_HOME] = {"Home", nullptr, homeDraw, homeTouch, nullptr, nullptr, nullptr, PF_TOOLBAR | PF_NOBACK};
    gPages[PG_PLAY] = {"Play", nullptr, playDraw, playTouch, nullptr, nullptr, nullptr, PF_TOOLBAR};
    gPages[PG_AI] = {"Computer", nullptr, aiDraw, aiTouch, nullptr, nullptr, nullptr, PF_TOOLBAR};
    gPages[PG_FRIEND_SETUP] = {"Challenge", nullptr, fsDraw, fsTouch, nullptr, nullptr, nullptr, PF_TOOLBAR};
    gPages[PG_WAIT] = {"Waiting", nullptr, waitDraw, waitTouch, waitTick, nullptr, nullptr, PF_TOOLBAR | PF_NOBACK | PF_NOPUSH};
    gPages[PG_GAME] = {"Game", gameEnter, gameDraw, gameTouch, gameTick, gameLeave, nullptr, PF_TOOLBAR | PF_NOPUSH, gamePartial};
    gPages[PG_ONGOING] = {"Ongoing games", ogEnter, ogDraw, ogTouch, nullptr, nullptr, ogEvent, PF_TOOLBAR};
    gLeaveGuard = leaveGuard;
    gPages[PG_PUZZLE] = {"Puzzles", nullptr, puzzleDraw, puzzleTouch, nullptr, nullptr, nullptr, PF_TOOLBAR};
    gPages[PG_HISTORY] = {"History", historyEnter, listDraw, historyTouch, nullptr, nullptr, listEvent, PF_TOOLBAR};
    gPages[PG_FRIENDS] = {"Friends", friendsEnter, friendsDraw, friendsTouch, nullptr, nullptr, listEvent, PF_TOOLBAR};
    gPages[PG_SETTINGS] = {"Settings", nullptr, settingsDraw, settingsTouch, nullptr, nullptr, nullptr, PF_TOOLBAR};
    gPages[PG_WIFI] = {"Wi-Fi", wifiEnter, wifiDraw, wifiTouch, nullptr, nullptr, listEvent, PF_TOOLBAR};
    gPages[PG_BOARDTEST] = {"Board test", nullptr, btDraw, btTouch, btTick, [](){ ledsClear();}, nullptr, PF_TOOLBAR};
    gPages[PG_ABOUT] = {"About", nullptr, aboutDraw, nullptr, nullptr, nullptr, nullptr, PF_TOOLBAR};
}
