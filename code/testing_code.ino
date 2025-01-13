#include "DHT.h"
#define DHTTYPE DHT11  // DHT11

const int dht11Pin = 42;    // Digital pin connected to the DHT11 sensor
const int mq2Pin = 4;       // Digital pin connected to the mq2 sensor
const int redLedPin = 9;    // Digital pin connected to the Red LED
const int greenLedPin = 5;  // Digital pin connected to the Green LED
const int buzzerPin = 12;

const float HUMIDITY_THRESHOLD = 80.0;  // Maximum acceptable humidity
const int TEMPERATURE_THRESHOLD = 35;   // Maximum acceptable temperature (Celsius)
const int GAS_THRESHOLD = 300;          // Dangerous gas level (analog value from MQ2)

unsigned long buzzerStartTime = 0;
const unsigned long BUZZER_DURATION = 5000;  // 5 seconds

DHT dht(dht11Pin, DHTTYPE);

void setup() {
  Serial.begin(9600);  // Initiate serial communication
  dht.begin();         // Initialize DHT sensor

  pinMode(mq2Pin, INPUT);
  pinMode(redLedPin, OUTPUT);    // Set Red LED as output
  pinMode(greenLedPin, OUTPUT);  // Set Green LED as output
  pinMode(buzzerPin, OUTPUT);    // Set Buzzer as output
}

bool isBuzzerOn = false;
void loop() {
  delay(5000);  // Set a delay between readings

  float h = dht.readHumidity();       // Read humidity
  int t = dht.readTemperature();      // Read temperature
  int mq2Value = analogRead(mq2Pin);  // Read analog value from MQ2 sensor

  // Check whether the given floating-point number argument is a NaN value
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from the DHT sensor");
    return;
  }

  // Print the value to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" degree Celsius, Humidity: ");
  Serial.print(h);
  Serial.println(" %");
  Serial.print("MQ2 Sensor Value: ");
  Serial.println(mq2Value);

  // LED Alert Logic
  if (h > HUMIDITY_THRESHOLD || t > TEMPERATURE_THRESHOLD || mq2Value > GAS_THRESHOLD) {
    digitalWrite(redLedPin, HIGH);   // Turn on Red LED
    digitalWrite(greenLedPin, LOW);  // Turn off Green LED

    if (!isBuzzerOn) {
      buzzerStartTime = millis();
      tone(buzzerPin, 2000);  // Start buzzer at 2 kHz
      isBuzzerOn = true;      // Set flag
    }

    // Stop buzzer after 10 seconds (testing mode)
    if (isBuzzerOn && millis() - buzzerStartTime >= BUZZER_DURATION) {
      noTone(buzzerPin);    // Turn off buzzer
      buzzerStartTime = 0;  // Reset buzzer start time
      isBuzzerOn = false;   // Reset flag
    }

  } else {
    digitalWrite(redLedPin, LOW);     // Turn off Red LED
    digitalWrite(greenLedPin, HIGH);  // Turn on Green LED

    if (isBuzzerOn) {
      noTone(buzzerPin);    // Ensure buzzer is off
      isBuzzerOn = false;   // Reset flag
      buzzerStartTime = 0;  // Reset buzzer timing
    }
  }
}
