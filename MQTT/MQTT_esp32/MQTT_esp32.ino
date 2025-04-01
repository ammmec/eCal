#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include "mqtt_secure.h"

WiFiClientSecure espClient;
PubSubClient client(espClient);

const char *schedule_topics [] = {"schedule/A6/0/A6001", "schedule/A6/0", "schedule/A6", "schedule"};
const char *config_topics [] = {"config/A6/0/A6001", "config/A6/0", "config/A6", "config"};
const char *ann_topics [] = {"ann/A6/0/A6001", "ann/A6/0", "ann/A6", "ann"};

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    espClient.setCACert(ca_cert);
    
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
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
    // Subscribe to the relevant topics
    for (int i = 0; i < 4; ++i) {
      client.subscribe(topic[i]);
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
    Serial.println();
    Serial.println("-----------------------");

    if(strcmp(topic, schedule_topics[0]) == 0) { // schedule: clasroom specific
    }
    else if(strcmp(topic, schedule_topics[1]) == 0) { // schedule: floor specific
    }
    else if(strcmp(topic, schedule_topics[2]) == 0) { // schedule: building specific
    }
    else if(strcmp(topic, schedule_topics[3]) == 0) { // schedule: all devices
    }
    else if(strcmp(topic, config_topics[0]) == 0) { // config: classroom specific
    }
    else if(strcmp(topic, config_topics[1]) == 0) { // config: floor specific
    }
    else if(strcmp(topic, config_topics[2]) == 0) { // config: building specific
    }
    else if(strcmp(topic, config_topics[3]) == 0) { // config: all devices
    }
    else if(strcmp(topic, ann_topics[0]) == 0) { // announcements: classroom specific
    }
    else if(strcmp(topic, ann_topics[1]) == 0) { // announcements: floor specific
    }
    else if(strcmp(topic, ann_topics[2]) == 0) { // announcements: building specific
    }
    else if(strcmp(topic, ann_topics[3]) == 0) { // announcements: all devices
    }
}

void loop() {
    client.loop();
}
