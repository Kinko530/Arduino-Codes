#include <ESP8266WiFi.h>
#include <espnow.h>

// Ultrasonic sensor pins
const int trigPin = D0;  // Trigger pin for sensor 1
const int echoPin = D1;  // Echo pin for sensor 1
const int trigP = D2;    // Trigger pin for sensor 2
const int echoP = D3;    // Echo pin for sensor 2

// LED and buzzer pins
const int ledPin = D4;    // LED pin
const int buzzerPin = D5; // Buzzer pin

// Button pins
#define leftB D8
#define rightB D7
#define stopB D6

// Variables to store duration and distance for both sensors
long duration1, distance1;
long duration2, distance2;

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x4C, 0x75, 0x25, 0x2F, 0xEB, 0xA9};

// Structure to send data (only button states)
typedef struct struct_message {
  int a;  // State of left button
  int b; // State of right button
  int c;  // State of stop button
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

// Function to measure distance for sensor 1
void getDist1() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the pulse duration with a timeout of 10,000 microseconds (10 ms)
  duration1 = pulseIn(echoPin, HIGH, 10000);

  // Calculate the distance in centimeters
  if (duration1 == 0) {
    distance1 = -1; // Indicate no valid reading (timeout occurred)
  } else {
    distance1 = duration1 * 0.034 / 2;
  }
}

// Function to measure distance for sensor 2
void getDist2() {
  digitalWrite(trigP, LOW);
  delayMicroseconds(2);
  digitalWrite(trigP, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigP, LOW);

  // Measure the pulse duration with a timeout of 10,000 microseconds (10 ms)
  duration2 = pulseIn(echoP, HIGH, 10000);

  // Calculate the distance in centimeters
  if (duration2 == 0) {
    distance2 = -1; // Indicate no valid reading (timeout occurred)
  } else {
    distance2 = duration2 * 0.034 / 2;
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set pin modes for ultrasonic sensors
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);

  // Set pin modes for LED and buzzer
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  // Set pin modes for buttons
  pinMode(leftB, INPUT_PULLUP);
  pinMode(rightB, INPUT_PULLUP);
  pinMode(stopB, INPUT_PULLUP);

  // Initialize LED and buzzer to OFF
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzerPin, LOW);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  // Measure distances from both sensors
  getDist1();
  getDist2();

  // Print the distances to the Serial Monitor
  Serial.print("Distance1: ");
  if (distance1 == -1) {
    Serial.println("Timeout or no object detected");
  } else {
    Serial.print(distance1);
    Serial.println(" cm");
  }

  Serial.print("Distance2: ");
  if (distance2 == -1) {
    Serial.println("Timeout or no object detected");
  } else {
    Serial.print(distance2);
    Serial.println(" cm");
  }

  // Check if either distance is below 50 cm
  if ((distance1 < 50 && distance1 != -1) || (distance2 < 50 && distance2 != -1)) {
    // Turn on the LED and buzzer
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzerPin, HIGH);
  } else {
    // Turn off the LED and buzzer
    digitalWrite(ledPin, LOW);
    digitalWrite(buzzerPin, LOW);
  }

  // Read button states
  myData.a = digitalRead(leftB);
  myData.b = digitalRead(rightB);
  myData.c = digitalRead(stopB);

  // Send button states via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  // Add a small delay between readings
  delay(10);
}
