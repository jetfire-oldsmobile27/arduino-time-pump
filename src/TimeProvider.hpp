#pragma once
#include <Arduino.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

class TimeProvider {
public:
  TimeProvider(uint8_t io, uint8_t sclk, uint8_t ce);
  
  void begin();
  RtcDateTime getDateTime();
  bool setDateTime(const RtcDateTime& dt);
  bool isDateTimeValid();
  bool isWriteProtected();
  void setWriteProtected(bool protect);
  bool isRunning();
  void setRunning(bool running);

private:
  ThreeWire wire_;
  RtcDS1302<ThreeWire> rtc_;
  uint8_t nestedCheckup_ = 0;
  static constexpr uint8_t MAX_NESTED = 3;
  
  bool timeCheckup(const RtcDateTime& target);
};