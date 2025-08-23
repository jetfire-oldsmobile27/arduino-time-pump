#include <Arduino.h>
#include "defines.h"
#include "TimeProvider.h"
#include "StorageProvider.h"
#include "TimeUtils.h"


TimeProvider rtc(Defines::PIN_IO, Defines::PIN_SCLK, Defines::PIN_CE);
StorageProvider storage;

void DailyRoutine() {
  Serial.println("✅ Daily task executed!");
  RtcDateTime now = rtc.getDateTime();
  Serial.print("📅 Executed at: ");
  printDateTime(now);
  Serial.println();
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  delay(1500);
  rtc.begin();
}

void loop() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck >= Defines::check_interval) {
    lastCheck = millis();
    
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
  
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial.print("Received: ");
      Serial.println(line);
      
      RtcDateTime dt;
      if (parseDateTimeString(line, dt)) {
        Serial.print("Parsed: ");
        printDateTime(dt);
        Serial.println();
        
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
}