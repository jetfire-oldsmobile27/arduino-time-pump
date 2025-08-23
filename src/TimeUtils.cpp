#include "TimeUtils.h"

void printDateTime(const RtcDateTime& dt) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%04u-%02u-%02u %02u:%02u:%02u",
          dt.Year(), dt.Month(), dt.Day(),
          dt.Hour(), dt.Minute(), dt.Second());
  Serial.print(buf);
}

bool parseDateTimeString(const String& s, RtcDateTime& outDt) {
  String t = s;
  t.trim();
  if (t.length() < 17) return false;

  char buf[48];
  t.toCharArray(buf, sizeof(buf));
  int y, mo, d, hh, mm, ss;
  int matched = sscanf(buf, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &hh, &mm, &ss);
  if (matched != 6) {
    matched = sscanf(buf, "%d/%d/%d %d:%d:%d", &y, &mo, &d, &hh, &mm, &ss);
    if (matched != 6) {
      matched = sscanf(buf, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &hh, &mm, &ss);
      if (matched != 6) return false;
    }
  }

  if (y < 2000 || y > 2099) return false;
  if (mo < 1 || mo > 12) return false;
  if (d < 1 || d > 31) return false;
  if (hh < 0 || hh > 23) return false;
  if (mm < 0 || mm > 59) return false;
  if (ss < 0 || ss > 59) return false;

  outDt = RtcDateTime((uint16_t)y, (uint8_t)mo, (uint8_t)d, (uint8_t)hh, (uint8_t)mm, (uint8_t)ss);
  return true;
}

bool dt_greater(const RtcDateTime& a, const RtcDateTime& b) {
  if (a.Year() != b.Year()) return a.Year() > b.Year();
  if (a.Month() != b.Month()) return a.Month() > b.Month();
  if (a.Day() != b.Day()) return a.Day() > b.Day();
  if (a.Hour() != b.Hour()) return a.Hour() > b.Hour();
  if (a.Minute() != b.Minute()) return a.Minute() > b.Minute();
  if (a.Second() != b.Second()) return a.Second() > b.Second();
  return false;
}

bool dt_equal(const RtcDateTime& a, const RtcDateTime& b) {
  return a.Year() == b.Year() &&
         a.Month() == b.Month() &&
         a.Day() == b.Day() &&
         a.Hour() == b.Hour() &&
         a.Minute() == b.Minute() &&
         a.Second() == b.Second();
}