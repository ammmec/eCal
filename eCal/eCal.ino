#include "mqtt.h"

int16_t durations[NUM_CLASSES];
char classes[NUM_CLASSES][32];

void setup() {
  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout(DEFAULT_LAYOUT, true, false, true);
  //setLines(false);

  Serial.begin(115200);

  //drawSchedule(classes, durations, "");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected.");

  setupMQTT();
  getSchedule(classes, durations);

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  drawSchedule(classes, durations, "I am an announcement");
}

void loop() {}