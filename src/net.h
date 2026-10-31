// net.h - runs on CORE 1. Wi-Fi manager, Lichess client (streams + REST), puzzle store, link receiver.
// Nothing here touches the display. It talks to the UI only through qNet (commands) / qUi (events) / gNet, gList, gPuz.
// External API: netBegin()  (creates the task)  and the netCmd(NC_xxx, ...) helper from events.h
#pragma once
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "events.h"
#include "storage.h"
#include "chess_rules.h"
#include "debug.h"

static char gToken[72];
static bool gLoggedIn = false, gNeedAccount = false, gWifiWanted = true, gLinkOn = false;
static int8_t gSeekSlot = -1;
static uint32_t gExpectUntil = 0;
static bool gOngoingDirty = false;
static bool gPollNow = false;           // poll for the new game immediately
static bool gChallengeScopeOk = true;   // false if the token may not read challenges
static char gScopes[96] = "";            // permissions of the token, as reported by Lichess
static char gKnown[24][14];             // game ids that existed before the user started a new game
static uint8_t gKnownN = 0;
static bool gOngoingFetched = false;
// ---------------- minimal HTTPS client with streaming + chunked decoding + keep-alive ----------------
#define LINE_CAP 3072
struct HttpStream {
    WiFiClientSecure cli;
    bool chunked = false, ended = true;
    bool srvClose = false;      // server sent "Connection: close"
    bool keepAlive = false;     // this response was fully framed, socket can be reused
    int status = 0;
    uint8_t cs = 0;
    uint8_t tl = 0;             // length of the current trailer line (chunked end)
    uint32_t chunkLeft = 0;
    int32_t bodyLeft = -1;      // remaining Content-Length bytes, -1 = unknown (read until EOF)
    char scopes[96] = "";       // value of the x-oauth-scopes header of the last answer
    char* line = nullptr;
    uint16_t ll = 0;            // allocated on first use (saves static RAM)

    // End of response, but keep the TLS socket open for the next request.
    void finish() {
        ended = true;
        ll = 0;
    }
    // Really close the socket.
    void close() {
        cli.stop();
        ended = true;
        keepAlive = false;
        ll = 0;
    }
    bool alive() {
        return !ended && (cli.connected() || cli.available());
    }

    // reuse = true: HTTP/1.1 keep-alive, the existing socket is reused if it is still connected.
    // reuse = false: HTTP/1.0 + Connection: close, always a fresh connection (game streams).
    bool open(const char* method, const char* path, const char* body = nullptr, const char* accept = "application/json", bool reuse = false) {
        finish();
        keepAlive = false;

        if (reuse && cli.connected()) {
            while (cli.available()) cli.read();   // drop stale leftovers of the previous response
            DBG(HTTP, ">> reusing open TLS connection");
        } else {
            cli.stop();
            cli.setInsecure();  // NOTE: add the Lichess root CA with cli.setCACert() for production
            if (!cli.connect(LICHESS_HOST, 443)) {
                DBG(HTTP, "!! TLS connect to %s failed (wifi=%d heap=%u)", LICHESS_HOST, (int)WiFi.status(), (unsigned)ESP.getFreeHeap());
                return false;
            }
        }
        DBG(HTTP, ">> %s https://%s%s", method, LICHESS_HOST, path);

        String req = String(method) + " " + path + (reuse ? " HTTP/1.1" : " HTTP/1.0") +
                     "\r\nHost: " LICHESS_HOST "\r\nUser-Agent: Gchess/" GC_VERSION "\r\nAccept: " + accept + "\r\n";
        if (gToken[0]) req += "Authorization: Bearer " + String(gToken) + "\r\n";
        if (body) req += "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: " + String(strlen(body)) + "\r\n";
        else if (!strcmp(method, "POST")) req += "Content-Length: 0\r\n";
        req += reuse ? "Connection: keep-alive\r\n\r\n" : "Connection: close\r\n\r\n";
        if (body) req += body;
        DBG(HTTP, ">> headers: Accept: %s | Authorization: Bearer %s | Content-Length: %d", accept, gToken[0] ? dbgSecret(gToken) : "(none)", body ? (int)strlen(body) : 0);
        if (body) DBG(HTTP, ">> body: %s", body);
        cli.print(req);

        status = 0;
        chunked = false;
        srvClose = false;
        int32_t contentLen = -1;
        uint32_t t0 = millis();
        while (millis() - t0 < 10000) {
            if (!cli.connected() && !cli.available()) return false;   // dead socket -> caller reconnects
            if (!cli.available()) {
                delay(5);
                continue;
            }
            String h = cli.readStringUntil('\n');
            h.trim();
            if (!h.length()) break;
            DBG(HTTP, "<< %s", h.c_str());
            String hl = h;
            hl.toLowerCase();
            if (hl.startsWith("x-oauth-scopes:")) strlcpy(scopes, h.c_str() + 15, sizeof(scopes));
            if (hl.startsWith("http/")) status = h.substring(9, 12).toInt();
            else if (hl.startsWith("transfer-encoding") && hl.indexOf("chunked") > 0) chunked = true;
            else if (hl.startsWith("content-length:")) contentLen = hl.substring(15).toInt();
            else if (hl.startsWith("connection:") && hl.indexOf("close") > 0) srvClose = true;
        }
        cs = 0;
        tl = 0;
        chunkLeft = 0;
        ll = 0;
        ended = false;
        bodyLeft = chunked ? -1 : contentLen;
        if (status == 204 || status == 304 || (!chunked && contentLen == 0)) ended = true;   // no body at all
        keepAlive = reuse && !srvClose && (chunked || contentLen >= 0 || ended);
        return status > 0;
    }

    int getByte() {
        if (ended) return -1;
        if (!chunked && bodyLeft == 0) {          // Content-Length reached: response complete
            ended = true;
            return -1;
        }
        for (;;) {
            if (!cli.available()) return -1;
            int c = cli.read();
            if (c < 0) return -1;
            if (!chunked) {
                if (bodyLeft > 0) bodyLeft--;
                return c;
            }
            switch (cs) {
                case 0:                           // chunk size line
                    if (c == '\r') {
                    } else if (c == '\n') {
                        if (chunkLeft == 0) {
                            cs = 3;               // last chunk: now consume the trailer
                            tl = 0;
                        } else {
                            cs = 1;
                        }
                    } else {
                        int d = isdigit(c) ? c - '0' : (c >= 'a' && c <= 'f') ? c - 'a' + 10 : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
                        if (d >= 0) chunkLeft = chunkLeft * 16 + d;
                    }
                    break;
                case 1:                           // chunk data
                    if (--chunkLeft == 0) cs = 2;
                    return c;
                case 2:                           // CRLF after chunk data
                    if (c == '\n') {
                        cs = 0;
                        chunkLeft = 0;
                    }
                    break;
                case 3:                           // trailer lines, an empty line ends the response
                    if (c == '\n') {
                        if (tl == 0) {
                            ended = true;
                            return -1;
                        }
                        tl = 0;
                    } else if (c != '\r') {
                        tl++;
                    }
                    break;
                default:
                    return -1;
            }
        }
    }

    // 1 = a full line copied to out, 0 = nothing yet, -1 = stream closed
    int pollLine(char* out, size_t cap) {
        if (!line) {
            line = (char*)malloc(LINE_CAP);
            if (!line) return -1;
        }
        for (int g = 0; g < 4096; g++) {
            int c = getByte();
            if (c < 0) break;
            if (c == '\n') {
                if (!ll) continue;
                line[ll] = 0;
                strlcpy(out, line, cap);
                ll = 0;
                DBG(HTTP, "<< stream line (%d bytes): %.200s", (int)strlen(out), out);
                return 1;
            }
            if (c != '\r' && ll < LINE_CAP - 1) line[ll++] = c;
        }
        if (ended || (!cli.connected() && !cli.available())) return -1;
        return 0;
    }
};
static HttpStream hGameS[2], hReq;   // live game stream(s) + one-shot requests (there is no events stream any more)
static uint32_t gStreamRetryAt[2] = {0, 0};   // when a dropped / paused game stream may reconnect
static uint8_t gPausedMask = 0;               // bit n = game stream n was paused to make room for a request

// (tlsMakeRoom and tlsResume stay exactly as they are)
// After the request: let paused streams reconnect (the new "gameFull" line brings everything up to date).
static void tlsResume() {
    for (int i = 0; i < 2; i++) {
        if (gPausedMask & (1 << i)) {
            gPausedMask &= ~(1 << i);
            gStreamRetryAt[i] = millis() + 150;
        }
    }
}

// Give TLS room to connect: pause the live game stream(s). They reconnect by themselves (tlsResume).
static void tlsMakeRoom(bool force) {
    if (!force && ESP.getFreeHeap() >= TLS_MIN_FREE_HEAP) return;
    for (int i = 0; i < 2; i++) {
        if (hGameS[i].alive() && i != gSeekSlot) {
            DBG(NET, "pausing game stream %d to free memory (free=%u)", i, (unsigned)ESP.getFreeHeap());
            hGameS[i].close();
            gPausedMask |= 1 << i;
        }
    }
}

// One complete HTTPS request over the kept-alive connection. Returns the status code; the body goes into "out".
static int httpReq(const char* method, const char* path, const char* body, String& out, const char* accept = "application/json", size_t cap = 24000) {
    out = "";

    // The TLS buffers are already allocated if the socket is still open, so only make room for a new handshake.
    if (!hReq.cli.connected()) tlsMakeRoom(false);

    bool ok = hReq.open(method, path, body, accept, true);
    if (!ok) {                                            // stale socket (server idle-closed it) or no RAM
        hReq.close();                                     // frees the old TLS session
        tlsMakeRoom(false);
        ok = hReq.open(method, path, body, accept, true);
    }
    if (!ok) {                                            // still out of memory? pause the game streams
        hReq.close();
        tlsMakeRoom(true);
        ok = hReq.open(method, path, body, accept, true);
    }
    if (!ok) {
        hReq.close();
        tlsResume();
        return -1;
    }

    uint32_t t0 = millis();
    while (millis() - t0 < 15000) {
        int c = hReq.getByte();
        if (c >= 0) {
            if (out.length() < cap) out += (char)c;
            continue;
        }
        if (hReq.ended || (!hReq.cli.connected() && !hReq.cli.available())) break;
        delay(2);
    }

    int st = hReq.status;
    if (hReq.ended && hReq.keepAlive) hReq.finish();      // fully read: keep the TLS socket for the next request
    else hReq.close();                                    // timeout / truncated / server said close
    DBG(HTTP, "<< status %d, body %d bytes: %.300s", st, (int)out.length(), out.c_str());
   tlsResume();
    return st;
}

// ---------------- Wi-Fi manager: strongest saved network, auto roam when weak ----------------
static uint32_t wifiLastScan = 0;
static int8_t lastState = -1, lastRssi = 0;
// Connect to one network and wait up to timeout ms.
static bool wifiConnectTo(const char* ssid, const char* pass, const uint8_t* bssid = nullptr, uint32_t timeout = 10000) {
    DBG(NET, "WiFi connecting to '%s' (password %d chars)%s", ssid, (int)strlen(pass), bssid ? " on a chosen access point" : "");
    if (bssid) WiFi.begin(ssid, pass, 0, bssid);
    else WiFi.begin(ssid, pass);
    uint32_t t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < timeout) delay(100);
    DBG(NET, "WiFi %s ip=%s rssi=%d", WiFi.status() == WL_CONNECTED ? "CONNECTED" : "FAILED", WiFi.localIP().toString().c_str(), (int)WiFi.RSSI());
    return WiFi.status() == WL_CONNECTED;
}

// Scan and connect to the strongest SAVED network. roaming = already connected, only switch if clearly better.
static void wifiPickBest(bool roaming) {
    int n = WiFi.scanNetworks(false, false, false, 300);
    DBG(NET, "WiFi scan found %d networks (roaming=%d)", n, (int)roaming);
    int best = -1, bestRssi = -127, bestSaved = -1;
    for (int i = 0; i < n; i++) {
        int s = wifiFind(WiFi.SSID(i).c_str());
        if (s < 0) continue;
        if (WiFi.RSSI(i) > bestRssi) {
            bestRssi = WiFi.RSSI(i);
            best = i;
            bestSaved = s;
        }
    }
    if (best >= 0) {
        bool better = true;
        if (roaming && WiFi.status() == WL_CONNECTED) better = bestRssi > WiFi.RSSI() + 8;  // hysteresis
        if (better) wifiConnectTo(gWifi[bestSaved].ssid, gWifi[bestSaved].pass, WiFi.BSSID(best));
    }
    WiFi.scanDelete();
}

// Wi-Fi manager: reconnect when lost, look for a better network when the signal is weak.
static void wifiTick() {
    if (!gWifiWanted) {
        if (WiFi.status() == WL_CONNECTED) WiFi.disconnect(true, false);
        return;
    }
    uint32_t now = millis();
    if (WiFi.status() == WL_CONNECTED) {
        if (WiFi.RSSI() < WIFI_WEAK_RSSI && now - wifiLastScan > WIFI_RESCAN_MS) {
            wifiLastScan = now;
            wifiPickBest(true);
        }
    } else if (now - wifiLastScan > WIFI_RETRY_MS) {
        wifiLastScan = now;
        wifiPickBest(false);
    }
}

// Tell the UI (once a second, only if changed) how good the connection is.
static void netReportStatus() {
    static uint32_t last = 0;
    if (millis() - last < 1000) return;
    last = millis();
    int8_t st = !gWifiWanted ? 0 : (WiFi.status() == WL_CONNECTED ? (gLoggedIn && !gNeedAccount ? 3 : 2) : 1);
    int8_t rssi = WiFi.status() == WL_CONNECTED ? (WiFi.RSSI() / 5) * 5 : 0;
    if (st != lastState || rssi != lastRssi) {
        DBG(NET, "status %d (0=off 1=no wifi 2=wifi 3=lichess) rssi=%d", (int)st, (int)rssi);
        lastState = st;
        lastRssi = rssi;
        uiPost(UE_NET, "", "", st, rssi);
    }
}

// ---------------- Lichess game / event handling ----------------
// Copy moves / clocks / status / draw offers from a Lichess "gameState" into the game slot.
static void gameApplyState(NetGame& g, JsonObjectConst s) {
    // caller holds gNetMux
    strlcpy(g.moves, s["moves"] | "", sizeof(g.moves));
    g.wtime = s["wtime"] | 0;
    g.btime = s["btime"] | 0;
    g.stamp = millis();
    strlcpy(g.status, s["status"] | "started", sizeof(g.status));
    strlcpy(g.winner, s["winner"] | "", sizeof(g.winner));
    g.wdraw = s["wdraw"] | false;
    g.bdraw = s["bdraw"] | false;
    g.wtake = s["wtakeback"] | false;
    g.btake = s["btakeback"] | false;
}

// Which live slot (0/1) holds this game id? -1 = none.
static int slotFor(const char* id) {
    for (int i = 0; i < 2; i++) if (gNetG[i].active && !strcmp(gNetG[i].id, id)) return i;
    return -1;
}

// A free live slot (only MAX_LIVE_GAMES exist) or -1.
static int slotFree() {
    for (int i = 0; i < MAX_LIVE_GAMES; i++) if (!gNetG[i].active && !hGameS[i].alive()) return i;
    return -1;
}

// One line from a game stream (gameFull or gameState).
static void onGameLine(int slot, const char* line) {
    DBG(NET, "slot%d game line received", slot);
    JsonDocument d;
    if (deserializeJson(d, line)) return;
    const char* t = d["type"] | "";
    bool full = !strcmp(t, "gameFull");
    if (!full && strcmp(t, "gameState")) return;
    NetGame& g = gNetG[slot];
    xSemaphoreTake(gNetMux, portMAX_DELAY);
    if (full) {
        for (int side = 0; side < 2; side++) {
            JsonObjectConst p = d[side ? "black" : "white"].as<JsonObjectConst>();
            char* dst = side ? g.black : g.white;
            if (p["aiLevel"].is<int>()) snprintf(dst, 26, "Stockfish L%d", (int)p["aiLevel"]);
            else strlcpy(dst, p["name"] | (side ? "Black" : "White"), 26);
            (side ? g.bRating : g.wRating) = p["rating"] | 0;
        }
        strlcpy(g.initialFen, d["initialFen"] | "startpos", sizeof(g.initialFen));
        gameApplyState(g, d["state"].as<JsonObjectConst>());
    } else gameApplyState(g, d.as<JsonObjectConst>());
    bool over = strcmp(g.status, "created") && strcmp(g.status, "started");
    if (over) g.active = false;
    char st[16], win[8];
    strlcpy(st, g.status, 16);
    strlcpy(win, g.winner, 8);
    xSemaphoreGive(gNetMux);
    uiPost(UE_GAME_UPDATE, "", "", full ? 1 : 0, slot);
    if (over) {
        uiPost(UE_GAME_END, st, win, slot);
        hGameS[slot].close();
    }
}

// Load a game into a free slot and open its stream.
static void startGame(const char* id, bool white, const char* opp, int oppRating) {
    DBG(NET, "startGame id=%s white=%d opponent='%s' rating=%d", id, (int)white, opp, oppRating);
    if (slotFor(id) >= 0) return;  // already known (the event stream repeats gameStart on reconnect)
    int s = slotFree();
    if (s < 0 && gSeekSlot >= 0) {
        s = gSeekSlot;
        hGameS[s].close();
        gSeekSlot = -1;
    }
    // reuse the slot that was waiting for a seek
    if (s < 0) {
        uiPost(UE_TOAST, "A game is already live", "Only one game is kept live (memory limit of the ESP32). Open Home > Ongoing and Park the current game (it keeps running on Lichess), then open this one.");
        return;
    }
    xSemaphoreTake(gNetMux, portMAX_DELAY);
    memset(&gNetG[s], 0, sizeof(NetGame));
    strlcpy(gNetG[s].id, id, sizeof(gNetG[s].id));
    gNetG[s].iAmWhite = white;
    gNetG[s].active = true;
    strcpy(gNetG[s].initialFen, "startpos");
    xSemaphoreGive(gNetMux);
    char path[64];
    snprintf(path, sizeof(path), "/api/board/game/stream/%s", id);
    if (!hGameS[s].open("GET", path, nullptr, "application/x-ndjson") || hGameS[s].status != 200) {
        uiPost(UE_TOAST, "Could not open game");
        gNetG[s].active = false;
        return;
    }
    uiPost(UE_GAME_START, opp, id, (white ? 1 : 0) | (s << 1), oppRating);
}

static void fetchOngoing(bool lookForNew = false);

// Read the live game stream(s) without waiting; reconnect them when they dropped or were paused.
static void pollStreams() {
    static char buf[3072];
    int r;
    for (int i = 0; i < 2; i++) {
        if (hGameS[i].alive()) {
            while ((r = hGameS[i].pollLine(buf, sizeof(buf))) == 1) {
                if (i != gSeekSlot) onGameLine(i, buf);
            }
            if (r < 0) {
                hGameS[i].close();
                if (i == gSeekSlot) gSeekSlot = -1;
                else if (gNetG[i].active) {
                    DBG(NET, "game stream %d closed by the server, will reconnect", i);
                    gStreamRetryAt[i] = millis() + 1500;
                }
            }
        } else if (gNetG[i].active && WiFi.status() == WL_CONNECTED && i != gSeekSlot && !(gPausedMask & (1 << i))) {
            // reconnect an unfinished game
            if (millis() > gStreamRetryAt[i]) {
                gStreamRetryAt[i] = millis() + 4000;
                char p[64];
                snprintf(p, sizeof(p), "/api/board/game/stream/%s", gNetG[i].id);
                DBG(NET, "reconnecting game stream %d (%s)", i, gNetG[i].id);
                hGameS[i].open("GET", p, nullptr, "application/x-ndjson");
            }
        }
    }
}

// ---------------- puzzles (offline pack in LittleFS: id|rating|fen|sol sol sol) ----------------
#define PUZ_FILE "/puz.txt"
// Replay the puzzle game up to the puzzle start and return its FEN.
static bool puzzleFromPgn(const char* pgn, JsonArrayConst sol, char* fenOut) {
    cr::Position p;
    cr::setStart(p);
    char tok[16];
    const char* c = pgn;
    while (*c) {
        while (*c == ' ' || *c == '\n') c++;
        if (!*c) break;
        int k = 0;
        while (*c && *c != ' ' && *c != '\n' && k < 15) tok[k++] = *c++;
        tok[k] = 0;
        if (isdigit(tok[0]) && (strchr(tok, '.') || strchr(tok, '-'))) continue;
        cr::Move m;
        if (!cr::fromSan(p, tok, &m)) return false;
        cr::makeMove(p, m);
    }
    cr::Move m;
    if (!cr::fromUci(p, sol[0] | "", &m)) return false;  // first solution move must be the solver's
    cr::toFen(p, fenOut);
    return true;
}

// Turn one puzzle from Lichess into one saved line: id|rating|fen|solution.
static bool puzzleLine(JsonObjectConst e, char* line, size_t cap) {
    const char* pgn = e["game"]["pgn"] | "";
    JsonArrayConst sol = e["puzzle"]["solution"].as<JsonArrayConst>();
    char fen[96];
    if (!pgn[0] || sol.isNull() || !puzzleFromPgn(pgn, sol, fen)) return false;
    String s;
    for (JsonVariantConst u : sol) {
        if (s.length()) s += ' ';
        s += u.as<const char*>();
    }
    snprintf(line, cap, "%s|%d|%s|%s", e["puzzle"]["id"] | "?", (int)(e["puzzle"]["rating"] | 0), fen, s.c_str());
    return true;
}

// Give a puzzle to the UI.
static void puzzlePublish(const char* line) {
    xSemaphoreTake(gNetMux, portMAX_DELAY);
    char tmp[220];
    strlcpy(tmp, line, sizeof(tmp));
    char* f[4] = {tmp, 0, 0, 0};
    int n = 1;
    for (char* c = tmp; *c && n < 4; c++) if (*c == '|') {
        *c = 0;
        f[n++] = c + 1;
    }
    if (n == 4) {
        strlcpy(gPuz.id, f[0], 10);
        gPuz.rating = atoi(f[1]);
        strlcpy(gPuz.fen, f[2], 96);
        strlcpy(gPuz.sol, f[3], 96);
    }
    xSemaphoreGive(gNetMux);
    if (n == 4) uiPost(UE_PUZZLE, gPuz.id, "", 1);
}

// How many puzzles are saved offline.
static int puzzleStoreCount() {
    if (!LittleFS.exists(PUZ_FILE)) return 0;   // no pack yet
    File f = LittleFS.open(PUZ_FILE);
    if (!f) return 0;
    int n = 0;
    while (f.available()) {
        if (f.read() == '\n') n++;
    }
    f.close();
    return n;
}

// Pick a random offline puzzle.
static bool puzzleStorePop(char* out, size_t cap) {
    // random puzzle from the offline pack (kept in file)
    int n = puzzleStoreCount();
    if (!n) return false;
    int pick = esp_random() % n;
    File f = LittleFS.open(PUZ_FILE);
    for (int i = 0; i <= pick; i++) {
        String s = f.readStringUntil('\n');
        if (i == pick) {
            strlcpy(out, s.c_str(), cap);
        }
    }
    f.close();
    return out[0];
}

// Download several puzzles for offline play.
static void puzzleBatch(int count) {
    if (!strstr(gScopes, "puzzle:read")) {
        uiPost(UE_TOAST, "Token needs puzzle:read", "Offline puzzles need the permission puzzle:read. Link the account again and allow it.");
        return;
    }
    String o;
    JsonDocument filter;
    filter["puzzles"][0]["game"]["pgn"] = true;
    filter["puzzles"][0]["puzzle"]["id"] = true;
    filter["puzzles"][0]["puzzle"]["rating"] = true;
    filter["puzzles"][0]["puzzle"]["solution"] = true;
    char path[64];
    snprintf(path, sizeof(path), "/api/puzzle/batch/mix?nb=%d", count);
    int st = httpReq("GET", path, nullptr, o, "application/json", 60000);
    if (st != 200) {
        uiPost(UE_TOAST, st == 401 || st == 403 ? "Token lacks puzzle:read" : "Puzzle download failed");
        return;
    }
    JsonDocument d;
    if (deserializeJson(d, o, DeserializationOption::Filter(filter))) {
        uiPost(UE_TOAST, "Bad puzzle data");
        return;
    }
    File f = LittleFS.open(PUZ_FILE, FILE_APPEND);
    int saved = 0;
    char line[240];
    for (JsonVariantConst ev : d["puzzles"].as<JsonArrayConst>()) if (puzzleLine(ev.as<JsonObjectConst>(), line, sizeof(line))) {
        f.println(line);
        saved++;
    }
    f.close();
    char msg[40];
    snprintf(msg, sizeof(msg), "%d puzzles saved offline", saved);
    uiPost(UE_TOAST, msg);
}

// Get one puzzle: 0 = daily, 1 = next for my rating, 2 = offline.
static void puzzleGet(int mode) {
    if (mode == 1 && !strstr(gScopes, "puzzle:read")) mode = 0;   // the token may not ask for puzzles of my rating
    // 0 daily, 1 next from account, 2 offline random
    char line[240] = "";
    if (mode == 2 && puzzleStorePop(line, sizeof(line))) {
        puzzlePublish(line);
        return;
    }
    String o;
    int st = httpReq("GET", mode == 0 ? "/api/puzzle/daily" : "/api/puzzle/next", nullptr, o, "application/json", 16000);
    JsonDocument d;
    if (st != 200 || deserializeJson(d, o)) {
        if (puzzleStorePop(line, sizeof(line))) {
            puzzlePublish(line);
            return;
        }
        uiPost(UE_TOAST, "No puzzle available");
        return;
    }
    if (puzzleLine(d.as<JsonObjectConst>(), line, sizeof(line))) puzzlePublish(line);
    else uiPost(UE_TOAST, "Puzzle parse error");
}

// ---------------- lists: friends / search / history / wifi ----------------
// Tell the UI that a list (friends, history, ...) is ready.
static void listPost(ListKind k, uint8_t n) {
    gList[k].n = n;
    uiPost(UE_LIST, "", "", k, n);
}

// Load the accounts I follow.
static void fetchFriends() {
    String o;
    if (httpReq("GET", "/api/rel/following", nullptr, o, "application/x-ndjson", 12000) != 200) {
        uiPost(UE_TOAST, "Could not load friends");
        listPost(LK_FRIENDS, 0);
        return;
    }
    uint8_t n = 0;
    int p = 0;
    while (p < (int)o.length() && n < 24) {
        int e = o.indexOf('\n', p);
        if (e < 0) e = o.length();
        String ln = o.substring(p, e);
        p = e + 1;
        JsonDocument d;
        if (ln.length() < 3 || deserializeJson(d, ln)) continue;
        strlcpy(gList[LK_FRIENDS].it[n].main, (d["username"].is<const char*>() ? d["username"].as<const char*>() : d["name"] | "?"), 30);
        strlcpy(gList[LK_FRIENDS].it[n].sub, (d["online"] | false) ? "online" : "", 26);
        strlcpy(gList[LK_FRIENDS].it[n].id, gList[LK_FRIENDS].it[n].main, 14);
        n++;
    }
    listPost(LK_FRIENDS, n);
}

// Report a failed request with HTTP code and the reason Lichess gave.
static void netFail(const char* title, int st, const String& body) {
    char det[128];
    JsonDocument d;
    const char* err = nullptr;
    if (!deserializeJson(d, body)) err = d["error"] | (const char*)nullptr;
    snprintf(det, sizeof(det), "HTTP %d%s%s", st, err ? ": " : "", err ? err : (st == -1 ? " (no connection to lichess.org)" : ""));
    uiPost(UE_TOAST, title, det);
}

// Is this game id one that existed before the user started a new game?
static bool isKnownGame(const char* id) {
    for (int i = 0; i < gKnownN; i++) if (!strcmp(gKnown[i], id)) return true;
    return false;
}

// Remember the current games (called when the user starts / accepts a game).
static void snapshotKnown() {
    if (!gOngoingFetched) fetchOngoing(false);
    gKnownN = gList[LK_ONGOING].n;
    for (int i = 0; i < gKnownN && i < 24; i++) strlcpy(gKnown[i], gList[LK_ONGOING].it[i].id, 14);
    gPollNow = true;
}

// Light list of ALL my running games (also long correspondence games). Only metadata, no streams.
// lookForNew = the user just started a game: the first game that did not exist before is loaded live.
static void fetchOngoing(bool lookForNew) {
    String o;
    JsonDocument filter;
    JsonObject f = filter["nowPlaying"][0].to<JsonObject>();
    f["gameId"] = true;
    f["color"] = true;
    f["isMyTurn"] = true;
    f["secondsLeft"] = true;
    f["perf"] = true;
    f["opponent"]["username"] = true;
    f["opponent"]["rating"] = true;
    f["opponent"]["ai"] = true;
    int st = httpReq("GET", "/api/account/playing?nb=30", nullptr, o, "application/json", 40000);
    uint8_t c = 0;
    bool found = false, foundWhite = true;
    char foundId[14] = "", foundName[32] = "";
    int foundRating = 0;
    {
        JsonDocument d;
        if (st == 200 && !deserializeJson(d, o, DeserializationOption::Filter(filter))) {
            for (JsonVariantConst v : d["nowPlaying"].as<JsonArrayConst>()) {
                if (c >= 24) break;
                JsonObjectConst g = v.as<JsonObjectConst>();
                ListItem& it = gList[LK_ONGOING].it[c];
                bool white = !strcmp(g["color"] | "white", "white"), mine = g["isMyTurn"] | false;
                int sl = g["secondsLeft"] | 0;
                if (g["opponent"]["ai"].is<int>()) snprintf(it.main, 30, "Stockfish L%d", (int)g["opponent"]["ai"]);
                else strlcpy(it.main, g["opponent"]["username"] | "Opponent", 30);
                strlcpy(it.id, g["gameId"] | "", 14);
                it.val = (white ? 1 : 0) | (mine ? 2 : 0);
                if (sl >= 86400) snprintf(it.sub, 26, "%s  %dd %dh", mine ? "YOUR MOVE" : "waiting", sl / 86400, (sl % 86400) / 3600);
                else if (sl > 0) snprintf(it.sub, 26, "%s  %dh %dm", mine ? "YOUR MOVE" : "waiting", sl / 3600, (sl % 3600) / 60);
                else snprintf(it.sub, 26, "%s  %s", mine ? "YOUR MOVE" : "waiting", g["perf"] | "");
                // a game that did not exist before the user pressed "start" = the new game
                if (lookForNew && !found && millis() < gExpectUntil && slotFor(it.id) < 0 && !isKnownGame(it.id)) {
                    found = true;
                    foundWhite = white;
                    strlcpy(foundId, it.id, 14);
                    strlcpy(foundName, it.main, 32);
                    foundRating = g["opponent"]["rating"] | 0;
                }
                c++;
            }
            gOngoingFetched = true;
        } else if (st != 200) netFail("Could not load your games", st, o);
    }
    o = "";                                  // free the answer before opening a new TLS connection
    listPost(LK_ONGOING, c);
    DBG(NET, "ongoing games: %d (lookForNew=%d found=%d)", (int)c, (int)lookForNew, (int)found);
    if (found) {
        gExpectUntil = 0;
        startGame(foundId, foundWhite, foundName, foundRating);
    }
}

// Incoming challenges are found by polling (an events stream would cost 45 KB of RAM).
static char gSeenCh[6][12];
static uint8_t gSeenChN = 0;
static void pollChallenges() {
    String o;
    JsonDocument filter;
    JsonObject f = filter["in"][0].to<JsonObject>();
    f["id"] = true;
    f["rated"] = true;
    f["challenger"]["name"] = true;
    f["timeControl"]["show"] = true;
    int st = httpReq("GET", "/api/challenge", nullptr, o, "application/json", 8000);
    if (st == 401 || st == 403) {
        gChallengeScopeOk = false;      // the token has no challenge:read permission: stop asking
        DBG(NET, "challenge polling disabled: token has no challenge:read");
        return;
    }
    if (st != 200) return;
    JsonDocument d;
    if (deserializeJson(d, o, DeserializationOption::Filter(filter))) return;
    for (JsonVariantConst v : d["in"].as<JsonArrayConst>()) {
        JsonObjectConst c = v.as<JsonObjectConst>();
        const char* id = c["id"] | "";
        bool seen = !id[0];
        for (int i = 0; i < gSeenChN && !seen; i++) if (!strcmp(gSeenCh[i], id)) seen = true;
        if (seen) continue;
        strlcpy(gSeenCh[gSeenChN % 6], id, 12);
        gSeenChN++;
        char info[48];
        snprintf(info, sizeof(info), "%s  %s%s", c["challenger"]["name"] | "?", c["timeControl"]["show"] | "", (c["rated"] | false) ? " rated" : "");
        uiPost(UE_CHALLENGE, id, info);
    }
}

// Username search (autocomplete).
static void searchUsers(const char* term) {
    String o, path = String("/api/player/autocomplete?term=") + term;
    JsonDocument d;
    uint8_t n = 0;
    if (httpReq("GET", path.c_str(), nullptr, o) == 200 && !deserializeJson(d, o)) for (JsonVariantConst uv : d.as<JsonArrayConst>()) {
        if (n >= 24) break;
        const char* u = uv.as<const char*>();
        strlcpy(gList[LK_SEARCH].it[n].main, u, 30);
        gList[LK_SEARCH].it[n].sub[0] = 0;
        strlcpy(gList[LK_SEARCH].it[n].id, u, 14);
        n++;
    }
    listPost(LK_SEARCH, n);
}

// Load my last finished games.
static void fetchHistory() {
    String o, path = String("/api/games/user/") + gMyUser + "?max=20&moves=false&tags=false&clocks=false&opening=false";
    uint8_t n = 0;
    if (httpReq("GET", path.c_str(), nullptr, o, "application/x-ndjson", 30000) == 200) {
        int p = 0;
        while (p < (int)o.length() && n < 20) {
            int e = o.indexOf('\n', p);
            if (e < 0) e = o.length();
            String ln = o.substring(p, e);
            p = e + 1;
            JsonDocument d;
            if (ln.length() < 5 || deserializeJson(d, ln)) continue;
            const char* w = d["players"]["white"]["user"]["name"] | "Stockfish";
            const char* b = d["players"]["black"]["user"]["name"] | "Stockfish";
            const char* win = d["winner"] | "";
            ListItem& it = gList[LK_HISTORY].it[n];
            snprintf(it.main, 30, "%.12s vs %.12s", w, b);
            snprintf(it.sub, 26, "%s  %s", !strcmp(win, "white") ? "1-0" : !strcmp(win, "black") ? "0-1" : "1/2", d["speed"] | "");
            strlcpy(it.id, d["id"] | "", 14);
            n++;
        }
    } else uiPost(UE_TOAST, "Could not load history");
    listPost(LK_HISTORY, n);
}

// Download the moves of one finished game for review.
static void exportGame(const char* id) {
    String o, path = String("/game/export/") + id + "?moves=true&tags=false&clocks=false&evals=false&opening=false";
    JsonDocument d;
    if (httpReq("GET", path.c_str(), nullptr, o, "application/json") != 200 || deserializeJson(d, o)) {
        uiPost(UE_TOAST, "Could not load game");
        return;
    }
    xSemaphoreTake(gNetMux, portMAX_DELAY);
    strlcpy(gNetX.moves, d["moves"] | "", sizeof(gNetX.moves));
    strlcpy(gNetX.id, id, sizeof(gNetX.id));
    strlcpy(gNetX.white, d["players"]["white"]["user"]["name"] | "White", 26);
    strlcpy(gNetX.black, d["players"]["black"]["user"]["name"] | "Black", 26);
    gNetX.wRating = d["players"]["white"]["rating"] | 0;
    gNetX.bRating = d["players"]["black"]["rating"] | 0;
    xSemaphoreGive(gNetMux);
    uiPost(UE_EXPORT, id);
}

// Scan and give the list of networks to the UI.
static void scanWifiList() {
    int n = WiFi.scanNetworks(false, false, false, 300);
    uint8_t c = 0;
    for (int i = 0; i < n && c < 24; i++) {
        String s = WiFi.SSID(i);
        if (!s.length()) continue;
        bool dup = false;
        for (int j = 0; j < c; j++) if (!strcmp(gList[LK_WIFI].it[j].main, s.c_str())) dup = true;
        if (dup) continue;
        ListItem& it = gList[LK_WIFI].it[c];
        strlcpy(it.main, s.c_str(), 30);
        it.val = WiFi.RSSI(i);
        snprintf(it.sub, 26, "%d dBm %s%s", WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "lock", wifiFind(s.c_str()) >= 0 ? " saved" : "");
        strlcpy(it.id, WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "o" : "p", 14);
        c++;
    }
    WiFi.scanDelete();
    listPost(LK_WIFI, c);
}

// ---------------- account linking (your Gchess-Server protocol) ----------------
// QR = LINK_SITE_URL + "?d=<board ip>". The website then talks to this board over the local network:
//   POST /pair/start   -> {ok, sessionId, expiresIn}          (optional step, sessionId is not a secret)
//   POST /pair/token   {sessionId?, accessToken}  -> {ok, stored, username}
//   GET  /pair/status  -> {state: waiting|connected|expired}
// No pair secret. Safety comes from: pairing window only open while the "Link account" page is on screen
// (5 min max), and every token is verified against Lichess (/api/account) before it is stored.
#define LINK_WINDOW_MS 300000UL
static WebServer linkSrv(80);
static uint32_t linkStartedAt = 0;
static bool linkStored = false, linkRoutes = false;
static char linkSession[33] = "";
// Headers that let the website (another origin) talk to the board.
static void linkCors() {
    linkSrv.sendHeader("Access-Control-Allow-Origin", "*");  // tighten to your site origin if you like
    linkSrv.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    linkSrv.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    linkSrv.sendHeader("Access-Control-Allow-Private-Network", "true");
    linkSrv.sendHeader("Access-Control-Max-Age", "600");
    linkSrv.sendHeader("Cache-Control", "no-store");
}

// Send a JSON answer to the website.
static void linkJson(int code, JsonDocument& d) {
    String o;
    serializeJson(d, o);
    linkCors();
    linkSrv.send(code, "application/json", o);
}

// Send a JSON error to the website.
static void linkErr(int code, const char* e, const char* m) {
    JsonDocument d;
    d["ok"] = false;
    d["error"] = e;
    d["message"] = m;
    linkJson(code, d);
}

// Linking is only allowed while the "Link account" page is open (max 5 minutes).
static bool linkWindowOpen() {
    return gLinkOn && millis() - linkStartedAt < LINK_WINDOW_MS;
}

// 0 ok, 1 bad format, 2 rejected by Lichess, 3 board full
static int linkAcceptToken(const String& token, char* userOut) {
    if (token.length() < 8 || token.length() >= sizeof(gToken)) return 1;
    char saved[72];
    strlcpy(saved, gToken, 72);
    strlcpy(gToken, token.c_str(), 72);
    String o;
    int st = httpReq("GET", "/api/account", nullptr, o);
    strlcpy(gToken, saved, 72);
    JsonDocument d;
    if (st != 200 || deserializeJson(d, o)) return 2;
    strlcpy(userOut, d["username"] | "", 24);
    int slot = accAdd(userOut, token.c_str());
    if (slot < 0) return 3;
    linkStored = true;
    uiPost(UE_LINK_DONE, userOut, "", slot);
    return 0;
}

// Answer the browser pre-flight request.
static void linkOnOptions() {
    linkCors();
    linkSrv.send(204);
}

// GET /  : tells the website the board is ready.
static void linkOnRoot() {
    JsonDocument d;
    d["ok"] = true;
    d["device"] = GC_NAME;
    d["pairing"] = linkWindowOpen() ? "ready" : "closed";
    linkJson(200, d);
}

// POST /pair/start : optional session id.
static void linkOnStart() {
    DBG(NET, "LINK: POST /pair/start from %s", linkSrv.client().remoteIP().toString().c_str());
    if (!linkWindowOpen()) {
        linkErr(410, "PAIRING_CLOSED", "Open 'Link account' on the board first");
        return;
    }
    snprintf(linkSession, sizeof(linkSession), "%08x%08x%08x%08x", (unsigned)esp_random(), (unsigned)esp_random(), (unsigned)esp_random(), (unsigned)esp_random());
    linkStored = false;
    JsonDocument d;
    d["ok"] = true;
    d["sessionId"] = linkSession;
    d["expiresIn"] = (int)((LINK_WINDOW_MS - (millis() - linkStartedAt)) / 1000);
    linkJson(200, d);
}

// POST /pair/token : the website delivers the Lichess token here.
static void linkOnToken() {
    DBG(NET, "LINK: POST /pair/token from %s body=%d bytes", linkSrv.client().remoteIP().toString().c_str(), (int)linkSrv.arg("plain").length());
    if (!linkWindowOpen()) {
        linkErr(410, "PAIRING_CLOSED", "Open 'Link account' on the board first");
        return;
    }
    JsonDocument d;
    String body = linkSrv.arg("plain");
    if (body.length() == 0 || body.length() > 5000 || deserializeJson(d, body)) {
        linkErr(400, "INVALID_JSON", "Invalid JSON request");
        return;
    }
    const char* sid = d["sessionId"] | "";
    const char* tok = d["accessToken"] | "";
    if (sid[0] && strcmp(sid, linkSession) != 0) {
        linkErr(401, "INVALID_SESSION", "The pairing session is invalid");
        return;
    }
    // sessionId optional
    char user[24] = "";
    int r = linkAcceptToken(String(tok), user);
    if (r == 1) {
        linkErr(400, "INVALID_TOKEN", "The token is missing or malformed");
        return;
    }
    if (r == 2) {
        linkErr(401, "TOKEN_REJECTED", "Lichess rejected this token");
        uiPost(UE_TOAST, "Token rejected");
        return;
    }
    if (r == 3) {
        linkErr(507, "BOARD_FULL", "Board holds 4 accounts. Remove one in Settings");
        return;
    }
    JsonDocument ok;
    ok["ok"] = true;
    ok["stored"] = true;
    ok["username"] = user;
    ok["message"] = "Token stored on the board";
    linkJson(200, ok);
}

// GET /pair/status : waiting / connected / expired.
static void linkOnStatus() {
    JsonDocument d;
    d["ok"] = true;
    String s = linkSrv.arg("sessionId");
    if (linkStored) {
        d["state"] = "connected";
        d["stored"] = true;
    } else if (linkWindowOpen()) {
        d["state"] = "waiting";
        d["stored"] = false;
    } else {
        d["state"] = "expired";
        d["stored"] = false;
    }
    linkJson(200, d);
}

// Fallback: GET /link?token=... (if the browser blocks https -> http requests).
static void linkOnGetLink() {
    // fallback if a browser blocks https->http fetch: navigate to http://<ip>/link?token=...
    char user[24] = "";
    int r = linkWindowOpen() ? linkAcceptToken(linkSrv.arg("token"), user) : 9;
    String h = String("<html><meta name=viewport content='width=device-width,initial-scale=1'><body style='font-family:sans-serif;text-align:center;padding:40px'><h2>") + GC_NAME + "</h2><p>" + (r == 0 ? "Linked! You can close this page." : "Linking failed. Try again from the board.") + "</p></body></html>";
    linkSrv.send(r == 0 ? 200 : 400, "text/html", h);
}

// Open the linking window and prepare the QR code address.
static void linkStart() {
    DBG(NET, "LINK: receiver started, window %lu s", (unsigned long)(LINK_WINDOW_MS / 1000));
    linkStartedAt = millis();
    linkStored = false;
    linkSession[0] = 0;
    if (!linkRoutes) {
        linkRoutes = true;
        linkSrv.on("/", HTTP_GET, linkOnRoot);
        linkSrv.on("/link", HTTP_GET, linkOnGetLink);
        linkSrv.on("/pair/start", HTTP_OPTIONS, linkOnOptions);
        linkSrv.on("/pair/token", HTTP_OPTIONS, linkOnOptions);
        linkSrv.on("/pair/status", HTTP_OPTIONS, linkOnOptions);
        linkSrv.on("/pair/start", HTTP_POST, linkOnStart);
        linkSrv.on("/pair/token", HTTP_POST, linkOnToken);
        linkSrv.on("/pair/status", HTTP_GET, linkOnStatus);
        linkSrv.onNotFound([]() {
            if (linkSrv.method() == HTTP_OPTIONS) linkOnOptions();
            else linkErr(404, "NOT_FOUND", "Unknown endpoint");
        });
    }
    linkSrv.begin();
    gLinkOn = true;
    snprintf(gLinkUrl, sizeof(gLinkUrl), "%s?d=%s", LINK_SITE_URL, WiFi.localIP().toString().c_str());
    uiPost(UE_LINK_URL, "");
}

// ---------------- command dispatcher ----------------
// Download my profile (name, rating) and check the token is still valid.
static void accountFetch() {
    String o;
    int st = httpReq("GET", "/api/account", nullptr, o);
    JsonDocument d;
    DBG(NET, "account fetch status=%d", st);
    if (st == 200 && !deserializeJson(d, o)) {
        strlcpy(gMyUser, d["username"] | (const char*)gMyUser, 24);
        int r = d["perfs"]["blitz"]["rating"] | 0;
        if (!r) r = d["perfs"]["rapid"]["rating"] | 0;
        gMyRating = r;
        gNeedAccount = false;
        gOngoingDirty = true;
        strlcpy(gScopes, hReq.scopes, sizeof(gScopes));
        gChallengeScopeOk = strstr(gScopes, "challenge:read") != nullptr;
        DBG(NET, "token permissions: %s", gScopes[0] ? gScopes : "(none reported)");
        uiPost(UE_LOGIN, gMyUser, "", 1, r);
        // Tell the user which permissions the linked token lacks.
        static const char* need[] = {"board:play", "challenge:read", "challenge:write", "puzzle:read"};
        char missing[96] = "";
        for (auto p : need) if (!strstr(gScopes, p)) { strlcat(missing, p, sizeof(missing)); strlcat(missing, " ", sizeof(missing)); }
        if (missing[0]) {
            char det[140];
            snprintf(det, sizeof(det), "Missing permissions: %s. Link the account again and allow them (your token only has: %s).", missing, gScopes[0] ? gScopes : "?");
            uiPost(UE_TOAST, "Token has limited permissions", det);
        }
    } else if (st == 401) {
        gNeedAccount = false;
        gLoggedIn = false;
        uiPost(UE_LOGIN, "", "", -1);
    }
}

// POST request without body (resign, abort, accept ...).
static void postSimple(const char* fmt, const char* a = "", const char* b = "") {
    char path[96];
    snprintf(path, sizeof(path), fmt, a, b);
    String o;
    int st = httpReq("POST", path, nullptr, o);
    if (st != 200 && st != 201) netFail("Lichess refused the request", st, o);
}

// Execute one command from the UI.
static void handleCmd(NetCmd& c) {
    DBG(NET, "command %s s1='%s' a=%d b=%d c=%d", NC_NAMES[c.t], c.s1, c.a, c.b, c.c);
    char body[160], path[96];
    String o;
    switch (c.t) {
        case NC_LOGIN: strlcpy(gToken, c.s2, 72);
        strlcpy(gMyUser, c.s1, 24);
        gLoggedIn = true;
        gNeedAccount = true;
        break;
        case NC_LOGOUT: gLoggedIn = false;
        gToken[0] = 0;
        hGameS[0].close();
        hGameS[1].close();
        for (int i = 0; i < 2; i++) gNetG[i].active = false;
        gSeekSlot = -1;
        break;
        case NC_AI: snapshotKnown();
        gExpectUntil = millis() + 120000;
        snprintf(body, sizeof(body), "level=%d&clock.limit=%d&clock.increment=%d&color=%s", c.a, c.b, c.c, c.s1[0] ? c.s1 : "random");
        {
            int st = httpReq("POST", "/api/challenge/ai", body, o);
            if (st != 201 && st != 200) netFail("AI challenge failed", st, o);
            else gPollNow = true;
        } break;
        case NC_CHALLENGE: snapshotKnown();
        gExpectUntil = millis() + 120000;
        snprintf(path, sizeof(path), "/api/challenge/%s", c.s1);
        snprintf(body, sizeof(body), "rated=%s&clock.limit=%d&clock.increment=%d&color=%s", c.c ? "true" : "false", c.a, c.b, c.s2[0] ? c.s2 : "random");
        {
            int st = httpReq("POST", path, body, o);
            if (st > 201 || st < 0) netFail("Challenge failed", st, o);
            else uiPost(UE_TOAST, "Challenge sent");
        } break;
        case NC_SEEK: snapshotKnown();
        gExpectUntil = millis() + 180000;
        snprintf(body, sizeof(body), "rated=%s&time=%d&increment=%d&color=random", c.c ? "true" : "false", c.a, c.b);
        gSeekSlot = slotFree();
        if (gSeekSlot < 0 || !hGameS[gSeekSlot].open("POST", "/api/board/seek", body, "application/json")) {
            if (gSeekSlot >= 0) hGameS[gSeekSlot].close();
            gSeekSlot = -1;
            uiPost(UE_TOAST, "Seek failed");
        } break;
        case NC_CANCEL_SEEK: if (gSeekSlot >= 0) {
            hGameS[gSeekSlot].close();
            gSeekSlot = -1;
        } break;
        case NC_ACCEPT: snapshotKnown();
        gExpectUntil = millis() + 60000;
        postSimple("/api/challenge/%s/accept", c.s1);
        break;
        case NC_DECLINE: postSimple("/api/challenge/%s/decline", c.s1);
        break;
        case NC_MOVE: snprintf(path, sizeof(path), "/api/board/game/%s/move/%s", c.s2, c.s1);
        {
            int st = httpReq("POST", path, nullptr, o);
            if (st != 200) netFail("Move rejected by server", st, o);
        } break;
        case NC_RESIGN: postSimple("/api/board/game/%s/resign", c.s2);
        break;
        case NC_ABORT: postSimple("/api/board/game/%s/abort", c.s2);
        break;
        case NC_DRAW: snprintf(path, sizeof(path), "/api/board/game/%s/draw/%s", c.s2, c.s1);
        httpReq("POST", path, nullptr, o);
        break;
        case NC_TAKEBACK: snprintf(path, sizeof(path), "/api/board/game/%s/takeback/%s", c.s2, c.s1);
        httpReq("POST", path, nullptr, o);
        break;
        case NC_PUZ_DAILY: puzzleGet(c.a);
        break;
        case NC_PUZ_BATCH: puzzleBatch(c.a ? c.a : 10);
        break;
        case NC_SEARCH: searchUsers(c.s1);
        break;
        case NC_FRIENDS: fetchFriends();
        break;
        case NC_ONGOING: fetchOngoing(false);
        break;
        case NC_OPEN_GAME: startGame(c.s1, c.a != 0, c.s2, 0);
        break;  // load one game into a live slot (on demand)
        case NC_PARK_GAME: if (c.a >= 0 && c.a < 2) {
            hGameS[c.a].close();
            xSemaphoreTake(gNetMux, portMAX_DELAY);
            gNetG[c.a].active = false;
            xSemaphoreGive(gNetMux);
        } break;  // frees the slot; the game keeps running on Lichess
        case NC_HISTORY: fetchHistory();
        break;
        case NC_GAME_EXPORT: exportGame(c.s1);
        break;
        case NC_WIFI_SCAN: scanWifiList();
        break;
        case NC_WIFI_CONNECT: {
            const char* pass = c.s2;
            int sv = wifiFind(c.s1);
            if (!c.s2[0] && sv >= 0) pass = gWifi[sv].pass;
            bool ok = wifiConnectTo(c.s1, pass);
            if (ok) wifiSave(c.s1, pass);
            uiPost(UE_TOAST, ok ? "Wi-Fi connected" : "Wi-Fi failed (wrong password?)");
            break;
        }
        case NC_WIFI_ENABLE: gWifiWanted = c.a;
        break;
        case NC_LINK_START: linkStart();
        break;
        case NC_LINK_STOP: if (gLinkOn) {
            linkSrv.stop();
            gLinkOn = false;
        } break;
        default: break;
    }
}

// The network task (core 1): runs forever.
static void netTask(void*) {
    WiFi.mode(WIFI_STA);
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    for (;;) {
        NetCmd c;
        while (xQueueReceive(qNet, &c, 0) == pdTRUE) handleCmd(c);
        wifiTick();
        if (gLinkOn) linkSrv.handleClient();
        if (gNeedAccount && gLoggedIn && WiFi.status() == WL_CONNECTED) {
            static uint32_t t = 0;
            if (millis() - t > 6000) {
                t = millis();
                accountFetch();
            }
        }
        // Background polling (replaces the events stream):
        if (gLoggedIn && !gNeedAccount && WiFi.status() == WL_CONNECTED) {
            static uint32_t tChallenges = 0, tList = 0, tNew = 0;
            uint32_t now = millis();
            bool live = hGameS[0].alive() || hGameS[1].alive();       // a live stream is open: do not disturb it
            bool expecting = now < gExpectUntil && gSeekSlot < 0;      // the user waits for a game to start
            if (expecting && (gPollNow || now - tNew > POLL_NEWGAME_MS)) {
                gPollNow = false;
                tNew = now;
                fetchOngoing(true);
            } else if (!live) {
                if (gOngoingDirty || now - tList > POLL_LIST_MS) {
                    gOngoingDirty = false;
                    tList = now;
                    fetchOngoing(false);
                } else if (gChallengeScopeOk && now - tChallenges > POLL_CHALLENGES_MS) {
                    tChallenges = now;
                    pollChallenges();
                }
            }
        }
        pollStreams();
        netReportStatus();
        vTaskDelay(pdMS_TO_TICKS(8));
    }
}

// Start the network task on core 1.
inline void netBegin() {
    xTaskCreatePinnedToCore(netTask, "net", 16384, nullptr, 1, nullptr, 1);
}

// CORE 1
