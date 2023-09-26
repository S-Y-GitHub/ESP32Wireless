#pragma once

#ifndef SERIAL_UTIL
#define SERIAL_UTIL

#include "Data.hpp"

#include <unordered_map>
#include <queue>
#include <PacketSerial.h>

void serialWrite(const Data&);

bool serialBegin();

bool serialBegin(Stream&);

bool serialEnd();

size_t serialAvailable();

Data serialRead();

#endif