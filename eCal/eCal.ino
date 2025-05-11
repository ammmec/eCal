#include "mqtt.h"
#define uS2M 1000000ULL //1000000ULL*60  // Conversion from micro seconds to minutes
#define HOUR_SLEEP 10U       // Time the microcontroller will be asleep for in a normal setting
#define RETRY_SLEEP 5U
#define NIGHT_SLEEP 5U
#define WEEKEND_SLEEP 5U

int16_t durations[NUM_CLASSES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char classes[NUM_CLASSES][32] = {"", "", "", "", "", "", "", "", "", "", "", "", ""};
change_t changed[NUM_CLASSES] = {NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE};
char announcements[256] = "I am an announcement\n";

RTC_DATA_ATTR bool gotSchedule = false;

void deepSleep(uint16_t minutes) {
  esp_sleep_enable_timer_wakeup(minutes * uS2M);
  Serial.println("Setup ESP32 to sleep for every " + String(minutes) + " minutes");

  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void setup() {
  Serial.begin(115200);
  // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
  setupLayout(MINIMALIST_LAYOUT, true, false, true);
  delay(1000);  //Take some time to open up the Serial Monitor

  Serial.print("Good morning! Got schedule? ");
  Serial.println(gotSchedule);

  connectWiFi();
  setupMQTT();

  if (!gotSchedule) {
    needRefresh = true;
    if (getSchedule(classes, durations)) gotSchedule = true;
    else {
      Serial.println("Did not get schedule... Retry sleep!");
      deepSleep(RETRY_SLEEP); // If it did not manage to get the full schedule, try again after some time
    }
  }

  // if (checkAnnouncements()) getAnnouncements;
  getChanges();

  disconnectMQTT();
  disconnectWiFi();

  // Refresh display
  Serial.println("Printing schedule");
  drawSchedule(classes, durations, announcements, changed);
  
  if (currentHour > 21) {
    gotSchedule = false;
    restartData(); // prepare data for a new day (e.g. initialize arrays to 0)

    if (weekday == FRIDAY) {
      Serial.println("Weekend sleep!");
      deepSleep(WEEKEND_SLEEP);
    }
    Serial.println("Night sleep!");
    deepSleep(NIGHT_SLEEP);
  }

  needRefresh = false;
  Serial.println("Hour sleep!");
  deepSleep(HOUR_SLEEP);
}

void loop() {}
