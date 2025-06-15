# eCal
This repository contains the necessary source code to reproduce a dynamic e-paper classroom schedule, made as a bachelor thesis in Facultat d'Informàtica de Barcelona, UPC. It is meant for the classrooms of the faculty (FIB), but can be adapted if necessary.

## Contents
- [Features](#features)
- [Components](#components)
  * [Wiring](#wiring)
- [Project Structure](#project-structure)

## Features
- Ultra-low power.
- Long battery life.
- 3-color e-paper display (red, black & white).
- Easily rechargable.
- 3D printed fitted casing.

## Components
The following components were used to create the project:
- 7.5inch e-paper (B) by Waveshare
- Firebeetle 2 ESP32-C6 by DFRobot
- Li-Po battery 3.7V 2500mAh with JST-PH connector
- Solar Panel - 5V 60mA - 68x37mm

### Wiring
To correctly setup the device, use the following wiring:
![a61561e88b862fee8ae2c5b064a682b26159ba7b](https://github.com/user-attachments/assets/0f53115c-a3e0-4b9a-8cb7-ebed6a40a378)
Other connections can be used, as long as the pin definition in eCal/schedule_lib.h is changed to reflect those changes.

## Project Structure
The repository has three folders. Each are linked to a specific device/server.
- [eCal](https://github.com/ammmec/eCal/tree/main/eCal): source code for the microcontroller.
- [eCal_PC](https://github.com/ammmec/eCal/tree/main/eCal_PC): source code for the daily script.
- [raco](https://github.com/ammmec/eCal/tree/main/raco): proposed addition for _el Racó_.
- [stl](https://github.com/ammmec/eCal/tree/main/stl): casing files to 3D print.
Inside the eCal and eCal_PC folders, a file needs to be added, which were ommitted in the repository for safety reasons.
The template for the file in eCal, called wifi_secure.h is the following:
```C
#ifndef WIFI_SECURE_H
#define WIFI_SECURE_H
// WiFi
inline const char* ssid = WIFI NAME GOES HERE;
inline const char* password = PASSWORD OF WIFI GOES HERE;

// MQTT Broker
inline const char *mqtt_broker = LINK TO MQTT BROKER;
inline const char *mqtt_username = USERNAME IN BROKER;
inline const char *mqtt_password = PASSWORD IN BROKER;
inline const int mqtt_port = BROKER'S PORT;
inline static const char ca_cert[]
PROGMEM = R"EOF(
  CERTIFICATE FOR SECURE CONNECTION
)EOF";
#endif
```
And the template for the file in eCal_PC, called data_secure.py:
```python
CLIENT_ID = PUBLIC APPLICATION OF THE RACÓ'S API

username = MQTT USERNAME
password = MQTT PASSWORD

broker = LINK TO MQTT BROKER
port = BROKER'S PORT
```
