#include "schedule_lib.h"

void setup() {
  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout(SIMPLE_LAYOUT, true, false, false);
  //setLines(false);

  Serial.begin(115200);

  char classes[NUM_CLASSES][32] = {
    "FM 10 L", "FM 10 L", "SO2 20 T", "SO2 10 T", "", "", "", "DSBM 10 L",
    "DSBM 10 L", "", "", "", "CI 10 T"
  };
  int16_t durations[NUM_CLASSES] = {
    2, -1, 2, -1, 0, 0, 0,
    2, -1, 0, 0, 0, 1
  };

  drawSchedule(classes, durations, "");
}

void loop() {}