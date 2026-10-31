// =====================================================================================
//  debug.h  -  Serial logging helpers
//
//  Use:   DBG(NET, "connected to %s rssi=%d", ssid, rssi);
//  The first word is the category; it is switched on/off in gchess_config.h (DBG_xxx).
//  Printing from both CPU cores is protected by a mutex so lines never mix.
// =====================================================================================
#pragma once
#include <Arduino.h>
#include <stdarg.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "gchess_config.h"

static SemaphoreHandle_t gDbgMux = nullptr;

// Call once at the very start of setup().
inline void dbgBegin() {
  Serial.begin(115200);
  gDbgMux = xSemaphoreCreateMutex();
}

// Print one formatted line with time stamp, CPU core and category.
inline void dbgPrint(const char* cat, const char* fmt, ...) {
  char text[400];
  va_list args;
  va_start(args, fmt);
  vsnprintf(text, sizeof(text), fmt, args);
  va_end(args);

  uint32_t ms = millis();
  if (gDbgMux) xSemaphoreTake(gDbgMux, pdMS_TO_TICKS(100));
  Serial.printf("[%7lu.%03lu] C%d %-6s %s\n", (unsigned long)(ms / 1000), (unsigned long)(ms % 1000), (int)xPortGetCoreID(), cat, text);
  if (gDbgMux) xSemaphoreGive(gDbgMux);
}

// DBG(CAT, ...) prints only when DBG_CAT is 1 in the config.
#define DBG(cat, ...)                                  \
  do {                                                 \
    if (DBG_##cat) dbgPrint(#cat, __VA_ARGS__);        \
  } while (0)

// Hide secrets (Lichess tokens) unless DEBUG_SHOW_SECRETS is 1.
// Example: "lip_AbCdEfGhIjKl" -> "lip_Ab...Kl (16 chars)"
inline const char* dbgSecret(const char* s) {
  static char buf[48];
  if (DEBUG_SHOW_SECRETS || !s) return s ? s : "(null)";
  size_t n = strlen(s);
  if (n <= 8) return "***";
  snprintf(buf, sizeof(buf), "%.6s...%s (%u chars)", s, s + n - 2, (unsigned)n);
  return buf;
}

// Free memory report (helps to find memory leaks).
inline void dbgHeap(const char* where) {
  DBG(SYS, "heap@%s: free=%u min_ever=%u biggest_block=%u", where, (unsigned)ESP.getFreeHeap(), (unsigned)ESP.getMinFreeHeap(), (unsigned)ESP.getMaxAllocHeap());
}

// Name of a square index (0 = a1 ... 63 = h8) for sensor / move logs.
inline void dbgSquares(uint64_t mask, char* out, size_t n) {
  size_t k = 0;
  out[0] = 0;
  for (int sq = 0; sq < 64; sq++) {
    if (!((mask >> sq) & 1)) continue;
    if (k + 4 >= n) break;
    out[k++] = 'a' + (sq & 7);
    out[k++] = '1' + (sq >> 3);
    out[k++] = ' ';
    out[k] = 0;
  }
}
