// sound.h - passive buzzer, non-blocking melodies.
// External API: soundBegin() soundPlay(SND_xxx) soundTick() soundSetVolume(0..3) soundBeep(freq, ms)
#pragma once
#include "gchess_config.h"
#include "storage.h"
#include "debug.h"

enum Snd : uint8_t {
    SND_NONE,
    SND_POWER_ON,
    SND_POWER_OFF,
    SND_MOVE_OPP,
    SND_MOVE_OK,
    SND_WRONG,
    SND_TAP,
    SND_SUCCESS,
    SND_FAIL,
    SND_NOTIFY,
    SND_WARN_HOLD,
    SND_COUNT
};
struct Note {
    uint16_t f;
    uint16_t ms;
};
static const Note S_ON[] = {{523,90},{659,90},{784,90},{1047,160},{0,0}};
static const Note S_OFF[] = {{1047,90},{784,90},{659,90},{392,200},{0,0}};
static const Note S_OPP[] = {{880,70},{0,30},{1175,110},{0,0}};
static const Note S_OK[] = {{1568,35},{0,0}};
static const Note S_WRONG[]= {{220,160},{0,60},{196,260},{0,0}};
static const Note S_TAP[] = {{2200,12},{0,0}};
static const Note S_WIN[] = {{523,90},{659,90},{784,90},{1047,90},{784,60},{1047,220},{0,0}};
static const Note S_LOSE[] = {{392,160},{330,160},{262,320},{0,0}};
static const Note S_NOTE[] = {{988,80},{0,40},{988,80},{0,0}};
static const Note S_HOLD[] = {{1760,120},{0,0}};
static const Note* const SONGS[SND_COUNT] = {nullptr, S_ON, S_OFF, S_OPP, S_OK, S_WRONG, S_TAP, S_WIN, S_LOSE, S_NOTE, S_HOLD};

static const Note* sndCur = nullptr;
static uint8_t sndIdx = 0;
static uint32_t sndNext = 0;

// Play one tone (0 = silence) at the chosen volume.
inline void buzzTone(uint16_t f) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcWriteTone(PIN_BUZZER, f);
#else
    ledcWriteTone(0, f);
#endif
    if (f) {
        static const uint8_t duty[4] = {0, 12, 60, 128};  // volume steps (50% duty = loudest)
#if ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcWrite(PIN_BUZZER, duty[gSet.volume & 3]);
#else
        ledcWrite(0, duty[gSet.volume & 3]);
#endif
    }
}

// Start the buzzer.
inline void soundBegin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcAttach(PIN_BUZZER, 2000, 8);
#else
    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_BUZZER, 0);
#endif
    buzzTone(0);
}

// Start a melody (does not wait).
inline void soundPlay(Snd s) {
    DBG(UI, "sound %d (volume %d)", (int)s, (int)gSet.volume);
    if (!gSet.volume || s == SND_NONE) return;
    sndCur = SONGS[s];
    sndIdx = 0;
    sndNext = 0;
}

// One simple beep.
inline void soundBeep(uint16_t f, uint16_t ms) {
    static Note one[2];
    one[0] = {f, ms};
    one[1] = {0, 0};
    sndCur = one;
    sndIdx = 0;
    sndNext = 0;
}

// Volume 0..3.
inline void soundSetVolume(uint8_t v) {
    gSet.volume = v & 3;
}

// Play the next note when its time has come.
inline void soundTick() {
    if (!sndCur) return;
    uint32_t now = millis();
    if (now < sndNext) return;
    const Note& n = sndCur[sndIdx];
    if (n.ms == 0) {
        buzzTone(0);
        sndCur = nullptr;
        return;
    }
    buzzTone(n.f);
    sndNext = now + n.ms;
    sndIdx++;
}
