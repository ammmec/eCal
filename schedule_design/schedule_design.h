#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include "schedule.h"

#include <WiFi.h>
#include <time.h>
#include "wifi_secure.h"

// Pin definition
#define PWR  D11  // 7
#define BUSY D7   // 18
#define RST  D6   // 1
#define DC   D3   // 14
#define CS   SS   // 5

// Display size
#define DISPLAY_HEIGHT 800
#define DISPLAY_WIDTH 480

#define CLASS_BP_WIDTH  24
#define CLASS_BP_HEIGHT 60
#define QR_SIZE 125

enum Layout {
  DEFAULT_LAYOUT,
  DEFAULT_NO_LINES,
  HALF_DAY
  // add more as needed
};