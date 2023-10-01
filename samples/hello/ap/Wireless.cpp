#include "Wireless.hpp"

// 送受信可能な最大バイト数
static const constexpr uint16_t MAX_PACKET_SIZE = 256;

static AsyncUDP udp;

bool wlConnect(const char *ssid, const char *password, IPAddress ip, IPAddress gateway, IPAddress subnet) {
  if (!WiFi.config(ip, gateway, subnet))
    return false;
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
    delay(10);
  return true;
}

bool wlAPBegin(const char *ssid, const char *password, IPAddress ip, IPAddress gateway, IPAddress subnet) {
  if (!WiFi.softAP(ssid, password))
    return false;
  if (!WiFi.softAPConfig(ip, gateway, subnet))
    return false;
  return true;
}

bool wlDisconnect() {
  return WiFi.disconnect(true, true);
}


struct Address {
  IPAddress ip;
  uint16_t port;

  bool operator==(const Address &other) const noexcept {
    return other.ip == ip && other.port == port;
  }

  bool operator!=(const Address &other) const noexcept {
    return !operator==(other);
  }
};


class TxChannel {
private:
  std::vector<Address> _addresses;
public:
  TxChannel(IPAddress ip, uint16_t port)
    : _addresses{ Address{ ip, port } } {}

  void send(const Data &data) {
    uint8_t buf[MAX_PACKET_SIZE];
    size_t size = data.serialize(buf, MAX_PACKET_SIZE);
    if (size != 0)
      for (const Address &address : _addresses)
        udp.writeTo(buf, size, address.ip, address.port);
  }

  void attach(IPAddress ip, uint16_t port) {
    Address adr{ ip, port };
    for (const Address &address : _addresses)
      if (address == adr) return;
    _addresses.push_back(adr);
  }

  void detach(IPAddress ip, uint16_t port) {
    Address adr{ ip, port };
    for (auto it = _addresses.begin(); it != _addresses.end(); ++it)
      if (*it == adr) {
        _addresses.erase(it);
        break;
      }
  }

  void detach(IPAddress ip) {
    for (auto it = _addresses.begin(); it != _addresses.end();)
      if (it->ip == ip)
        it = _addresses.erase(it);
      else
        ++it;
  }

  void detach(uint16_t port) {
    for (auto it = _addresses.begin(); it != _addresses.end();)
      if (it->port == port)
        it = _addresses.erase(it);
      else
        ++it;
  }
};

static std::unordered_map<uint8_t, TxChannel> tx_channels;

void wlTxAttach(IPAddress ip, uint16_t port, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    found->second.attach(ip, port);
  else
    tx_channels.emplace(channel, TxChannel(ip, port));
}

void wlTxDetach(IPAddress ip, uint16_t port, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    found->second.detach(ip, port);
}

void wlTxDetach(IPAddress ip, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    found->second.detach(ip);
}

void wlTxDetach(uint8_t port, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    found->second.detach(port);
}

void wlTxWrite(const Data *buf, size_t size, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    for (size_t i = 0; i < size; ++i)
      found->second.send(buf[i]);
}

void wlTxWrite(const Data &data, uint8_t channel) {
  auto found = tx_channels.find(channel);
  if (found != tx_channels.end())
    found->second.send(data);
}

class RxListener {
private:
  std::unique_ptr<AsyncUDP> _listener;
  std::set<uint8_t> _channels;
public:
  RxListener(uint16_t /* port */, uint8_t /* channel */);

  void add_channel(uint8_t channel) {
    _channels.insert(channel);
  }

  bool remove_channel(uint8_t channel) {
    _channels.erase(channel);
    return _channels.size() != 0;
  }
};

static std::unordered_map<uint8_t, std::queue<Data>> rx_bufs;
static std::unordered_map<uint16_t, RxListener> listeners;

static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
static volatile bool mux_flag = false;
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

RxListener::RxListener(uint16_t port, uint8_t channel)
  : _listener(new AsyncUDP()), _channels{ channel } {
  _listener->listen(port);
  _listener->onPacket([port](AsyncUDPPacket &packet) {
    mutEnter();
    auto found = listeners.find(port);
    if (found != listeners.end()) {
      Data data;
      if (Data::deserialize(packet.data(), packet.length(), &data))
        for (uint8_t channel : found->second._channels)
          rx_bufs[channel].push(data);
    }
    mutExit();
  });
}

size_t wlRxAvailable(uint8_t channel) {
  size_t available;
  mutEnter();
  available = rx_bufs[channel].size();
  mutExit();
  return available;
}

void wlRxAttach(uint16_t port, uint8_t channel) {
  mutEnter();
  auto found = listeners.find(port);
  if (found != listeners.end())
    found->second.add_channel(channel);
  else
    listeners.emplace(port, RxListener(port, channel));
  mutExit();
}


void wlRxDettach(uint16_t port, uint8_t channel) {
  mutEnter();
  auto found = listeners.find(port);
  if (found != listeners.end())
    if (!found->second.remove_channel(channel))
      listeners.erase(port);
  mutExit();
}

Data wlRxRead(uint8_t channel) {
  mutEnter();
  std::queue<Data> &rx_buf = rx_bufs[channel];
  Data data;
  if (rx_buf.empty())
    data = nullptr;
  else {
    data = rx_buf.front();
    rx_buf.pop();
  }

  mutExit();
  return data;
}

size_t wlRxRead(Data *buffer, size_t size, uint8_t channel) {
  mutEnter();
  size_t read_bytes = 0;
  auto found = rx_bufs.find(channel);
  if (found != rx_bufs.end()) {
    std::queue<Data> &rx_buf = found->second;
    while (read_bytes < size && !rx_buf.empty()) {
      buffer[read_bytes++] = rx_buf.front();
      rx_buf.pop();
    }
  }
  mutExit();
  return read_bytes;
}