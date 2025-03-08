#include <esp_now.h>
#include <WiFi.h>

// Replace with the MAC address of the ESP8266 (slave)
uint8_t esp8266MacAddress[] = {0x4C, 0x75, 0x25, 0x2F, 0xEB, 0xA9}; // Replace with ESP8266's MAC address

// Define the GPIO pins for the push buttons
#define rightB 25
#define leftB 26
#define stopB 27

// Ultrasonic sensor pins
const int trigPin = 13;  // Trigger pin for sensor 1
const int echoPin = 12;  // Echo pin for sensor 1
const int trigP = 14;    // Trigger pin for sensor 2
const int echoP = 33;    // Echo pin for sensor 2

// LED and buzzer pins
#define buz 32
#define rearW 19
#define frontW 18

// Variables to store duration and distance for both sensors
long duration1, distance1;
long duration2, distance2;

// Variables for non-blocking buzzer and LED control
unsigned long previousMillis1 = 0; // Stores the last time buzzerSound1 was played
unsigned long previousMillis2 = 0; // Stores the last time buzzerSound2 was played
unsigned long previousBlinkMillis = 0; // Stores the last time rearW was toggled
const long buzzerInterval1 = 300;  // Interval for buzzerSound1 (300 ms)
const long buzzerInterval2 = 600;  // Interval for buzzerSound2 (600 ms)
const long blinkInterval = 500;    // Interval for rearW blinking (500 ms)
bool rearWState = LOW;             // Current state of rearW (for blinking)

// Structure to send data (button states)
typedef struct struct_message {
  int a;  // State of left button
  int b;  // State of right button
  int c;  // State of stop button
} struct_message;

// Create a struct_message called myData
struct_message myData;

void setup() {
  Serial.begin(115200);

  // Set up button pins as inputs
  pinMode(rightB, OUTPUT);
  pinMode(leftB, OUTPUT);
  pinMode(stopB, OUTPUT);

  // Set pin modes for ultrasonic sensors
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(trigP, OUTPUT);
  pinMode(echoP, INPUT);

  // Set pin modes for LED and buzzer
  pinMode(buz, OUTPUT);
  pinMode(rearW, OUTPUT);
  pinMode(frontW, OUTPUT);

  // Initialize LED and buzzer to OFF
  digitalWrite(buz, LOW);
  digitalWrite(rearW, LOW);
  digitalWrite(frontW, LOW);

  // Initialize WiFi in STA mode
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer (ESP8266)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, esp8266MacAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
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

// Function to play a specific buzzer sound for distance1
void buzzerSound1() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis1 >= buzzerInterval1) {
    previousMillis1 = currentMillis; // Save the last time the buzzer was toggled
    tone(buz, 1000, 100); // Play a 1 kHz tone for 100 ms
  }
}

// Function to play a specific buzzer sound for distance2
void buzzerSound2() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis2 >= buzzerInterval2) {
    previousMillis2 = currentMillis; // Save the last time the buzzer was toggled
    tone(buz, 500, 200); // Play a 500 Hz tone for 200 ms
  }
}

// Function to blink rearW
void blinkRearW() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis; // Save the last time rearW was toggled
    rearWState = !rearWState; // Toggle the state of rearW
    digitalWrite(rearW, rearWState);
  }
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

  // Check if both distances are less than 50 cm
  if (distance1 < 50 && distance1 != -1 && distance2 < 50 && distance2 != -1) {
    // Continuous buzzer and blinking rearW
    tone(buz, 1000); // Continuous 1 kHz tone
    blinkRearW();    // Blink rearW
    digitalWrite(frontW, HIGH); // Turn on frontW
  } else {
    // Normal operation for distance1
    if (distance1 < 50 && distance1 != -1) {
      digitalWrite(frontW, HIGH); // Turn on front warning light
      buzzerSound1();             // Play buzzer sound for distance1
    } else {
      digitalWrite(frontW, LOW);  // Turn off front warning light
    }

    // Normal operation for distance2
    if (distance2 < 50 && distance2 != -1) {
      digitalWrite(rearW, HIGH);  // Turn on rear warning light
      buzzerSound2();             // Play buzzer sound for distance2
    } else {
      digitalWrite(rearW, LOW);   // Turn off rear warning light
      noTone(buz);               // Stop the buzzer
    }
  }

  // Read the state of the buttons
  myData.a = digitalRead(leftB);
  myData.b = digitalRead(rightB);
  myData.c = digitalRead(stopB);

  // Send the button states to the ESP8266
  esp_now_send(esp8266MacAddress, (uint8_t *)&myData, sizeof(myData));

  // Print the button states to the Serial Monitor for debugging
  Serial.print("Button 1: ");
  Serial.print(myData.a);
  Serial.print(" | Button 2: ");
  Serial.print(myData.b);
  Serial.print(" | Button 3: ");
  Serial.println(myData.c);

  // Add a small delay to avoid flooding the ESP-NOW network
  delay(20);
}
