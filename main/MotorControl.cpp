#include "Shared.h"
#include "MotorControl.h"

void initMotors()
{
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // PWM is used for motor speed control
    ledcSetup(PWM_CHANNEL_A, PWM_FREQ, PWM_RESOLUTION);
    ledcSetup(PWM_CHANNEL_B, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(ENA, PWM_CHANNEL_A);
    ledcAttachPin(ENB, PWM_CHANNEL_B);
    setMotorSpeed(0, 0);
}

void setMotorSpeed(int speedLeft, int speedRight)
{
    if (speedLeft > 0)
    {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
    }
    else
    {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
    }
    ledcWrite(PWM_CHANNEL_A, constrain(abs(speedLeft), 0, 255));

    if (speedRight > 0)
    {
        digitalWrite(IN3, HIGH);
        digitalWrite(IN4, LOW);
    }
    else
    {
        digitalWrite(IN3, LOW);
        digitalWrite(IN4, HIGH);
    }
    ledcWrite(PWM_CHANNEL_B, constrain(abs(speedRight), 0, 255));
}