#include "mqtt.h"
#define uS2M  1000000ULL*60  // 1000000ULL // Conversion from micro seconds to minutes
// Time the microcontroller will be asleep for in different moments
#define RETRY_SLEEP (((rawConfig >> 4) & 0x03F) == 0x3F) ? 5U : (rawConfig >> 4) & 0x03F  //Default value if not changed yet
#define NIGHT_SLEEP 9U                        // 10 hours: 9 + offset
#define WEEKEND_SLEEP 57U                     // 58 hours (10 for morning, 48 for weekend): 57 + offset

int16_t durations[NUM_CLASSES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char classes[NUM_CLASSES][32] = { "", "", "", "", "", "", "", "", "", "", "", "", "" };
change_t changed[NUM_CLASSES] = { NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE };
char announcements[256];

RTC_DATA_ATTR bool gotSchedule = false;

void deepSleep(uint16_t minutes) {
  esp_sleep_enable_timer_wakeup(minutes * uS2M);
  #ifdef DEBUG
  Serial.println("Setup ESP32 to sleep for every " + String(minutes) + " minutes");

  Serial.println("Going to sleep now");
  #endif
  Serial.flush();
  esp_deep_sleep_start();
}

void setup() {
  #ifdef DEBUG
  Serial.begin(115200);
  delay(1000);  //Take some time to open up the Serial Monitor

  Serial.print("Good morning! Got schedule? ");
  Serial.println(gotSchedule);
  #endif

  setupLayout();

  uint8_t hourSleep = 60;
  connectWiFi(hourSleep);
  setupMQTT();

  if (!gotSchedule) {
    if (getSchedule(classes, durations)) {
      gotSchedule = true;
      needRefresh = true;
    } else {
      if (needRefresh) {
        drawNoSchedule();
        needRefresh = false;
      }
      #ifdef DEBUG
      Serial.println("Did not get schedule... Retry sleep!");
      #endif
      disconnectMQTT();
      disconnectWiFi();
      deepSleep(RETRY_SLEEP);  // If it did not manage to get the full schedule, try again after some time
    }
  }

  getDetails();
  disconnectMQTT();
  disconnectWiFi();

  // Refresh display
  #ifdef DEBUG
  Serial.println("Printing schedule");
  #endif

  drawSchedule(classes, durations, announcements, changed);
  
  if (currentHour > 21) {
    gotSchedule = false;
    restartData(); // prepare data for a new day (e.g. initialize arrays to 0)

    if (weekday == FRIDAY) {
      #ifdef DEBUG
      Serial.println("Weekend sleep!");
      #endif
      deepSleep(WEEKEND_SLEEP+hourSleep);
    }
    #ifdef DEBUG
    Serial.println("Night sleep!");
    #endif
    deepSleep(NIGHT_SLEEP+hourSleep);
  }

  needRefresh = false;
  #ifdef DEBUG
  Serial.println("Hour sleep!");
  #endif
  deepSleep(hourSleep);
}

void loop() {}
