#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <chessEngine.h>
#include <TFT_eSPI.h>

// Pins constants

extern bool grid8x8[8][8];


// Function declarations
void physicalBoardSetup();
void scanPhyBoard();
void physicalLed();
void setBoardLedBrightness(uint8_t brightness);
