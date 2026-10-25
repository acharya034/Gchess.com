#include "TFTChessBoard.h"
#include <general.h>
#include <chessEngine.h>

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


inline void drawPiece(int16_t x, int16_t y, uint16_t size, char p, uint16_t bg_color) {
    // this will latter change to bitmap for the piece gui
    if (p == ' ' || p == '\0') return;

    int16_t centerX = x + size / 2;
    int16_t centerY = y + size / 2;

    sprite.setTextDatum(MC_DATUM);
    sprite.fillSmoothCircle(
        centerX - 1,
        centerY - 2,
        size * 0.4,
        TFT_BROWN,
        bg_color
    );

    sprite.setTextColor(
        isupper(p) ? board_lightPieceColor : board_darkPieceColor
    );

    sprite.drawString(String((char)toupper(p)), centerX - 1, centerY + 3, 4);
}

void tftDrawChessBoard(int32_t x, int32_t y, int32_t width) {
    int32_t squareSize = width / 8;



    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            char state = board[i][j][1];

            // // Flip character case if rotated
            // if (screen_rotated) {
            //     if (state >= 'a' && state <= 'z') state -= 32;
            //     else if (state >= 'A' && state <= 'Z') state += 32;
            // }

            uint16_t color;

            switch (state) {
                case 's': color = board_darkSelectedColor;  break;
                case 'S': color = board_lightSelectedColor; break;

                case 'o': color = board_darkOptionColor;     break;
                case 'O': color = board_lightOptionColor;    break;

                case 'c': color = board_darkCheckColor;      break;
                case 'C': color = board_lightCheckColor;     break;

                case 't': color = board_darkTakeableColor;   break;
                case 'T': color = board_lightTakeableColor;  break;

                case 'n': color = board_darkColor;           break;
                case 'N': color = board_lightColor;          break;

                default:  color = board_darkColor;           break;
            }

            // draw square
            sprite.fillRect(x + j * squareSize, y + i * squareSize, squareSize, squareSize, color);

            drawPiece(x + j * squareSize, y + i * squareSize, squareSize, board[i][j][0], color);
        }
    }
}

bool isTftChessBoardTouch(
    int16_t touchX, int16_t touchY,
    int16_t board_x, int16_t board_y,
    int16_t board_width,
    uint8_t &a, uint8_t &b
) {
    int16_t board_sqWidth = board_width / 8;

    if (touchX < board_x || touchX >= board_x + board_width ||
        touchY < board_y || touchY >= board_y + board_width) {
        return false;
    }

    a = (touchX - board_x) / board_sqWidth;
    b = (touchY - board_y) / board_sqWidth;

    return true;
}


