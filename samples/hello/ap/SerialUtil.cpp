#include "HardwareSerial.h"
#include "SerialUtil.hpp"

static const constexpr uint16_t MAX_PACKET_SIZE = 256;           // 送受信可能な最大バイト数
static const constexpr uint32_t RECEIVE_TASK_STACK_SIZE = 4096;  // 受信タスクのスタックメモリサイズ
static const constexpr UBaseType_t RECEIVE_TASK_PRIORITY = 5;    // 受信タスクの優先度
static const constexpr BaseType_t RECEIVE_TASK_CORE = 1;         // 受信タスクを実行するコア

static TaskHandle_t task_handle;
static PacketSerial packetSerial;
static std::queue<Data> rx_buf;  // 受信バッファ
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
  for (;;) {
    mutEnter();
    packetSerial.update();
    mutExit();
    delay(1);
  }
}

static void packetHandler(const uint8_t* buf, size_t size) {
  Data data;
  if (Data::deserialize(buf, size, &data))
    rx_buf.push(data);
}

static void initPacketSerial(Stream& stream) {
  run_flag = true;
  packetSerial.setStream(&stream);
  packetSerial.setPacketHandler(packetHandler);
  xTaskCreatePinnedToCore(serialReceiveTask, "serialReceiveTask", RECEIVE_TASK_STACK_SIZE, nullptr, RECEIVE_TASK_PRIORITY, &task_handle, RECEIVE_TASK_CORE);
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
    initPacketSerial(Serial);
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
    initPacketSerial(stream);
  }
  mutExit();
  return res;
}

bool serialEnd() {
  bool res;
  mutEnter();
  if (run_flag) {
    res = true;
    vTaskDelete(task_handle);
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