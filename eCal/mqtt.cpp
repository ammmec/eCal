#include "mqtt.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* topics[4][4] = {
  {"schedule/A6001",      "", "", ""},
  {"changes/A6001",       "",  "",  ""},
  {"announcements/A6001", "",  "",  ""},
  {"config/A6/0/A6001",   "config/A6/0",   "config/A6",   "config"}
};

char gotUpdate;

bool setupMQTT() {
  if (WiFi.status() != WL_CONNECTED) return false;
  if (client.connected()) return true;
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
          if (!connectWiFi()) return false;
        }
        Serial.print("failed with state ");
        Serial.print(client.state());
        delay(2000);
    }
  }
  return true;
}

void disconnectMQTT() {
  client.disconnect();
}

bool getSchedule(char classes[][32], int16_t durations[]) {
  gotUpdate = false;
  if (!setupMQTT()) return false;
  client.setCallback(callbackSchedule);
  // Subscribe to most priority schedule topic, others are not used
  client.subscribe(topics[SCHEDULE][0]);
  
  uint8_t attempts = 0;
  while (!gotUpdate && attempts++ < 50) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }
  Serial.println("Got schedule");
  return gotUpdate;
}

bool getDetails() {
  gotUpdate = 0;
  char numUpdates = 2;
  Serial.println("Getting changes & announcements");
  if (!setupMQTT()) return false;
  client.setCallback(callbackDetails);

  client.subscribe(topics[ANNOUNCEMENTS][0]); // Subscribe to announcement topic
  client.subscribe(topics[CHANGES][0]); // Subscribe to changes topic, others are not used

  uint8_t attempts = 0;
  while (gotUpdate < numUpdates && attempts++ < 50) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }
  return gotUpdate >= numUpdates;
}

void getChanges(byte *payload, unsigned int length) {
  if (payload[0]==0x00) { // No changes are made, return
    #ifdef DEBUG
    Serial.println("No changes made");
    #endif
    client.unsubscribe(topics[CHANGES][0]); // Unsubscribe from the topic
    return;
  }
  needRefresh = true; // If there are any changes the screen has to be refreshed

  // Got changes, fill changes array
  unsigned int i = 0;
  while (i < length) { // Parse the whole sent message
    uint8_t pos = (uint8_t)payload[i]>>4; // Start position (hour-8)
    uint16_t duration = (uint16_t)payload[i++]&0x0F; // Duration in hours
    changed[pos] = static_cast<change_t>((uint8_t)(payload[i++]>>4) + 1); // Type of change

    #ifdef DEBUG
    Serial.print("Start: ");
    Serial.print((uint8_t)pos);
    Serial.print(" Duration: ");
    Serial.print(duration);
    Serial.print(" Type: ");
    /*  NONE    -> 0
        GENERAL -> 1
        ADD     -> 2
        CANCEL  -> 3
    */
    Serial.println((int)changed[pos]);
    #endif

    if (changed[pos]==ADD) { // Addition, make a new class
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
      Serial.print(classes[pos]);
      Serial.print(" ");
      Serial.println(durations[pos]);
      #endif
    }
  }
  client.unsubscribe(topics[CHANGES][0]); // Unsubscribe from the topic
  byte cl[1] = {0x00};
  client.publish(topics[CHANGES][0], cl, 1, true); // Reset changes made for future checks
  Serial.println("Got changes");
}

void getAnnouncements(byte *payload, unsigned int length) {
  if (payload[0]==0x00) { // No announcements were sent, return
    #ifdef DEBUG
    Serial.println("No announcements sent");
    #endif
    client.unsubscribe(topics[ANNOUNCEMENTS][0]); // Unsubscribe from the topic
    return;
  }
  needRefresh = true; // If there are any changes the screen has to be refreshed

  // Write announcements to appropiate array
  if (announcements[0] == '\0') strncpy (announcements, (char*)payload, min(length, SIZE_ANNOUNCEMENTS)); // Announcements empty, just copy them
  else { // Concatenate strings
    strncat(announcements, (char*)payload, min(length, SIZE_ANNOUNCEMENTS - strlen(announcements) - 1));
  }

  client.unsubscribe(topics[ANNOUNCEMENTS][0]); // Unsubscribe from the topic
  byte cl[1] = {0x00};
  client.publish(topics[ANNOUNCEMENTS][0], cl, 1, true); // Reset announcements made for future checks
  Serial.println("Got announcements");
}

void callbackDetails(char *topic, byte *payload, unsigned int length) {
  gotUpdate++; // Indicates changes/announcements was able to be updated

  if (strcmp(topics[CHANGES][0], topic) == 0) getChanges(payload, length);
  else if (strcmp(topics[ANNOUNCEMENTS][0], topic) == 0) getAnnouncements(payload, length);
}

void callbackSchedule(char *topic, byte *payload, unsigned int length) {
  #ifdef DEBUG
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  #endif
  gotUpdate = true; // Indicates schedule was able to be updated
  needRefresh = true; // receiving a schedule means the screen has to be refreshed

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
