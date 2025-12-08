#ifndef SHARED_H
#define SHARED_H

#include <Arduino.h>
#include <vector>
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

// Pin Definitions
#define ENA 5
#define IN1 26
#define IN2 27
#define ENB 18
#define IN3 32
#define IN4 33
#define MPU_INT 19
#define SDA_PIN 21
#define SCL_PIN 22
#define BUTTON_PIN 0

#define PWM_FREQ 5000
#define PWM_RESOLUTION 8
#define PWM_CHANNEL_A 0
#define PWM_CHANNEL_B 1

// Data Structures
struct RobotCommand
{
    int type; // 0=Stop, 1=Move, 2=Turn
    float val;
};

// Global Externs
extern QueueHandle_t commandQueue;
extern SemaphoreHandle_t dataMutex;
extern TimerHandle_t playbackTimer;

extern std::vector<String> recordedCommands;
extern bool isCurrentlyRecording;
extern bool isPlaying;
extern int playbackIndex;

// Control Variables
extern volatile double moveOffset;
extern volatile double turnOffset;

#endif