/*
Smart Indoor Air Quality Monitoring and Alert System
*/

#include "VOneMqttClient.h"
#include "DHT.h"
#define DHTTYPE DHT11

// Define device IDs (replace with YOUR device IDs from V-One Cloud)
const char* DHT11sensorID = "da75413f-1065-40c1-a807-35b40f10a045";  // Replace with the device ID for the DHT11 sensor
const char* MQ2sensorID = "9f9d8bb6-058d-45c0-afa0-948ff9c1af22";    // Replace with the device ID for the MQ2 sensor

// Used Pins
const int dht11Pin = 42;    // Right side Maker Port
const int mq2Pin = 4;       // Analog pin for MQ2 gas sensor (connect to a suitable analog pin)
const int redLedPin = 9;    // Digital pin connected to the Red LED
const int greenLedPin = 5;  // Digital pin connected to the Green LED

// Threshold value
const float HUMIDITY_MAX_THRESHOLD = 80.0;  // Maximum acceptable humidity
const float HUMIDITY_MIN_THRESHOLD = 30.0;  // Minimum acceptable humidity
const int TEMPERATURE_THRESHOLD = 40;       // Maximum acceptable temperature (Celsius)
const int GAS_THRESHOLD = 300;              // Dangerous gas level (analog value from MQ2)

// Input sensor definitions
DHT dht(dht11Pin, DHTTYPE);

// Create an instance of VOneMqttClient
VOneMqttClient voneClient;

// Timing variables (last message time)
volatile unsigned long sensorInterval = 900000;  // Default interval: 15 minutes and volatile for dynamic 
unsigned long lastMsgTime = 0;

// Wi-Fi setup function
void setup_wifi() {
  delay(10);

  //connect to wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi and MQTT
  setup_wifi();
  voneClient.setup();

  // Initialize sensors
  dht.begin();
  pinMode(mq2Pin, INPUT);
  pinMode(redLedPin, OUTPUT);  // Set Red LED as output
  pinMode(greenLedPin, OUTPUT);
}

void loop() {
  // Reconnect to MQTT broker if disconnected
  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "Sensor Connection Fail";
    voneClient.publishDeviceStatusEvent(DHT11sensorID, true);
    voneClient.publishDeviceStatusEvent(MQ2sensorID, true);
  }
  voneClient.loop();

  // Read and send sensor data at regular intervals
  unsigned long cur = millis();
  if (cur - lastMsgTime > sensorInterval) {
    lastMsgTime = cur;

    // Read data from DHT11
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Read data from MQ2
    int gasValue = analogRead(mq2Pin);

    // Prepare and publish telemetry data
    JSONVar payloadObject;

    // Publish DHT11 data
    payloadObject["Humidity"] = humidity;
    payloadObject["Temperature"] = temperature;
    voneClient.publishTelemetryData(DHT11sensorID, payloadObject);

    // Publish MQ2 data
    payloadObject = JSONVar();  // Clear the payload object
    payloadObject["Gas detector"] = gasValue;
    voneClient.publishTelemetryData(MQ2sensorID, payloadObject);

    // Trigger alerts for unsafe conditions
    if (gasValue > GAS_THRESHOLD || humidity < HUMIDITY_MIN_THRESHOLD || humidity > HUMIDITY_MAX_THRESHOLD || temperature > TEMPERATURE_THRESHOLD) {
      
      String alertMsg = "Unsafe air quality detected!";
      sensorInterval = 300000;               // Switch to 5 minutes in unsafe conditions
      digitalWrite(redLedPin, HIGH);   // Turn on Red LED
      digitalWrite(greenLedPin, LOW);  // Turn off Green LED
      voneClient.publishDeviceStatusEvent(DHT11sensorID, false, alertMsg.c_str());
      voneClient.publishDeviceStatusEvent(MQ2sensorID, false, alertMsg.c_str());
    } else {
      // Safe conditions 
      sensorInterval = 900000;                // Switch back to 15 minutes in safe conditions
      digitalWrite(redLedPin, LOW);     // Turn off Red LED
      digitalWrite(greenLedPin, HIGH);  // Turn on Green LED
    }
  }
}
