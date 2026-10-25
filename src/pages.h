#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

extern uint8_t pageNo;
extern uint8_t pagecount;
extern uint8_t pageHistory[100];

void updatePage(uint8_t givenPage);
void backPage(uint8_t back = 1);

/*
rules while making pages:
    - do the action at the end of last page not at the starting # as pages recalled multiple times which can do the work multiple times.
    - use the fix page struct
        0 -> intro page while all the startup task are done.
        1-100 -> all basic pages which will called from only one place 
        101-200 -> all pages that has more then one calling point.
        201-255 -> all setting pages that can be called from any where.
*/
/* page details:
    0: intro page
    1: home page
    2: puzzle page
    3: game page
    4: progress page
    5: setting page
    6: play page
    7: ai play setting
    8: random play setting
    9: friend play setting
    10: open play setting
    

*/

// todo: those defination just takes flask memory space the better apporach is to get the index and generate a function name.
void introPage();

extern void (*pages[])(uint16_t touchX, uint16_t touchY);



// enum pageStatusEnum {IDEAL, PRESSACTION, PRESSED, RELEASEACTION};
// pageStatusEnum pageStatus;
// // bool lastPressed = false;
// int8_t repectedTime = 2; // reaction time * 50 mill + 200 (min 0 max 9) and +200 for first reaction;

// unsigned long lastActionTime = millis();
// void tftpage(bool pressed = false, uint8_t touchX = 0, uint8_t touchY = 0){
    //     if (pageStatus == RELEASEACTION && !pressed){
//         pageStatus = IDEAL;
//     } 
//     else if (pageStatus == IDEAL && pressed){
//         pageStatus = PRESSACTION;
//         lastActionTime = millis() - 200; // 200 more for first repected action;
//     } 
//     else if (pageStatus == PRESSACTION && pressed){
//         pageStatus = PRESSED;
//     } 
//     else if (pageStatus == PRESSED && !pressed){
//         pageStatus = RELEASEACTION;
//     } 

//     // do the action after every some intervel if its stay pressed 
//     if (pageStatus == PRESSED && lastActionTime - millis() >= 200 + repectedTime * 50){
//         pageStatus = PRESSACTION;
//         lastActionTime = millis();
//     }

//     pages[pageNo](pageStatus, touchX, touchY);
// }

