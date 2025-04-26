#include "mqtt.h"
#define DEBUG true

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* topics[4][4] = {
  {"schedule/A6001",      "", "", ""},
  {"changes/A6001",       "",  "",  ""},
  {"announcements/A6001", "",  "",  ""},
  {"config/A6/0/A6001",   "config/A6/0",   "config/A6",   "config"}
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
          if (WiFi.status() != WL_CONNECTED) {
            if (!connectWiFi()) return;
          }
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
  #ifdef DEBUG
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  #endif
  gotUpdate = true; // Indicates schedule was able to be updated

  // Initialize durations to 0
  for (int i = 0; i < NUM_CLASSES; ++i) durations[i] = 0;

  if (payload[0]==0x00) { // No classes are scheduled for that day, return
    #ifdef DEBUG
    Serial.println("No classes today");
    #endif
    client.unsubscribe(topics[SCHEDULE][0]); // Unsubscribe from the topic
    return;
  }

  // Got a schedule, fill classes and durations array
  unsigned int i = 0;
  while (i < length) { // Parse the whole sent message
    uint8_t pos = (uint8_t)payload[i]>>4; // Start position (hour-8)
    uint16_t duration = (uint16_t)payload[i++]&0x0F; // Duration in hours

    // Format duration array
    durations[pos] = duration;
    for (int j = pos+duration-1; j > pos; --j) durations[j] = -(--duration);

    // Parse name of the subject
    unsigned int nameEnd = (int)payload[i++]+i;
    uint8_t k = 0;
    for (i = i; i < nameEnd; i++) {
      classes[pos][k++] = (char)payload[i];
    }
    #ifdef DEBUG
    Serial.print("Start: ");
    Serial.print((uint8_t)payload[i]>>4);
    Serial.print(" Duration: ");
    Serial.print(duration);
    Serial.print(" Length name: ");
    Serial.print(nameEnd-i-1);
    Serial.print(" Name: ");
    Serial.println(classes[pos]);
    #endif
  }
  #ifdef DEBUG
  Serial.println("-----------------------");
  #endif
  client.unsubscribe(topics[SCHEDULE][0]); // Unsubscribe from the topic
}
