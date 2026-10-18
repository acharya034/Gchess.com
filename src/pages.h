#pragma once
#include <Arduino.h>
#include <TFT_eSPI.h>

extern TFT_eSPI display = TFT_eSPI();
extern TFT_eSprite tft = TFT_eSprite(&display); 

uint8_t pageNo = 0;
uint8_t pagecount = 0;
uint8_t pageHistory[100];

void pushPageHistory();
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

void page0(bool pressed, uint8_t touchX, uint8_t touchY);
void page1(bool pressed, uint8_t touchX, uint8_t touchY);
void page2(bool pressed, uint8_t touchX, uint8_t touchY);
void page3(bool pressed, uint8_t touchX, uint8_t touchY);
void page4(bool pressed, uint8_t touchX, uint8_t touchY);
void page5(bool pressed, uint8_t touchX, uint8_t touchY);
void page6(bool pressed, uint8_t touchX, uint8_t touchY);
void page7(bool pressed, uint8_t touchX, uint8_t touchY);
void page8(bool pressed, uint8_t touchX, uint8_t touchY);
void page9(bool pressed, uint8_t touchX, uint8_t touchY);
void page10(bool pressed, uint8_t touchX, uint8_t touchY);
void page11(bool pressed, uint8_t touchX, uint8_t touchY);
void page12(bool pressed, uint8_t touchX, uint8_t touchY);
void page13(bool pressed, uint8_t touchX, uint8_t touchY);
void page14(bool pressed, uint8_t touchX, uint8_t touchY);
void page15(bool pressed, uint8_t touchX, uint8_t touchY);
void page16(bool pressed, uint8_t touchX, uint8_t touchY);
void page17(bool pressed, uint8_t touchX, uint8_t touchY);
void page18(bool pressed, uint8_t touchX, uint8_t touchY);
void page19(bool pressed, uint8_t touchX, uint8_t touchY);
void page20(bool pressed, uint8_t touchX, uint8_t touchY);
void page21(bool pressed, uint8_t touchX, uint8_t touchY);
void page22(bool pressed, uint8_t touchX, uint8_t touchY);
void page23(bool pressed, uint8_t touchX, uint8_t touchY);
void page24(bool pressed, uint8_t touchX, uint8_t touchY);
void page25(bool pressed, uint8_t touchX, uint8_t touchY);
void page26(bool pressed, uint8_t touchX, uint8_t touchY);
void page27(bool pressed, uint8_t touchX, uint8_t touchY);
void page28(bool pressed, uint8_t touchX, uint8_t touchY);
void page29(bool pressed, uint8_t touchX, uint8_t touchY);
void page30(bool pressed, uint8_t touchX, uint8_t touchY);
void page31(bool pressed, uint8_t touchX, uint8_t touchY);
void page32(bool pressed, uint8_t touchX, uint8_t touchY);
void page33(bool pressed, uint8_t touchX, uint8_t touchY);
void page34(bool pressed, uint8_t touchX, uint8_t touchY);
void page35(bool pressed, uint8_t touchX, uint8_t touchY);
void page36(bool pressed, uint8_t touchX, uint8_t touchY);
void page37(bool pressed, uint8_t touchX, uint8_t touchY);
void page38(bool pressed, uint8_t touchX, uint8_t touchY);
void page39(bool pressed, uint8_t touchX, uint8_t touchY);
void page40(bool pressed, uint8_t touchX, uint8_t touchY);
void page41(bool pressed, uint8_t touchX, uint8_t touchY);
void page42(bool pressed, uint8_t touchX, uint8_t touchY);
void page43(bool pressed, uint8_t touchX, uint8_t touchY);
void page44(bool pressed, uint8_t touchX, uint8_t touchY);
void page45(bool pressed, uint8_t touchX, uint8_t touchY);
void page46(bool pressed, uint8_t touchX, uint8_t touchY);
void page47(bool pressed, uint8_t touchX, uint8_t touchY);
void page48(bool pressed, uint8_t touchX, uint8_t touchY);
void page49(bool pressed, uint8_t touchX, uint8_t touchY);
void page50(bool pressed, uint8_t touchX, uint8_t touchY);
void page51(bool pressed, uint8_t touchX, uint8_t touchY);
void page52(bool pressed, uint8_t touchX, uint8_t touchY);
void page53(bool pressed, uint8_t touchX, uint8_t touchY);
void page54(bool pressed, uint8_t touchX, uint8_t touchY);
void page55(bool pressed, uint8_t touchX, uint8_t touchY);
void page56(bool pressed, uint8_t touchX, uint8_t touchY);
void page57(bool pressed, uint8_t touchX, uint8_t touchY);
void page58(bool pressed, uint8_t touchX, uint8_t touchY);
void page59(bool pressed, uint8_t touchX, uint8_t touchY);
void page60(bool pressed, uint8_t touchX, uint8_t touchY);
void page61(bool pressed, uint8_t touchX, uint8_t touchY);
void page62(bool pressed, uint8_t touchX, uint8_t touchY);
void page63(bool pressed, uint8_t touchX, uint8_t touchY);
void page64(bool pressed, uint8_t touchX, uint8_t touchY);
void page65(bool pressed, uint8_t touchX, uint8_t touchY);
void page66(bool pressed, uint8_t touchX, uint8_t touchY);
void page67(bool pressed, uint8_t touchX, uint8_t touchY);
void page68(bool pressed, uint8_t touchX, uint8_t touchY);
void page69(bool pressed, uint8_t touchX, uint8_t touchY);
void page70(bool pressed, uint8_t touchX, uint8_t touchY);
void page71(bool pressed, uint8_t touchX, uint8_t touchY);
void page72(bool pressed, uint8_t touchX, uint8_t touchY);
void page73(bool pressed, uint8_t touchX, uint8_t touchY);
void page74(bool pressed, uint8_t touchX, uint8_t touchY);
void page75(bool pressed, uint8_t touchX, uint8_t touchY);
void page76(bool pressed, uint8_t touchX, uint8_t touchY);
void page77(bool pressed, uint8_t touchX, uint8_t touchY);
void page78(bool pressed, uint8_t touchX, uint8_t touchY);
void page79(bool pressed, uint8_t touchX, uint8_t touchY);
void page80(bool pressed, uint8_t touchX, uint8_t touchY);
void page81(bool pressed, uint8_t touchX, uint8_t touchY);
void page82(bool pressed, uint8_t touchX, uint8_t touchY);
void page83(bool pressed, uint8_t touchX, uint8_t touchY);
void page84(bool pressed, uint8_t touchX, uint8_t touchY);
void page85(bool pressed, uint8_t touchX, uint8_t touchY);
void page86(bool pressed, uint8_t touchX, uint8_t touchY);
void page87(bool pressed, uint8_t touchX, uint8_t touchY);
void page88(bool pressed, uint8_t touchX, uint8_t touchY);
void page89(bool pressed, uint8_t touchX, uint8_t touchY);
void page90(bool pressed, uint8_t touchX, uint8_t touchY);
void page91(bool pressed, uint8_t touchX, uint8_t touchY);
void page92(bool pressed, uint8_t touchX, uint8_t touchY);
void page93(bool pressed, uint8_t touchX, uint8_t touchY);
void page94(bool pressed, uint8_t touchX, uint8_t touchY);
void page95(bool pressed, uint8_t touchX, uint8_t touchY);
void page96(bool pressed, uint8_t touchX, uint8_t touchY);
void page97(bool pressed, uint8_t touchX, uint8_t touchY);
void page98(bool pressed, uint8_t touchX, uint8_t touchY);
void page99(bool pressed, uint8_t touchX, uint8_t touchY);
void page100(bool pressed, uint8_t touchX, uint8_t touchY);
void page101(bool pressed, uint8_t touchX, uint8_t touchY);
void page102(bool pressed, uint8_t touchX, uint8_t touchY);
void page103(bool pressed, uint8_t touchX, uint8_t touchY);
void page104(bool pressed, uint8_t touchX, uint8_t touchY);
void page105(bool pressed, uint8_t touchX, uint8_t touchY);
void page106(bool pressed, uint8_t touchX, uint8_t touchY);
void page107(bool pressed, uint8_t touchX, uint8_t touchY);
void page108(bool pressed, uint8_t touchX, uint8_t touchY);
void page109(bool pressed, uint8_t touchX, uint8_t touchY);
void page110(bool pressed, uint8_t touchX, uint8_t touchY);
void page111(bool pressed, uint8_t touchX, uint8_t touchY);
void page112(bool pressed, uint8_t touchX, uint8_t touchY);
void page113(bool pressed, uint8_t touchX, uint8_t touchY);
void page114(bool pressed, uint8_t touchX, uint8_t touchY);
void page115(bool pressed, uint8_t touchX, uint8_t touchY);
void page116(bool pressed, uint8_t touchX, uint8_t touchY);
void page117(bool pressed, uint8_t touchX, uint8_t touchY);
void page118(bool pressed, uint8_t touchX, uint8_t touchY);
void page119(bool pressed, uint8_t touchX, uint8_t touchY);
void page120(bool pressed, uint8_t touchX, uint8_t touchY);
void page121(bool pressed, uint8_t touchX, uint8_t touchY);
void page122(bool pressed, uint8_t touchX, uint8_t touchY);
void page123(bool pressed, uint8_t touchX, uint8_t touchY);
void page124(bool pressed, uint8_t touchX, uint8_t touchY);
void page125(bool pressed, uint8_t touchX, uint8_t touchY);
void page126(bool pressed, uint8_t touchX, uint8_t touchY);
void page127(bool pressed, uint8_t touchX, uint8_t touchY);
void page128(bool pressed, uint8_t touchX, uint8_t touchY);
void page129(bool pressed, uint8_t touchX, uint8_t touchY);
void page130(bool pressed, uint8_t touchX, uint8_t touchY);
void page131(bool pressed, uint8_t touchX, uint8_t touchY);
void page132(bool pressed, uint8_t touchX, uint8_t touchY);
void page133(bool pressed, uint8_t touchX, uint8_t touchY);
void page134(bool pressed, uint8_t touchX, uint8_t touchY);
void page135(bool pressed, uint8_t touchX, uint8_t touchY);
void page136(bool pressed, uint8_t touchX, uint8_t touchY);
void page137(bool pressed, uint8_t touchX, uint8_t touchY);
void page138(bool pressed, uint8_t touchX, uint8_t touchY);
void page139(bool pressed, uint8_t touchX, uint8_t touchY);
void page140(bool pressed, uint8_t touchX, uint8_t touchY);
void page141(bool pressed, uint8_t touchX, uint8_t touchY);
void page142(bool pressed, uint8_t touchX, uint8_t touchY);
void page143(bool pressed, uint8_t touchX, uint8_t touchY);
void page144(bool pressed, uint8_t touchX, uint8_t touchY);
void page145(bool pressed, uint8_t touchX, uint8_t touchY);
void page146(bool pressed, uint8_t touchX, uint8_t touchY);
void page147(bool pressed, uint8_t touchX, uint8_t touchY);
void page148(bool pressed, uint8_t touchX, uint8_t touchY);
void page149(bool pressed, uint8_t touchX, uint8_t touchY);
void page150(bool pressed, uint8_t touchX, uint8_t touchY);
void page151(bool pressed, uint8_t touchX, uint8_t touchY);
void page152(bool pressed, uint8_t touchX, uint8_t touchY);
void page153(bool pressed, uint8_t touchX, uint8_t touchY);
void page154(bool pressed, uint8_t touchX, uint8_t touchY);
void page155(bool pressed, uint8_t touchX, uint8_t touchY);
void page156(bool pressed, uint8_t touchX, uint8_t touchY);
void page157(bool pressed, uint8_t touchX, uint8_t touchY);
void page158(bool pressed, uint8_t touchX, uint8_t touchY);
void page159(bool pressed, uint8_t touchX, uint8_t touchY);
void page160(bool pressed, uint8_t touchX, uint8_t touchY);
void page161(bool pressed, uint8_t touchX, uint8_t touchY);
void page162(bool pressed, uint8_t touchX, uint8_t touchY);
void page163(bool pressed, uint8_t touchX, uint8_t touchY);
void page164(bool pressed, uint8_t touchX, uint8_t touchY);
void page165(bool pressed, uint8_t touchX, uint8_t touchY);
void page166(bool pressed, uint8_t touchX, uint8_t touchY);
void page167(bool pressed, uint8_t touchX, uint8_t touchY);
void page168(bool pressed, uint8_t touchX, uint8_t touchY);
void page169(bool pressed, uint8_t touchX, uint8_t touchY);
void page170(bool pressed, uint8_t touchX, uint8_t touchY);
void page171(bool pressed, uint8_t touchX, uint8_t touchY);
void page172(bool pressed, uint8_t touchX, uint8_t touchY);
void page173(bool pressed, uint8_t touchX, uint8_t touchY);
void page174(bool pressed, uint8_t touchX, uint8_t touchY);
void page175(bool pressed, uint8_t touchX, uint8_t touchY);
void page176(bool pressed, uint8_t touchX, uint8_t touchY);
void page177(bool pressed, uint8_t touchX, uint8_t touchY);
void page178(bool pressed, uint8_t touchX, uint8_t touchY);
void page179(bool pressed, uint8_t touchX, uint8_t touchY);
void page180(bool pressed, uint8_t touchX, uint8_t touchY);
void page181(bool pressed, uint8_t touchX, uint8_t touchY);
void page182(bool pressed, uint8_t touchX, uint8_t touchY);
void page183(bool pressed, uint8_t touchX, uint8_t touchY);
void page184(bool pressed, uint8_t touchX, uint8_t touchY);
void page185(bool pressed, uint8_t touchX, uint8_t touchY);
void page186(bool pressed, uint8_t touchX, uint8_t touchY);
void page187(bool pressed, uint8_t touchX, uint8_t touchY);
void page188(bool pressed, uint8_t touchX, uint8_t touchY);
void page189(bool pressed, uint8_t touchX, uint8_t touchY);
void page190(bool pressed, uint8_t touchX, uint8_t touchY);
void page191(bool pressed, uint8_t touchX, uint8_t touchY);
void page192(bool pressed, uint8_t touchX, uint8_t touchY);
void page193(bool pressed, uint8_t touchX, uint8_t touchY);
void page194(bool pressed, uint8_t touchX, uint8_t touchY);
void page195(bool pressed, uint8_t touchX, uint8_t touchY);
void page196(bool pressed, uint8_t touchX, uint8_t touchY);
void page197(bool pressed, uint8_t touchX, uint8_t touchY);
void page198(bool pressed, uint8_t touchX, uint8_t touchY);
void page199(bool pressed, uint8_t touchX, uint8_t touchY);
void page200(bool pressed, uint8_t touchX, uint8_t touchY);
void page201(bool pressed, uint8_t touchX, uint8_t touchY);
void page202(bool pressed, uint8_t touchX, uint8_t touchY);
void page203(bool pressed, uint8_t touchX, uint8_t touchY);
void page204(bool pressed, uint8_t touchX, uint8_t touchY);
void page205(bool pressed, uint8_t touchX, uint8_t touchY);
void page206(bool pressed, uint8_t touchX, uint8_t touchY);
void page207(bool pressed, uint8_t touchX, uint8_t touchY);
void page208(bool pressed, uint8_t touchX, uint8_t touchY);
void page209(bool pressed, uint8_t touchX, uint8_t touchY);
void page210(bool pressed, uint8_t touchX, uint8_t touchY);
void page211(bool pressed, uint8_t touchX, uint8_t touchY);
void page212(bool pressed, uint8_t touchX, uint8_t touchY);
void page213(bool pressed, uint8_t touchX, uint8_t touchY);
void page214(bool pressed, uint8_t touchX, uint8_t touchY);
void page215(bool pressed, uint8_t touchX, uint8_t touchY);
void page216(bool pressed, uint8_t touchX, uint8_t touchY);
void page217(bool pressed, uint8_t touchX, uint8_t touchY);
void page218(bool pressed, uint8_t touchX, uint8_t touchY);
void page219(bool pressed, uint8_t touchX, uint8_t touchY);
void page220(bool pressed, uint8_t touchX, uint8_t touchY);
void page221(bool pressed, uint8_t touchX, uint8_t touchY);
void page222(bool pressed, uint8_t touchX, uint8_t touchY);
void page223(bool pressed, uint8_t touchX, uint8_t touchY);
void page224(bool pressed, uint8_t touchX, uint8_t touchY);
void page225(bool pressed, uint8_t touchX, uint8_t touchY);
void page226(bool pressed, uint8_t touchX, uint8_t touchY);
void page227(bool pressed, uint8_t touchX, uint8_t touchY);
void page228(bool pressed, uint8_t touchX, uint8_t touchY);
void page229(bool pressed, uint8_t touchX, uint8_t touchY);
void page230(bool pressed, uint8_t touchX, uint8_t touchY);
void page231(bool pressed, uint8_t touchX, uint8_t touchY);
void page232(bool pressed, uint8_t touchX, uint8_t touchY);
void page233(bool pressed, uint8_t touchX, uint8_t touchY);
void page234(bool pressed, uint8_t touchX, uint8_t touchY);
void page235(bool pressed, uint8_t touchX, uint8_t touchY);
void page236(bool pressed, uint8_t touchX, uint8_t touchY);
void page237(bool pressed, uint8_t touchX, uint8_t touchY);
void page238(bool pressed, uint8_t touchX, uint8_t touchY);
void page239(bool pressed, uint8_t touchX, uint8_t touchY);
void page240(bool pressed, uint8_t touchX, uint8_t touchY);
void page241(bool pressed, uint8_t touchX, uint8_t touchY);
void page242(bool pressed, uint8_t touchX, uint8_t touchY);
void page243(bool pressed, uint8_t touchX, uint8_t touchY);
void page244(bool pressed, uint8_t touchX, uint8_t touchY);
void page245(bool pressed, uint8_t touchX, uint8_t touchY);
void page246(bool pressed, uint8_t touchX, uint8_t touchY);
void page247(bool pressed, uint8_t touchX, uint8_t touchY);
void page248(bool pressed, uint8_t touchX, uint8_t touchY);
void page249(bool pressed, uint8_t touchX, uint8_t touchY);
void page250(bool pressed, uint8_t touchX, uint8_t touchY);
void page251(bool pressed, uint8_t touchX, uint8_t touchY);
void page252(bool pressed, uint8_t touchX, uint8_t touchY);
void page253(bool pressed, uint8_t touchX, uint8_t touchY);
void page254(bool pressed, uint8_t touchX, uint8_t touchY);
void page255(bool pressed, uint8_t touchX, uint8_t touchY);

void (*pages[])(bool pressed, uint8_t touchX, uint8_t touchY) = {
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

