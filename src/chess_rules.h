// chess_rules.h - Portable chess rules (no Arduino dependency, unit-tested with perft)
// Squares: a1=0, b1=1 ... h1=7, a2=8 ... h8=63.  Pieces: +white / -black, P=1 N=2 B=3 R=4 Q=5 K=6
#pragma once
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

namespace cr {

    enum : int8_t {
        EMPTY = 0,
        P = 1,
        N = 2,
        B = 3,
        R = 4,
        Q = 5,
        K = 6
    };
    enum : uint8_t {
        F_CAP = 1,
        F_EP = 2,
        F_CASTLE = 4,
        F_DBL = 8
    };
    enum : uint8_t {
        C_WK = 1,
        C_WQ = 2,
        C_BK = 4,
        C_BQ = 8
    };

    struct Move {
        uint8_t from, to, promo, flags;
    };  // promo = 0 or N/B/R/Q (2..5)

    struct Position {
        int8_t b[64];
        bool white;  // side to move
        uint8_t castle;
        int8_t ep;  // en-passant target square or -1
        uint8_t half;
        uint16_t full;
    };

    static const int8_t KN_D[8][2] = {{1,2},{2,1},{2,-1},{1,-2},{-1,-2},{-2,-1},{-2,1},{-1,2}};
    static const int8_t KG_D[8][2] = {{1,0},{1,1},{0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1}};
    static const int8_t SL_D[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};  // 0-3 rook, 4-7 bishop

    // Set up the standard start position.
    inline void setStart(Position& p) {
        static const int8_t back[8] = {R, N, B, Q, K, B, N, R};
        memset(p.b, 0, sizeof(p.b));
        for (int f = 0; f < 8; f++) {
            p.b[f] = back[f];
            p.b[8 + f] = P;
            p.b[48 + f] = -P;
            p.b[56 + f] = -back[f];
        }
        p.white = true;
        p.castle = 15;
        p.ep = -1;
        p.half = 0;
        p.full = 1;
    }

    // Is square sq attacked by the given colour?
    inline bool attacked(const Position& p, int sq, bool byWhite) {
        int f = sq & 7, r = sq >> 3, s = byWhite ? 1 : -1;
        int pr = byWhite ? r - 1 : r + 1;
        if (pr >= 0 && pr < 8) {
            if (f > 0 && p.b[pr * 8 + f - 1] == s * P) return true;
            if (f < 7 && p.b[pr * 8 + f + 1] == s * P) return true;
        }
        for (int i = 0; i < 8; i++) {
            int nf = f + KN_D[i][0], nr = r + KN_D[i][1];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8 && p.b[nr * 8 + nf] == s * N) return true;
            nf = f + KG_D[i][0];
            nr = r + KG_D[i][1];
            if (nf >= 0 && nf < 8 && nr >= 0 && nr < 8 && p.b[nr * 8 + nf] == s * K) return true;
        }
        for (int d = 0; d < 8; d++) {
            int nf = f + SL_D[d][0], nr = r + SL_D[d][1];
            while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                int v = p.b[nr * 8 + nf];
                if (v) {
                    if (v * s > 0) {
                        int t = abs(v);
                        if (t == Q || (d < 4 && t == R) || (d >= 4 && t == B)) return true;
                    } break;
                }
                nf += SL_D[d][0];
                nr += SL_D[d][1];
            }
        }
        return false;
    }

    // Square of the king of one colour.
    inline int kingSq(const Position& p, bool white) {
        int8_t k = white ? K : -K;
        for (int i = 0; i < 64; i++) if (p.b[i] == k) return i;
        return -1;
    }
    // Is the side to move in check?
    inline bool inCheck(const Position& p) {
        int k = kingSq(p, p.white);
        return k >= 0 && attacked(p, k, !p.white);
    }

    // Play a move on the board (captures, en passant, castling, promotion, castling rights, clocks).
    inline void makeMove(Position& p, const Move& m) {
        int s = p.white ? 1 : -1;
        int8_t pc = p.b[m.from];
        int t = abs(pc);
        bool cap = p.b[m.to] != 0 || (m.flags & F_EP);
        p.half = (t == P || cap) ? 0 : p.half + 1;
        if (m.flags & F_EP) p.b[m.to - 8 * s] = 0;
        p.b[m.to] = m.promo ? (int8_t)(s * m.promo) : pc;
        p.b[m.from] = 0;
        if (m.flags & F_CASTLE) {
            if (m.to > m.from) {
                p.b[m.to - 1] = p.b[m.to + 1];
                p.b[m.to + 1] = 0;
            } else {
                p.b[m.to + 1] = p.b[m.to - 2];
                p.b[m.to - 2] = 0;
            }
        }
        if (t == K) p.castle &= p.white ? ~(C_WK | C_WQ) : ~(C_BK | C_BQ);
        int sqs[2] = {m.from, m.to};
        for (int i = 0; i < 2; i++) {
            switch (sqs[i]) {
                case 0: p.castle &= ~C_WQ;
                break;
                case 7: p.castle &= ~C_WK;
                break;
                case 56: p.castle &= ~C_BQ;
                break;
                case 63: p.castle &= ~C_BK;
                break;
            }
        }
        p.ep = (m.flags & F_DBL) ? (m.from + m.to) / 2 : -1;
        if (!p.white) p.full++;
        p.white = !p.white;
    }

    // Append one move to a list.
    inline void addMove(Move* out, int& n, int from, int to, int promo, int flags) {
        out[n].from = from;
        out[n].to = to;
        out[n].promo = promo;
        out[n].flags = flags;
        n++;
    }
    // Append a pawn move (a move to the last rank becomes four promotion moves).
    inline void addPawnMove(Move* out, int& n, int from, int to, int flags, bool white) {
        int rank = to >> 3;
        if ((white && rank == 7) || (!white && rank == 0)) {
            for (int pr = Q; pr >= N; pr--) addMove(out, n, from, to, pr, flags);
        } else addMove(out, n, from, to, 0, flags);
    }

    // Fills out[] (min 256 entries) with fully legal moves. Returns count.
    inline int genLegal(const Position& p, Move* out) {
        Move ps[256];
        int n = 0;
        int s = p.white ? 1 : -1;
        for (int sq = 0; sq < 64; sq++) {
            int v = p.b[sq];
            if (v * s <= 0) continue;
            int t = abs(v), f = sq & 7, r = sq >> 3;
            if (t == P) {
                int dir = p.white ? 1 : -1, startR = p.white ? 1 : 6, nr = r + dir;
                if (nr >= 0 && nr < 8) {
                    int to = nr * 8 + f;
                    if (!p.b[to]) {
                        addPawnMove(ps, n, sq, to, 0, p.white);
                        if (r == startR && !p.b[(r + 2 * dir) * 8 + f]) addMove(ps, n, sq, (r + 2 * dir) * 8 + f, 0, F_DBL);
                    }
                    for (int df = -1; df <= 1; df += 2) {
                        int nf = f + df;
                        if (nf < 0 || nf > 7) continue;
                        int t2 = nr * 8 + nf;
                        if (p.b[t2] * s < 0) addPawnMove(ps, n, sq, t2, F_CAP, p.white);
                        else if (t2 == p.ep) addMove(ps, n, sq, t2, 0, F_EP | F_CAP);
                    }
                }
            } else if (t == N || t == K) {
                for (int i = 0; i < 8; i++) {
                    int nf = f + (t == N ? KN_D[i][0] : KG_D[i][0]), nr = r + (t == N ? KN_D[i][1] : KG_D[i][1]);
                    if (nf < 0 || nf > 7 || nr < 0 || nr > 7) continue;
                    int to = nr * 8 + nf;
                    if (p.b[to] * s > 0) continue;
                    addMove(ps, n, sq, to, 0, p.b[to] ? F_CAP : 0);
                }
            } else {
                int d0 = (t == B) ? 4 : 0, d1 = (t == R) ? 4 : 8;
                for (int d = d0; d < d1; d++) {
                    int nf = f + SL_D[d][0], nr = r + SL_D[d][1];
                    while (nf >= 0 && nf < 8 && nr >= 0 && nr < 8) {
                        int to = nr * 8 + nf;
                        if (p.b[to] * s > 0) break;
                        addMove(ps, n, sq, to, 0, p.b[to] ? F_CAP : 0);
                        if (p.b[to]) break;
                        nf += SL_D[d][0];
                        nr += SL_D[d][1];
                    }
                }
            }
        }
        // castling
        bool w = p.white;
        int e = w ? 4 : 60;
        if (p.b[e] == s * K && !attacked(p, e, !w)) {
            uint8_t ks = w ? C_WK : C_BK, qs = w ? C_WQ : C_BQ;
            if ((p.castle & ks) && !p.b[e + 1] && !p.b[e + 2] && p.b[e + 3] == s * R && !attacked(p, e + 1, !w) && !attacked(p, e + 2, !w)) addMove(ps, n, e, e + 2, 0, F_CASTLE);
            if ((p.castle & qs) && !p.b[e - 1] && !p.b[e - 2] && !p.b[e - 3] && p.b[e - 4] == s * R && !attacked(p, e - 1, !w) && !attacked(p, e - 2, !w)) addMove(ps, n, e, e - 2, 0, F_CASTLE);
        }
        int cnt = 0;
        for (int i = 0; i < n; i++) {
            Position q = p;
            makeMove(q, ps[i]);
            int k = kingSq(q, w);
            if (k >= 0 && !attacked(q, k, !w)) out[cnt++] = ps[i];
        }
        return cnt;
    }

    // 0 = ongoing, 1 = checkmate (side to move lost), 2 = stalemate
    inline int gameStatus(const Position& p) {
        Move m[256];
        if (genLegal(p, m) > 0) return 0;
        return inCheck(p) ? 1 : 2;
    }

    // Which squares hold a piece? (64 bit mask, bit 0 = a1)
    inline uint64_t occupancy(const Position& p) {
        uint64_t o = 0;
        for (int i = 0; i < 64; i++) if (p.b[i]) o |= 1ULL << i;
        return o;
    }
    // The occupancy mask after a move - compared with the hall sensors to detect that move.
    inline uint64_t occupancyAfter(const Position& p, const Move& m) {
        Position q = p;
        makeMove(q, m);
        return occupancy(q);
    }

    // ---------- notation ----------
    inline void sqName(int sq, char* o) {
        o[0] = 'a' + (sq & 7);
        o[1] = '1' + (sq >> 3);
        o[2] = 0;
    }
    // Move -> "e2e4" (or "e7e8q").
    inline void toUci(const Move& m, char* o) {
        // o >= 6 bytes
        sqName(m.from, o);
        sqName(m.to, o + 2);
        o[4] = 0;
        if (m.promo) {
            o[4] = "??nbrq"[m.promo];
            o[5] = 0;
        }
    }
    // "e2e4" -> a legal move of this position. Returns false if illegal.
    inline bool fromUci(const Position& p, const char* u, Move* out) {
        if (!u || strlen(u) < 4) return false;
        int f = (u[0] - 'a') + 8 * (u[1] - '1'), t = (u[2] - 'a') + 8 * (u[3] - '1');
        if (f < 0 || f > 63 || t < 0 || t > 63) return false;
        int pr = 0;
        if (u[4]) {
            switch (u[4]) {
                case 'n': pr = N;
                break;
                case 'b': pr = B;
                break;
                case 'r': pr = R;
                break;
                case 'q': pr = Q;
                break;
            }
        }
        Move l[256];
        int n = genLegal(p, l);
        for (int i = 0; i < n; i++) if (l[i].from == f && l[i].to == t && l[i].promo == pr) {
            *out = l[i];
            return true;
        }
        return false;
    }

    // Read a FEN string.
    inline bool fromFen(Position& p, const char* fen) {
        memset(&p, 0, sizeof(p));
        p.ep = -1;
        p.full = 1;
        int f = 0, r = 7;
        const char* c = fen;
        for (; *c && *c != ' '; c++) {
            if (*c == '/') {
                r--;
                f = 0;
            } else if (*c >= '1' && *c <= '8') f += *c - '0';
            else {
                int8_t t = 0;
                char l = *c | 0x20;
                switch (l) {
                    case 'p': t = P;
                    break;
                    case 'n': t = N;
                    break;
                    case 'b': t = B;
                    break;
                    case 'r': t = R;
                    break;
                    case 'q': t = Q;
                    break;
                    case 'k': t = K;
                    break;
                    default: return false;
                }
                if (f > 7 || r < 0) return false;
                p.b[r * 8 + f++] = (*c >= 'a') ? -t : t;
            }
        }
        if (*c != ' ') return false;
        c++;
        p.white = (*c != 'b');
        while (*c && *c != ' ') c++;
        if (*c) c++;
        for (; *c && *c != ' '; c++) switch (*c) {
            case 'K': p.castle |= C_WK;
            break;
            case 'Q': p.castle |= C_WQ;
            break;
            case 'k': p.castle |= C_BK;
            break;
            case 'q': p.castle |= C_BQ;
            break;
        }
        if (*c) c++;
        if (*c && *c != '-') {
            p.ep = (c[0] - 'a') + 8 * (c[1] - '1');
            while (*c && *c != ' ') c++;
        } else if (*c) c++;
        if (*c) c++;
        if (*c) {
            p.half = atoi(c);
            while (*c && *c != ' ') c++;
            if (*c) p.full = atoi(c + 1);
            if (!p.full) p.full = 1;
        }
        return true;
    }

    // Write the position as FEN.
    inline void toFen(const Position& p, char* o) {
        // o >= 90 bytes
        int k = 0;
        for (int r = 7; r >= 0; r--) {
            int e = 0;
            for (int f = 0; f < 8; f++) {
                int v = p.b[r * 8 + f];
                if (!v) {
                    e++;
                    continue;
                }
                if (e) {
                    o[k++] = '0' + e;
                    e = 0;
                }
                char ch = "?pnbrqk"[abs(v)];
                if (v > 0) ch -= 32;
                o[k++] = ch;
            }
            if (e) o[k++] = '0' + e;
            if (r) o[k++] = '/';
        }
        k += sprintf(o + k, " %c ", p.white ? 'w' : 'b');
        if (!p.castle) o[k++] = '-';
        else {
            if (p.castle & C_WK) o[k++] = 'K';
            if (p.castle & C_WQ) o[k++] = 'Q';
            if (p.castle & C_BK) o[k++] = 'k';
            if (p.castle & C_BQ) o[k++] = 'q';
        } o[k++] = ' ';
        if (p.ep < 0) o[k++] = '-';
        else {
            sqName(p.ep, o + k);
            k += 2;
        }
        sprintf(o + k, " %d %d", p.half, p.full);
    }

    // SAN output (suffix=true adds + / #)
    inline void toSan(const Position& p, const Move& m, char* o, bool suffix = true) {
        int k = 0;
        int t = abs(p.b[m.from]);
        if (m.flags & F_CASTLE) {
            k += sprintf(o, (m.to > m.from) ? "O-O" : "O-O-O");
        } else {
            if (t != P) {
                o[k++] = "?PNBRQK"[t];
                Move l[256];
                int n = genLegal(p, l);
                bool amb = false, sameF = false, sameR = false;
                for (int i = 0; i < n; i++) if (l[i].to == m.to && l[i].from != m.from && abs(p.b[l[i].from]) == t) {
                    amb = true;
                    if ((l[i].from & 7) == (m.from & 7)) sameF = true;
                    if ((l[i].from >> 3) == (m.from >> 3)) sameR = true;
                }
                if (amb) {
                    if (!sameF) o[k++] = 'a' + (m.from & 7);
                    else if (!sameR) o[k++] = '1' + (m.from >> 3);
                    else {
                        o[k++] = 'a' + (m.from & 7);
                        o[k++] = '1' + (m.from >> 3);
                    }
                }
            } else if (m.flags & F_CAP) o[k++] = 'a' + (m.from & 7);
            if (m.flags & F_CAP) o[k++] = 'x';
            sqName(m.to, o + k);
            k += 2;
            if (m.promo) {
                o[k++] = '=';
                o[k++] = "?PNBRQK"[m.promo];
            } o[k] = 0;
        }
        if (suffix) {
            Position q = p;
            makeMove(q, m);
            if (inCheck(q)) {
                o[k++] = (gameStatus(q) == 1) ? '#' : '+';
                o[k] = 0;
            }
        }
    }
    // Standard notation -> legal move.
    inline bool fromSan(const Position& p, const char* san, Move* out) {
        char clean[16];
        int k = 0;
        for (const char* c = san; *c && k < 14; c++) if (*c != '+' && *c != '#' && *c != '!' && *c != '?') clean[k++] = *c;
        clean[k] = 0;
        if (!strcmp(clean, "0-0")) strcpy(clean, "O-O");
        else if (!strcmp(clean, "0-0-0")) strcpy(clean, "O-O-O");
        Move l[256];
        int n = genLegal(p, l);
        for (int i = 0; i < n; i++) {
            char s[16];
            toSan(p, l[i], s, false);
            if (!strcmp(s, clean)) {
                *out = l[i];
                return true;
            }
        }
        return false;
    }

}

// namespace cr
