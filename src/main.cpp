#include <Arduino.h>
#include <GyverPower.h>
#include "defines.hpp"
#include "TimeProvider.hpp"
#include "StorageProvider.hpp"
#include "TimeUtils.hpp"
#include "TaskProvider.hpp"

TimeProvider rtc(Defines::PIN_IO, Defines::PIN_SCLK, Defines::PIN_CE);
StorageProvider storage;
TaskProvider task_provider;

void DailyRoutine() {
  if (Defines::DEBUG) {
    Serial.println("✅ Daily task executed!");
    RtcDateTime now = rtc.getDateTime();
    Serial.print("📅 Executed at: ");
    printDateTime(now);
    Serial.println();
  }
  // task_provider.exec(Defines::TaskType::EnablePumpOnce); // TODO
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  if (Defines::DEBUG) {
    Serial.begin(115200);
    delay(1500); 
  }

  rtc.begin();

  power.correctMillis(true);
  if (!Defines::DEBUG) {
    
    power.calibrate();
    power.setSleepMode(POWERDOWN_SLEEP);
  } else {
    power.setSleepMode(POWERSAVE_SLEEP);
  }
}

void loop() {
  if (Defines::DEBUG) {
    static unsigned long lastCheck = millis();

    if (Serial.available()) {
      String line = Serial.readStringUntil('\n');
      line.trim();
      if (line.length() > 0) {
        RtcDateTime dt;
        if (parseDateTimeString(line, dt)) {
          if (rtc.setDateTime(dt)) {
            Serial.println("OK");
            RtcDateTime now = rtc.getDateTime();
            if (storage.isExecutedToday(now)) {
              RtcDateTime yesterday = RtcDateTime(now.Year(), now.Month(), now.Day()-1, 0,0,0);
              storage.forceResetTo(yesterday);
            }
          } else {
            Serial.println("ERR");
          }
        } else {
          Serial.println("BAD FORMAT");
        }
      }
    }

    unsigned long nowMs = millis();
    if (nowMs - lastCheck >= Defines::check_interval) {
      lastCheck = nowMs;

      RtcDateTime now = rtc.getDateTime();
      Serial.print("Now: ");
      printDateTime(now);
      Serial.println();

      RtcDateTime lastExec = storage.getLastExecutionTime();
      bool executedToday = storage.isExecutedToday(now);

      Serial.print("Daily task ");
      if (executedToday) {
        Serial.print("executed today at: ");
        printDateTime(lastExec);
      } else {
        Serial.print("not executed today (last: ");
        printDateTime(lastExec);
        Serial.print(")");
      }
      Serial.println();

      if (!executedToday && storage.isNewDay(now)) {
        DailyRoutine();
        if (storage.markExecuted(now)) {
          Serial.println("📅 Execution marked in EEPROM");
        }
      }
    }
    delay(100);
  } else {
    power.sleepDelay(Defines::check_interval);
    RtcDateTime now = rtc.getDateTime();

    RtcDateTime lastExec = storage.getLastExecutionTime();
    bool executedToday = storage.isExecutedToday(now);

    if (!executedToday && storage.isNewDay(now)) {
      DailyRoutine();
      storage.markExecuted(now);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    }
    delay(10);
  }
}
