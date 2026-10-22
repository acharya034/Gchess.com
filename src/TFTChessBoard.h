#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <chessEngine.h>

// Declare the external display variable so the namespace functions can see it
extern TFT_eSPI display = TFT_eSPI();
extern TFT_eSprite sprite = TFT_eSprite(&display); 

bool screen_rotated = false; // 1 false. 3 true

extern char board[8][8][2];

// Theme Customization Colors
// 16-bit RGB565 Hex Codes for TFT_eSPI
uint16_t board_darkColor         = 0x9B84; // Walnut / Brownish Wood
uint16_t board_lightColor        = 0xF7BE; // Maple / Light Tan Wood

uint16_t board_darkPieceColor    = 0x2104; // Charcoal (Soft Deep Black)
uint16_t board_lightPieceColor   = 0xFFDE; // Ivory White

uint16_t board_darkSelectedColor = 0xBDE5; // Antique Pale Blue (Highlight dark square)
uint16_t board_lightSelectedColor= 0xD674; // Lighter Pastel Blue (Highlight light square)

uint16_t board_darkOptionColor   = 0x4CE9; // Muted Green dot/indicator for valid moves
uint16_t board_lightOptionColor  = 0x764D; // Lighter Green dot/indicator for valid moves

uint16_t board_darkCheckColor    = 0xC204; // Crimson Red (Alert state on dark square)
uint16_t board_lightCheckColor   = 0xF3E6; // Pastel Soft Red (Alert state on light square)

uint16_t board_darkTakeableColor = 0xD4A0; // Burnt Ochre/Orange (Target piece can be captured)
uint16_t board_lightTakeableColor= 0xE586; // Muted Coral/Orange (Target piece can be captured)

// --- Function Forward Declarations ---
void tftDrawChessBoard(int32_t x, int32_t y, int32_t width);
void drawPiece(int16_t x, int16_t y, uint16_t size, char p, uint16_t bg_color);




// inline bool promotion(String& moveStr){

//     if (moveStr.length() != 4) return false;

//     int16_t r1, c1, r2, c2, x1, y1, x2, y2;
//     matPosition(r1, c1, moveStr.substring(0, 2), x1, y1);
//     matPosition(r2, c2, moveStr.substring(2, 4), x2, y2);

//     char piece = cfen[r1][c1];

//     if ((r1 != 1 && r1 != 6) || (r2 != 0 && r2 != 7) || piece == ' ' || (piece != 'p' && piece != 'P')) return false ;

//     // move(moveStr.substring(0, 4)); 
//     drawPiece(piece, c2, r2);


//     int16_t X = (c2 < 2) ? board_x : ((c2 > 5) ? board_x + board_sqWidth * 4 : x2 - board_sqWidth * 1.5);
//     int16_t Y = y2;

//     tft.fillSmoothRoundRect(X , Y + board_width* .01, board_sqWidth * 4, board_sqWidth * 0.99, 5, TFT_BROWN);
    
//     int16_t x = X + board_sqWidth / 2;
//     int16_t y = Y + board_sqWidth / 2;
    
//     char promopieces[4] = {'n', 'r', 'q', 'b'};
    
//     for (int i = 0; i < 4; i++){
//         char p = isupper(piece) ? toupper(promopieces[i]) : promopieces[i];
//         drawp(p, x , y, TFT_BROWN);
//         x += board_sqWidth;
//     }
//     tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_CYAN);
//     delay(100);


//     uint16_t tx, ty;
//     while(1){
//         if(tft.getTouch(&tx, &ty)){
//             if (X <= tx && tx <= X + board_sqWidth * 4 && Y <= ty && ty  <= Y + board_sqWidth){
//                 x = X;
//                 for(int i = 0; i < 4; i++){
//                     if (tx >= x){
//                         x += board_sqWidth;
//                         if (tx <= x){
//                             // the actual task
//                             moveStr += promopieces[i];
//                             delay(90);
//                             draw();
//                             return true;
//                         }
//                     }
//                 }
//             }
//             else{
//                 for (int i = 0; i < 3; i++){
//                     tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_BLUE);
//                     delay(50);
//                     tft.drawSmoothRoundRect(X, Y, 5, 3, board_sqWidth * 4, board_sqWidth ,  TFT_CYAN);
//                     delay(100);
//                 }
//             }
//         }
//     }

// }

inline bool isTftChessBoardTouch(
    int16_t touchX, int16_t touchY,
    int16_t board_x, int16_t board_y,
    int16_t board_width,
    int16_t &a, int16_t &b
);