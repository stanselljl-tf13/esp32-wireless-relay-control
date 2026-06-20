/*
 * Wireless Relay Control System — Transmitter
 * Author: [Your Name]
 * Date: June 2026
 *
 * Description:
 * This ESP32 acts as a heartbeat transmitter using ESP-NOW.
 * It continuously sends a heartbeat signal to a paired receiver ESP32
 * mounted on a relay module. As long as the receiver detects the heartbeat,
 * the relay stays closed and the connected device stays powered.
 * Stopping the transmitter cuts power to the device within the timeout window.
 *
 * Heartbeat Interval: 200ms
 * Receiver Timeout: 800ms
 * Channel: 1
 */

#include <esp_now.h>
#include <WiFi.h>

// ─── Receiver MAC Address ─────────────────────────────────────────────────────
// Only one address should be active at a time — comment out the unused one
uint8_t relayMacAddress[] = {0xD8, 0xBC, 0x38, 0xFD, 0x94, 0x8C}; // Receiver #2
                          //{0xC0, 0x5D, 0x89, 0xDE, 0x34, 0x90}  // Receiver #1

// ─── Send Callback ────────────────────────────────────────────────────────────
// Fires after every transmission attempt and reports whether the receiver
// acknowledged the packet at the MAC layer
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Connection Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "CONNECTED / SUCCESS" : "DISCONNECTED / FAIL");
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  // Initialize WiFi in station mode
  // Channel is forced to 1 via peer config below to match the receiver
  WiFi.mode(WIFI_STA);
  WiFi.printDiag(Serial); 

  // Initialize ESP-NOW — halt if initialization fails
  if (esp_now_init() != ESP_OK) return;

  // Register send callback to monitor connection status
  esp_now_register_send_cb(OnDataSent);

  // Register the receiver ESP32 as a peer on Channel 1 with no encryption
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, relayMacAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
  // Send a single byte heartbeat payload to the receiver ESP32
  uint8_t msg = 1;
  esp_now_send(relayMacAddress, &msg, sizeof(msg));
  Serial.println("Heartbeat Sent to Relay...");
  
  // digitalWrite(2, HIGH);
  // delay(100);
  // digitalWrite(2, LOW);
  
  // Transmit every 200ms — receiver timeout is 800ms (4x ratio for reliability)
  delay(200); 
}
