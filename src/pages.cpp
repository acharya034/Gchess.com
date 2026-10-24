#include <Arduino.h>
#include <TFTInput.h>
#include <TFT_eSPI.h>
#include <general.h>
#include <lichessProcessing.h>
#include <lichessAccount.h>
#include "pages.h"
#include <TFTChessBoard.h>
#include <chessEngine.h>

// Page state definitions — define each only once.
uint8_t pageNo = 0;
uint8_t pagecount = 0;
uint8_t pageHistory[100];

extern TFT_eSprite sprite;
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
);


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

void introPage(){
    sprite.fillScreen(TFT_CYAN);

}

// lock page
void page0(uint16_t x, uint16_t y){
    
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
    // Page 7: Play with default mode

    sprite.fillScreen(TFT_BLACK);

    uint16_t button_color = TFT_GREEN;
    uint16_t button_pcolor = TFT_DARKGREEN;
    uint16_t text_color = TFT_WHITE;

    int16_t x = 12, y = 127, w = 216, h = 62;
    uint8_t button_id = 0;

    const char* label = "Play Default";

    uint16_t color = button_color;

    if (touchContained(touchX, touchY, x, y, w, h)) {
        color = button_pcolor;
        buttonPressed(button_id);
    }

    sprite.fillSmoothRoundRect(x, y, w, h, 5, color, TFT_BLACK);
    sprite.drawSmoothRoundRect(x, y, 5, 3, w, h, TFT_WHITE, TFT_BLACK);

    sprite.setTextDatum(MC_DATUM);
    sprite.setTextSize(2);
    sprite.setTextColor(text_color, color);
    sprite.drawString(label, x + w / 2, y + h / 2);

    if (isButtonReleased(button_id)) {
        lichessRequest(CHALLENGE_AI);
        updatePage(100);
    }

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

void page100(uint16_t touchX, uint16_t touchY){
    drawToolbar(touchX, touchY, "waiting");
    sprite.setTextDatum(MC_DATUM);
    sprite.drawString("Waiting for opponent", 100, 200);
}

void page101(uint16_t touchX, uint16_t touchY){
    uint8_t a, b;
    isTftChessBoardTouch(touchX, touchY, 0, 40, 240, a, b);
    calculateMove(a, b);
    drawToolbar(touchX, touchY, "AI");
    tftDrawChessBoard(0, 40, 240);
}

// void pushPageHistory(){
//     if (pageHistory[pagecount] != pageNo){
//         pageHistory[++pagecount] = pageNo;
//     }
// }

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



void (*pages[])(uint16_t touchX, uint16_t touchY) = {
    page0, page1, page2, page3, page4, page5, page6, page7,
    page8, page9, page10, page11, page12, page13, page14, page15,
    page16, page17, page18, page19, page20, page21, page22, page23,
    page24, page25, page26, page27, page28, page29, page30, page31,
    page32, page33, page34, page35, page36, page37, page38, page39,
    page40, page41, page42, page43, page44, page45, page46, page47,
    page48, page49, page50, page51, page52, page53, page54, page55,
    page56, page57, page58, page59, page60, page61, page62, page63,
    page64, page65, page66, page67, page68, page69, page70, page71,
    page72, page73, page74, page75, page76, page77, page78, page79,
    page80, page81, page82, page83, page84, page85, page86, page87,
    page88, page89, page90, page91, page92, page93, page94, page95,
    page96, page97, page98, page99, page100, page101, page102, page103,
    page104, page105, page106, page107, page108, page109, page110, page111,
    page112, page113, page114, page115, page116, page117, page118, page119,
    page120, page121, page122, page123, page124, page125, page126, page127,
    page128, page129, page130, page131, page132, page133, page134, page135,
    page136, page137, page138, page139, page140, page141, page142, page143,
    page144, page145, page146, page147, page148, page149, page150, page151,
    page152, page153, page154, page155, page156, page157, page158, page159,
    page160, page161, page162, page163, page164, page165, page166, page167,
    page168, page169, page170, page171, page172, page173, page174, page175,
    page176, page177, page178, page179, page180, page181, page182, page183,
    page184, page185, page186, page187, page188, page189, page190, page191,
    page192, page193, page194, page195, page196, page197, page198, page199,
    page200, page201, page202, page203, page204, page205, page206, page207,
    page208, page209, page210, page211, page212, page213, page214, page215,
    page216, page217, page218, page219, page220, page221, page222, page223,
    page224, page225, page226, page227, page228, page229, page230, page231,
    page232, page233, page234, page235, page236, page237, page238, page239,
    page240, page241, page242, page243, page244, page245, page246, page247,
    page248, page249, page250, page251, page252, page253, page254, page255
};