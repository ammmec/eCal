#ifndef MQTT_H
#define MQTT_H
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "wifi_secure.h"
#include "schedule_lib.h"

#define NUM_TRIES 10

extern WiFiClientSecure espClient;
extern PubSubClient client;

extern const char *topics[5][4];

enum Messages {
  SCHEDULE,
  CHANGES,
  ANNOUNCEMENTS,
  CONFIG,
  META
};

bool setupMQTT();
void disconnectMQTT();
bool getSchedule(char classes[][32], int16_t durations[]);
bool getDetails();
void getConfig(char *topic, byte *payload, unsigned int length);
void getChanges(byte *payload, unsigned int length);
void getAnnouncements(byte *payload, unsigned int length);
void callbackMeta(char *topic, byte *payload, unsigned int length);
void callbackDetails(char *topic, byte *payload, unsigned int length);
void callbackSchedule(char *topic, byte *payload, unsigned int length);

#endif