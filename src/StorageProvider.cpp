#include "StorageProvider.h"
#include <Arduino.h>
#include <EEPROM.h>

bool StorageProvider::isNewDay(const RtcDateTime& current) {
  LastExecution le;
  EEPROM.get(eepromAddr_, le);
  
  if (calcChecksum(le) != le.checksum) {
    return true;
  }

  return (current.Year() > le.year) || 
         (current.Year() == le.year && current.Month() > le.month) ||
         (current.Year() == le.year && current.Month() == le.month && current.Day() > le.day);
}

bool StorageProvider::markExecuted(const RtcDateTime& dt) {
  LastExecution le = {
    .year = dt.Year(),
    .month = dt.Month(),
    .day = dt.Day(),
    .hour = dt.Hour(),
    .minute = dt.Minute(),
    .second = dt.Second(),
    .checksum = 0
  };
  le.checksum = calcChecksum(le);

  noInterrupts();
  EEPROM.put(eepromAddr_, le);
  interrupts();

  LastExecution verify;
  EEPROM.get(eepromAddr_, verify);
  return (verify.year == le.year && 
          verify.month == le.month && 
          verify.day == le.day &&
          verify.checksum == le.checksum);
}

bool StorageProvider::isExecutedToday(const RtcDateTime& current) {
  LastExecution le;
  EEPROM.get(eepromAddr_, le);
  
  if (calcChecksum(le) != le.checksum) return false;
  
  return (current.Year() == le.year && 
          current.Month() == le.month && 
          current.Day() == le.day);
}

void StorageProvider::forceResetTo(const RtcDateTime& dt) {
  LastExecution le = {
    .year = static_cast<uint16_t>(dt.Year()),
    .month = static_cast<uint8_t>(dt.Month()),
    .day = static_cast<uint8_t>(dt.Day() - 1),
    .hour = 0,
    .minute = 0,
    .second = 0,
    .checksum = 0
  };
  le.checksum = calcChecksum(le);
  
  noInterrupts();
  EEPROM.put(eepromAddr_, le);
  interrupts();
}

RtcDateTime StorageProvider::getLastExecutionTime() const {
  LastExecution le;
  EEPROM.get(eepromAddr_, le);
  
  if (calcChecksum(le) != le.checksum) {
    return RtcDateTime(2000, 1, 1, 0, 0, 0);
  }
  
  return RtcDateTime(le.year, le.month, le.day, le.hour, le.minute, le.second);
}

uint8_t StorageProvider::calcChecksum(const LastExecution& le) {
  return le.year + le.month + le.day + le.hour + le.minute + le.second;
}