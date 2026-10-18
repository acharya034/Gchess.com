// includes
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

#include <physicalBoard.h>
#include <general.h>
#include <pages.h>

// global variables.
TFT_eSPI display = TFT_eSPI();

TFT_eSprite tft = TFT_eSprite(&display); 

extern uint8_t pageNo;

uint16_t constTouchZ = 600; // this is default value;

// note the time and calculate the fps
ulong fpsTime = 0;

// ===========function decelation.==============
void core0loop();
// void core1loop();

void calFPS();

// pins constant
const int hallpin[3][4] = {
  {13, 12, 14, 27}, // hall sensor positive
  {26, 25, 33, 32}, // hall sensor negitive
  {35, 34, 39, 36}  // hall sensor data pin
};

// one time function
void setup() {
  
  
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 4; j++){
      pinMode(hallpin[i][j], INPUT);
    }
  }
}

// core 1 loop (by default core 1 do the loop job)
void loop() {
  uint16_t touchX;
  uint16_t touchY;

  scanPhyBoard();
  
  physicalLed();
  // ==== the tft part here===
  bool pressed = display.getTouch(&touchX, &touchY, constTouchZ);

  tftInput(pressed, touchX, touchY);
  
  pages[pageNo](pressed, touchX, touchY);
  
  calFPS(); 
  tft.pushSprite(0,0);
  
  pushPageHistory();
}

// core 0 loop for time consuming or freezing task.
/* 
Task of core0:
- send http request:
  using httpstate
*/
void core0loop(){
  while(true){
    
  }
}

void calFPS(){
  int change_in_time = millis() - fpsTime;
  float current_fps = 1000 / change_in_time;

  tft.drawRect(200, 310, 40, 10,TFT_BLACK);
  tft.drawString(String(current_fps) + "fps", 201, 301);

  fpsTime = millis();
}