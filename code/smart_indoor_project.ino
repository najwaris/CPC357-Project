/*
Smart Indoor Air Quality Monitoring and Alert System
*/

#include "VOneMqttClient.h"
#include "DHT.h"
#define DHTTYPE DHT11

// Define device IDs (replace with YOUR device IDs from V-One Cloud)
const char* DHT11sensorID = "a8b1d1d4-ffca-4ce1-b251-39c2ee5ddc57";    // Device ID for the DHT11 sensor
const char* MQ2sensorID = "19135568-a0d3-482b-95f6-9aeede887e77";      // Device ID for the MQ2 sensor
const char* RelayActuatorID = "027d42af-8ac1-4cf8-914a-0f6a7ebdc2b1";  // Device ID for the Relay actuator
const char* ButtonID = "94c83ecd-df3f-4607-a3f8-d6e423c54b1c";         // Device ID for the Push Button

// Used Pins
const int dht11Pin = 42;    // Right side Maker Port
const int mq2Pin = 4;       // Analog pin for MQ2 gas sensor (connect to a suitable analog pin)
const int redLedPin = 9;    // Digital pin connected to the Red LED
const int greenLedPin = 5;  // Digital pin connected to the Green LED
const int buzzerPin = 12;   // Built-in buzzer pin
const int relayPin = 39;    // Optocoupler Relay pin
const int buttonPin = 38;   // Button pin

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
unsigned long buzzerStartTime = 0;

// flag for logic in the system
bool buzzerActive = false;
bool propellerActive = false;
bool propellerManualOverride = false;

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
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void handleBuzzer() {
  if (buzzerActive) {
    if (millis() - buzzerStartTime > 2000) {
      noTone(buzzerPin);
      buzzerActive = false;
    }
  }
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
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);  // External button with pull-up resistor

  // Set initial states: all are OFF at startup
  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, LOW);
  digitalWrite(buzzerPin, LOW);
  digitalWrite(relayPin, false);
}

void loop() {

  // Reconnect to MQTT broker if disconnected
  if (!voneClient.connected()) {
    voneClient.reconnect();
    String errorMsg = "Sensor Connection Fail";
    voneClient.publishDeviceStatusEvent(DHT11sensorID, true);
    voneClient.publishDeviceStatusEvent(MQ2sensorID, true);
    voneClient.publishDeviceStatusEvent(RelayActuatorID, true);
  }

  voneClient.loop();
  handleBuzzer();

  // Read and send sensor data at regular intervals
  unsigned long cur = millis();

  if (cur - lastMsgTime > sensorInterval || lastMsgTime == 0) {
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

      // Turn on the propeller if these conditions are met
      if (humidity < HUMIDITY_MIN_THRESHOLD || temperature > TEMPERATURE_THRESHOLD || gasValue > GAS_THRESHOLD) {

        digitalWrite(relayPin, true);
        propellerActive = true;

        sensorInterval = 300000;         // Switch to 5 minutes
        digitalWrite(redLedPin, HIGH);   // Turn on Red LED
        digitalWrite(greenLedPin, LOW);  // Turn off Green LED

        if (!buzzerActive) {
          tone(buzzerPin, 2000);
          buzzerStartTime = millis();
          buzzerActive = true;
        }

        String alertMsg = "";
        if (gasValue > GAS_THRESHOLD) {
          alertMsg += "High gas levels detected. ";
        }
        if (humidity < HUMIDITY_MIN_THRESHOLD) {
          alertMsg += "Humidity too low. ";
        }
        if (temperature > TEMPERATURE_THRESHOLD) {
          alertMsg += "High temperature detected. ";
        }

        // Send alert message without disconnecting the sensor
        voneClient.publishDeviceStatusEvent(DHT11sensorID, true, alertMsg.c_str());
        voneClient.publishDeviceStatusEvent(MQ2sensorID, true, alertMsg.c_str());
        voneClient.publishDeviceStatusEvent(RelayActuatorID, true);
      } else {

        // Skip propeller if it doesnt meet the conditions
        sensorInterval = 300000;         // Switch to 5 minutes
        digitalWrite(redLedPin, HIGH);   // Turn on Red LED
        digitalWrite(greenLedPin, LOW);  // Turn off Green LED

        if (!buzzerActive) {
          tone(buzzerPin, 2000);
          buzzerStartTime = millis();
          buzzerActive = true;
        }

        // Publish alert with status still 'true' (no disconnection)
        String alertMsg = "";
        if (humidity > HUMIDITY_MAX_THRESHOLD) {
          alertMsg += "Humidity too high. ";
        }

        // Send alert message without disconnecting the sensor
        voneClient.publishDeviceStatusEvent(DHT11sensorID, true, alertMsg.c_str());
        voneClient.publishDeviceStatusEvent(MQ2sensorID, true, alertMsg.c_str());
        voneClient.publishDeviceStatusEvent(RelayActuatorID, false);
      }
    } else {

      // Safe conditions
      sensorInterval = 900000;          // Switch back to 15 minutes in safe conditions
      digitalWrite(redLedPin, LOW);     // Turn off Red LED
      digitalWrite(greenLedPin, HIGH);  // Turn on Green LED

      noTone(buzzerPin);
      buzzerActive = false;

      // Turn off propeller if button is pressed
      if (!digitalRead(buttonPin)) {
        digitalWrite(relayPin, false);
        propellerActive = false;
      }

      // Send normal status (true)
      voneClient.publishDeviceStatusEvent(DHT11sensorID, true);
      voneClient.publishDeviceStatusEvent(MQ2sensorID, true);
      voneClient.publishDeviceStatusEvent(RelayActuatorID, false);
    }
  }

  // Turn off propeller manually
  if (!digitalRead(buttonPin) && propellerActive) {
    digitalWrite(relayPin, false);
    propellerActive = false;
  }
}
