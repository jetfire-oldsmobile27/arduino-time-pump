#pragma once 
#include "Arduino.h"

namespace Defines {


constexpr uint32_t check_interval  = 30000;  // 30 сек

constexpr uint8_t PIN_IO = 4;
constexpr uint8_t PIN_SCLK = 5;
constexpr uint8_t PIN_CE = 2;

constexpr uint8_t pump_pin = (uint8_t)14U; //A0
constexpr uint8_t relay_pin{0};

enum class TaskType {
        EnablePumpOnce = 0,
        EnableRelay,
        DisableRelay
};

}