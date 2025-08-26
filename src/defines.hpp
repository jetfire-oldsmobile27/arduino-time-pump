#pragma once 
#include "Arduino.h"

#define BUILD_DEBUG 1  // 1 - debug, 0 - release

namespace Defines {


constexpr uint32_t check_interval  = 30000;  // 30 сек

#if BUILD_DEBUG
  constexpr bool DEBUG = true;
#else 
  constexpr bool DEBUG = false;
#endif

/* Platform-specific block 
* Warning! Actual only for Arduino Uno(ATmega 328p old bootloader)
*/
constexpr uint8_t PIN_IO = 4;
constexpr uint8_t PIN_SCLK = 5;
constexpr uint8_t PIN_CE = 2;
constexpr uint8_t pump_pin = DD3;
constexpr uint8_t relay_pin{0};
//

enum class TaskType {
        EnablePumpOnce = 0,
        EnableRelay,
        DisableRelay
};

}