#pragma once
#include <Arduino.h>
#include <TFTInput.h>

bool lastPressed = false;

extern TFT_eSPI display = TFT_eSPI();
extern TFT_eSprite tft = TFT_eSprite(&display); 

// define max no of buttons and slider as needed for any page.
TFTButton button1(tft);
TFTButton button2(tft);
TFTButton button3(tft);
TFTButton button4(tft);
TFTButton button5(tft);
TFTButton button6(tft);
TFTButton button7(tft);
TFTButton button8(tft);
TFTButton button9(tft);
TFTButton button10(tft);

TFTSlider slider1(tft);
TFTSlider slider2(tft);
TFTSlider slider3(tft);
TFTSlider slider4(tft); 


void tftInput(bool pressed, uint8_t x, uint8_t y);

void drawWifiIndicator(uint8_t signalStrength, int8_t x, int8_t y, uint8_t size, uint16_t bg_color = TFT_BLACK);

void drawBatteryIndicator(uint8_t batteryLevel, int8_t x, int8_t y, uint8_t size, uint16_t bg_color = TFT_BLACK);

