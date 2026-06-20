/*
 * Wireless Relay Control System — Receiver
 * Author: Jackson Stansell
 * Date: June 2026
 *
 * Description:
 * This ESP32 is mounted on a relay module and acts as a heartbeat receiver
 * using ESP-NOW. As long as it receives a heartbeat signal from the paired
 * transmitter within the timeout window, the relay stays closed and keeps
 * the connected device powered. If the heartbeat signal is lost, the relay
 * opens and cuts power to the device automatically.
 *
 * Relay Pin: GPIO 16
 * Heartbeat Timeout: 800ms
 * Channel: 1
 */

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

// ─── Configuration ────────────────────────────────────────────────────────────
#define RELAY_PIN 16          // GPIO pin controlling the relay on the module
#define HEARTBEAT_TIMEOUT 800 // Time in ms before relay opens if no heartbeat received
                              // Set to 4x the transmitter interval for reliability

// ─── State ────────────────────────────────────────────────────────────────────
// Marked volatile because it is written in an interrupt callback and read in loop()
volatile unsigned long lastHeartbeatTime = 0;

// ─── Receive Callback ─────────────────────────────────────────────────────────
// Fires every time a packet is received from the transmitter
// Only updates the timestamp — relay control is handled in loop()
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  lastHeartbeatTime = millis();
}

// ─── Setup ────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Initialize relay pin and ensure it starts OFF (fail-safe default)
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Initialize WiFi in station mode and lock to Channel 1 to match transmitter
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  // Initialize ESP-NOW — halt if initialization fails
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Register receive callback to track incoming heartbeat signals
  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("Receiver ready — waiting for heartbeat...");
}

// ─── Main Loop ────────────────────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // Relay stays ON only if a heartbeat has been received AND it was recent enough
  // lastHeartbeatTime > 0 prevents the relay from switching on before any signal arrives
  bool heartbeatAlive = (lastHeartbeatTime > 0) &&
                        ((now - lastHeartbeatTime) < HEARTBEAT_TIMEOUT);

  // Drive relay based on heartbeat state
  digitalWrite(RELAY_PIN, heartbeatAlive ? HIGH : LOW);

  delay(500);
}
