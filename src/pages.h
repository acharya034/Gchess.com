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

void page0(uint16_t touchX, uint16_t touchY);
void page1(uint16_t touchX, uint16_t touchY);
void page2(uint16_t touchX, uint16_t touchY);
void page3(uint16_t touchX, uint16_t touchY);
void page4(uint16_t touchX, uint16_t touchY);
void page5(uint16_t touchX, uint16_t touchY);
void page6(uint16_t touchX, uint16_t touchY);
void page7(uint16_t touchX, uint16_t touchY); 
void page8(uint16_t touchX, uint16_t touchY);
void page9(uint16_t touchX, uint16_t touchY);
void page10(uint16_t touchX, uint16_t touchY);
void page11(uint16_t touchX, uint16_t touchY){}
void page12(uint16_t touchX, uint16_t touchY){}
void page13(uint16_t touchX, uint16_t touchY){}
void page14(uint16_t touchX, uint16_t touchY){}
void page15(uint16_t touchX, uint16_t touchY){}
void page16(uint16_t touchX, uint16_t touchY){}
void page17(uint16_t touchX, uint16_t touchY){}
void page18(uint16_t touchX, uint16_t touchY){}
void page19(uint16_t touchX, uint16_t touchY){}
void page20(uint16_t touchX, uint16_t touchY){}
void page21(uint16_t touchX, uint16_t touchY){}
void page22(uint16_t touchX, uint16_t touchY){}
void page23(uint16_t touchX, uint16_t touchY){}
void page24(uint16_t touchX, uint16_t touchY){}
void page25(uint16_t touchX, uint16_t touchY){}
void page26(uint16_t touchX, uint16_t touchY){}
void page27(uint16_t touchX, uint16_t touchY){}
void page28(uint16_t touchX, uint16_t touchY){}
void page29(uint16_t touchX, uint16_t touchY){}
void page30(uint16_t touchX, uint16_t touchY){}
void page31(uint16_t touchX, uint16_t touchY){}
void page32(uint16_t touchX, uint16_t touchY){}
void page33(uint16_t touchX, uint16_t touchY){}
void page34(uint16_t touchX, uint16_t touchY){}
void page35(uint16_t touchX, uint16_t touchY){}
void page36(uint16_t touchX, uint16_t touchY){}
void page37(uint16_t touchX, uint16_t touchY){}
void page38(uint16_t touchX, uint16_t touchY){}
void page39(uint16_t touchX, uint16_t touchY){}
void page40(uint16_t touchX, uint16_t touchY){}
void page41(uint16_t touchX, uint16_t touchY){}
void page42(uint16_t touchX, uint16_t touchY){}
void page43(uint16_t touchX, uint16_t touchY){}
void page44(uint16_t touchX, uint16_t touchY){}
void page45(uint16_t touchX, uint16_t touchY){}
void page46(uint16_t touchX, uint16_t touchY){}
void page47(uint16_t touchX, uint16_t touchY){}
void page48(uint16_t touchX, uint16_t touchY){}
void page49(uint16_t touchX, uint16_t touchY){}
void page50(uint16_t touchX, uint16_t touchY){}
void page51(uint16_t touchX, uint16_t touchY){}
void page52(uint16_t touchX, uint16_t touchY){}
void page53(uint16_t touchX, uint16_t touchY){}
void page54(uint16_t touchX, uint16_t touchY){}
void page55(uint16_t touchX, uint16_t touchY){}
void page56(uint16_t touchX, uint16_t touchY){}
void page57(uint16_t touchX, uint16_t touchY){}
void page58(uint16_t touchX, uint16_t touchY){}
void page59(uint16_t touchX, uint16_t touchY){}
void page60(uint16_t touchX, uint16_t touchY){}
void page61(uint16_t touchX, uint16_t touchY){}
void page62(uint16_t touchX, uint16_t touchY){}
void page63(uint16_t touchX, uint16_t touchY){}
void page64(uint16_t touchX, uint16_t touchY){}
void page65(uint16_t touchX, uint16_t touchY){}
void page66(uint16_t touchX, uint16_t touchY){}
void page67(uint16_t touchX, uint16_t touchY){}
void page68(uint16_t touchX, uint16_t touchY){}
void page69(uint16_t touchX, uint16_t touchY){}
void page70(uint16_t touchX, uint16_t touchY){}
void page71(uint16_t touchX, uint16_t touchY){}
void page72(uint16_t touchX, uint16_t touchY){}
void page73(uint16_t touchX, uint16_t touchY){}
void page74(uint16_t touchX, uint16_t touchY){}
void page75(uint16_t touchX, uint16_t touchY){}
void page76(uint16_t touchX, uint16_t touchY){}
void page77(uint16_t touchX, uint16_t touchY){}
void page78(uint16_t touchX, uint16_t touchY){}
void page79(uint16_t touchX, uint16_t touchY){}
void page80(uint16_t touchX, uint16_t touchY){}
void page81(uint16_t touchX, uint16_t touchY){}
void page82(uint16_t touchX, uint16_t touchY){}
void page83(uint16_t touchX, uint16_t touchY){}
void page84(uint16_t touchX, uint16_t touchY){}
void page85(uint16_t touchX, uint16_t touchY){}
void page86(uint16_t touchX, uint16_t touchY){}
void page87(uint16_t touchX, uint16_t touchY){}
void page88(uint16_t touchX, uint16_t touchY){}
void page89(uint16_t touchX, uint16_t touchY){}
void page90(uint16_t touchX, uint16_t touchY){}
void page91(uint16_t touchX, uint16_t touchY){}
void page92(uint16_t touchX, uint16_t touchY){}
void page93(uint16_t touchX, uint16_t touchY){}
void page94(uint16_t touchX, uint16_t touchY){}
void page95(uint16_t touchX, uint16_t touchY){}
void page96(uint16_t touchX, uint16_t touchY){}
void page97(uint16_t touchX, uint16_t touchY){}
void page98(uint16_t touchX, uint16_t touchY){}
void page99(uint16_t touchX, uint16_t touchY){}
void page100(uint16_t touchX, uint16_t touchY);
void page101(uint16_t touchX, uint16_t touchY);
void page102(uint16_t touchX, uint16_t touchY){}
void page103(uint16_t touchX, uint16_t touchY){}
void page104(uint16_t touchX, uint16_t touchY){}
void page105(uint16_t touchX, uint16_t touchY){}
void page106(uint16_t touchX, uint16_t touchY){}
void page107(uint16_t touchX, uint16_t touchY){}
void page108(uint16_t touchX, uint16_t touchY){}
void page109(uint16_t touchX, uint16_t touchY){}
void page110(uint16_t touchX, uint16_t touchY){}
void page111(uint16_t touchX, uint16_t touchY){}
void page112(uint16_t touchX, uint16_t touchY){}
void page113(uint16_t touchX, uint16_t touchY){}
void page114(uint16_t touchX, uint16_t touchY){}
void page115(uint16_t touchX, uint16_t touchY){}
void page116(uint16_t touchX, uint16_t touchY){}
void page117(uint16_t touchX, uint16_t touchY){}
void page118(uint16_t touchX, uint16_t touchY){}
void page119(uint16_t touchX, uint16_t touchY){}
void page120(uint16_t touchX, uint16_t touchY){}
void page121(uint16_t touchX, uint16_t touchY){}
void page122(uint16_t touchX, uint16_t touchY){}
void page123(uint16_t touchX, uint16_t touchY){}
void page124(uint16_t touchX, uint16_t touchY){}
void page125(uint16_t touchX, uint16_t touchY){}
void page126(uint16_t touchX, uint16_t touchY){}
void page127(uint16_t touchX, uint16_t touchY){}
void page128(uint16_t touchX, uint16_t touchY){}
void page129(uint16_t touchX, uint16_t touchY){}
void page130(uint16_t touchX, uint16_t touchY){}
void page131(uint16_t touchX, uint16_t touchY){}
void page132(uint16_t touchX, uint16_t touchY){}
void page133(uint16_t touchX, uint16_t touchY){}
void page134(uint16_t touchX, uint16_t touchY){}
void page135(uint16_t touchX, uint16_t touchY){}
void page136(uint16_t touchX, uint16_t touchY){}
void page137(uint16_t touchX, uint16_t touchY){}
void page138(uint16_t touchX, uint16_t touchY){}
void page139(uint16_t touchX, uint16_t touchY){}
void page140(uint16_t touchX, uint16_t touchY){}
void page141(uint16_t touchX, uint16_t touchY){}
void page142(uint16_t touchX, uint16_t touchY){}
void page143(uint16_t touchX, uint16_t touchY){}
void page144(uint16_t touchX, uint16_t touchY){}
void page145(uint16_t touchX, uint16_t touchY){}
void page146(uint16_t touchX, uint16_t touchY){}
void page147(uint16_t touchX, uint16_t touchY){}
void page148(uint16_t touchX, uint16_t touchY){}
void page149(uint16_t touchX, uint16_t touchY){}
void page150(uint16_t touchX, uint16_t touchY){}
void page151(uint16_t touchX, uint16_t touchY){}
void page152(uint16_t touchX, uint16_t touchY){}
void page153(uint16_t touchX, uint16_t touchY){}
void page154(uint16_t touchX, uint16_t touchY){}
void page155(uint16_t touchX, uint16_t touchY){}
void page156(uint16_t touchX, uint16_t touchY){}
void page157(uint16_t touchX, uint16_t touchY){}
void page158(uint16_t touchX, uint16_t touchY){}
void page159(uint16_t touchX, uint16_t touchY){}
void page160(uint16_t touchX, uint16_t touchY){}
void page161(uint16_t touchX, uint16_t touchY){}
void page162(uint16_t touchX, uint16_t touchY){}
void page163(uint16_t touchX, uint16_t touchY){}
void page164(uint16_t touchX, uint16_t touchY){}
void page165(uint16_t touchX, uint16_t touchY){}
void page166(uint16_t touchX, uint16_t touchY){}
void page167(uint16_t touchX, uint16_t touchY){}
void page168(uint16_t touchX, uint16_t touchY){}
void page169(uint16_t touchX, uint16_t touchY){}
void page170(uint16_t touchX, uint16_t touchY){}
void page171(uint16_t touchX, uint16_t touchY){}
void page172(uint16_t touchX, uint16_t touchY){}
void page173(uint16_t touchX, uint16_t touchY){}
void page174(uint16_t touchX, uint16_t touchY){}
void page175(uint16_t touchX, uint16_t touchY){}
void page176(uint16_t touchX, uint16_t touchY){}
void page177(uint16_t touchX, uint16_t touchY){}
void page178(uint16_t touchX, uint16_t touchY){}
void page179(uint16_t touchX, uint16_t touchY){}
void page180(uint16_t touchX, uint16_t touchY){}
void page181(uint16_t touchX, uint16_t touchY){}
void page182(uint16_t touchX, uint16_t touchY){}
void page183(uint16_t touchX, uint16_t touchY){}
void page184(uint16_t touchX, uint16_t touchY){}
void page185(uint16_t touchX, uint16_t touchY){}
void page186(uint16_t touchX, uint16_t touchY){}
void page187(uint16_t touchX, uint16_t touchY){}
void page188(uint16_t touchX, uint16_t touchY){}
void page189(uint16_t touchX, uint16_t touchY){}
void page190(uint16_t touchX, uint16_t touchY){}
void page191(uint16_t touchX, uint16_t touchY){}
void page192(uint16_t touchX, uint16_t touchY){}
void page193(uint16_t touchX, uint16_t touchY){}
void page194(uint16_t touchX, uint16_t touchY){}
void page195(uint16_t touchX, uint16_t touchY){}
void page196(uint16_t touchX, uint16_t touchY){}
void page197(uint16_t touchX, uint16_t touchY){}
void page198(uint16_t touchX, uint16_t touchY){}
void page199(uint16_t touchX, uint16_t touchY){}
void page200(uint16_t touchX, uint16_t touchY){}
void page201(uint16_t touchX, uint16_t touchY){}
void page202(uint16_t touchX, uint16_t touchY){}
void page203(uint16_t touchX, uint16_t touchY){}
void page204(uint16_t touchX, uint16_t touchY){}
void page205(uint16_t touchX, uint16_t touchY){}
void page206(uint16_t touchX, uint16_t touchY){}
void page207(uint16_t touchX, uint16_t touchY){}
void page208(uint16_t touchX, uint16_t touchY){}
void page209(uint16_t touchX, uint16_t touchY){}
void page210(uint16_t touchX, uint16_t touchY){}
void page211(uint16_t touchX, uint16_t touchY){}
void page212(uint16_t touchX, uint16_t touchY){}
void page213(uint16_t touchX, uint16_t touchY){}
void page214(uint16_t touchX, uint16_t touchY){}
void page215(uint16_t touchX, uint16_t touchY){}
void page216(uint16_t touchX, uint16_t touchY){}
void page217(uint16_t touchX, uint16_t touchY){}
void page218(uint16_t touchX, uint16_t touchY){}
void page219(uint16_t touchX, uint16_t touchY){}
void page220(uint16_t touchX, uint16_t touchY){}
void page221(uint16_t touchX, uint16_t touchY){}
void page222(uint16_t touchX, uint16_t touchY){}
void page223(uint16_t touchX, uint16_t touchY){}
void page224(uint16_t touchX, uint16_t touchY){}
void page225(uint16_t touchX, uint16_t touchY){}
void page226(uint16_t touchX, uint16_t touchY){}
void page227(uint16_t touchX, uint16_t touchY){}
void page228(uint16_t touchX, uint16_t touchY){}
void page229(uint16_t touchX, uint16_t touchY){}
void page230(uint16_t touchX, uint16_t touchY){}
void page231(uint16_t touchX, uint16_t touchY){}
void page232(uint16_t touchX, uint16_t touchY){}
void page233(uint16_t touchX, uint16_t touchY){}
void page234(uint16_t touchX, uint16_t touchY){}
void page235(uint16_t touchX, uint16_t touchY){}
void page236(uint16_t touchX, uint16_t touchY){}
void page237(uint16_t touchX, uint16_t touchY){}
void page238(uint16_t touchX, uint16_t touchY){}
void page239(uint16_t touchX, uint16_t touchY){}
void page240(uint16_t touchX, uint16_t touchY){}
void page241(uint16_t touchX, uint16_t touchY){}
void page242(uint16_t touchX, uint16_t touchY){}
void page243(uint16_t touchX, uint16_t touchY){}
void page244(uint16_t touchX, uint16_t touchY){}
void page245(uint16_t touchX, uint16_t touchY){}
void page246(uint16_t touchX, uint16_t touchY){}
void page247(uint16_t touchX, uint16_t touchY){}
void page248(uint16_t touchX, uint16_t touchY){}
void page249(uint16_t touchX, uint16_t touchY){}
void page250(uint16_t touchX, uint16_t touchY){}
void page251(uint16_t touchX, uint16_t touchY){}
void page252(uint16_t touchX, uint16_t touchY){}
void page253(uint16_t touchX, uint16_t touchY){}
void page254(uint16_t touchX, uint16_t touchY){}
void page255(uint16_t touchX, uint16_t touchY){}

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

