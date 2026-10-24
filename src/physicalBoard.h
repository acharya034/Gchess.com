#pragma once
#include <Arduino.h>
#include <FastLED.h>
#include <chessEngine.h>
#include <TFT_eSPI.h>

// global variables.
TFT_eSPI display = TFT_eSPI();

TFT_eSprite tft = TFT_eSprite(&display); 


// Pins constants
const int hallpin[3][4] = {
  {13, 12, 14, 27}, // Hall sensor positive / power pins
  {26, 25, 33, 32}, // Hall sensor negative / ground pins
  {35, 34, 39, 36}  // Hall sensor data pins
};

extern char board[8][8][2];

// LED setup configuration
const int ledPin = 1;       // Pin connected to the LED data line
const int NUM_LEDS = 64;    // 8x8 chessboard matrix = 64 LEDs
CRGB leds[NUM_LEDS];        // Corrected type from 'fastle' to 'CRGB'

// Variables and state arrays
bool hallSensor[4][4][4];   

// Function declarations
void physicalBoardSetup();
void scanPhyBoard();
void physicalLed();
void setBoardLedBrightness(uint8_t brightness);
