#include <Arduino.h>
#include <TFTInput.h>
#include <TFT_eSPI.h>
#include <general.h>
#include "pages.h"


void page0(uint16_t x, uint16_t y){
    sprite.fillScreen(TFT_CYAN);
    // drawLogo()
}

void page1(uint16_t touchX, uint16_t touchY){

    // 5 buttons data
    const char* label[5] = {"Puzzles", "Games", "Progress", "Settings", "PLAY"};
    uint16_t button_color[5]  = {TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN};
    uint16_t button_pcolor[5] = {TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN};
    uint16_t text_color[5]    = {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE};

    int16_t x[5] = {12, 126, 12, 126, 12};
    int16_t y[5] = {55, 55, 127, 127, 207};
    int16_t h[5] = {62, 62, 62, 62, 70};
    int16_t w[5] = {102, 102, 102, 102, 216};
    uint8_t button_id[5] = {0, 1, 2, 3, 4};
    uint8_t destPageNo[5] = {2, 3, 4, 5, 6};

    sprite.fillScreen(TFT_BLACK);

    drawButtons(
        button_color, button_pcolor, text_color,
        x, y, w, h,
        button_id, destPageNo, label,
        5, touchX, touchY
    );
    drawToolbar(touchX, touchY, "Home");
    
}

// puzzle page
void page2(uint16_t touchX, uint16_t touchY){
    drawToolbar(touchX, touchY, "Puzzles");
}

// Games page
void page3(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "Games");
}

// progress page
void page4(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "Progress");
}

// setting page
void page5(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "Setting");
}

// Play page
void page6(uint16_t touchX, uint16_t touchY){
    // Play page: AI, Random, Friend, Open
    uint16_t play_button_color[4]  = {TFT_GREEN, TFT_GREEN, TFT_GREEN, TFT_GREEN};
    uint16_t play_button_pcolor[4] = {TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN, TFT_DARKGREEN};
    uint16_t play_text_color[4]    = {TFT_WHITE, TFT_WHITE, TFT_WHITE, TFT_WHITE};

    int16_t play_x[4] = {12, 12, 12, 12};
    int16_t play_y[4] = {55, 127, 199, 271};
    int16_t play_w[4] = {216, 216, 216, 216};
    int16_t play_h[4] = {62, 62, 62, 40};

    uint8_t play_button_id[4] = {0, 1, 2, 3};
    uint8_t play_destPageNo[4] = {7, 8, 9, 10};

    const char* play_label[4] = {"AI", "Random", "Friend", "Open"};

    // Draw play page buttons
    drawButtons(
        play_button_color, play_button_pcolor, play_text_color,
        play_x, play_y, play_w, play_h,
        play_button_id, play_destPageNo, play_label,
        4, touchX, touchY
    );    

    drawToolbar(touchX, touchY, "Play");
    
}

// play ai setting
void page7(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "PlayAI");
}

// play Random setting
void page8(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "Random");
}

// play Friend setting
void page9(uint16_t touchX, uint16_t touchY){
    
    drawToolbar(touchX, touchY, "Friend");
}

// play open setting
void page10(uint16_t touchX, uint16_t touchY){

    drawToolbar(touchX, touchY, "Open");
}



// void pushPageHistory(){
//     if (pageHistory[pagecount] != pageNo){
//         pageHistory[++pagecount] = pageNo;
//     }
// }

void backPage(uint8_t back){
    if (back <= pagecount){
        pagecount -= back;
        pageNo = pageHistory[pagecount];
    }
}

void updatePage(uint8_t givenPage){
    if (givenPage != pageNo){
        pageNo = givenPage;
        pageHistory[++pagecount] = givenPage;
    }
}

void drawButtons(
    uint16_t* button_color,
    uint16_t* button_pcolor,
    uint16_t* text_color,
    int16_t* x, int16_t* y,
    int16_t* w, int16_t* h,
    uint8_t* button_id,
    uint8_t* destPageNo,
    const char** label,
    uint8_t count,
    uint16_t touchX, uint16_t touchY
) {
    for (uint8_t i = 0; i < count; i++) {
        uint16_t color = button_color[i];

        if (touchContained(touchX, touchY, x[i], y[i], w[i], h[i])) {
            color = button_pcolor[i];
            buttonPressed(button_id[i]);
        }

        sprite.fillSmoothRoundRect(
            x[i], y[i], w[i], h[i], 5, color, TFT_BLACK
        );

        sprite.drawSmoothRoundRect(
            x[i], y[i], 5, 3, w[i], h[i], TFT_WHITE, TFT_BLACK
        );

        sprite.setTextDatum(MC_DATUM);
        sprite.setTextSize(2);
        sprite.setTextColor(text_color[i], color);

        sprite.drawString(
            label[i],
            x[i] + w[i] / 2,
            y[i] + h[i] / 2
        );

        if (isButtonReleased(button_id[i])) {
            updatePage(destPageNo[i]);
        }
    }
}


