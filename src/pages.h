#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI display = TFT_eSPI();
extern TFT_eSprite sprite = TFT_eSprite(&display); 

uint8_t pageNo = 0;
uint8_t pagecount = 0;
uint8_t pageHistory[100];

inline void pushPageHistory();
inline void backPage(uint8_t back = 1);

/*
rules while making pages:
    - do the action at the end of last page not at the starting # as pages recalled multiple times which can do the work multiple times.
    - use the fix page struct
        0 -> intro page while all the startup task are done.
        1-100 -> all basic pages which will called from only one place 
        101-200 -> all pages that has more then one calling point.
        201-255 -> all setting pages that can be called from any where.
*/

void page0(uint8_t touchX, uint8_t touchY); // intro page
void page1(uint8_t touchX, uint8_t touchY); // home page
void page2(uint8_t touchX, uint8_t touchY); // puzzle page
void page3(uint8_t touchX, uint8_t touchY); // Watch page
void page4(uint8_t touchX, uint8_t touchY); // setting page
void page5(uint8_t touchX, uint8_t touchY); // play page
void page6(uint8_t touchX, uint8_t touchY); // daily puzzles page
void page7(uint8_t touchX, uint8_t touchY); // 
void page8(uint8_t touchX, uint8_t touchY);
void page9(uint8_t touchX, uint8_t touchY);
void page10(uint8_t touchX, uint8_t touchY);
void page11(uint8_t touchX, uint8_t touchY);
void page12(uint8_t touchX, uint8_t touchY);
void page13(uint8_t touchX, uint8_t touchY);
void page14(uint8_t touchX, uint8_t touchY);
void page15(uint8_t touchX, uint8_t touchY);
void page16(uint8_t touchX, uint8_t touchY);
void page17(uint8_t touchX, uint8_t touchY);
void page18(uint8_t touchX, uint8_t touchY);
void page19(uint8_t touchX, uint8_t touchY);
void page20(uint8_t touchX, uint8_t touchY);
void page21(uint8_t touchX, uint8_t touchY);
void page22(uint8_t touchX, uint8_t touchY);
void page23(uint8_t touchX, uint8_t touchY);
void page24(uint8_t touchX, uint8_t touchY);
void page25(uint8_t touchX, uint8_t touchY);
void page26(uint8_t touchX, uint8_t touchY);
void page27(uint8_t touchX, uint8_t touchY);
void page28(uint8_t touchX, uint8_t touchY);
void page29(uint8_t touchX, uint8_t touchY);
void page30(uint8_t touchX, uint8_t touchY);
void page31(uint8_t touchX, uint8_t touchY);
void page32(uint8_t touchX, uint8_t touchY);
void page33(uint8_t touchX, uint8_t touchY);
void page34(uint8_t touchX, uint8_t touchY);
void page35(uint8_t touchX, uint8_t touchY);
void page36(uint8_t touchX, uint8_t touchY);
void page37(uint8_t touchX, uint8_t touchY);
void page38(uint8_t touchX, uint8_t touchY);
void page39(uint8_t touchX, uint8_t touchY);
void page40(uint8_t touchX, uint8_t touchY);
void page41(uint8_t touchX, uint8_t touchY);
void page42(uint8_t touchX, uint8_t touchY);
void page43(uint8_t touchX, uint8_t touchY);
void page44(uint8_t touchX, uint8_t touchY);
void page45(uint8_t touchX, uint8_t touchY);
void page46(uint8_t touchX, uint8_t touchY);
void page47(uint8_t touchX, uint8_t touchY);
void page48(uint8_t touchX, uint8_t touchY);
void page49(uint8_t touchX, uint8_t touchY);
void page50(uint8_t touchX, uint8_t touchY);
void page51(uint8_t touchX, uint8_t touchY);
void page52(uint8_t touchX, uint8_t touchY);
void page53(uint8_t touchX, uint8_t touchY);
void page54(uint8_t touchX, uint8_t touchY);
void page55(uint8_t touchX, uint8_t touchY);
void page56(uint8_t touchX, uint8_t touchY);
void page57(uint8_t touchX, uint8_t touchY);
void page58(uint8_t touchX, uint8_t touchY);
void page59(uint8_t touchX, uint8_t touchY);
void page60(uint8_t touchX, uint8_t touchY);
void page61(uint8_t touchX, uint8_t touchY);
void page62(uint8_t touchX, uint8_t touchY);
void page63(uint8_t touchX, uint8_t touchY);
void page64(uint8_t touchX, uint8_t touchY);
void page65(uint8_t touchX, uint8_t touchY);
void page66(uint8_t touchX, uint8_t touchY);
void page67(uint8_t touchX, uint8_t touchY);
void page68(uint8_t touchX, uint8_t touchY);
void page69(uint8_t touchX, uint8_t touchY);
void page70(uint8_t touchX, uint8_t touchY);
void page71(uint8_t touchX, uint8_t touchY);
void page72(uint8_t touchX, uint8_t touchY);
void page73(uint8_t touchX, uint8_t touchY);
void page74(uint8_t touchX, uint8_t touchY);
void page75(uint8_t touchX, uint8_t touchY);
void page76(uint8_t touchX, uint8_t touchY);
void page77(uint8_t touchX, uint8_t touchY);
void page78(uint8_t touchX, uint8_t touchY);
void page79(uint8_t touchX, uint8_t touchY);
void page80(uint8_t touchX, uint8_t touchY);
void page81(uint8_t touchX, uint8_t touchY);
void page82(uint8_t touchX, uint8_t touchY);
void page83(uint8_t touchX, uint8_t touchY);
void page84(uint8_t touchX, uint8_t touchY);
void page85(uint8_t touchX, uint8_t touchY);
void page86(uint8_t touchX, uint8_t touchY);
void page87(uint8_t touchX, uint8_t touchY);
void page88(uint8_t touchX, uint8_t touchY);
void page89(uint8_t touchX, uint8_t touchY);
void page90(uint8_t touchX, uint8_t touchY);
void page91(uint8_t touchX, uint8_t touchY);
void page92(uint8_t touchX, uint8_t touchY);
void page93(uint8_t touchX, uint8_t touchY);
void page94(uint8_t touchX, uint8_t touchY);
void page95(uint8_t touchX, uint8_t touchY);
void page96(uint8_t touchX, uint8_t touchY);
void page97(uint8_t touchX, uint8_t touchY);
void page98(uint8_t touchX, uint8_t touchY);
void page99(uint8_t touchX, uint8_t touchY);
void page100(uint8_t touchX, uint8_t touchY);
void page101(uint8_t touchX, uint8_t touchY);
void page102(uint8_t touchX, uint8_t touchY);
void page103(uint8_t touchX, uint8_t touchY);
void page104(uint8_t touchX, uint8_t touchY);
void page105(uint8_t touchX, uint8_t touchY);
void page106(uint8_t touchX, uint8_t touchY);
void page107(uint8_t touchX, uint8_t touchY);
void page108(uint8_t touchX, uint8_t touchY);
void page109(uint8_t touchX, uint8_t touchY);
void page110(uint8_t touchX, uint8_t touchY);
void page111(uint8_t touchX, uint8_t touchY);
void page112(uint8_t touchX, uint8_t touchY);
void page113(uint8_t touchX, uint8_t touchY);
void page114(uint8_t touchX, uint8_t touchY);
void page115(uint8_t touchX, uint8_t touchY);
void page116(uint8_t touchX, uint8_t touchY);
void page117(uint8_t touchX, uint8_t touchY);
void page118(uint8_t touchX, uint8_t touchY);
void page119(uint8_t touchX, uint8_t touchY);
void page120(uint8_t touchX, uint8_t touchY);
void page121(uint8_t touchX, uint8_t touchY);
void page122(uint8_t touchX, uint8_t touchY);
void page123(uint8_t touchX, uint8_t touchY);
void page124(uint8_t touchX, uint8_t touchY);
void page125(uint8_t touchX, uint8_t touchY);
void page126(uint8_t touchX, uint8_t touchY);
void page127(uint8_t touchX, uint8_t touchY);
void page128(uint8_t touchX, uint8_t touchY);
void page129(uint8_t touchX, uint8_t touchY);
void page130(uint8_t touchX, uint8_t touchY);
void page131(uint8_t touchX, uint8_t touchY);
void page132(uint8_t touchX, uint8_t touchY);
void page133(uint8_t touchX, uint8_t touchY);
void page134(uint8_t touchX, uint8_t touchY);
void page135(uint8_t touchX, uint8_t touchY);
void page136(uint8_t touchX, uint8_t touchY);
void page137(uint8_t touchX, uint8_t touchY);
void page138(uint8_t touchX, uint8_t touchY);
void page139(uint8_t touchX, uint8_t touchY);
void page140(uint8_t touchX, uint8_t touchY);
void page141(uint8_t touchX, uint8_t touchY);
void page142(uint8_t touchX, uint8_t touchY);
void page143(uint8_t touchX, uint8_t touchY);
void page144(uint8_t touchX, uint8_t touchY);
void page145(uint8_t touchX, uint8_t touchY);
void page146(uint8_t touchX, uint8_t touchY);
void page147(uint8_t touchX, uint8_t touchY);
void page148(uint8_t touchX, uint8_t touchY);
void page149(uint8_t touchX, uint8_t touchY);
void page150(uint8_t touchX, uint8_t touchY);
void page151(uint8_t touchX, uint8_t touchY);
void page152(uint8_t touchX, uint8_t touchY);
void page153(uint8_t touchX, uint8_t touchY);
void page154(uint8_t touchX, uint8_t touchY);
void page155(uint8_t touchX, uint8_t touchY);
void page156(uint8_t touchX, uint8_t touchY);
void page157(uint8_t touchX, uint8_t touchY);
void page158(uint8_t touchX, uint8_t touchY);
void page159(uint8_t touchX, uint8_t touchY);
void page160(uint8_t touchX, uint8_t touchY);
void page161(uint8_t touchX, uint8_t touchY);
void page162(uint8_t touchX, uint8_t touchY);
void page163(uint8_t touchX, uint8_t touchY);
void page164(uint8_t touchX, uint8_t touchY);
void page165(uint8_t touchX, uint8_t touchY);
void page166(uint8_t touchX, uint8_t touchY);
void page167(uint8_t touchX, uint8_t touchY);
void page168(uint8_t touchX, uint8_t touchY);
void page169(uint8_t touchX, uint8_t touchY);
void page170(uint8_t touchX, uint8_t touchY);
void page171(uint8_t touchX, uint8_t touchY);
void page172(uint8_t touchX, uint8_t touchY);
void page173(uint8_t touchX, uint8_t touchY);
void page174(uint8_t touchX, uint8_t touchY);
void page175(uint8_t touchX, uint8_t touchY);
void page176(uint8_t touchX, uint8_t touchY);
void page177(uint8_t touchX, uint8_t touchY);
void page178(uint8_t touchX, uint8_t touchY);
void page179(uint8_t touchX, uint8_t touchY);
void page180(uint8_t touchX, uint8_t touchY);
void page181(uint8_t touchX, uint8_t touchY);
void page182(uint8_t touchX, uint8_t touchY);
void page183(uint8_t touchX, uint8_t touchY);
void page184(uint8_t touchX, uint8_t touchY);
void page185(uint8_t touchX, uint8_t touchY);
void page186(uint8_t touchX, uint8_t touchY);
void page187(uint8_t touchX, uint8_t touchY);
void page188(uint8_t touchX, uint8_t touchY);
void page189(uint8_t touchX, uint8_t touchY);
void page190(uint8_t touchX, uint8_t touchY);
void page191(uint8_t touchX, uint8_t touchY);
void page192(uint8_t touchX, uint8_t touchY);
void page193(uint8_t touchX, uint8_t touchY);
void page194(uint8_t touchX, uint8_t touchY);
void page195(uint8_t touchX, uint8_t touchY);
void page196(uint8_t touchX, uint8_t touchY);
void page197(uint8_t touchX, uint8_t touchY);
void page198(uint8_t touchX, uint8_t touchY);
void page199(uint8_t touchX, uint8_t touchY);
void page200(uint8_t touchX, uint8_t touchY);
void page201(uint8_t touchX, uint8_t touchY);
void page202(uint8_t touchX, uint8_t touchY);
void page203(uint8_t touchX, uint8_t touchY);
void page204(uint8_t touchX, uint8_t touchY);
void page205(uint8_t touchX, uint8_t touchY);
void page206(uint8_t touchX, uint8_t touchY);
void page207(uint8_t touchX, uint8_t touchY);
void page208(uint8_t touchX, uint8_t touchY);
void page209(uint8_t touchX, uint8_t touchY);
void page210(uint8_t touchX, uint8_t touchY);
void page211(uint8_t touchX, uint8_t touchY);
void page212(uint8_t touchX, uint8_t touchY);
void page213(uint8_t touchX, uint8_t touchY);
void page214(uint8_t touchX, uint8_t touchY);
void page215(uint8_t touchX, uint8_t touchY);
void page216(uint8_t touchX, uint8_t touchY);
void page217(uint8_t touchX, uint8_t touchY);
void page218(uint8_t touchX, uint8_t touchY);
void page219(uint8_t touchX, uint8_t touchY);
void page220(uint8_t touchX, uint8_t touchY);
void page221(uint8_t touchX, uint8_t touchY);
void page222(uint8_t touchX, uint8_t touchY);
void page223(uint8_t touchX, uint8_t touchY);
void page224(uint8_t touchX, uint8_t touchY);
void page225(uint8_t touchX, uint8_t touchY);
void page226(uint8_t touchX, uint8_t touchY);
void page227(uint8_t touchX, uint8_t touchY);
void page228(uint8_t touchX, uint8_t touchY);
void page229(uint8_t touchX, uint8_t touchY);
void page230(uint8_t touchX, uint8_t touchY);
void page231(uint8_t touchX, uint8_t touchY);
void page232(uint8_t touchX, uint8_t touchY);
void page233(uint8_t touchX, uint8_t touchY);
void page234(uint8_t touchX, uint8_t touchY);
void page235(uint8_t touchX, uint8_t touchY);
void page236(uint8_t touchX, uint8_t touchY);
void page237(uint8_t touchX, uint8_t touchY);
void page238(uint8_t touchX, uint8_t touchY);
void page239(uint8_t touchX, uint8_t touchY);
void page240(uint8_t touchX, uint8_t touchY);
void page241(uint8_t touchX, uint8_t touchY);
void page242(uint8_t touchX, uint8_t touchY);
void page243(uint8_t touchX, uint8_t touchY);
void page244(uint8_t touchX, uint8_t touchY);
void page245(uint8_t touchX, uint8_t touchY);
void page246(uint8_t touchX, uint8_t touchY);
void page247(uint8_t touchX, uint8_t touchY);
void page248(uint8_t touchX, uint8_t touchY);
void page249(uint8_t touchX, uint8_t touchY);
void page250(uint8_t touchX, uint8_t touchY);
void page251(uint8_t touchX, uint8_t touchY);
void page252(uint8_t touchX, uint8_t touchY);
void page253(uint8_t touchX, uint8_t touchY);
void page254(uint8_t touchX, uint8_t touchY);
void page255(uint8_t touchX, uint8_t touchY);

void (*pages[])(uint8_t touchX, uint8_t touchY) = {
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

