// includes
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>

#include <physicalBoard.h>
#include <lichessAccount.h>
#include <general.h>
#include <pages.h>

// global variables.
// note the time and calculate the fps
ulong fpsTime = 0;

// ===========function decelation.==============
void core0loop();
// void core1loop();

void calFPS();

// one time function
void setup() {
  introPage(); // the intro page 

  // wifi init and try to connect to a network
  initWifiManager();
  maintainWifiConnection();
  
  beginLichessAccounts();
  physicalBoardSetup();



}

// core 1 loop (by default core 1 do the loop job)
void loop() {
  uint16_t touchX;
  uint16_t touchY;

  scanPhyBoard();
  
  physicalLed();
  // ==== the tft part here===
  display.getTouch(&touchX, &touchY, constTouchZ);
  
  pages[pageNo](touchX, touchY);

  
  calFPS(); 
  sprite.pushSprite(0,0);

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

  sprite.drawRect(200, 310, 40, 10,TFT_BLACK);
  sprite.drawString(String(current_fps) + "fps", 201, 301);

  fpsTime = millis();
}