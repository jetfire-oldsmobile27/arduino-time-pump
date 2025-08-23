#pragma once
#include <Arduino.h>
#include "defines.hpp"

using TaskFunc = void (*)();  

class TaskProvider {
public:
    
    void exec(const Defines::TaskType& task2exec);
    TaskFunc Task2Func(const Defines::TaskType& type);
};
