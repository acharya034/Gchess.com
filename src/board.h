// =====================================================================================
//  board.h  -  reads the 64 hall sensors (4 x 4 x 4 matrix, 12 pins)
//
//  Every sensor has 3 wires:
//     VCC  -> positive line i  (switched by a PNP transistor, pin LOW = on)
//     GND  -> negative line j  (switched by an NPN transistor, pin HIGH = on)
//     OUT  -> data line k      (read directly, 10k pull-up to 3V3)
//  So sensor (k,i,j) only works while positive line i AND negative line j are on.
//
//  Public API:
//     boardBegin()          set the pins up (call once)
//     boardScan()           call very often (hardware task); debounces the result
//     boardOccupancy()      64 bit mask, bit n = a magnet is on square n (a1 = bit 0)
// =====================================================================================
#pragma once
#include "gchess_config.h"
#include "debug.h"
#include "leds.h"   // hwToSq(): converts (row, column) on the board to a chess square

// Shared result of the scanner. A tiny "sequence lock" lets the UI task read the 64 bit
// value without a mutex (it retries if the scanner was writing at that moment).
static volatile uint32_t occLo = 0, occHi = 0, occSeq = 0;
static uint64_t rawLast = 0;      // result of the previous scan
static uint8_t rawStable = 0;     // how many scans in a row gave the same result
static uint32_t scanLast = 0;     // time of the last scan
static volatile bool occChanged = false;

// Convert "data line k, positive line i, negative line j" into a chess square (0..63).
inline int sensorToSquare(int k, int i, int j) {
  int row = SENSOR_ROW(k, i, j);   // 0 = top row of the board (as the user sees it)
  int col = SENSOR_COL(k, i, j);   // 0 = left column
  return hwToSq(row, col);
}

// Put every line in the OFF state: positive lines HIGH (PNP off), negative lines LOW (NPN off).
inline void boardBegin() {
  for (int n = 0; n < 4; n++) {
    pinMode(hallpin[0][n], OUTPUT);
    digitalWrite(hallpin[0][n], HIGH);
    pinMode(hallpin[1][n], OUTPUT);
    digitalWrite(hallpin[1][n], LOW);
    pinMode(hallpin[2][n], INPUT);   // GPIO 34/35/36/39 have no internal pull-up -> external 10k
  }
  DBG(SENS, "hall matrix ready: pos=%d,%d,%d,%d neg=%d,%d,%d,%d data=%d,%d,%d,%d", hallpin[0][0], hallpin[0][1], hallpin[0][2], hallpin[0][3], hallpin[1][0], hallpin[1][1], hallpin[1][2], hallpin[1][3], hallpin[2][0], hallpin[2][1], hallpin[2][2], hallpin[2][3]);
}

// One complete raw scan of all 64 sensors. Takes about 1 ms.
inline uint64_t boardReadRaw() {
  uint64_t occ = 0;

  for (int i = 0; i < 4; i++) {                     // positive line i
    digitalWrite(hallpin[0][i], LOW);               // PNP on: row i gets power

    for (int j = 0; j < 4; j++) {                   // negative line j
      digitalWrite(hallpin[1][j], HIGH);            // NPN on: column j gets ground
      delayMicroseconds(HALL_SETTLE_US);            // let the sensors wake up

      for (int k = 0; k < 4; k++) {                 // read the 4 data lines together
        bool magnet = digitalRead(hallpin[2][k]) == (HALL_ACTIVE_LOW ? LOW : HIGH);
        if (magnet) occ |= 1ULL << sensorToSquare(k, i, j);
      }

      digitalWrite(hallpin[1][j], LOW);             // column j off again
    }

    digitalWrite(hallpin[0][i], HIGH);              // row i off again
  }
  return occ;
}

// Called from the hardware task all the time. Publishes a new value only after the
// same result was seen SENSOR_DEBOUNCE times in a row (ignores flicker while a piece slides).
inline void boardScan() {
  uint32_t now = millis();
  if (now - scanLast < SENSOR_SCAN_MS) return;
  scanLast = now;

  uint64_t raw = boardReadRaw();
  if (raw == rawLast) {
    if (rawStable < 255) rawStable++;
  } else {
    rawLast = raw;
    rawStable = 0;
  }

  if (rawStable == SENSOR_DEBOUNCE) {
    uint32_t lo = (uint32_t)raw, hi = (uint32_t)(raw >> 32);
    if (lo != occLo || hi != occHi) {
      uint64_t before = ((uint64_t)occHi << 32) | occLo;
      occSeq++;            // odd number = "writing"
      occLo = lo;
      occHi = hi;
      occSeq++;            // even number = "done"
      occChanged = true;

      char added[48], removed[48];
      dbgSquares(raw & ~before, added, sizeof(added));
      dbgSquares(before & ~raw, removed, sizeof(removed));
      DBG(SENS, "occupancy %016llx  pieces=%d  +[%s] -[%s]", (unsigned long long)raw, __builtin_popcountll(raw), added, removed);
    }
  }
}

// Latest stable result (safe to call from the UI task).
inline uint64_t boardOccupancy() {
  uint32_t s1, s2, lo, hi;
  do {
    s1 = occSeq;
    lo = occLo;
    hi = occHi;
    s2 = occSeq;
  } while (s1 != s2 || (s1 & 1));
  return ((uint64_t)hi << 32) | lo;
}

// true once after the occupancy changed (used by the board test page)
inline bool boardRawChanged() {
  bool c = occChanged;
  occChanged = false;
  return c;
}
