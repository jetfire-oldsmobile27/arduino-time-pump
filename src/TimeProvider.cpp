#include "TimeProvider.h"


TimeProvider::TimeProvider(uint8_t io, uint8_t sclk, uint8_t ce)
  : wire_(io, sclk, ce), rtc_(wire_) {}

void TimeProvider::begin() {
  rtc_.Begin();
}

RtcDateTime TimeProvider::getDateTime() {
  return rtc_.GetDateTime();
}

bool TimeProvider::setDateTime(const RtcDateTime& dt) {
  nestedCheckup_ = 0;
  return timeCheckup(dt);
}

bool TimeProvider::isDateTimeValid() {
  return rtc_.IsDateTimeValid();
}

bool TimeProvider::isWriteProtected() {
  return rtc_.GetIsWriteProtected();
}

void TimeProvider::setWriteProtected(bool protect) {
  rtc_.SetIsWriteProtected(protect);
}

bool TimeProvider::isRunning() {
  return rtc_.GetIsRunning();
}

void TimeProvider::setRunning(bool running) {
  rtc_.SetIsRunning(running);
}

bool TimeProvider::timeCheckup(const RtcDateTime& target) {
  RtcDateTime now;
  
  while (nestedCheckup_++ < MAX_NESTED) {
    now = getDateTime();
    
    if (!isDateTimeValid()) {
      rtc_.SetDateTime(target);
      continue;
    }

    if (isWriteProtected()) {
      setWriteProtected(false);
      continue;
    }

    if (!isRunning()) {
      setRunning(true);
      continue;
    }

    auto dateEqual = [](const RtcDateTime& a, const RtcDateTime& b) {
      return a.Year() == b.Year() && 
             a.Month() == b.Month() && 
             a.Day() == b.Day();
    };

    if (dateEqual(now, target)) {
      nestedCheckup_ = 0;
      return true;
    }

    rtc_.SetDateTime(target);
    now = getDateTime();
    if (dateEqual(now, target)) {
      nestedCheckup_ = 0;
      return true;
    }
  }
  
  nestedCheckup_ = 0;
  return false;
}