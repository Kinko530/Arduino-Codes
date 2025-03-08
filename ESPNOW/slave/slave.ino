#include <ESP8266WiFi.h>
#include <espnow.h>

// Define LED pins
#define rightL D5  
#define leftL D6  
#define stopL D7 

// Structure to receive data
typedef struct struct_message {
  int a;
  int b;
  int c;
} struct_message;

struct_message myData;

// Variables for LED blinking
unsigned long blinkStartTime = 0;  // Timestamp when blinking starts
bool blinkActive = false;          // Flag to indicate if blinking is active
int blinkLEDPin = -1;              // Pin of the LED currently blinking

// Callback function when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("a: ");
  Serial.println(myData.a);
  Serial.print("b: ");
  Serial.println(myData.b);
  Serial.print("c: ");
  Serial.println(myData.c);
  Serial.println();

  // Check if a, b, or c is 1 and start blinking the corresponding LED
  if (myData.c == 1) {
    blinkLEDPin = rightL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'a'");
  } else if (myData.a == 1) {
    blinkLEDPin = leftL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'b'");
  } else if (myData.b == 1) {
    blinkLEDPin = stopL;
    blinkStartTime = millis();
    blinkActive = true;
    Serial.println("Blinking LED for 'c'");
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set LED pins as output
  pinMode(rightL, OUTPUT);
  pinMode(leftL, OUTPUT);
  pinMode(stopL, OUTPUT);

  // Turn off all LEDs initially
  digitalWrite(rightL, HIGH);
  digitalWrite(leftL, HIGH);
  digitalWrite(stopL, HIGH);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback for receiving data
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
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
