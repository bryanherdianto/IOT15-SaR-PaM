#include "Shared.h"
#include "MotorControl.h"
#include "Network.h"
#include "MotionControl.h"

// Global Variables
QueueHandle_t commandQueue;
SemaphoreHandle_t dataMutex;
TimerHandle_t playbackTimer;

std::vector<String> recordedCommands;
bool isCurrentlyRecording = false;
bool isPlaying = false;
int playbackIndex = 0;

volatile double moveOffset = 0;
volatile double turnOffset = 0;

TaskHandle_t TaskPIDHandle;
TaskHandle_t TaskWiFiHandle;

// Setup
void setup()
{
    Serial.begin(115200);

    // Initialize RTOS Objects
    commandQueue = xQueueCreate(10, sizeof(RobotCommand));
    dataMutex = xSemaphoreCreateMutex();
    playbackTimer = xTimerCreate("PlaybackTimer", pdMS_TO_TICKS(500), pdTRUE, (void *)0, playbackTimerCallback);

    // Initialize Modules
    initMotors();
    initWiFi();
    initMotion();

    // Create Tasks and pin them to cores
    xTaskCreatePinnedToCore(TaskPID, "PID_Task", 4096, NULL, 2, &TaskPIDHandle, 1);
    xTaskCreatePinnedToCore(TaskWiFi, "WiFi_Task", 4096, NULL, 1, &TaskWiFiHandle, 0);
}

void loop()
{
    vTaskDelete(NULL);
}