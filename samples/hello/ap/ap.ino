#include "Wireless.hpp"

void setup() {
  Serial.begin(9600);

  wlAPBegin("esp32", "password");
  wlTxAttach(IPAddress(192, 168, 1, 2), 50000);
  wlRxAttach(50000);
}

void loop() {
  static uint32_t count = 0;
  if (wlRxAvailable() > 0) {
    String msg = wlRxRead();
    Serial.print("received: \"");
    Serial.print(msg);
    Serial.println('"');
    ++count;
    Serial.print("sent: ");
    Serial.println(count);
    wlTxWrite(count);
  } else {
    Serial.println("no data");
  }

  delay(1000);
}
