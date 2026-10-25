#include "physicalBoard.h"
#include <general.h>

uint8_t rotation = 0;

// LED setup configuration
const int ledPin = 1;       // Pin connected to the LED data line
CRGB leds[64];        // Corrected type from 'fastle' to 'CRGB'

// Variables and state arrays
bool hallSensor[4][4][4];  
bool grid8x8[8][8];

// Initialize hardware pins and FastLED matrix
void physicalBoardSetup() {
  // 1. Initialize FastLED with your specifications
  FastLED.addLeds<WS2812B, ledPin, GRB>(leds, 64);
  FastLED.clear();
  FastLED.show();

  // this may not be done on setup as this is first set and then operation on the the scan board loop 
//     const int hallpin[3][4] = {
//         {13, 12, 14, 27}, // Hall sensor positive / power pins
//         {26, 25, 33, 32}, // Hall sensor negative / ground pins
//         {35, 34, 39, 36}  // Hall sensor data pins
//     };

//   // 2. Configure Hall sensor matrix pins using a loop
//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 3; j++){
//         pinMode(hallpin[j][i], INPUT);
//     }
//   }

  // 3. Configure TFT screen and touch.
    display.init();
    display.setRotation(0);
    display.fillScreen(TFT_BLACK);

  // 4. Setup the virtual sprite memory second
    sprite.setColorDepth(16);          // 16-bit color matches standard TFTs
    sprite.createSprite(240, 360);      // Width and Height of your sprite in pixels
    
    uint16_t calData[] = { 426, 3465, 299, 3463, 7 };
    display.setTouch(calData); 
}

void scanPhyBoard(){

    const int hallpin[3][4] = {
        {13, 12, 14, 27}, // Hall sensor positive / power pins
        {26, 25, 33, 32}, // Hall sensor negative / ground pins
        {35, 34, 39, 36}  // Hall sensor data pins
    };
    
    // Ensure your 2D grid is defined globally or before this loop:

    for(int hallp = 0; hallp < 4; hallp++){
        pinMode(hallpin[0][hallp], OUTPUT);
        digitalWrite(hallpin[0][hallp], LOW);

        for(int halln = 0; halln < 4; halln++){
            pinMode(hallpin[1][halln], OUTPUT);
            digitalWrite(hallpin[1][halln], HIGH);
            for(int halld = 0; halld < 4; halld++){
                pinMode(hallpin[2][halld], INPUT);
                
                // Read the physical sensor state
                bool sensorValue = digitalRead(hallpin[2][halld]);
                
                // Calculate 2D destination coordinates based on quadrant (halld)
                // halld = 0 or 1 -> Row offset 0 | halld = 2 or 3 -> Row offset 4
                int targetRow = hallp + ((halld >= 2) ? 4 : 0);
                
                // halld = 0 or 2 -> Col offset 0 | halld = 1 or 3 -> Col offset 4
                int targetCol = halln + ((halld % 2 == 1) ? 4 : 0);
                
                // Save directly to the 8x8 matrix
                grid8x8[targetRow][targetCol] = sensorValue;
                
                // (Optional) You can still keep your 3D array if needed:
                // hallSensor[hallp][halln][halld] = sensorValue;
            }

            // digitalWrite(hallpin[1][halln], LOW); // Lowing the power is useless as nextstep disconnect at all.
            pinMode(hallpin[1][halln], INPUT);

        }
        
        // digitalWrite(hallpin[0][hallp], HIGH);
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