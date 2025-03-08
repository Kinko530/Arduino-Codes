/*
 * This is the master code using ESP8266.
 * Written by Kinko
 */
#include <ESP8266WiFi.h>
#include <espnow.h>
#define leftB D8
#define rightB D7
#define stopB D6
// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x4C, 0x75, 0x25, 0x2F, 0xEB, 0xA9};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int a;
  int b;
  int c;
} struct_message;

// Create a struct_message called myData
struct_message myData;

//unsigned long lastTime = 0;  
//unsigned long timerDelay = 2000;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pinMode(leftB, OUTPUT);
  pinMode(rightB, OUTPUT);
  pinMode(stopB, OUTPUT);
//  pinMode(buz, OUTPUT);
//  digitalWrite(buz, LOW);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
    // Set values to send
    myData.a = digitalRead(rightB);
    myData.b = digitalRead(leftB);
    myData.c = digitalRead(stopB);

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    delay(10);
}
