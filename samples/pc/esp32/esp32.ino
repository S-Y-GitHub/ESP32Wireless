#include "Wireless.hpp"

void setup() {
  Serial.begin(9600);

  while (!wlAPBegin("esp32", "password")) {
    Serial.println("ap begin failed");
  }
  Serial.println("ap start");

  wlRxAttach(50000, 0);
  wlRxAttach(50001, 1);
  wlRxAttach(50002, 2);
  wlRxAttach(50003, 3);

  wlTxAttach(IPAddress(192, 168, 1, 2), 50000, 0);
  wlTxAttach(IPAddress(192, 168, 1, 2), 50001, 1);
  wlTxAttach(IPAddress(192, 168, 1, 2), 50002, 2);
  wlTxAttach(IPAddress(192, 168, 1, 2), 50003, 3);
}

void loop() {
  bool b1, b2, b3, b4;
  if (b1 = wlRxAvailable(0) > 0) {
    String msg = wlRxRead(0);
    Serial.print("received (ch0): \"");
    Serial.print(msg);
    Serial.println('"');
  }
  wlTxWrite("Hello. I'm esp32", 0);
  Serial.println("sent (ch0): \"Hello. I'm esp32\"");

  if (b2 = wlRxAvailable(1) > 0) {
    uint32_t val = wlRxRead(1);
    Serial.print("received (ch1): ");
    Serial.println(val);

    ++val;

    wlTxWrite(val, 2);
    Serial.print("sent (ch2): ");
    Serial.println(val);
  }
  if (b3 = wlRxAvailable(2) > 0) {
    uint32_t val = wlRxRead(2);
    Serial.print("received (ch2): ");
    Serial.println(val);

    ++val;

    wlTxWrite(val, 1);
    Serial.print("sent (ch1): ");
    Serial.println(val);
  }
  if (b4 = wlRxAvailable(3) > 0) {
    std::vector<Data> ary = wlRxRead(3);
    Serial.print("received (ch3): [");
    Serial.print(ary[0] ? "true" : "false");
    Serial.print(", ");
    Serial.print(ary[1] ? "true" : "false");
    Serial.print(", ");
    Serial.print(ary[2] ? "true" : "false");
    Serial.print(", ");
    Serial.print(ary[3] ? "true" : "false");
    Serial.println(']');
  }
  wlTxWrite({ b1, b2, b3, b4 }, 3);
  Serial.print("sent (ch3): [");
  Serial.print(b1 ? "true" : "false");
  Serial.print(", ");
  Serial.print(b2 ? "true" : "false");
  Serial.print(", ");
  Serial.print(b3 ? "true" : "false");
  Serial.print(", ");
  Serial.print(b4 ? "true" : "false");
  Serial.println(']');
  delay(1000);
}
