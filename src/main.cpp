#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "secrets.h"  

#define SCAN_TIME 10 // seconds
#define BigEndianU16(a0, a1) (((a0) << 8) | (a1))

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER;
const char* mqtt_user = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;
const char* mqtt_topic = MQTT_TOPIC;

extern PubSubClient client; 
void sendData(const String &jsonPayload);  


unsigned long lastScanStart = 0;
bool scanning = false;
const unsigned long SCAN_INTERVAL_MS = 10000; 

union FloatRaw {
    float fval;
    uint32_t ival;
};

float decodeFloat(const std::vector<uint8_t>& data, size_t index) {
    FloatRaw raw;
    raw.ival = (data[index] << 24) | (data[index + 1] << 16) | (data[index + 2] << 8) | data[index + 3];
    return raw.fval;
}

int16_t decodeInt16(const std::vector<uint8_t>& data, size_t index) {
    return (data[index] << 8) | data[index + 1];
}

uint16_t decodeUInt16(const std::vector<uint8_t>& data, size_t index) {
    return (data[index] << 8) | data[index + 1];
}

std::unordered_set<std::string> receivedMessages;
std::string latestData;


void processData(const std::string &input, const char* deviceName, int rssi) {
    if (input.size() < 24) {
        Serial.println("Invalid data length!");
        return;
    }

    std::vector<uint8_t> data(input.begin(), input.end());

    // Header check
    if (!(data[0]==0x52 && data[1]==0x41 && data[2]==0x50 && data[3]==0x54)) {
        Serial.println("Invalid message header!");
        return;
    }

    // Decode payload fields
    uint8_t cc = data[6];
    float gravityVelocity   = (cc == 0x01) ? decodeFloat(data, 7) : 0.0f;
    uint16_t tempRaw        = BigEndianU16(data[11], data[12]);
    float temperature       = (tempRaw/128.0f) - 273.15f;
    float specificGravity   = decodeFloat(data, 13) / 1000.0f;
    uint16_t batteryRaw     = BigEndianU16(data[23], data[22]);
    float batteryPercentage = std::min(batteryRaw/256.0f, 100.0f);

    // Signal quality based on RSSI
    std::string signalQuality;
    if      (rssi >= -50) signalQuality = "Excellent";
    else if (rssi >= -70) signalQuality = "Good";
    else if (rssi >= -85) signalQuality = "Fair";
    else                  signalQuality = "Weak";

    // Build JSON
    DynamicJsonDocument doc(1024);
    doc["device"] = deviceName;
    auto obj = doc.createNestedObject(deviceName);
    obj["gravityVelocity"] = gravityVelocity;
    obj["temperature"]     = round(temperature*100.0f)/100.0f;
    obj["specificGravity"] = round(specificGravity*10000.0f)/10000.0f;
    obj["battery"]         = batteryPercentage;
    obj["rssi"]            = rssi;
    obj["signalQuality"]   = signalQuality.c_str();

    String jsonPayload;
    serializeJson(doc, jsonPayload);
    sendData(jsonPayload);
}


// Helper function to convert binary string to hex representation
std::string bytesToHex(const std::string &data) {
    std::stringstream ss;
    for (unsigned char c : data) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return ss.str();
}

void onResult(BLEScanResults scanResults) {
    for (int i = 0; i < scanResults.getCount(); i++) {
        BLEAdvertisedDevice device = scanResults.getDevice(i);
        std::string devAddress = device.getAddress().toString();
        int rssi = device.getRSSI();  // proper RSSI

        for (int j = 0; j < NUM_MONITORED_DEVICES; j++) {
            // Case-insensitive compare so uppercase/lowercase MACs match
            if (strcasecmp(devAddress.c_str(), MONITORED_MACS[j]) == 0) {
                std::string rawData = device.getManufacturerData();
                std::string hexData;
                // convert to hex-string for dedupe
                std::stringstream ss;
                for (auto c : rawData) {
                    ss << std::hex << std::setw(2) << std::setfill('0') << (int)(uint8_t)c;
                }
                hexData = ss.str();

                if (receivedMessages.insert(hexData).second) {
                    Serial.printf("New data from %s (%s)\n",
                                  MONITORED_MACS[j], DEVICE_NAMES[j]);
                    processData(rawData, DEVICE_NAMES[j], rssi);
                }
                break;
            }
        }
    }
}

WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {  
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
    } else {
        Serial.println("WiFi connection failed. Continuing without WiFi.");
    }
}


// Function to reconnect to MQTT Broker
void reconnect() {
    int attempts = 0;
    while (!client.connected() && attempts < 5) {  
        Serial.print("Connecting to MQTT...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("Connected to MQTT!");
            return;
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Retrying in 5 seconds...");
            delay(5000);
            attempts++;
        }
    }
    Serial.println("MQTT connection failed after 5 attempts.");
}


void sendData(const String &jsonPayload) {
    if (client.publish(MQTT_TOPIC, jsonPayload.c_str())) {
        Serial.println("Published JSON to MQTT:");
        Serial.println(jsonPayload);
    } else {
        Serial.println("MQTT Publish Failed!");
    }
}


void setup() {
    delay(1000); // Give serial monitor time to connect
    Serial.begin(115200);
    Serial.println("Starting up..."); 
    BLEDevice::init("ESP32_BLE_Listener");
    setup_wifi();
    client.setServer(mqtt_server, 1883);
}

void loop() {
    unsigned long currentMillis = millis();
  
    // If not scanning and it's time to start a new scan:
    if (!scanning && (currentMillis - lastScanStart >= SCAN_INTERVAL_MS)) {
      BLEScan *pBLEScan = BLEDevice::getScan();
      pBLEScan->setActiveScan(true);
      // Start a non-blocking scan (second parameter set to true)
      pBLEScan->start(SCAN_TIME, true);
      scanning = true;
      lastScanStart = currentMillis;
      Serial.println("BLE scan started non-blocking.");
    }
  
    // Check if a scan is in progress and if the scan duration has elapsed:
    if (scanning && (currentMillis - lastScanStart >= SCAN_TIME * 1000UL)) {
      BLEScan *pBLEScan = BLEDevice::getScan();
      BLEScanResults results = pBLEScan->getResults(); 
      onResult(results);                             
      pBLEScan->clearResults();                       
      scanning = false;                       
      // Clear duplicate filtering set after processing the results.
      receivedMessages.clear();
      Serial.println("BLE scan completed and results processed.");
    }
  
    // Maintain MQTT connectivity and processing
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
