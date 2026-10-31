// =====================================================================================
//  gchess_config.h  -  EVERYTHING you may want to change lives here
//  (pins, brand text, timings, debug switches)
// =====================================================================================
#pragma once
#include <Arduino.h>

// ---------- Brand -------------------------------------------------------------------
#define GC_NAME      "Gchess"
#define GC_TAGLINE   "Real pieces. Real games. Online."
#define GC_VERSION   "0.2.0"

// ---------- Account linking website -------------------------------------------------
// The QR code on the board shows:  LINK_SITE_URL + "?d=<board ip>"
// The website then calls  http://<ip>/pair/start  and  http://<ip>/pair/token
#define LINK_SITE_URL "https://acharya034.github.io/Gchess-Server/"
#define LICHESS_HOST  "lichess.org"

// ---------- Serial debug switches (1 = print, 0 = silent) ---------------------------
// Open the serial monitor at 115200. Every line looks like:
//   [   12.345] C1 NET    >> GET https://lichess.org/api/account
//    time(s)    core cat   message
#define DBG_TOUCH   1   // every touch down / up / move (moves are rate limited)
#define DBG_UI      1   // page changes, buttons, menus, dialogs, notifications
#define DBG_EVT     1   // every message between core 0 (UI) and core 1 (network)
#define DBG_NET     1   // Wi-Fi, Lichess streams, game slots
#define DBG_HTTP    1   // full requests and responses (url, headers, body)
#define DBG_GAME    1   // moves, hints, wrong-move alarms, board ownership
#define DBG_SENS    1   // hall sensor changes (which squares changed)
#define DBG_POWER   1   // power button, battery, charger
#define DBG_SYS     1   // boot info, free memory every 10 s, task stack usage
#define DEBUG_SHOW_SECRETS 0   // 1 = print full Lichess tokens (careful!)

// ---------- Pins (ESP32 DevKit, 30 pin) ---------------------------------------------
// TFT + touch share the SPI bus (VSPI)
#define PIN_TFT_SCLK 18
#define PIN_TFT_MOSI 23
#define PIN_TFT_MISO 19
#define PIN_TFT_CS   15
#define PIN_TFT_DC   17
#define PIN_TFT_RST  -1   // not connected: tie the display RESET pin to 3V3
#define PIN_TFT_BL    5   // backlight (PWM)
#define PIN_TOUCH_CS 21

#define PIN_LED_DATA  0   // addressable LEDs (through 330R into LED #0). Keep GPIO0 free of pull-downs.
#define PIN_BUZZER   22   // passive buzzer

// Power: ONE pin does everything (see wiring diagram "Power").
//   idle  = driven LOW  (keeps the latch happy)
//   read  = every 5 ms it is briefly switched to INPUT_PULLUP to see if the button is pressed
//   off   = driven HIGH for a moment -> the latch releases and the board powers down
#define PIN_POWER     2

// Battery and charger readings (both must be ADC1 pins: 32..39)
#define PIN_BAT      32   // battery gauge: pulse it HIGH to charge the capacitor, then read it as ADC
#define PIN_CHG      33   // charger LEDs through two different resistors -> voltage tells the state

// ---------- Hall sensor matrix  (4 x 4 x 4 = 64 sensors, 12 pins) ------------------
//   row 0 : "positive" lines  -> PNP transistors, pin LOW  = that row gets power
//   row 1 : "negative" lines  -> NPN transistors, pin HIGH = that column gets ground
//   row 2 : data lines        -> inputs (10k pull-up to 3V3 on each)
// Scan: switch on one positive line, then one negative line, read the 4 data pins,
// repeat for all 4 x 4 combinations.
const int hallpin[3][4] = {
    {13, 12, 14, 27},   // positive / power lines   (GPIO12 note: see README, boot pin)
    {26, 25, 16, 4},    // negative / ground lines (GPIO15 is now the display CS, so line 3 uses GPIO4)
    {35, 34, 39, 36}    // data lines (input only pins)
};

// How a sensor (data line k, positive line i, negative line j) maps to the board:
//   k = quadrant  (0 top-left, 1 top-right, 2 bottom-left, 3 bottom-right)
//   i = row inside the quadrant (0 = top row of that quadrant)
//   j = column inside the quadrant (0 = left column of that quadrant)
// If your wiring is different, change ONLY these two lines:
#define SENSOR_ROW(k, i, j)  (((k) >> 1) * 4 + (i))
#define SENSOR_COL(k, i, j)  (((k) & 1) * 4 + (j))

#define HALL_ACTIVE_LOW   1    // sensor output is LOW when a magnet is near
#define HALL_SETTLE_US   60    // wait after switching a row/column before reading
#define SENSOR_DEBOUNCE   4    // number of identical scans before a change is accepted
#define SENSOR_SCAN_MS    2    // time between full scans

// ---------- LEDs ---------------------------------------------------------------------
#define LED_COUNT        65    // #0 = sacrificial level shifter, #1..#64 = squares
#define LED_MAX_MA     1500    // power limit for the LED strip

// ---------- Power button timing -------------------------------------------------------
#define PWR_SAMPLE_MS     5    // how often the button is sampled
#define PWR_READ_US      25    // pull-up window length
#define BTN_RESTART_MS 2000    // hold this long  -> restart
#define BTN_OFF_MS     4000    // hold this long  -> power off

// ---------- Battery gauge -------------------------------------------------------------
#define BAT_PERIOD_MS 20000    // measure every 20 s
#define BAT_PULSE_MS     15    // HIGH pulse that charges the capacitor
#define BAT_SETTLE_MS    40    // wait before reading
#define BAT_DIV         2.0f   // divider ratio (100k + 100k)
#define BAT_CAL         1.00f  // fine calibration: multiply result (measure with a multimeter)

// ---------- Charger LED reader (millivolts seen on PIN_CHG, 100k to GND at the pin) ---
// charging: ~2000 mV    full: ~900 mV    no USB: ~0 mV
// The values are printed on the serial monitor every second - adjust if yours differ.
#define CHG_MV_CHARGING_MIN 1500
#define CHG_MV_FULL_MIN      500

// ---------- Lichess connection / memory ------------------------------------------------
// One secure (TLS) connection costs ~45 KB of RAM and a new one needs ~70 KB free while it connects.
// An ESP32 without extra RAM cannot hold several at once, so:
//   * only MAX_LIVE_GAMES games get a permanent connection (a "stream"); every other game is
//     only a line in the Ongoing list (refreshed by polling)
//   * there is NO permanent "events" connection: challenges and new games are found by polling
//   * when a request needs memory, the live game stream is paused for a moment and resumes by itself
#define MAX_LIVE_GAMES        1
#define TLS_MIN_FREE_HEAP 100000   // below this many free bytes the game stream is paused during a request
#define POLL_CHALLENGES_MS 12000   // look for incoming challenges (when no game stream is open)
#define POLL_NEWGAME_MS     2500   // while waiting for a game to start
#define POLL_LIST_MS      120000   // refresh the list of running games

// ---------- Wi-Fi ----------------------------------------------------------------------
#define WIFI_WEAK_RSSI  (-78)  // below this we look for a better access point
#define WIFI_RESCAN_MS  20000
#define WIFI_RETRY_MS   12000
