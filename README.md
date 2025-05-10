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

## MQTT Payload

Each BLE advertisement from a RAPT Pill sensor becomes a single JSON message. Example:

{
  "device": "rapt1",
  "rapt1": {
    "specificGravity": 1.0250,
    "gravityVelocity": 0.0012,
    "temperature": 20.75,
    "battery": 87,
    "rssi": -62,
    "signalQuality": "Good"
  }
}

| Field             | Type   | Units         | Description                                     |        |        |          |
| ----------------- | ------ | ------------- | ----------------------------------------------- | ------ | ------ | -------- |
| `device`          | string | —             | Human-readable sensor identifier                |        |        |          |
| `<device>` object | object | —             | Container for all the measurements              |        |        |          |
| `specificGravity` | float  | SG (unitless) | Current measured gravity, rounded to 4 decimals |        |        |          |
| `gravityVelocity` | float  | SG / second   | Instantaneous change rate of gravity            |        |        |          |
| `temperature`     | float  | °C            | Sensor temperature, rounded to 2 decimals       |        |        |          |
| `battery`         | int    | %             | Battery level in percent                        |        |        |          |
| `rssi`            | int    | dBm           | Raw BLE signal strength                         |        |        |          |
| `signalQuality`   | string | —             | \`"Excellent"                                   | "Good" | "Fair" | "Weak"\` |
