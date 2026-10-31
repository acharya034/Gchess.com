// =====================================================================================
//  Gchess  -  smart chessboard firmware for ESP32  (Arduino framework, core 3.x)
//
//  HOW THE PROGRAM IS ORGANISED
//    CORE 0  hwTask : reads the sensors, drives LEDs and buzzer, watches the power button
//    CORE 0  uiTask : touch screen, pages, game logic, notifications
//    CORE 1  netTask: Wi-Fi, Lichess streams, requests, account linking
//  The two cores only talk through two queues (see events.h):
//    netCmd(...)  UI  -> network        uiPost(...)  network -> UI
//
//  FILES (all in the same folder)
//    gchess_config.h  pins, timings, debug switches   <- edit this first
//    debug.h          Serial.print helpers (DBG macro)
//    events.h         messages between the cores
//    storage.h        permanent memory (settings, accounts, Wi-Fi list)
//    analytics.h      usage counters (pages, buttons, time)
//    sound.h          buzzer melodies
//    power.h          one-pin power latch, button, battery, charger
//    leds.h           64 square LEDs
//    board.h          4x4x4 hall sensor matrix
//    chess_rules.h    chess rules (moves, check, FEN, SAN) - no hardware inside
//    game.h           joins sensors + touch + LEDs + rules into "games"
//    gfx.h            display driver, colours, pieces, logos
//    ui.h             page system, toolbar, quick menu, dialogs, keyboard
//    net.h            Wi-Fi and Lichess client
//    pages.h          every screen of the device
// =====================================================================================
#include "gchess_config.h"
#include "debug.h"
#include "events.h"
#include "storage.h"
#include "analytics.h"
#include "sound.h"
#include "power.h"
#include "leds.h"
#include "board.h"
#include "gfx.h"
#include "ui.h"
#include "game.h"
#include "net.h"
#include "pages.h"

// Drawn by powerOff() just before the board switches itself off.
static void goodbye() {
  tft.setBrightness(gSet.tftBright);
  tft.fillScreen(T.bg);
  logoMedium(120, 130);
  gfxText(120, 190, "See you soon!", 2, T.fg, TD_TC);
  gfxText(120, 224, "Saving your data...", 1, T.dim, TD_TC);
}

// Hardware task (core 0): must never wait for anything, so it can run about 1000 times a second.
static void hwTask(void*) {
  for (;;) {
    boardScan();       // hall sensors
    soundTick();       // next buzzer note
    ledsTick();        // refresh LEDs when something changed
    powerTick();       // button, battery, charger
    analyticsTick();   // save usage counters now and then
    vTaskDelay(1);
  }
}

// User interface task (core 0): about 60 times a second.
static void uiTask(void*) {
  uint32_t lastReport = 0;
  for (;;) {
    uiTouchPump();     // read the touch screen
    uiPumpEvents();    // messages from the network, power button, notifications
    physTick();        // the game that owns the physical board is always watched, on every page
    if (!gScreenOff && gModal == MD_NONE && gPages[gCur].tick) gPages[gCur].tick();   // page animations / redraws

    // Every 10 seconds: memory report. Falling "min_ever" numbers = a memory leak.
    if (millis() - lastReport > 10000) {
      lastReport = millis();
      dbgHeap("ui");
      DBG(SYS, "stack left (words): ui=%u hw=? page=%s wifi=%d games(on board)=%d", (unsigned)uxTaskGetStackHighWaterMark(nullptr), gPages[gCur].name, (int)WiFi.status(), (int)(physLive() != nullptr));
    }
    vTaskDelay(pdMS_TO_TICKS(15));
  }
}

void setup() {
  powerBegin();            // FIRST: hold the power latch so releasing the button does not switch us off
  dbgBegin();              // Serial monitor at 115200
  DBG(SYS, "==== %s v%s booting, reset reason %d, cpu %d MHz ====", GC_NAME, GC_VERSION, (int)esp_reset_reason(), (int)getCpuFrequencyMhz());
  dbgHeap("boot");

  storageBegin();          // settings / accounts / Wi-Fi from flash
  analyticsBegin();
  soundBegin();
  ledsBegin();
  boardBegin();
  gfxBegin();              // display + touch (first boot: touch calibration screen)
  gBeforeOff = goodbye;

  // Queues and the mutex that protect data shared by the two cores
  qNet = xQueueCreate(16, sizeof(NetCmd));
  qUi = xQueueCreate(24, sizeof(UiEvt));
  gNetMux = xSemaphoreCreateMutex();
  memset(gNetG, 0, sizeof(gNetG));
  memset(&gNetX, 0, sizeof(gNetX));

  gamesInit();             // allocate the 3 game slots
  pagesBegin();            // register every page
  if (!gSet.wifiOn) netCmd(NC_WIFI_ENABLE, "", "", 0);

  // Start on the intro page
  gCur = PG_INTRO;
  analyticsPage(PG_INTRO);
  gPages[PG_INTRO].enter();
  gPages[PG_INTRO].draw();

  netBegin();                                                          // core 1
  xTaskCreatePinnedToCore(hwTask, "hw", 3072, nullptr, 4, nullptr, 0); // core 0, high priority
  xTaskCreatePinnedToCore(uiTask, "ui", 8192, nullptr, 3, nullptr, 0);
  dbgHeap("after start");
}

void loop() {
  vTaskDelete(nullptr);    // everything runs in the tasks above
}
