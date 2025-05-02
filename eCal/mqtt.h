#ifndef MQTT_H
#define MQTT_H
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "wifi_secure.h"
#include "schedule_lib.h"

#define NUM_LEVELS 4

extern WiFiClientSecure espClient;
extern PubSubClient client;

extern const char *topics[4][4];

enum Messages {
  SCHEDULE,
  CHANGES,
  ANNOUNCEMENTS,
  CONFIG,
};

void setupMQTT();
void getSchedule(char classes[][32], int16_t durations[]);
void getChanges();
void callbackChanges(char *topic, byte *payload, unsigned int length);
void callbackSchedule(char *topic, byte *payload, unsigned int length);

#endif