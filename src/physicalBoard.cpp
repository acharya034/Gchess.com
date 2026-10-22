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
                // ================= SELECTED STATES =================
                case 's': // Black / Dark Selected
                    color = CRGB(184, 220, 40);   // Matches board_darkSelectedColor (0xBDE5)
                    break;
                case 'S': // White / Light Selected
                    color = CRGB(216, 206, 160);  // Matches board_lightSelectedColor (0xD674)
                    break;

                // ================= MOVE OPTION STATES =================
                case 'o': // Black / Dark Move Option
                    color = CRGB(72, 157, 72);    // Matches board_darkOptionColor (0x4CE9)
                    break;
                case 'O': // White / Light Move Option
                    color = CRGB(112, 201, 104);  // Matches board_lightOptionColor (0x764D)
                    break;

                // ================= CHECK STATES =================
                case 'c': // Black / Dark King in Check
                    color = CRGB(192, 40, 32);    // Matches board_darkCheckColor (0xC204)
                    break;
                case 'C': // White / Light King in Check
                    color = CRGB(240, 124, 48);   // Matches board_lightCheckColor (0xF3E6)
                    break;

                // ================= CAPTURE STATES =================
                case 't': // Black / Dark Capture Available
                    color = CRGB(208, 148, 0);    // Matches board_darkTakeableColor (0xD4A0)
                    break;
                case 'T': // White / Light Capture Available
                    color = CRGB(224, 176, 48);   // Matches board_lightTakeableColor (0xE586)
                    break;

                // ================= NORMAL / OFF STATES =================
                case 'n': // Normal square -> LED Off
                case 'N': // Normal square -> LED Off
                default:  // Catch-all for empty or unknown states
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


// function to read the board (call this function in loop)

bool scanPhysicalBoard(int &rowIndex, int &colIndex);