#ifndef SECRETS_H
#define SECRETS_H

// Wi-Fi Credentials
const char* WIFI_SSID = "xxx";
const char* WIFI_PASSWORD = "xxx";

// MQTT Credentials
//const char* MQTT_SERVER = "homeassistant.local"; // Use IP if needed
const char* MQTT_SERVER = "x.x.x.x"; // Use IP if needed
const char* MQTT_USER = "xxx";  // MQTT username
const char* MQTT_PASSWORD = "xxx";  // MQTT users password
const char* MQTT_TOPIC = "xxx";

// Monitored Devices
const int NUM_MONITORED_DEVICES = 2;  // Number of deviced to listen
const char* MONITORED_MACS[NUM_MONITORED_DEVICES] = {
    "aa:bb:cc:dd:ee:ff",  // Rapt Pill MAC address
    "aa:bb:cc:dd:ee:ff"   // Rapt Pill MAC address
};

const char* DEVICE_NAMES[NUM_MONITORED_DEVICES] = {
    "xxx2",   // Unique identifier for first device
    "xxx1"  // Unique identifier for second device
};

#endif // SECRETS_H
