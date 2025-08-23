#pragma once

#include <RtcDS1302.h>

struct LastExecution {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  uint8_t checksum;
};

class StorageProvider {
public:
  explicit StorageProvider(int addr = 0) : eepromAddr_(addr) {}
  
  bool isNewDay(const RtcDateTime& current);
  bool markExecuted(const RtcDateTime& dt);
  bool isExecutedToday(const RtcDateTime& current);
  void forceResetTo(const RtcDateTime& dt);
  RtcDateTime getLastExecutionTime() const;

private:
  int eepromAddr_;
  static uint8_t calcChecksum(const LastExecution& le);
};