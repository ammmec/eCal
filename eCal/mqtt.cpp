#include "mqtt.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char* topics[5][4] = {
  {"schedule/A6001",      nullptr,  nullptr,  nullptr},
  {"changes/A6001",       nullptr,  nullptr,  nullptr},
  {"announcements/A6001", nullptr,  nullptr,  nullptr},
  {"config/A6/0/A6001",   "config/A6/0",   "config/A6",   "config"},
  {"meta/A6001",          nullptr,  nullptr,  nullptr}
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
  while (!gotUpdate && attempts++ < NUM_TRIES) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }
  Serial.println("Got schedule");
  return gotUpdate;
}

bool newChange;
bool newAnn;
char newConfig;

bool getDetails() {
  gotUpdate = 0;
  newChange = false;
  newAnn = false;
  newConfig = -1;
  Serial.println("Getting meta topic");
  if (!setupMQTT()) return false;
  client.setCallback(callbackMeta);
  client.subscribe(topics[META][0]); // Get status of topics

  uint8_t attempts = 0;
  while (!gotUpdate && attempts++ < NUM_TRIES) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }

  gotUpdate = 0;
  client.setCallback(callbackDetails);

  char numUpdates = 0;
  if (newConfig != (char)(-1)) {
    client.subscribe(topics[CONFIG][newConfig]);
    ++numUpdates;
  }
  if (newChange) {
    client.subscribe(topics[CHANGES][0]); // Subscribe to changes topic
    ++numUpdates;
  }
  if (newAnn) {
    client.subscribe(topics[ANNOUNCEMENTS][0]); // Subscribe to announcement topic
    ++numUpdates;
  }

  attempts = 0;
  while (gotUpdate < numUpdates && attempts++ < NUM_TRIES) { // Keep looking for updates until resolved
    client.loop();
    delay(500);
  }
  return gotUpdate >= numUpdates;
}

void callbackMeta(char *topic, byte *payload, unsigned int length) {
  gotUpdate = true;
  #ifdef DEBUG
  Serial.print("Meta package: ");
  Serial.println(payload[0], HEX);
  #endif
  if (!(payload[0]&0x98)) { // No meta changes made
    #ifdef DEBUG
    Serial.println("No meta changes!");
    #endif
    client.unsubscribe(topics[META][0]); // Unsubscribe from the topic
    return;
  }
  if (payload[0]&0x80) {
    newConfig = ((payload[0])>>5)&0x03;
    #ifdef DEBUG
    Serial.print("New configuration: ");
    Serial.println(newConfig);
    #endif
  }
  if (payload[0]&0x10) {
    newChange = true;
    #ifdef DEBUG
    Serial.println("New change");
    #endif
  }
  if (payload[0]&0x08) {
    newAnn = true;
    #ifdef DEBUG
    Serial.println("New Announcement");
    #endif
  }
  client.unsubscribe(topics[META][0]); // Unsubscribe from the topic

  byte cl[1] = {payload[0]&0x60}; // Flag as read
  client.publish(topics[META][0], cl, 1, true); // Reset changes made for future checks
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

void getConfig(char *topic, byte *payload, unsigned int length) {
  uint16_t receivedConfig = ((uint16_t)(payload[0]) << 8) | (uint16_t)payload[1];
  #ifdef DEBUG
  Serial.print("Previous config: ");
  Serial.println(rawConfig, HEX);
  Serial.print("Received: ");
  Serial.println(receivedConfig, HEX);
  #endif

  if (receivedConfig != rawConfig) {
    needRefresh = true;
    Layout l = static_cast<Layout>((uint8_t)(payload[0]>>5));
    bool lines = ((uint8_t)payload[0]>>4)&0x01;
    bool saveEnergy = ((uint8_t)payload[0]>>3)&0x01;
    bool staticSchedule = ((uint8_t)payload[0]>>2)&0x01;
    // setupLayout(Layout layout, bool lines, bool saveEnergy, bool staticSchedule)
    setupLayout(l, lines, saveEnergy, staticSchedule);
    #ifdef DEBUG
    Serial.print("Layout: ");
    Serial.println(((uint8_t)(payload[0]>>5)));
    Serial.print("Lines: ");
    Serial.println(lines);
    Serial.print("SaveEnergy: ");
    Serial.println(saveEnergy);
    Serial.print("staticSchedule: ");
    Serial.println(staticSchedule);
    Serial.print("Retry time: ");
    Serial.println((receivedConfig>>4)&0x03F);
    #endif
    rawConfig = receivedConfig;
  }
}

void callbackDetails(char *topic, byte *payload, unsigned int length) {
  gotUpdate++; // Indicates changes/announcements was able to be updated

  if (strcmp(topics[CHANGES][0], topic) == 0) getChanges(payload, length);
  else if (strcmp(topics[ANNOUNCEMENTS][0], topic) == 0) getAnnouncements(payload, length);
  else getConfig(topic, payload, length);// topic is a configuration message
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
    Serial.print(durations[pos]);
    Serial.print(" Name: ");
    Serial.println(classes[pos]);
    #endif
  }
  #ifdef DEBUG
  Serial.println("-----------------------");
  #endif
  client.unsubscribe(topics[SCHEDULE][0]); // Unsubscribe from the topic
}
