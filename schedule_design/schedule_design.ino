#include "schedule_lib.h"

void setup() {
  setupLayout(WIDE_LAYOUT);
  setLines(false);

  char classes[NUM_CLASSES][128] = {
    "SRGGE-MIRI 10 L", "GEI FM 10T", "GEI LP 10T", "GEI LP 10T", "GEI IA 10T", "GEI IA 10T", "GEI IA 10T",
    "GEI FM 20T", "", "GEI PRO1 20T", "TANCAT", "TANCAT", ""
  };
  int16_t durations[NUM_CLASSES] = {
    1, 1, 2, -1, 3, -1, -2,
    1, 0, 1, 2, -1, 0
  };

  drawSchedule(classes, durations);
}

void loop() {}