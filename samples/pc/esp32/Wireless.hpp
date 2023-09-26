#pragma once
#ifndef WIRELESS
#define WIRELESS

#include <AsyncUDP.h>
#include <WiFi.h>

#include <memory>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

#include "Data.hpp"

// 送受信可能な最大バイト数
static const constexpr uint16_t MAX_PACKET_SIZE = 1024;
/*
  無線LANに接続します。
*/
bool wlConnect(const char* /* ssid */, const char* /* password */, IPAddress /* ip */,
               IPAddress /* gateway */ = IPAddress(192, 168, 1, 1), IPAddress /* subnet */ = IPAddress(255, 255, 255, 0));
/*
  無線LANのアクセスポイントを起動します。
*/
bool wlAPBegin(const char* /* ssid */, const char* /* pasword */, IPAddress /* ip */ = IPAddress(192, 168, 1, 1), IPAddress /* gateway */ = IPAddress(192, 168, 1, 1), IPAddress /* subnet */ = IPAddress(255, 255, 255, 0));
/*
  無線LANを切断します。
*/
bool wlDissconnect();

/*
  IPアドレスとポート番号を送信チャンネルに接続します。
*/
void wlTxAttach(IPAddress /* ip */, uint16_t /* port */, uint8_t /* channel */ = 0);

/*
  IPアドレスとポート番号を送信チャンネルから切り離します。
*/
void wlTxDetach(IPAddress /* ip */, uint16_t /* port */, uint8_t /* channel */ = 0);
/*
  IPアドレスが一致するすべてのポートを送信チャンネルから切り離します。
*/
void wlTxDetach(IPAddress /* ip */, uint8_t /* channel */ = 0);
/*
  ポート番号が一致するすべてのポートを送信チャンネルから切り離します。
*/
void wlTxDetach(uint16_t /* port */, uint8_t /* channel */ = 0);
/*
  送信チャンネルにすべてのデータを送信します。
*/
void wlTxWrite(const Data* /* buffer */, size_t /* size */, uint8_t /* channel */ = 0);
/*
  送信チャンネルにデータを送信します。
*/
void wlTxWrite(const Data& /* data */, uint8_t /* channel */ = 0);
/*
  受信チャンネルから取り出すことができるデータ数を取得します。
*/
size_t wlRxAvailable(uint8_t /* channel */ = 0);
/*
  ポートを受信チャンネルに接続します。
*/
void wlRxAttach(uint16_t /* port */, uint8_t /* channel */ = 0);
/*
  ポートを受信チャンネルから切り離します。
*/
void wlRxDetach(uint16_t /* port */, uint8_t /* channel */ = 0);
/*
  チャンネルからデータを取り出します。
  データがない場合はブロックされます。
*/
Data wlRxRead(uint8_t /* channel */ = 0);
/*
  チャンネルからデータを取り出してバッファに格納します。
  取り出したデータ数を返します。
*/
size_t wlRxRead(Data* /* buffer */, size_t /* size */, uint8_t /* channel */ = 0);
#endif