#include "TaskProvider.hpp"

void TaskProvider::exec(const Defines::TaskType& task2exec) {
        TaskFunc task = Task2Func(task2exec);
        if(task) {
            task();
        };
};

TaskFunc TaskProvider::Task2Func(const Defines::TaskType& type) {
        switch (type) {
        case Defines::TaskType::EnablePumpOnce:
            return []() {
                if(Defines::pump_pin == 0) {
                    Serial.print("pump_pin not realized");
                }
                digitalWrite(Defines::pump_pin, HIGH); 
                delay(2000);
                digitalWrite(Defines::pump_pin, LOW);
            };
            break;// на всякий случай
        case Defines::TaskType::EnableRelay:
            return []() {
                if(Defines::relay_pin == 0) {
                    Serial.print("relay_pin not realized");
                }
                digitalWrite(Defines::relay_pin, 255);
            };
            break;
        case Defines::TaskType::DisableRelay:
            return [](){
                if(Defines::relay_pin == 0) {
                    Serial.print("relay_pin not realized");
                }
                digitalWrite(Defines::relay_pin, 0);
            };
            break;
        default:
            return nullptr;
        }
};

