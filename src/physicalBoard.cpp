#include "physicalBoard.h"

// Initialize hardware pins and FastLED matrix
void physicalBoardSetup() {
  // 1. Initialize FastLED with your specifications
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();

  // 2. Configure Hall sensor matrix pins using a loop
  for (int i = 0; i < 4; i++) {
    pinMode(hallpin[0][i], OUTPUT); // Positive power lines
    pinMode(hallpin[1][i], OUTPUT); // Negative ground lines
    pinMode(hallpin[2][i], INPUT);  // Data read lines
  }
}

void scanPhyBoard(){
    for(int hallp = 0; hallp < 4; hallp++){
        pinMode(hallpin[0][hallp], OUTPUT);
        digitalWrite(hallpin[0][hallp], LOW);
        
    for(int halln = 0; halln < 4; halln++){
        pinMode(hallpin[1][halln], OUTPUT);
        digitalWrite(hallpin[1][halln], HIGH);
        
      for(int halld = 0; halld < 4; halld++){
          hallSensor[hallp][halln][halld] = digitalRead(hallpin[2][halld]);
        }
        
        digitalWrite(hallpin[1][halln], LOW);
        pinMode(hallpin[1][halln], INPUT);
    }
    
    digitalWrite(hallpin[0][hallp], HIGH);
    pinMode(hallpin[0][hallp], INPUT);
  }
}  

void physicalLed() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {

            int ledIndex = (j % 2 == 0)
                         ? j * 8 + i
                         : 8 * j + (7 - i);

            CRGB color;

            switch (board[i][j][1]) {

                case 's':  // Black selected
                case 'S':  // White selected
                    color = CRGB::Yellow;
                    break;

                case 'o':  // Black move option
                case 'O':  // White move option
                    color = CRGB::Green;
                    break;

                case 'c':  // Black in check
                case 'C':  // White in check
                    color = CRGB::Red;
                    break;

                case 't':  // Black capture available
                case 'T':  // White capture available
                    color = CRGB::Orange;
                    break;

                case 'n':  // Normal square
                case 'N':
                default:   // Unknown or empty state
                    color = CRGB::Black;
                    break;
            }

            leds[ledIndex] = color;
        }
    }

    FastLED.show();
}

void setBoardLedBrightness(uint8_t brightness) {
  // Sets the master brightness limit for the entire LED matrix (0 = off, 255 = full brightness)
  FastLED.setBrightness(brightness);
  FastLED.show();
}
