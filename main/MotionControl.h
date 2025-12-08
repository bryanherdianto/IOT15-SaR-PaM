#ifndef MOTIONCONTROL_H
#define MOTIONCONTROL_H

#include "freertos/timers.h"

void initMotion();
void TaskPID(void *pvParameters);
void playbackTimerCallback(TimerHandle_t xTimer);

#endif