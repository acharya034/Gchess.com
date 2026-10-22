#pragma once

#include <Arduino.h>
// Board state tracking
// board[x][y][0] = piece type/color
char board[8][8][2];

// Compact chess state
char castleRights = 0x0F;
// Bit 0: White kingside
// Bit 1: White queenside
// Bit 2: Black kingside
// Bit 3: Black queenside

char enPassant = '-';  // '-' means unavailable; otherwise file 'a'–'h'
char enPassantRank = '0'; // Target rank '3' or '6'
char sideToMove = 'w';

bool calculateMove(int selectedRow, int selectedCol);
bool engineMove(String move);
bool loadFEN(String fen);
