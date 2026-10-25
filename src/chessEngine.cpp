#include "chessEngine.h"
#include <lichessProcessing.h>

// board array for pieces and sq data
char board[8][8][2];

// Compact chess state
char castleRights = 0x0F;


int8_t selectedSq = 99;

char enPassant = '-';  // '-' means unavailable; otherwise file 'a'–'h'
char enPassantRank = '0'; // Target rank '3' or '6'
char sideToMove = 'w';

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
bool calculateMove(int selectedRow, int selectedCol) {
    if (!insideBoard(selectedRow, selectedCol)) {
        return false;
    }

    
    if (selectedSq > 77){
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
    else {
        // 1. Check if the target square is empty ('n' or 'N' for none)
        char sqtype = board[selectedRow][selectedCol][1];
        if (sqtype == 'n' || sqtype == 'N') return false;

        // 2. Decode the starting coordinates
        int8_t fromrow = selectedSq / 10;
        int8_t fromcol = selectedSq % 10; 

        // 3. Execute the move on the board array
        board[selectedRow][selectedCol][0] = board[fromrow][fromcol][0]; 
        board[fromrow][fromcol][0] = ' '; 

        // 4. Construct the UCI move string (e.g., "e2e4") using Arduino String
        String move = "";
        move += (char)('a' + fromcol);
        move += (char)('8' - fromrow); // Assuming row 0 is rank 8
        move += (char)('a' + selectedCol);
        move += (char)('8' - selectedRow);

        return sendMove(move);
    }

}

bool isSquareAttacked(int tr, int tc, bool byWhite) {
    // Pawn attacks
    int pawnRow = tr + (byWhite ? 1 : -1);

    for (int dc = -1; dc <= 1; dc += 2) {
        int c = tc + dc;

        if (insideBoard(pawnRow, c) &&
            board[pawnRow][c][0] == (byWhite ? 'P' : 'p')) {
            return true;
        }
    }

    // Knight attacks
    const int knightMoves[8][2] = {
        {-2,-1}, {-2,1}, {-1,-2}, {-1,2},
        {1,-2}, {1,2}, {2,-1}, {2,1}
    };

    for (int i = 0; i < 8; i++) {
        int r = tr + knightMoves[i][0];
        int c = tc + knightMoves[i][1];

        if (insideBoard(r, c) &&
            board[r][c][0] == (byWhite ? 'N' : 'n')) {
            return true;
        }
    }

    // Enemy king attacks adjacent squares
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;

            int r = tr + dr;
            int c = tc + dc;

            if (insideBoard(r, c) &&
                board[r][c][0] == (byWhite ? 'K' : 'k')) {
                return true;
            }
        }
    }

    // Rook, bishop and queen attacks
    const int dirs[8][2] = {
        {-1,0}, {1,0}, {0,-1}, {0,1},
        {-1,-1}, {-1,1}, {1,-1}, {1,1}
    };

    for (int d = 0; d < 8; d++) {
        int r = tr + dirs[d][0];
        int c = tc + dirs[d][1];

        while (insideBoard(r, c)) {
            char p = board[r][c][0];

            if (!isEmptySquare(p)) {
                if (isWhitePiece(p) == byWhite) {
                    char type = toupper(p);

                    if (type == 'Q' ||
                        (d < 4 && type == 'R') ||
                        (d >= 4 && type == 'B')) {
                        return true;
                    }
                }

                // A piece blocks attacks beyond it.
                break;
            }

            r += dirs[d][0];
            c += dirs[d][1];
        }
    }

    return false;
}

bool loadFEN(String fen) {
    int row = 0, col = 0;
    int pos = 0;

    // Parse piece placement.
    while (pos < fen.length() && fen[pos] != ' ') {
        char ch = fen[pos++];

        if (ch == '/') {
            if (col != 8 || row >= 7) return false;
            row++;
            col = 0;
        }
        else if (ch >= '1' && ch <= '8') {
            int count = ch - '0';
            if (col + count > 8) return false;

            while (count--) {
                board[row][col][0] = '.';
                board[row][col][1] =
                    ((row + col) % 2 == 0) ? 'n' : 'N';
                col++;
            }
        }
        else {
            if (col >= 8 ||
                String("pnbrqkPNBRQK").indexOf(ch) < 0) {
                return false;
            }

            board[row][col][0] = ch;
            board[row][col][1] =
                ((row + col) % 2 == 0) ? 'n' : 'N';
            col++;
        }
    }

    if (row != 7 || col != 8 || pos >= fen.length()) return false;

    // Side to move.
    while (pos < fen.length() && fen[pos] == ' ') pos++;
    if (pos >= fen.length()) return false;

    sideToMove = fen[pos++];
    if (sideToMove != 'w' && sideToMove != 'b') return false;

    // Castling rights.
    while (pos < fen.length() && fen[pos] == ' ') pos++;
    if (pos >= fen.length()) return false;

    castleRights = 0;

    if (fen[pos] == '-') {
        pos++;
    } else {
        while (pos < fen.length() && fen[pos] != ' ') {
            switch (fen[pos++]) {
                case 'K': castleRights |= 0x01; break;
                case 'Q': castleRights |= 0x02; break;
                case 'k': castleRights |= 0x04; break;
                case 'q': castleRights |= 0x08; break;
                default: return false;
            }
        }
    }

    // En passant target square.
    while (pos < fen.length() && fen[pos] == ' ') pos++;
    if (pos >= fen.length()) return false;

    enPassant = '-';
    enPassantRank = '0';

    if (fen[pos] == '-') {
        pos++;
    } else {
        if (pos + 1 >= fen.length()) return false;

        char file = fen[pos++];
        char rank = fen[pos++];

        if (file < 'a' || file > 'h' ||
            (rank != '3' && rank != '6')) {
            return false;
        }

        enPassant = file;
        enPassantRank = rank;
    }

    return true;
}

inline void setNormalSquare(int r, int c) {
    board[r][c][1] = ((r + c) % 2 == 0) ? 'n' : 'N';
}

bool engineMove(String move) {
    move.trim();

    // UCI: e2e4, e7e8q, etc.
    if (move.length() != 4 && move.length() != 5)
        return false;

    int fc = move[0] - 'a';
    int fr = 8 - (move[1] - '0');
    int tc = move[2] - 'a';
    int tr = 8 - (move[3] - '0');

    if (!insideBoard(fr, fc) || !insideBoard(tr, tc))
        return false;

    char piece = board[fr][fc][0];
    char target = board[tr][tc][0];

    if (isEmptySquare(piece)) return false;

    bool white = isWhitePiece(piece);

    // Enforce side to move.
    if (white != (sideToMove == 'w')) return false;

    // Cannot capture own piece.
    if (!isEmptySquare(target) &&
        isWhitePiece(target) == white) {
        return false;
    }

    char type = toupper(piece);
    bool isPawn = type == 'P';
    bool isKing = type == 'K';

    // Promotion character, if supplied.
    char promotion = 0;

    if (move.length() == 5) {
        promotion = move[4];

        if (promotion >= 'A' && promotion <= 'Z')
            promotion += 'a' - 'A';

        if (String("qrbn").indexOf(promotion) < 0)
            return false;

        if (!isPawn || tr != (white ? 0 : 7))
            return false;
    }

    // A pawn reaching the last rank must promote.
    if (isPawn && tr == (white ? 0 : 7) && !promotion)
        return false;

    int dr = tr - fr;
    int dc = tc - fc;
    int dir = white ? -1 : 1;

    // Detect special moves.
    bool castle = isKing && fr == tr && abs(dc) == 2;

    bool enPassantCapture =
        isPawn &&
        abs(dc) == 1 &&
        dr == dir &&
        isEmptySquare(target) &&
        move[2] == enPassant &&
        move[3] == enPassantRank;

    // Validate the en passant victim before changing anything.
    int epCapturedRow = tr - dir;

    if (enPassantCapture) {
        if (board[epCapturedRow][tc][0] != (white ? 'p' : 'P'))
            return false;
    }

    // Validate castling rights and rook position.
    int homeRow = white ? 7 : 0;
    int rookFromCol = dc > 0 ? 7 : 0;
    int rookToCol = dc > 0 ? 5 : 3;
    char rook = white ? 'R' : 'r';

    if (castle) {
        char right = dc > 0
            ? (white ? 0x01 : 0x04)
            : (white ? 0x02 : 0x08);

        if (fr != homeRow || fc != 4 ||
            !(castleRights & right) ||
            board[homeRow][rookFromCol][0] != rook) {
            return false;
        }

        int step = dc > 0 ? 1 : -1;

        for (int c = fc + step; c != rookFromCol; c += step) {
            if (!isEmptySquare(board[homeRow][c][0]))
                return false;
        }

        // King cannot castle out of, through, or into check.
        for (int c = fc; ; c += step) {
            if (isSquareAttacked(homeRow, c, !white))
                return false;

            if (c == tc) break;
        }
    }

    // Save the captured piece before moving.
    char captured = enPassantCapture
        ? board[epCapturedRow][tc][0]
        : target;

    // Update castling rights when a king moves.
    if (piece == 'K') castleRights &= ~0x03;
    if (piece == 'k') castleRights &= ~0x0C;

    // Update rights when a rook moves from its original square.
    if (piece == 'R' && fr == 7 && fc == 0)
        castleRights &= ~0x02;
    if (piece == 'R' && fr == 7 && fc == 7)
        castleRights &= ~0x01;
    if (piece == 'r' && fr == 0 && fc == 0)
        castleRights &= ~0x08;
    if (piece == 'r' && fr == 0 && fc == 7)
        castleRights &= ~0x04;

    // Capturing a rook on its original square removes its rights.
    if (captured == 'R' && tr == 7 && tc == 0)
        castleRights &= ~0x02;
    if (captured == 'R' && tr == 7 && tc == 7)
        castleRights &= ~0x01;
    if (captured == 'r' && tr == 0 && tc == 0)
        castleRights &= ~0x08;
    if (captured == 'r' && tr == 0 && tc == 7)
        castleRights &= ~0x04;

    // En passant expires after this move.
    enPassant = '-';
    enPassantRank = '0';

    // Remove source piece.
    board[fr][fc][0] = '.';

    // Remove the captured pawn for en passant.
    if (enPassantCapture)
        board[epCapturedRow][tc][0] = '.';

    // Place the moving piece, promoting if necessary.
    if (promotion) {
        board[tr][tc][0] = white
            ? toupper(promotion)
            : promotion;
    } else {
        board[tr][tc][0] = piece;
    }

    // Move the rook during castling.
    if (castle) {
        board[homeRow][rookToCol][0] = rook;
        board[homeRow][rookFromCol][0] = '.';
    }

    // Record the en passant target after a two-square pawn move.
    if (isPawn && abs(dr) == 2) {
        enPassant = move[0];
        enPassantRank = '0' + (8 - ((fr + tr) / 2));
    }

    // Restore display states to normal.
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++)
            setNormalSquare(r, c);
    }

    // Switch side to move.
    sideToMove = white ? 'b' : 'w';

    return true;
}

bool setSideToMove(const char* text){
    if (strcmp(text, "white") == 0 || strcmp(text, "w") == 0){
        sideToMove = 'w';
        return true;
    }
    else if (strcmp(text, "black") == 0 || strcmp(text, "b") == 0) { 
        sideToMove = 'b'; 
        return true; 
    }
    return false;
}
