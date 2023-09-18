#include "Wireless.hpp"

static const char SSID[] = "esp32";
static const char PASSWORD[] = "password";
static const IPAddress IP(192, 168, 1, 2);

void setup() {
  Serial.begin(9600);

  while (!wlConnect(SSID, PASSWORD, IP)) {
    Serial.println("wireless connect failed");
    delay(10);
  }
  Serial.println("wireless connected");

  wlTxAttach(IPAddress(192, 168, 1, 1), 50000);
  wlRxAttach(50000);
}

void loop() {
  if (wlRxAvailable() > 0) {
    uint32_t count = wlRxRead();
    Serial.print("received: ");
    Serial.println(count);
    wlTxWrite("hello");
    Serial.println("sent: \"hello\"");
  } else {
    Serial.println("no data");
    wlTxWrite("hello");
  }

  delay(1000);
}
