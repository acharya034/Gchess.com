// analytics.h - local usage tracking (pages, buttons, time spent). Server upload is a stub for later.
// External API:
//   analyticsBegin()
//   analyticsPage(pageId)                 -> call when a page becomes visible (ui.h does it automatically)
//   analyticsButton(pageId, "name")       -> call on every button press (ui.h does it automatically)
//   analyticsEvent("name", value)         -> anything else (game_started, puzzle_solved, wrong_move ...)
//   analyticsFlush()                      -> save counters to permanent memory (called every 5 min + power off)
//   analyticsUpload()                     -> TODO: send counters to your server, then analyticsClear()
#pragma once
#include "storage.h"

struct AnPage {
    uint32_t visits;
    uint32_t ms;
};
struct AnBtn {
    uint8_t page;
    char name[15];
    uint16_t count;
};
struct AnEvt {
    char name[15];
    uint32_t count;
    int32_t last;
};
#define AN_PAGES 24
#define AN_BTNS  48
#define AN_EVTS  24
struct AnData {
    AnPage pg[AN_PAGES];
    AnBtn bt[AN_BTNS];
    AnEvt ev[AN_EVTS];
    uint32_t sessions;
    uint32_t uptimeS;
};

static AnData gAn;
static int8_t anCur = -1;
static uint32_t anSince = 0, anLastFlush = 0;

// Load the saved counters from flash and count this start as a new session.
inline void analyticsBegin() {
    memset(&gAn, 0, sizeof(gAn));
    if (gPrefs.isKey("an") && gPrefs.getBytesLength("an") == sizeof(gAn)) gPrefs.getBytes("an", &gAn, sizeof(gAn));
    gAn.sessions++;
    anLastFlush = millis();
}

// A page became visible: close the timer of the previous page and start one for this page.
inline void analyticsPage(uint8_t page) {
    DBG(UI, "analytics: page %d", page);
    uint32_t now = millis();
    if (anCur >= 0 && anCur < AN_PAGES) gAn.pg[anCur].ms += now - anSince;
    anCur = page < AN_PAGES ? page : -1;
    anSince = now;
    if (anCur >= 0) gAn.pg[anCur].visits++;
}

// Count one press of a button (page + button name).
inline void analyticsButton(uint8_t page, const char* name) {
    DBG(UI, "analytics: button '%s' on page %d", name, page);
    int free_ = -1;
    for (int i = 0; i < AN_BTNS; i++) {
        if (gAn.bt[i].count && gAn.bt[i].page == page && !strncmp(gAn.bt[i].name, name, 14)) {
            gAn.bt[i].count++;
            return;
        }
        if (!gAn.bt[i].count && free_ < 0) free_ = i;
    }
    if (free_ >= 0) {
        gAn.bt[free_].page = page;
        strlcpy(gAn.bt[free_].name, name, 15);
        gAn.bt[free_].count = 1;
    }
}

// Count a named event (for example "puzzle_solved") and remember its last value.
inline void analyticsEvent(const char* name, int32_t value = 0) {
    DBG(UI, "analytics: event '%s' value=%d", name, (int)value);
    int free_ = -1;
    for (int i = 0; i < AN_EVTS; i++) {
        if (gAn.ev[i].count && !strncmp(gAn.ev[i].name, name, 14)) {
            gAn.ev[i].count++;
            gAn.ev[i].last = value;
            return;
        }
        if (!gAn.ev[i].count && free_ < 0) free_ = i;
    }
    if (free_ >= 0) {
        strlcpy(gAn.ev[free_].name, name, 15);
        gAn.ev[free_].count = 1;
        gAn.ev[free_].last = value;
    }
}

// Save all counters to flash (done every 5 minutes and before power off).
inline void analyticsFlush() {
    DBG(SYS, "analytics saved to flash");
    uint32_t now = millis();
    if (anCur >= 0) {
        gAn.pg[anCur].ms += now - anSince;
        anSince = now;
    }
    gAn.uptimeS += (now - anLastFlush) / 1000;
    anLastFlush = now;
    gPrefs.putBytes("an", &gAn, sizeof(gAn));
}

// Forget all counters (call after a successful upload).
inline void analyticsClear() {
    memset(&gAn, 0, sizeof(gAn));
    anCur = -1;
}

// Placeholder: later this will send the counters to your server.
inline bool analyticsUpload() {
    /* TODO(server): POST gAn as JSON, on success analyticsClear(); */ return false;
}

// Call often; saves the counters when 5 minutes have passed.
inline void analyticsTick() {
    if (millis() - anLastFlush > 300000UL) analyticsFlush();
}
