#include <stdint.h>
#include "HardwareSerial.h"
#include "SerialUtil.hpp"
static PacketSerial packetSerial;
static const constexpr uint16_t MAX_PACKET_SIZE = 1024;  // 送受信可能な最大バイト数
static std::queue<Data> rx_buf;                          // 受信バッファ
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static volatile bool mux_flag = false;
static volatile bool run_flag = false;
static volatile bool default_serial_flag = false;
static inline void mutEnter() {
  for (;;) {
    bool got;
    portENTER_CRITICAL(&mux);
    if (!mux_flag) {
      mux_flag = true;
      got = true;
    } else
      got = false;
    portEXIT_CRITICAL(&mux);
    if (got)
      return;
    delay(1);
  }
}
static inline void mutExit() {
  portENTER_CRITICAL(&mux);
  mux_flag = false;
  portEXIT_CRITICAL(&mux);
}

void serialWrite(const Data& data) {
  uint8_t buf[MAX_PACKET_SIZE];
  size_t size = data.serialize(buf, MAX_PACKET_SIZE);
  if (size == 0) return;
  mutEnter();
  packetSerial.send(buf, size);
  mutExit();
}

static void serialReceiveTask(void*) {
  bool r;
  do {
    mutEnter();
    r = run_flag;
    if (r) packetSerial.update();
    mutExit();
  } while (r);
}

static void packetHandler(const uint8_t* buf, size_t size) {
  Data data;
  if (Data::deserialize(buf, size, &data))
    rx_buf.push(data);
}

bool serialBegin() {
  bool res;
  mutEnter();
  if (run_flag)
    res = false;
  else {
    res = true;
    default_serial_flag = true;
    Serial.begin(9600);
    packetSerial.setStream(&Serial);
    packetSerial.setPacketHandler(packetHandler);
    xTaskCreatePinnedToCore(serialReceiveTask, "serialReceiveTask", 4028, NULL, 5, NULL, 1);
  }
  mutExit();
  return res;
}

bool serialBegin(Stream& stream) {
  bool res;
  mutEnter();
  if (run_flag)
    res = false;
  else {
    res = true;
    packetSerial.setStream(&stream);
    packetSerial.setPacketHandler(packetHandler);
    xTaskCreatePinnedToCore(serialReceiveTask, "serialReceiveTask", 4028, NULL, 5, NULL, 1);
  }
  mutExit();
  return res;
}

bool serialEnd() {
  bool res;
  mutEnter();
  if (run_flag) {
    res = true;
    run_flag = false;
    if (default_serial_flag) Serial.end();
    default_serial_flag = false;
  } else
    res = false;
  mutExit();
  return res;
}

size_t serialAvailable() {
  size_t s;
  mutEnter();
  s = rx_buf.size();
  mutExit();
  return s;
}

Data serialRead() {
  Data data;
  mutEnter();
  if (rx_buf.empty())
    data = nullptr;
  else {
    data = rx_buf.front();
    rx_buf.pop();
  }
  mutExit();
  return data;
}