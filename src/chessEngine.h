#pragma once

#include <Arduino.h>
// Board state tracking
// board[x][y][0] = piece type/color
extern char board[8][8][2];

// Compact chess state
extern char castleRights;
// Bit 0: White kingside
// Bit 1: White queenside
// Bit 2: Black kingside
// Bit 3: Black queenside

extern char enPassant;  // '-' means unavailable; otherwise file 'a'–'h'
extern char enPassantRank; // Target rank '3' or '6'
extern char sideToMove;

bool calculateMove(int selectedRow, int selectedCol);
bool engineMove(String move);
bool loadFEN(String fen);
bool setSideToMove(const char* text);
