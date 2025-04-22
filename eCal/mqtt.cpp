#include "mqtt.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* topics[4][4] = {
  {"schedule/A6/0/A6001", "schedule/A6/0", "schedule/A6", "schedule"},
  {"changes/A6/0/A6001",  "changes/A6/0",  "changes/A6",  "changes"},
  {"config/A6/0/A6001",   "config/A6/0",   "config/A6",   "config"},
  {"ann/A6/0/A6001",      "ann/A6/0",      "ann/A6",      "ann"}
};

bool gotUpdate;

void setupMQTT() {
  espClient.setCACert(ca_cert);
  
  //connecting to the mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setBufferSize(512);
  while (!client.connected()) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          Serial.println("Public EMQX MQTT broker connected");
      } else {
          Serial.print("failed with state ");
          Serial.print(client.state());
          delay(2000);
      }
  }
}

void getSchedule(char classes[][32], int16_t durations[]) {
  gotUpdate = false;
  client.setCallback(callbackSchedule);
  // Subscribe to most priority schedule topic, others are not used
  client.subscribe(topics[SCHEDULE][0]);
  
  while (!gotUpdate) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }
  Serial.println("Got schedule");
}

void callbackSchedule(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  gotUpdate = true;

  if (payload[0]==0x00) { // No classes are scheduled for that day, stop waiting for an update
    client.unsubscribe(topics[SCHEDULE][0]); // Unsubscribe from the topic
    // no classes, duration = 0
    for (int i = 0; i < NUM_CLASSES; ++i) {
      durations[i] = 0;
    }
    Serial.println("No classes today");
    return;
  }

  // Got a schedule, fill classes and durations array

  // Initialize durations to 0
  for (int i = 0; i < NUM_CLASSES; ++i) {
      durations[i] = 0;
  }

  unsigned int i = 0;
  while (i < length) {
    Serial.print("Start: ");
    Serial.print((uint8_t)payload[i]>>4);
    uint8_t pos = (uint8_t)payload[i]>>4;

    Serial.print(" Duration: ");
    uint8_t duration = (uint8_t)payload[i++]&0x0F;
    Serial.print(duration);
    durations[pos] = duration--;
    for (int j = pos+1; j < pos+duration; ++j) {
      durations[j] = -(duration--);
    }

    Serial.print(" Length name: ");
    unsigned int nameEnd = (int)payload[i++]+i;
    Serial.print(nameEnd-i-1);

    Serial.print(" Name: ");
    uint8_t k = 0;
    for (int j = i; j < nameEnd; j++) {
      Serial.print((char) payload[j]);
      classes[pos][k++] = (char)payload[j];
      i = j;
    }
    ++i;
    Serial.println();
  }
  Serial.println("-----------------------");
}