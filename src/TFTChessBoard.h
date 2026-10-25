#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

// --- Function Forward Declarations ---
void tftDrawChessBoard(int32_t x, int32_t y, int32_t width);

bool isTftChessBoardTouch(
    int16_t touchX, int16_t touchY,
    int16_t board_x, int16_t board_y,
    int16_t board_width,
    uint8_t &a, uint8_t &b
);


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