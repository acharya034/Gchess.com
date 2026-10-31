# Gchess - smart chessboard firmware (ESP32)

Not compiled on a real ESP32 by me. The chess rules (`chess_rules.h`) and the game logic (`game.h`)
were tested on a PC. Build it, then send me the first compile errors / serial log.

## Project layout (PlatformIO)
Put all files from this folder into `src/` (rename `Gchess.ino` to `main.cpp` and add
`#include <Arduino.h>` at the top if PlatformIO asks for it).

```
src/
  Gchess.ino (main)  program start, the 3 tasks            <- read this first
  gchess_config.h    pins, timings, debug switches         <- edit this to change hardware
  debug.h            Serial.print helper  DBG(CATEGORY, "text %d", value)
  events.h           messages between core 0 (UI) and core 1 (network)
  storage.h          settings / accounts / Wi-Fi in flash
  analytics.h        usage counters (pages, buttons, time)
  sound.h            buzzer melodies
  power.h            ONE-pin power latch, button, battery gauge, charger state
  leds.h             64 square LEDs
  board.h            4 x 4 x 4 hall sensor matrix
  chess_rules.h      chess rules, FEN, SAN (no hardware inside)
  game.h             sensors + touch + LEDs + rules = games (3 slots)
  gfx.h              display, colours, pieces, logos
  ui.h               pages, toolbar, quick menu, dialogs, keyboard, notifications
  net.h              Wi-Fi + Lichess client + account linking server
  pages.h            every screen
Gchess_wiring.html   wiring diagrams (also published online)
```

`platformio.ini`:
```
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps =
    lovyan03/LovyanGFX
    fastled/FastLED
    bblanchon/ArduinoJson@^7
    ricmoo/QRCode
```
(ESP32 Arduino core 3.x)

## Serial debug (what to send me when something is wrong)
Open the serial monitor at 115200. Every line is `[time] core CATEGORY message`.
Switch categories on/off in `gchess_config.h` (`DBG_TOUCH`, `DBG_UI`, `DBG_EVT`, `DBG_NET`, `DBG_HTTP`,
`DBG_GAME`, `DBG_SENS`, `DBG_POWER`, `DBG_SYS`).
- `TOUCH` every finger down / up (x, y, page, menu)  - `UI` pages, buttons, dialogs, notifications
- `EVT` every message between UI and network core    - `HTTP` full request (url, headers, body) and response
- `GAME` moves + FEN, wrong moves, board owner       - `SENS` which squares changed
- `POWER` button, battery volts, charger millivolts  - `SYS` free memory every 10 s (a falling "min_ever" = leak)
Lichess tokens are hidden (`lip_Ab...xy`). Set `DEBUG_SHOW_SECRETS 1` only if you must; do not post it.

## Hardware summary (details in `Gchess_wiring.html`)
- Hall matrix: 4 positive lines (PNP, LOW = on) 13,12,14,27 - 4 negative lines (NPN, HIGH = on) 26,25,16,4 -
  4 data lines 35,34,39,36 (10k pull-ups). Changed vs your array: negative lines 3 and 4 moved from 33,32 to
  16,4 because 32 and 33 are needed as ADC pins (Wi-Fi blocks the other ADC pins).
- Power: one pin, GPIO2. Latch circuit with Q1..Q3 (diagram). Hold the button ~0.5 s to start.
- Battery gauge: GPIO32 (pulse + capacitor + MOSFET, no constant drain). Charger LEDs: GPIO33.
- Display: SCLK 18, MOSI 23, MISO 19, CS 15, DC 17, backlight 5, touch CS 21, RESET tied to 3V3 (no pin).
- LED data GPIO0, buzzer GPIO22.
- GPIO12 must be LOW at boot (add 10k to GND). GPIO0 must not be pulled LOW at boot.
- Sensor to square mapping: `SENSOR_ROW` / `SENSOR_COL` in `gchess_config.h`.

## Account linking
QR code = `LINK_SITE_URL?d=<board ip>`. The website calls `POST /pair/start` (optional) and
`POST /pair/token {accessToken}` on the board. The board checks the token with Lichess and saves it.
The window is only open while the Link page is on screen (5 minutes).

## Things still to know
- Pieces/logos are drawn with shapes (`gfxPiece`). Tokens and passcodes are stored in plain flash.
- TLS uses `setInsecure()`; add the Lichess root certificate before sharing the board.
- Two games are kept live (memory); all other games are only a light list (Home > Ongoing).
