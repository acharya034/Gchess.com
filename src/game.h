// game.h - the chess "brain" that joins sensors, touch, LEDs and the network.
// One Game object (G) is used by every mode: ONLINE (Lichess), LOCAL (pass & play), PUZZLE, REVIEW.
// External API (all called from the UI task on core 0):
//   gameReset(mode, fenOrNull, myWhite)
//   gameApplyMovesStr("e2e4 e7e5 ...")   sync with server move list (applies only the new plies)
//   gameApplyRemote(move)                 opponent / puzzle reply: lights blue->green and waits for the user to mirror it
//   gameSensorTick(occ)                   feed the debounced sensor bitboard, returns GEvt
//   gameTouchSquare(sq)                   touch input on the on-screen board, returns GEvt
//   gameSubmit(from,to,promo)             finish a move (e.g. after promotion choice), returns GE_MOVE
//   gameCanMoveNow()   gameLoadSan("e4 e5 ...")   gameRevGoto(i)   gamePuzzleCheck()
#pragma once
#include "chess_rules.h"
#include "leds.h"
#include "board.h"
#include "sound.h"
#include "debug.h"

enum GameMode : uint8_t {
    GM_NONE,
    GM_ONLINE,
    GM_LOCAL,
    GM_PUZZLE,
    GM_REVIEW
};
enum GEvt : uint8_t {
    GE_NONE,
    GE_MOVE,
    GE_NEED_PROMO,
    GE_WRONG,
    GE_SELECT,
    GE_SYNCED
};
enum PzRes : uint8_t {
    PZ_CONTINUE,
    PZ_SOLVED,
    PZ_WRONG
};

struct Game {
    GameMode mode = GM_NONE;
    cr::Position start, pos;
    cr::Move hist[512];
    uint16_t nHist = 0;
    bool myWhite = true, over = false;
    char result[28] = "";
    cr::Move legal[256];
    uint8_t nLegal = 0;
    uint64_t legalOcc[256];
    uint64_t expected = 0;
    int8_t selFrom = -1;
    uint64_t selDest = 0, selCap = 0;
    int8_t lastFrom = -1, lastTo = -1;
    bool syncing = false, wrongActive = false, promoPending = false;
    uint32_t mismatchSince = 0;
    uint8_t pFrom = 0, pTo = 0;
    int8_t slot = 2;  // 0,1 = online games, 2 = scratch (pass&play / puzzle / review)
    bool physical = false;  // this game owns the physical board (sensors + LEDs)
    bool touchAllowed = true;  // moves from the TFT allowed
    bool whiteBottom = true;
    bool promoByTouch = false;
    uint64_t emptied = 0;  // squares seen empty since the board last matched (needed to tell 'piece lifted' from 'capture finished')
    cr::Move lastSubmitted;
    // puzzle
    char puzId[10] = "";
    uint16_t puzRating = 0;
    char puzSol[14][6];
    uint8_t puzN = 0, puzIdx = 0;
    uint16_t puzStartHist = 0;
    // review
    cr::Move revAll[512];
    uint16_t revN = 0, revIdx = 0;
};
static Game* GS = nullptr;  // 3 games, allocated from the heap in gamesInit() (too big for static RAM)
static Game* gGp = nullptr;
#define G (*gGp)
// temporarily make another game the "current" one (all game functions use G)
struct GameScope {
    Game* old;
    GameScope(int slot) {
        old = gGp;
        gGp = &GS[slot];
        gGp->slot = slot;
    }
    ~GameScope() {
        gGp = old;
    }
};
// LED helpers: only the game that owns the physical board may touch the LEDs
inline void gLedsClear() {
    if (gGp->physical) ledsClear();
}

// Light one square - only if this game owns the physical board.
inline void gLedSq(int sq, CRGB c) {
    if (gGp->physical) ledSq(sq, c);
}

// Show the last move (blue = from, green = to) - only on the board owner.
inline void gLedsShowLastMove(int a, int b) {
    if (gGp->physical) ledsShowLastMove(a, b);
}

// Show the legal moves of a lifted piece - only on the board owner.
inline void gLedsShowOptions(int f, uint64_t d, uint64_t c) {
    if (gGp->physical) ledsShowOptions(f, d, c);
}

// Blink a set of squares - only on the board owner.
inline void gLedsFlashMask(uint64_t m, CRGB c, uint16_t ms) {
    if (gGp->physical) ledsFlashMask(m, c, ms);
}

// Recompute everything that depends on the position: legal moves and the expected sensor pattern.
inline void gameRecalc() {
    G.nLegal = cr::genLegal(G.pos, G.legal);
    for (int i = 0; i < G.nLegal; i++) G.legalOcc[i] = cr::occupancyAfter(G.pos, G.legal[i]);
    G.expected = cr::occupancy(G.pos);
}

// Forget the selected piece and its move hints.
inline void gameClearSel() {
    G.selFrom = -1;
    G.selDest = G.selCap = 0;
}

// Update the LEDs: options of the selected piece, otherwise the last move.
inline void gameShowHints() {
    if (G.selFrom >= 0) gLedsShowOptions(G.selFrom, G.selDest, G.selCap);
    else {
        gLedsClear();
        if (G.syncing || G.lastFrom >= 0) gLedsShowLastMove(G.lastFrom, G.lastTo);
    }
}

// Start a new game (mode, start position, which colour I play).
inline void gameReset(GameMode m, const char* fen, bool myWhite) {
    G.mode = m;
    G.myWhite = myWhite;
    G.whiteBottom = myWhite;
    G.over = false;
    G.result[0] = 0;
    G.nHist = 0;
    if (fen && fen[0] && strcmp(fen, "startpos") && cr::fromFen(G.start, fen)) {
    } else cr::setStart(G.start);
    G.pos = G.start;
    G.lastFrom = G.lastTo = -1;
    G.syncing = false;
    G.promoPending = false;
    G.promoByTouch = false;
    G.wrongActive = false;
    G.mismatchSince = 0;
    G.emptied = 0;
    gameClearSel();
    gameRecalc();
    G.syncing = G.physical && (boardOccupancy() != G.expected);
    gLedsClear();
}

// True when the user is allowed to move in the current game right now.
inline bool gameCanMoveNow() {
    if (G.over) return false;
    switch (G.mode) {
        case GM_LOCAL: return true;
        case GM_ONLINE: return G.pos.white == G.myWhite;
        case GM_PUZZLE: return G.pos.white == G.myWhite && G.puzIdx < G.puzN;
        default: return false;
    }
}

// Play a move on the logical board, update hints, LEDs and sound. remote = move came from the server.
inline void gameApply(const cr::Move& moveRef, bool remote) {
    cr::Move m = moveRef;   // copy: moveRef may point into G.legal[], which gameRecalc() overwrites
    if (G.nHist < 512) G.hist[G.nHist++] = m;
    cr::makeMove(G.pos, m);
    G.lastFrom = m.from;
    G.lastTo = m.to;
    gameClearSel();
    gameRecalc();
    int st = cr::gameStatus(G.pos);
    if (st == 1 && G.mode != GM_ONLINE) {
        G.over = true;
        strcpy(G.result, G.pos.white ? "Black wins - checkmate" : "White wins - checkmate");
    }
    if (st == 2 && G.mode != GM_ONLINE) {
        G.over = true;
        strcpy(G.result, "Draw - stalemate");
    }
    G.syncing = G.physical && (boardOccupancy() != G.expected);
    G.wrongActive = false;
    G.mismatchSince = 0;
    G.promoPending = false;
    G.promoByTouch = false;
    G.emptied = 0;
    gLedsClear();
    gLedsShowLastMove(G.lastFrom, G.lastTo);
    soundPlay(remote ? SND_MOVE_OPP : SND_MOVE_OK);
    {
        char u[8], fen[100];
        cr::toUci(m, u);
        cr::toFen(G.pos, fen);
        DBG(GAME, "slot%d MOVE %s (%s) ply=%d syncing=%d physical=%d  fen=%s", (int)G.slot, u, remote ? "remote/server" : "local", (int)G.nHist, (int)G.syncing, (int)G.physical, fen);
    }
}

// Play an opponent / scripted move; LEDs then guide the user to mirror it on the board.
inline void gameApplyRemote(const cr::Move& m) {
    gameApply(m, true);
}

// Play the legal move from->to (with promotion piece) chosen by the user.
inline GEvt gameSubmit(int from, int to, int promo) {
    for (int i = 0; i < G.nLegal; i++) if (G.legal[i].from == from && G.legal[i].to == to && G.legal[i].promo == promo) {
        G.lastSubmitted = G.legal[i];
        gameApply(G.legal[i], false);
        return GE_MOVE;
    }
    return GE_NONE;
}

// Server move list ("e2e4 e7e5 ...") -> bring local game up to date. Returns number of new plies.
inline int gameApplyMovesStr(const char* moves) {
    static char tok[512][6];
    int n = 0;
    const char* c = moves;
    while (*c && n < 512) {
        while (*c == ' ') c++;
        if (!*c) break;
        int k = 0;
        while (*c && *c != ' ' && k < 5) tok[n][k++] = *c++;
        tok[n][k] = 0;
        while (*c && *c != ' ') c++;
        n++;
    }
    bool same = n >= G.nHist;
    for (int i = 0; same && i < G.nHist; i++) {
        char u[8];
        cr::toUci(G.hist[i], u);
        if (strcmp(u, tok[i])) same = false;
    }
    int applied = 0;
    if (!same) {
        G.pos = G.start;
        G.nHist = 0;
        gameRecalc();
    }
    // server disagrees -> rebuild
    for (int i = G.nHist; i < n; i++) {
        cr::Move m;
        if (!cr::fromUci(G.pos, tok[i], &m)) break;
        bool mine = (G.pos.white == G.myWhite) && G.mode == GM_ONLINE;  // our own move echoed back: board already matches
        bool silent = mine && boardOccupancy() == cr::occupancyAfter(G.pos, m);
        gameApply(m, !silent);
        applied++;
    }
    return applied;
}

// Load SAN movetext ("e4 e5 Nf3 ...") into the review buffer
inline bool gameLoadSan(const char* san, const char* fen = nullptr) {
    G.mode = GM_REVIEW;
    cr::Position p;
    if (fen && fen[0] && strcmp(fen, "startpos") && cr::fromFen(p, fen)) {
    } else cr::setStart(p);
    G.start = p;
    G.revN = 0;
    char tok[16];
    const char* c = san;
    while (*c && G.revN < 512) {
        while (*c == ' ' || *c == '\n') c++;
        if (!*c) break;
        int k = 0;
        while (*c && *c != ' ' && *c != '\n' && k < 15) tok[k++] = *c++;
        tok[k] = 0;
        if ((tok[0] >= '0' && tok[0] <= '9' && (strchr(tok, '.') || strchr(tok, '-') || strchr(tok, '/')))) continue;  // move numbers / results
        cr::Move m;
        if (!cr::fromSan(p, tok, &m)) break;
        G.revAll[G.revN++] = m;
        cr::makeMove(p, m);
    }
    G.revIdx = 0;
    return G.revN > 0;
}

// Review mode: jump to move number idx.
inline void gameRevGoto(int idx) {
    if (idx < 0) idx = 0;
    if (idx > G.revN) idx = G.revN;
    G.pos = G.start;
    G.lastFrom = G.lastTo = -1;
    for (int i = 0; i < idx; i++) {
        cr::makeMove(G.pos, G.revAll[i]);
        G.lastFrom = G.revAll[i].from;
        G.lastTo = G.revAll[i].to;
    }
    G.revIdx = idx;
    gameRecalc();
    gLedsClear();
    gLedsShowLastMove(G.lastFrom, G.lastTo);
}

// ---- puzzles ----
// Start a puzzle from its start position and its list of solution moves.
inline void gamePuzzleStart(const char* id, int rating, const char* fen, const char* solution) {
    G.puzN = 0;
    G.puzIdx = 0;
    strlcpy(G.puzId, id, 10);
    G.puzRating = rating;
    const char* c = solution;
    while (*c && G.puzN < 14) {
        while (*c == ' ') c++;
        int k = 0;
        while (*c && *c != ' ' && k < 5) G.puzSol[G.puzN][k++] = *c++;
        G.puzSol[G.puzN][k] = 0;
        if (k) G.puzN++;
    }
    cr::Position p;
    cr::fromFen(p, fen);
    gameReset(GM_PUZZLE, fen, p.white);  // the side to move solves the puzzle
}

// Call after the user's move. Auto-plays the scripted reply.
inline PzRes gamePuzzleCheck() {
    char u[8];
    cr::toUci(G.lastSubmitted, u);
    if (strcmp(u, G.puzSol[G.puzIdx])) return PZ_WRONG;
    G.puzIdx++;
    if (G.puzIdx >= G.puzN) return PZ_SOLVED;
    cr::Move m;
    if (cr::fromUci(G.pos, G.puzSol[G.puzIdx], &m)) {
        G.puzIdx++;
        gameApply(m, true);
    }
    return (G.puzIdx >= G.puzN) ? PZ_SOLVED : PZ_CONTINUE;
}

// Wrong puzzle move: go back one move and show LEDs telling the user to put the piece back.
inline void gamePuzzleRewind() {
    // wrong move: go back one ply, user must undo physically
    if (!G.nHist) return;
    G.nHist--;
    G.pos = G.start;
    for (int i = 0; i < G.nHist; i++) cr::makeMove(G.pos, G.hist[i]);
    G.lastFrom = G.nHist ? G.hist[G.nHist - 1].from : -1;
    G.lastTo = G.nHist ? G.hist[G.nHist - 1].to : -1;
    gameClearSel();
    gameRecalc();
    G.syncing = true;
    gLedsClear();
    gLedsShowLastMove(G.lastSubmitted.to, G.lastSubmitted.from);  // reversed colours = "put it back"
}

// ---- who owns the physical board ----
// Give the physical board (sensors + LEDs) to one game; every other game becomes display-only.
inline void gameClaimBoard(int slot) {
    DBG(GAME, "board ownership -> slot%d", slot);
    for (int i = 0; i < 3; i++) if (i != slot && GS[i].physical) {
        GS[i].physical = false;
        GS[i].syncing = false;
        GS[i].selFrom = -1;
        GS[i].selDest = GS[i].selCap = 0;
        GS[i].touchAllowed = true;
    }
    GS[slot].physical = true;
    GS[slot].promoPending = false;
    GameScope sc(slot);
    G.syncing = (boardOccupancy() != G.expected);
    gLedsClear();
    if (G.lastFrom >= 0 || G.syncing) gLedsShowLastMove(G.lastFrom, G.lastTo);
}

// Allocate the 3 game slots (0,1 online games, 2 = local / puzzle / review).
inline void gamesInit() {
    if (!GS) GS = new Game[3];
    gGp = &GS[0];
    for (int i = 0; i < 3; i++) {
        GS[i].physical = false;
        GS[i].slot = i;
        GS[i].mode = GM_NONE;
    }
}

// ---- touch ----
// Select a piece and compute where it can go.
inline void gameSelect(int sq) {
    gameClearSel();
    G.selFrom = sq;
    for (int i = 0; i < G.nLegal; i++) if (G.legal[i].from == sq) {
        G.selDest |= 1ULL << G.legal[i].to;
        if (G.legal[i].flags & cr::F_CAP) G.selCap |= 1ULL << G.legal[i].to;
    }
    gameShowHints();
}

// The user tapped a square on the screen: select a piece or finish a move.
inline GEvt gameTouchSquare(int sq) {
    if (!gameCanMoveNow() || G.promoPending || !G.touchAllowed) return GE_NONE;  // works even when the physical board is not set up
    int s = G.pos.white ? 1 : -1;
    DBG(GAME, "touch on square %c%c (selected=%d)", 'a' + (sq & 7), '1' + (sq >> 3), (int)G.selFrom);
    if (G.selFrom >= 0 && (G.selDest >> sq & 1)) {
        bool promo = false;
        for (int i = 0; i < G.nLegal; i++) if (G.legal[i].from == G.selFrom && G.legal[i].to == sq && G.legal[i].promo) promo = true;
        if (promo) {
            G.pFrom = G.selFrom;
            G.pTo = sq;
            G.promoPending = true;
            G.promoByTouch = true;
            return GE_NEED_PROMO;
        }
        return gameSubmit(G.selFrom, sq, 0);
    }
    if (G.pos.b[sq] * s > 0 && sq != G.selFrom) {
        gameSelect(sq);
        return GE_SELECT;
    }
    gameClearSel();
    gameShowHints();
    return GE_SELECT;
}

// ---- sensors ----
// Feed the sensor pattern: detects lifted pieces, finished moves, castling, promotion and wrong moves.
inline GEvt gameSensorTick(uint64_t occ) {
    if (G.mode == GM_NONE || G.mode == GM_REVIEW || !G.physical || G.over) return GE_NONE;
    uint32_t now = millis();
    if (occ == G.expected) {
        G.mismatchSince = 0;
        G.wrongActive = false;
        G.emptied = 0;
        if (!G.promoByTouch) G.promoPending = false;  // piece put back => promotion cancelled
        bool was = G.syncing;
        G.syncing = false;
        if (was) DBG(GAME, "slot%d board in sync with the game", (int)G.slot);
        if (G.selFrom >= 0) {
            gameClearSel();
            gameShowHints();
        }
        if (was) {
            gameShowHints();
            return GE_SYNCED;
        }
        return GE_NONE;
    }
    if (G.syncing) return GE_NONE;  // user is mirroring a move: LEDs guide, no nagging
    bool myTurn = gameCanMoveNow();
    uint64_t added = occ & ~G.expected, lifted = G.expected & ~occ;
    G.emptied |= lifted;
    if (myTurn && !G.promoPending) {
        for (int i = 0; i < G.nLegal; i++) if (occ == G.legalOcc[i]) {
            cr::Move m = G.legal[i];
            if ((m.flags & cr::F_CAP) && !(m.flags & cr::F_EP) && !(G.emptied >> m.to & 1)) continue;  // still just 'lifted'
            if (m.promo) {
                G.pFrom = m.from;
                G.pTo = m.to;
                G.promoPending = true;
                DBG(GAME, "promotion needed (sensor)");
                return GE_NEED_PROMO;
            }
            G.lastSubmitted = m;
            gameApply(m, false);
            return GE_MOVE;
        }
        if (!added && __builtin_popcountll(lifted) == 1) {
            int sq = __builtin_ctzll(lifted);
            if ((G.pos.b[sq] * (G.pos.white ? 1 : -1)) > 0 && G.selFrom != sq) {
                DBG(GAME, "piece lifted from %c%c", 'a' + (sq & 7), '1' + (sq >> 3));
                gameSelect(sq);
                return GE_SELECT;
            }
        }
    }
    if (G.promoPending) return GE_NONE;
    if (added) {
        if (!G.mismatchSince) G.mismatchSince = now;
        uint64_t okDest = 0;
        if (myTurn) for (int i = 0; i < G.nLegal; i++) {
            okDest |= 1ULL << G.legal[i].to;
            if (G.legal[i].flags & cr::F_CASTLE) okDest |= 1ULL << ((G.legal[i].to > G.legal[i].from) ? G.legal[i].to - 1 : G.legal[i].to + 1);
        }
        uint32_t limit = (added & ~okDest) ? 700 : 3500;  // illegal square: fast warning; maybe-castling: be patient
        if (!G.wrongActive && now - G.mismatchSince > limit) {
            G.wrongActive = true;
            {
                char sq[48];
                dbgSquares(added, sq, sizeof(sq));
                DBG(GAME, "WRONG MOVE: unexpected pieces on [%s] after %lums", sq, (unsigned long)(now - G.mismatchSince));
            }
            gLedsFlashMask(added, CRGB(255, 0, 0), 2500);
            soundPlay(SND_WRONG);
            return GE_WRONG;
        }
    } else {
        G.mismatchSince = 0;
        G.wrongActive = false;
    }
    return GE_NONE;
}
