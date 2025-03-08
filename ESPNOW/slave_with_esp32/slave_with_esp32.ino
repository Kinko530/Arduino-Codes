#include <espnow.h>
#include <ESP8266WiFi.h>

// Structure to receive data
typedef struct struct_message {
  int a;
  int b;
  int c;
} struct_message;

struct_message myData;

// Define LED pins
#define rightL D6
#define leftL D5
#define stopL D7

// Variables to track LED blink timers
unsigned long blinkStartTime = 0;  // Timestamp when blinking starts
bool blinkActive = false;          // Flag to indicate if blinking is active
int blinkLEDPin = -1;              // Pin of the LED currently blinking


// Callback function when data is received
void onDataReceived(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));

  // Print the received button states
  Serial.print("Button 1: ");
  Serial.print(myData.a);
  Serial.print(" | Button 2: ");
  Serial.print(myData.b);
  Serial.print(" | Button 3: ");
  Serial.println(myData.c);

  if (myData.b == 1) {
    blinkLEDPin = rightL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'a'");
  } else if (myData.c == 1) {
    blinkLEDPin = leftL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'b'");
  } else if (myData.a == 1) {
    blinkLEDPin = stopL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'c'");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(rightL, OUTPUT);
  pinMode(leftL, OUTPUT);
  pinMode(stopL, OUTPUT);

  // Turn off all LEDs initially
  digitalWrite(rightL, HIGH);
  digitalWrite(leftL, HIGH);
  digitalWrite(stopL, HIGH);
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP8266 role to SLAVE
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  // Register callback function for receiving data
  esp_now_register_recv_cb(onDataReceived);
}

void loop() {
  // Handle LED blinking
  if (blinkActive) {
    unsigned long currentTime = millis();

    // Blink the LED for 10 seconds
    if (currentTime - blinkStartTime <= 10000) {  // 10 seconds (changed from 5000 to 10000)
      // Toggle the LED state every 500ms
      if (currentTime % 1000 < 500) {
        digitalWrite(blinkLEDPin, LOW);  // Turn on the LED
      } else {
        digitalWrite(blinkLEDPin, HIGH);   // Turn off the LED
      }
    } else {
      // Stop blinking after 10 seconds
      digitalWrite(blinkLEDPin, HIGH);  // Turn off the LED
      blinkActive = false;             // Reset the blink flag
      Serial.println("Blinking stopped");
    }
  }

  // Add a small delay to avoid overloading the loop
  delay(10);
}
