#pragma once

// Board state tracking
// board[x][y][0] = piece type/color
char board[8][8][2];

bool calculateMoveOptions(int selectedRow, int selectedCol);