#include "Shared.h"
#include "MotionControl.h"
#include "MotorControl.h"
#include "I2Cdev.h"
#include <PID_v1.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include <Wire.h>

MPU6050 mpu; // Initialize MPU6050 object
bool dmpReady = false;
uint8_t mpuIntStatus;
uint16_t packetSize;
uint16_t fifoCount;
uint8_t fifoBuffer[64];
Quaternion q;
VectorFloat gravity;
float ypr[3];

double originalSetpoint = 190;
volatile double setpoint = 190;
double Kp = 25.0, Kd = 1.2, Ki = 80.0; // PID Constants
double input, output;
PID pid(&input, &output, (double *)&setpoint, Kp, Ki, Kd, DIRECT);

unsigned long fallenStartTime = 0;
const unsigned long SLEEP_TIMEOUT = 10000; // Duration of light sleep

void initMotion()
{
    Wire.begin(SDA_PIN, SCL_PIN); // Connect to pin 21 and 22
    Wire.setClock(400000); // Set I2C clock to 400kHz
    mpu.initialize();

    if (mpu.dmpInitialize() == 0)
    {
        mpu.setXGyroOffset(-2);
        mpu.setYGyroOffset(74);
        mpu.setZGyroOffset(7);
        mpu.setZAccelOffset(968);
        mpu.setDMPEnabled(true);
        dmpReady = true;
        packetSize = mpu.dmpGetFIFOPacketSize();
        pid.SetMode(AUTOMATIC);
        pid.SetSampleTime(10);
        pid.SetOutputLimits(-255, 255);
    }
    else
    {
        Serial.println("DMP Failed");
    }

    pinMode(BUTTON_PIN, INPUT_PULLUP); // Set button pin as input with pull-up for wake-up
}

void playbackTimerCallback(TimerHandle_t xTimer)
{
    if (!isPlaying)
        return;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
    {
        if (playbackIndex < recordedCommands.size())
        {
            String cmd = recordedCommands[playbackIndex];
            RobotCommand pkg;
            if (cmd == "FORWARD")
                pkg = {1, -4.0};
            else if (cmd == "REVERSE")
                pkg = {1, 4.0};
            else if (cmd == "LEFT")
                pkg = {2, 30.0};
            else if (cmd == "RIGHT")
                pkg = {2, -30.0};
            else
                pkg = {0, 0};

            xQueueSend(commandQueue, &pkg, 0);
            playbackIndex++;
        }
        else
        {
            isPlaying = false;
            RobotCommand stopPkg = {0, 0};
            xQueueSend(commandQueue, &stopPkg, 0);
            xTimerStop(xTimer, 0);
            Serial.println("Playback Finished");
        }
        xSemaphoreGive(dataMutex);
    }
}

void TaskPID(void *pvParameters)
{
    RobotCommand receivedPkg;
    for (;;)
    {
        if (xQueueReceive(commandQueue, &receivedPkg, 0) == pdTRUE)
        {
            if (receivedPkg.type == 0)
            {
                moveOffset = 0;
                turnOffset = 0;
            }
            else if (receivedPkg.type == 1)
            {
                moveOffset = receivedPkg.val;
                turnOffset = 0;
            }
            else if (receivedPkg.type == 2)
            {
                turnOffset = receivedPkg.val;
            }
        }

        if (!dmpReady)
        {
            vTaskDelay(10 / portTICK_PERIOD_MS);
            continue;
        }

        mpuIntStatus = mpu.getIntStatus();
        fifoCount = mpu.getFIFOCount();

        if ((mpuIntStatus & 0x10) || fifoCount == 1024)
        {
            mpu.resetFIFO();
        }
        else if (mpuIntStatus & 0x02)
        {
            while (fifoCount < packetSize)
                fifoCount = mpu.getFIFOCount();
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            fifoCount -= packetSize;

            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            input = ypr[1] * 180 / M_PI + 180;

            // PID Control
            setpoint = originalSetpoint + moveOffset;
            if (!isnan(input))
                pid.Compute();

            double currentOutput = output;
            if (abs(currentOutput) < 10)
                currentOutput = 0;
            int left = currentOutput + turnOffset;
            int right = currentOutput - turnOffset;

            // Sleep Logic
            if (input < 140 || input > 230)
            {
                setMotorSpeed(0, 0);
                if (fallenStartTime == 0)
                    fallenStartTime = millis();
                if (millis() - fallenStartTime > SLEEP_TIMEOUT)
                {
                    Serial.println("Entering Sleep...");
                    delay(100);
                    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0);
                    esp_light_sleep_start();
                    Serial.println("Woke up!");
                    fallenStartTime = 0;
                }
            }
            else
            {
                setMotorSpeed(left, right);
                fallenStartTime = 0;
            }
        }
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}