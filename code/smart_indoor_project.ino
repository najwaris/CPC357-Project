/*
Smart Indoor Air Quality Monitoring and Alert System
*/

#include "VOneMqttClient.h"
#include "DHT.h"
#define DHTTYPE DHT11

// Define device IDs (replace with YOUR device IDs from V-One Cloud)
const char* DHT11sensorID = "a8b1d1d4-ffca-4ce1-b251-39c2ee5ddc57";  // Replace with the device ID for the DHT11 sensor
const char* MQ2sensorID = "19135568-a0d3-482b-95f6-9aeede887e77";    // Replace with the device ID for the MQ2 sensor

// Used Pins
const int dht11Pin = 42;    // Right side Maker Port
const int mq2Pin = 4;       // Analog pin for MQ2 gas sensor (connect to a suitable analog pin)
const int redLedPin = 9;    // Digital pin connected to the Red LED
const int greenLedPin = 5;  // Digital pin connected to the Green LED
const int buzzerPin = 12;   // Built-in buzzer pin

// Threshold value
const float HUMIDITY_MAX_THRESHOLD = 80.0;  // Maximum acceptable humidity
const float HUMIDITY_MIN_THRESHOLD = 30.0;  // Minimum acceptable humidity
const int TEMPERATURE_THRESHOLD = 35;       // Maximum acceptable temperature (Celsius)
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

  // Connect to Wi-Fi
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
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
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
  unsigned long lastBuzzerTime = 0;             // Track the last buzzer activation
  const unsigned long buzzerCooldown = 300000;  // Cooldown period (5 minutes)
  if (cur - lastMsgTime > sensorInterval) {
    lastMsgTime = cur;

    // Read data from DHT11
    float humidity = dht.readHumidity();
    int temperature = dht.readTemperature();

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
      sensorInterval = 300000;         // Switch to 1 minutes in unsafe conditions (300000 - 5 mins)
      digitalWrite(redLedPin, HIGH);   // Turn on Red LED
      digitalWrite(greenLedPin, LOW);  // Turn off Green LED

      if ((millis() - lastBuzzerTime > buzzerCooldown) && (gasValue > GAS_THRESHOLD || humidity < HUMIDITY_MIN_THRESHOLD || humidity > HUMIDITY_MAX_THRESHOLD || temperature > TEMPERATURE_THRESHOLD)) {
        lastBuzzerTime = millis();  // Update the last buzzer activation time
        tone(buzzerPin, 2000);      // Activate buzzer at 2kHz
        delay(2000);                // Buzzer sounds for 2 seconds
        noTone(buzzerPin);          // Turn off buzzer
      }

      // Publish alert with status still 'true' (no disconnection)
      String alertMsg = "";
      if (gasValue > GAS_THRESHOLD) {
        alertMsg += "High gas levels detected. ";
      }
      if (humidity < HUMIDITY_MIN_THRESHOLD) {
        alertMsg += "Humidity too low. ";
      }
      if (humidity > HUMIDITY_MAX_THRESHOLD) {
        alertMsg += "Humidity too high. ";
      }
      if (temperature > TEMPERATURE_THRESHOLD) {
        alertMsg += "High temperature detected. ";
      }

      // Send alert message without disconnecting the sensor
      voneClient.publishDeviceStatusEvent(DHT11sensorID, true, alertMsg.c_str());
      voneClient.publishDeviceStatusEvent(MQ2sensorID, true, alertMsg.c_str());
    } else {

      // Safe conditions
      sensorInterval = 900000;          // Switch back to 15 minutes in safe conditions
      digitalWrite(redLedPin, LOW);     // Turn off Red LED
      digitalWrite(greenLedPin, HIGH);  // Turn on Green LED
      noTone(buzzerPin);                // Ensure buzzer is off

      // Send normal status (true)
      voneClient.publishDeviceStatusEvent(DHT11sensorID, true);
      voneClient.publishDeviceStatusEvent(MQ2sensorID, true);
    }
  }
}
