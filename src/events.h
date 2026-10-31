// events.h - messages that cross between core 0 (UI/hardware) and core 1 (network)
#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include "debug.h"

enum NetCmdType : uint8_t {
    NC_NONE,
    NC_LOGIN,
    NC_LOGOUT,
    NC_AI,
    NC_CHALLENGE,
    NC_SEEK,
    NC_CANCEL_SEEK,
    NC_ACCEPT,
    NC_DECLINE,
    NC_MOVE,
    NC_RESIGN,
    NC_ABORT,
    NC_DRAW,
    NC_TAKEBACK,
    NC_PUZ_DAILY,
    NC_PUZ_BATCH,
    NC_SEARCH,
    NC_FRIENDS,
    NC_HISTORY,
    NC_GAME_EXPORT,
    NC_ONGOING,
    NC_OPEN_GAME,
    NC_PARK_GAME,
    NC_WIFI_SCAN,
    NC_WIFI_CONNECT,
    NC_WIFI_ENABLE,
    NC_LINK_START,
    NC_LINK_STOP
};
struct NetCmd {
    NetCmdType t;
    char s1[40];
    char s2[80];
    int16_t a, b, c;
};

enum UiEvtType : uint8_t {
    UE_NONE,
    UE_NET,
    UE_LOGIN,
    UE_GAME_START,
    UE_GAME_UPDATE,
    UE_GAME_END,
    UE_CHALLENGE,
    UE_LIST,
    UE_PUZZLE,
    UE_TOAST,
    UE_LINK_URL,
    UE_LINK_DONE,
    UE_EXPORT
};
struct UiEvt {
    UiEvtType t;
    char s1[48];
    char s2[128];
    int16_t a, b;
};

enum ListKind : uint8_t {
    LK_FRIENDS,
    LK_SEARCH,
    LK_HISTORY,
    LK_WIFI,
    LK_ONGOING,
    LK_COUNT
};
struct ListItem {
    char main[30];
    char sub[26];
    char id[14];
    int16_t val;
};
struct ListData {
    ListItem it[24];
    volatile uint8_t n;
};

// Current online game as reported by Lichess (written by net task, read by UI task under gNetMux)
struct NetGame {
    char id[12];
    char white[26], black[26];
    int16_t wRating, bRating;
    bool iAmWhite;
    char initialFen[96];
    char moves[2400];
    uint32_t wtime, btime;
    uint32_t stamp;  // clocks in ms at millis()==stamp
    char status[16];
    char winner[8];
    bool wdraw, bdraw, wtake, btake, active;
};

// Human readable names (same order as the enums above) - used only for the serial log.
static const char* const NC_NAMES[] = {"NONE", "LOGIN", "LOGOUT", "AI", "CHALLENGE", "SEEK", "CANCEL_SEEK", "ACCEPT", "DECLINE", "MOVE", "RESIGN", "ABORT", "DRAW", "TAKEBACK", "PUZ_DAILY", "PUZ_BATCH", "SEARCH", "FRIENDS", "HISTORY", "GAME_EXPORT", "ONGOING", "OPEN_GAME", "PARK_GAME", "WIFI_SCAN", "WIFI_CONNECT", "WIFI_ENABLE", "LINK_START", "LINK_STOP"};
static const char* const UE_NAMES[] = {"NONE", "NET", "LOGIN", "GAME_START", "GAME_UPDATE", "GAME_END", "CHALLENGE", "LIST", "PUZZLE", "TOAST", "LINK_URL", "LINK_DONE", "EXPORT"};

static QueueHandle_t qNet, qUi;
static SemaphoreHandle_t gNetMux;
static NetGame gNetG[2];  // up to two simultaneous online games
static NetGame gNetX;  // game loaded for review (history)
static ListData gList[LK_COUNT];

// UI -> network: put one command into the queue (never waits).
inline void netCmd(NetCmdType t, const char* s1 = "", const char* s2 = "", int a = 0, int b = 0, int c = 0) {
    NetCmd m;
    memset(&m, 0, sizeof(m));
    m.t = t;
    strlcpy(m.s1, s1, sizeof(m.s1));
    strlcpy(m.s2, s2, sizeof(m.s2));
    m.a = a;
    m.b = b;
    m.c = c;
    // Log: UI -> network. The token inside LOGIN is hidden.
    DBG(EVT, "UI -> NET  %s s1='%s' s2='%s' a=%d b=%d c=%d", NC_NAMES[t], s1, t == NC_LOGIN ? dbgSecret(s2) : s2, a, b, c);
    if (xQueueSend(qNet, &m, 0) != pdTRUE) DBG(EVT, "!! network queue FULL, command %s dropped", NC_NAMES[t]);
}

// Network -> UI: put one event into the queue (never waits).
inline void uiPost(UiEvtType t, const char* s1 = "", const char* s2 = "", int a = 0, int b = 0) {
    UiEvt e;
    memset(&e, 0, sizeof(e));
    e.t = t;
    strlcpy(e.s1, s1, sizeof(e.s1));
    strlcpy(e.s2, s2, sizeof(e.s2));
    e.a = a;
    e.b = b;
    // Log: network -> UI (UE_NET status is skipped, it would repeat every second).
    if (t != UE_NET) DBG(EVT, "NET -> UI  %s s1='%s' s2='%s' a=%d b=%d", UE_NAMES[t], s1, s2, a, b);
    if (xQueueSend(qUi, &e, 0) != pdTRUE) DBG(EVT, "!! UI queue FULL, event %s dropped", UE_NAMES[t]);
}

struct NetPuzzle {
    char id[10];
    int16_t rating;
    char fen[96];
    char sol[96];
};
static NetPuzzle gPuz;
static char gLinkUrl[160];
static char gLinkCode[8];
static char gMyUser[24];
static int16_t gMyRating = 0;
