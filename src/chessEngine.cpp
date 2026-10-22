#include "chessEngine.h"


// Empty-square check
inline bool isEmptySquare(char p) {
    return p == '.' || p == ' ' || p == '\0';
}

// Determine piece color
inline bool isWhitePiece(char p) {
    return p >= 'A' && p <= 'Z';
}

// Check whether a square is inside the board
inline bool insideBoard(int r, int c) {
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

// Restore old move highlights to normal square colors.
// Preserve check markers.
inline void clearMoveOptions() {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            char &state = board[r][c][1];

            if (state == 's' || state == 'S' ||
                state == 'o' || state == 'O' ||
                state == 't' || state == 'T') {

                state = ((r + c) % 2 == 0) ? 'n' : 'N';
            }
        }
    }
}

// Add a possible destination or capture.
inline void addMoveOption(
    int r, int c,
    bool white
) {
    char p = board[r][c][0];

    if (isEmptySquare(p)) {
        board[r][c][1] = white ? 'O' : 'o';
    }
    else if (isWhitePiece(p) != white) {
        board[r][c][1] = white ? 'T' : 't';
    }
}

// Sliding-piece movement: bishop, rook, queen.
inline void traceMoves(
    int r, int c,
    bool white,
    const int directions[][2],
    int directionCount
) {
    for (int d = 0; d < directionCount; d++) {
        int nr = r + directions[d][0];
        int nc = c + directions[d][1];

        while (insideBoard(nr, nc)) {
            char p = board[nr][nc][0];

            if (isEmptySquare(p)) {
                board[nr][nc][1] = white ? 'O' : 'o';
            }
            else {
                if (isWhitePiece(p) != white) {
                    board[nr][nc][1] = white ? 'T' : 't';
                }

                break; // Cannot move past any piece.
            }

            nr += directions[d][0];
            nc += directions[d][1];
        }
    }
}

// Calculate movement options for the selected piece.
inline bool calculateMove(int selectedRow, int selectedCol) {
    if (!insideBoard(selectedRow, selectedCol)) {
        return false;
    }

    char piece = board[selectedRow][selectedCol][0];

    if (isEmptySquare(piece)) {
        return false;
    }

    clearMoveOptions();

    bool white = isWhitePiece(piece);
    int r = selectedRow;
    int c = selectedCol;

    // Mark the selected piece.
    board[r][c][1] = white ? 'S' : 's';

    switch (piece >= 'A' && piece <= 'Z' ? piece : piece - 'a' + 'A') {

        // PAWN
        case 'P': {
            int dir = white ? -1 : 1;
            int startRow = white ? 6 : 1;

            // One square forward.
            int nr = r + dir;

            if (insideBoard(nr, c) &&
                isEmptySquare(board[nr][c][0])) {

                board[nr][c][1] = white ? 'O' : 'o';

                // Two squares forward from starting position.
                int nr2 = r + 2 * dir;

                if (r == startRow &&
                    isEmptySquare(board[nr2][c][0])) {
                    board[nr2][c][1] = white ? 'O' : 'o';
                }
            }

            // Diagonal captures.
            for (int dc = -1; dc <= 1; dc += 2) {
                int nc = c + dc;
                nr = r + dir;

                if (!insideBoard(nr, nc)) continue;

                char target = board[nr][nc][0];

                if (!isEmptySquare(target) &&
                    isWhitePiece(target) != white) {
                    board[nr][nc][1] = white ? 'T' : 't';
                }
            }
            break;
        }

        // KNIGHT
        case 'N': {
            const int moves[8][2] = {
                {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
                { 1,-2}, { 1,2}, { 2,-1}, { 2,1}
            };

            for (int i = 0; i < 8; i++) {
                int nr = r + moves[i][0];
                int nc = c + moves[i][1];

                if (!insideBoard(nr, nc)) continue;

                char target = board[nr][nc][0];

                if (isEmptySquare(target) ||
                    isWhitePiece(target) != white) {
                    addMoveOption(nr, nc, white);
                }
            }
            break;
        }

        // BISHOP
        case 'B': {
            const int dirs[4][2] = {
                {-1,-1}, {-1,1}, {1,-1}, {1,1}
            };
            traceMoves(r, c, white, dirs, 4);
            break;
        }

        // ROOK
        case 'R': {
            const int dirs[4][2] = {
                {-1,0}, {1,0}, {0,-1}, {0,1}
            };
            traceMoves(r, c, white, dirs, 4);
            break;
        }

        // QUEEN
        case 'Q': {
            const int dirs[8][2] = {
                {-1,-1}, {-1,1}, {1,-1}, {1,1},
                {-1,0}, {1,0}, {0,-1}, {0,1}
            };
            traceMoves(r, c, white, dirs, 8);
            break;
        }

        // KING
        case 'K': {
            for (int dr = -1; dr <= 1; dr++) {
                for (int dc = -1; dc <= 1; dc++) {
                    if (dr == 0 && dc == 0) continue;

                    int nr = r + dr;
                    int nc = c + dc;

                    if (!insideBoard(nr, nc)) continue;

                    char target = board[nr][nc][0];

                    if (isEmptySquare(target) ||
                        isWhitePiece(target) != white) {
                        addMoveOption(nr, nc, white);
                    }
                }
            }
            break;
        }

        default:
            board[r][c][1] = ((r + c) % 2 == 0) ? 'n' : 'N';
            return false;
    }

    return true;
}