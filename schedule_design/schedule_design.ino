#include "schedule_lib.h"

void setup() {
  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout();
  //setLines(false);

  char classes[NUM_CLASSES][128] = {
    "FM 10 L", "FM 10 L", "SO2 20 T", "SO2 10 T", "", "", "", "DSBM 10 L",
    "DSBM 10 L", "", "", "", "CI 10 T"
  };
  int16_t durations[NUM_CLASSES] = {
    2, -1, 2, -1, 0, 0, 0,
    2, -1, 0, 0, 0, 1
  };

  drawSchedule(classes, durations, "Aquesta aula quedara tancada automaticament mentres no hi hagi classe");
}

void loop() {}