#include "mqtt.h"

int16_t durations[NUM_CLASSES] = {0, 0, 2, -1, 0, 0, 2, -1, 2, -1, 0, 0, 0};
char classes[NUM_CLASSES][32] = {"", "", "GEI IES 10T", "", "", "", "GEI PAR 40 T", "", "GEI PROP 40T", "", "", "", ""};
change_t changed[NUM_CLASSES] = {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE};
char announcements[256] = "I am an announcement\n";

void setup() {
  Serial.begin(115200);

  /*if (connectWiFi()) {
    setupMQTT();
    getSchedule(classes, durations);
    disconnectWiFi();
  }*/

  if (connectWiFi()) {
    setupMQTT();
    getChanges();
    disconnectWiFi();
  }

  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout(VERBOSE_LAYOUT, true, false, true);
  //setLines(false);

  drawSchedule(classes, durations, announcements, changed);
}

void loop() {}
