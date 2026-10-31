// storage.h - permanent memory (NVS). Settings, accounts, Wi-Fi list.
// External API: storageBegin() storageSaveSettings() storageSaveAccounts() storageSaveWifi() storageSaveAll()
//               accAdd(user,token) accRemove(i) wifiSave(ssid,pass) wifiForget(i)
#pragma once
#include <Preferences.h>
#include "debug.h"
#include <LittleFS.h>

struct Settings {
    uint8_t tftBright = 200;  // 10..255
    uint8_t ledBright = 80;  // 5..255
    uint8_t rotation = 0;  // 0 or 2 only
    uint8_t volume = 2;  // 0..3
    uint8_t dark = 1;
    uint8_t flipHw = 0;  // board physically turned 180 deg (playing black)
    uint8_t wifiOn = 1;
    uint8_t lastAcc = 0;
};
struct Account {
    char user[24];
    char token[72];
    uint16_t pass;
    uint8_t used;
};
struct WifiEntry {
    char ssid[33];
    char pass[64];
    uint8_t used;
};

#define MAX_ACC  4
#define MAX_WIFI 8
static Settings gSet;
static Account gAcc[MAX_ACC];
static WifiEntry gWifi[MAX_WIFI];
static Preferences gPrefs;
// Number of saved accounts.
inline int accCountFwd() {
    int n = 0;
    for (auto& a : gAcc) n += a.used;
    return n;
}

// Write the settings to flash.
inline void storageSaveSettings() {
    gPrefs.putBytes("set", &gSet, sizeof(gSet));
    DBG(SYS, "settings saved (screen=%d leds=%d rot=%d vol=%d dark=%d)", gSet.tftBright, gSet.ledBright, gSet.rotation, gSet.volume, gSet.dark);
}

// Write the accounts to flash.
inline void storageSaveAccounts() {
    gPrefs.putBytes("acc", gAcc, sizeof(gAcc));
    DBG(SYS, "accounts saved (%d)", accCountFwd());
}

// Write the Wi-Fi list to flash.
inline void storageSaveWifi() {
    gPrefs.putBytes("wifi", gWifi, sizeof(gWifi));
    DBG(SYS, "wifi list saved");
}

// Write everything to flash.
inline void storageSaveAll() {
    storageSaveSettings();
    storageSaveAccounts();
    storageSaveWifi();
}

// Open flash storage and load settings, accounts, Wi-Fi list.
inline void storageBegin() {
    gPrefs.begin("gchess", false);
    memset(gAcc, 0, sizeof(gAcc));
    memset(gWifi, 0, sizeof(gWifi));
    if (gPrefs.isKey("set") && gPrefs.getBytesLength("set") == sizeof(gSet)) gPrefs.getBytes("set", &gSet, sizeof(gSet));
    if (gPrefs.isKey("acc") && gPrefs.getBytesLength("acc") == sizeof(gAcc)) gPrefs.getBytes("acc", gAcc, sizeof(gAcc));
    if (gPrefs.isKey("wifi") && gPrefs.getBytesLength("wifi") == sizeof(gWifi)) gPrefs.getBytes("wifi", gWifi, sizeof(gWifi));
    if (gSet.rotation != 0 && gSet.rotation != 2) gSet.rotation = 0;
    LittleFS.begin(true);
    DBG(SYS, "storage loaded: %d account(s), wifi saved: %d%s", accCountFwd(), (int)(gWifi[0].used + gWifi[1].used + gWifi[2].used + gWifi[3].used + gWifi[4].used + gWifi[5].used + gWifi[6].used + gWifi[7].used), gPrefs.isKey("set") ? "" : " (first boot, defaults used)");
}

// Number of saved accounts.
inline int accCount() {
    int n = 0;
    for (auto& a : gAcc) n += a.used;
    return n;
}

// Add an account or replace the token of an existing one. Returns the slot or -1.
inline int accAdd(const char* user, const char* token) {
    // replaces token if user exists
    int slot = -1;
    for (int i = 0; i < MAX_ACC; i++) if (gAcc[i].used && !strcasecmp(gAcc[i].user, user)) slot = i;
    if (slot < 0) for (int i = 0; i < MAX_ACC; i++) if (!gAcc[i].used) {
        slot = i;
        gAcc[i].pass = 0;
        break;
    }
    if (slot < 0) return -1;
    gAcc[slot].used = 1;
    strlcpy(gAcc[slot].user, user, sizeof(gAcc[slot].user));
    strlcpy(gAcc[slot].token, token, sizeof(gAcc[slot].token));
    storageSaveAccounts();
    return slot;
}

// Delete an account.
inline void accRemove(int i) {
    if (i >= 0 && i < MAX_ACC) {
        memset(&gAcc[i], 0, sizeof(Account));
        storageSaveAccounts();
    }
}

// Save a Wi-Fi network (or update its password).
inline bool wifiSave(const char* ssid, const char* pass) {
    int slot = -1;
    for (int i = 0; i < MAX_WIFI; i++) if (gWifi[i].used && !strcmp(gWifi[i].ssid, ssid)) slot = i;
    if (slot < 0) for (int i = 0; i < MAX_WIFI; i++) if (!gWifi[i].used) {
        slot = i;
        break;
    }
    if (slot < 0) return false;
    gWifi[slot].used = 1;
    strlcpy(gWifi[slot].ssid, ssid, 33);
    strlcpy(gWifi[slot].pass, pass, 64);
    storageSaveWifi();
    return true;
}

// Index of a saved network or -1.
inline int wifiFind(const char* ssid) {
    for (int i = 0; i < MAX_WIFI; i++) if (gWifi[i].used && !strcmp(gWifi[i].ssid, ssid)) return i;
    return -1;
}

// Delete a saved network.
inline void wifiForget(int i) {
    if (i >= 0 && i < MAX_WIFI) {
        memset(&gWifi[i], 0, sizeof(WifiEntry));
        storageSaveWifi();
    }
}
