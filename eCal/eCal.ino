#include "mqtt.h"

int16_t durations[NUM_CLASSES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char classes[NUM_CLASSES][32] = {"", "", "", "", "", "", "", "", "", "", "", "", ""};

void setup() {
  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout(VERBOSE_LAYOUT, true, false, true);
  //setLines(false);

  Serial.begin(115200);

  if (connectWiFi()) {
    setupMQTT();
    getSchedule(classes, durations);
    disconnectWiFi();
  }

  drawSchedule(classes, durations, "I am an announcement");
}

void loop() {}
