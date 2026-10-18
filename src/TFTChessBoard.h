#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

// Declare the external display variable so the namespace functions can see it
extern TFT_eSPI tft;
// Core Board Settings
inline int16_t board_x = 0;
inline int16_t board_y = 30;
inline int16_t board_width = 240;
inline int16_t board_sqWidth = 30;

// Theme Customization Colors
inline uint16_t board_darkColor = TFT_GREENYELLOW;
inline uint16_t board_lightColor = TFT_SILVER;
inline uint16_t board_darkPieceColor = TFT_BLACK;
inline uint16_t board_lightPieceColor = TFT_WHITE;

inline bool board_sideToMove = true;
inline bool board_rotated = false;

// Matrix Memory (Declared inline so it behaves correctly across files)
inline char pfen[8][8];
inline char cfen[8][8];
inline bool isHighlight[8][8];

// --- Function Forward Declarations ---
bool setFEN(String fen);
void drawPiece(char p, int16_t x, int16_t y);
void matPosition(int16_t& row, int16_t& col, String square, int16_t& x, int16_t& y);
void saveFEN();

// --- Function Implementations ---
inline void init(int16_t x = 0, int16_t y = 30, int16_t width = 240,
                    uint16_t darkColor = TFT_GREENYELLOW, uint16_t lightColor = TFT_SILVER,
                    uint16_t darkPieceColor = TFT_BLACK, uint16_t lightPieceColor = TFT_WHITE) {
                        
                        memset(cfen, ' ', sizeof(cfen));
                        memset(pfen, ' ', sizeof(pfen));
    memset(isHighlight, false, sizeof(isHighlight));
    
    board_x = x;
    board_y = y;
    board_width = width;
    board_sqWidth = width / 8;
    
    board_darkColor = darkColor;
    board_lightColor = lightColor;
    board_darkPieceColor = darkPieceColor;
    board_lightPieceColor = lightPieceColor;
    
    setFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

inline bool draw() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            drawPiece(cfen[i][j], j, i);
        }
    }
    saveFEN();
    return true;
}

inline bool update() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (cfen[i][j] != pfen[i][j]) {
                drawPiece(cfen[i][j], j, i);
            }
        }
    }
    saveFEN();
    return true;
}

inline bool setFEN(String fen) {
    int x = 0;
    int i = 0; 
    int j = 0; 

    memset(cfen, ' ', sizeof(cfen));

    while (x < fen.length()) {
        char c = fen[x++];
        
        if (c == '/') {
            i++;
            j = 0; 
            if (i >= 8) return false; 
            continue;
        }
        if (c == ' ') break;

        if (c >= '1' && c <= '8') {
            int blankSquares = c - '0';
            for (int a = 0; a < blankSquares; a++) {
                if (j < 8) {
                    cfen[i][j++] = ' ';
                }
            }
        } else {
            if (j < 8 && i < 8) {
                cfen[i][j++] = c;
            }
        }
    }

    if (x < fen.length()) {
        char turn = fen[x];
        if (turn == 'w') board_sideToMove = true;
        else if (turn == 'b') board_sideToMove = false;
    }
    // draw();
    return true;
}
inline void drawp(char p, int16_t X, int16_t Y, uint16_t bg_color){
    tft.setTextDatum(MC_DATUM);
    tft.fillSmoothCircle(X - 1, Y - 2, board_sqWidth * 0.4, TFT_BROWN, bg_color);
    tft.setTextColor(isupper(p) ? board_lightPieceColor : board_darkPieceColor);
    tft.drawString(String((char)toupper(p)), X -1, Y + 3, 4);
}

inline void drawPiece(char p, int16_t x, int16_t y) {
    int16_t X = board_x + x * board_sqWidth;
    int16_t Y = board_y + y * board_sqWidth; 
    
    bool isLightSquare = ((x + y) % 2 == 0);
    uint16_t fieldColor = isLightSquare ? board_lightColor : board_darkColor;
    tft.fillRect(X, Y, board_sqWidth, board_sqWidth, fieldColor);
    
    if (p == ' ' || p == 0) return; 
    
    X += (board_sqWidth / 2);  
    Y += (board_sqWidth / 2);
    
    drawp(p, X, Y, fieldColor);
}

inline bool promotion(String& moveStr){

    if (moveStr.length() != 4) return false;

    int16_t r1, c1, r2, c2, x1, y1, x2, y2;
    matPosition(r1, c1, moveStr.substring(0, 2), x1, y1);
    matPosition(r2, c2, moveStr.substring(2, 4), x2, y2);

    char piece = cfen[r1][c1];

    if ((r1 != 1 && r1 != 6) || (r2 != 0 && r2 != 7) || piece == ' ' || (piece != 'p' && piece != 'P')) return false ;

    // move(moveStr.substring(0, 4)); 
    drawPiece(piece, c2, r2);


    int16_t X = (c2 < 2) ? board_x : ((c2 > 5) ? board_x + board_sqWidth * 4 : x2 - board_sqWidth * 1.5);
    int16_t Y = y2;

    tft.fillSmoothRoundRect(X , Y + board_width* .01, board_sqWidth * 4, board_sqWidth * 0.99, 5, TFT_BROWN);
    
    int16_t x = X + board_sqWidth / 2;
    int16_t y = Y + board_sqWidth / 2;
    
    char promopieces[4] = {'n', 'r', 'q', 'b'};
    
    for (int i = 0; i < 4; i++){
        char p = isupper(piece) ? toupper(promopieces[i]) : promopieces[i];
        drawp(p, x , y, TFT_BROWN);
        x += board_sqWidth;
    }
    tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_CYAN);
    delay(100);


    uint16_t tx, ty;
    while(1){
        if(tft.getTouch(&tx, &ty)){
            if (X <= tx && tx <= X + board_sqWidth * 4 && Y <= ty && ty  <= Y + board_sqWidth){
                x = X;
                for(int i = 0; i < 4; i++){
                    if (tx >= x){
                        x += board_sqWidth;
                        if (tx <= x){
                            // the actual task
                            moveStr += promopieces[i];
                            delay(90);
                            draw();
                            return true;
                        }
                    }
                }
            }
            else{
                for (int i = 0; i < 3; i++){
                    tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_BLUE);
                    delay(50);
                    tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_CYAN);
                    delay(100);
                }
            }
        }
    }

}

inline bool move(String moveStr) {
    if (moveStr.length() < 4 || moveStr.length() > 5) return false;

    int16_t r1, c1, r2, c2, x, y;
    matPosition(r1, c1, moveStr.substring(0, 2), x, y);
    matPosition(r2, c2, moveStr.substring(2, 4), x, y);
    
    if(r1 < 0 || r1 > 7 || c1 < 0 || c1 > 7 || r2 < 0 || r2 > 7 || c2 < 0 || c2 > 7) {
        return false;
    }

    if (cfen[r1][c1] == ' ') return false;
    
    char piece = cfen[r1][c1];
    cfen[r1][c1] = ' ';
    
    if (moveStr.length() == 5) {
        char promo = moveStr[4];
        cfen[r2][c2] = isupper(piece) ? toupper(promo) : tolower(promo);
    } else {
        cfen[r2][c2] = piece;
    }
    
    // update();
    return true;
}

inline void matPosition(int16_t& row, int16_t& col, String square, int16_t& x, int16_t& y) {
    col = square.charAt(0) - 'a';          
    row = 7 - (square.charAt(1) - '1');    
    x = board_x + col * board_sqWidth;
    y = board_y + row * board_sqWidth;
}

inline void saveFEN() {
    memcpy(pfen, cfen, sizeof(cfen));
}

inline void highlight(String sq, uint16_t highlightColor) {
    int16_t x, y;
    int16_t r, c;
    matPosition(r, c, sq.substring(0, 2), x, y);
    isHighlight[r][c] = true;
    tft.drawRect(x, y, board_sqWidth, board_sqWidth, highlightColor);
    tft.drawRect(x + 1, y + 1, board_sqWidth - 2, board_sqWidth - 2, highlightColor);
    tft.drawRect(x + 2, y + 2, board_sqWidth - 4, board_sqWidth - 4, highlightColor);
}

inline void highlight(String square[], uint8_t size, uint16_t highlightColor) {
    for(int i = 0; i < size ; i++){
        highlight(square[i], highlightColor);
    }
}

inline void clearHighlight() {
    for (int r = 0; r < 8; r++){
        for (int c = 0; c < 8; c++){
            if (isHighlight[r][c]){
                isHighlight[r][c] = false;
                drawPiece(cfen[r][c], c, r);
            }
        }
    }
}

inline bool isBoardTouch(int16_t x, int16_t y, String &square) {
    if (x < board_x || x > board_x + board_width || y < board_y || y > board_y + board_width) return false;

    int16_t a = (x - board_x) / board_sqWidth;
    int16_t b = 7 - (y - board_y) / board_sqWidth;

    square = (char)('a' + a);
    square += (char)('1' + b);
    return true; 
}
inline bool undoMove(String moveStr){
    String sq1 = moveStr.substring(0, 2);
    String sq2 = moveStr.substring(2, 4);
    return move(sq2 + sq1);
}

