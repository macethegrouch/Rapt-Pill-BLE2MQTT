# BLE Sensor Gateway for RAPT Pill

This project runs on an ESP32 and scans for BLE broadcasts from RAPT Pill hydrometers. It decodes the payload and sends the data to a backend HTTP API using Wi-Fi and MQTT.

## Features

- BLE scanning for RAPT sensors
- Parses SG, temperature, velocity, and battery from BLE advertisements
- Sends data to MQTT 
- Duplicate message filtering


## Getting Started

There are two ESP32 boards set at the platformio.ini. If you have other type of board, configure it there.
Before building for the first time copy secrets.example.h to secrets.h and fill in your own values.

After this build and upload to ESP32.
