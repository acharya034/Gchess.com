#include <Arduino.h>
#include <TFTInput.h>
#include <TFT_eSPI.h>
#include "pages.h"


void page0(bool pressed, uint8_t x, uint8_t y){
    tft.fillScreen(TFT_CYAN);
    // drawLogo()
}



void pushPageHistory(){
    if (pageHistory[pagecount] != pageNo){
        pageHistory[++pagecount] = pageNo;
    }
}

void backPage(uint8_t back){
    if (back <= pagecount){
        pagecount -= back;
        pageNo = pageHistory[pagecount];
    }
}



