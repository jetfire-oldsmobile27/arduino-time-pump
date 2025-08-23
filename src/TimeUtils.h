#pragma once
#include <Arduino.h>
#include <RtcDS1302.h>

void printDateTime(const RtcDateTime& dt);

bool parseDateTimeString(const String& s, RtcDateTime& outDt);

bool dt_greater(const RtcDateTime& a, const RtcDateTime& b);

bool dt_equal(const RtcDateTime& a, const RtcDateTime& b);