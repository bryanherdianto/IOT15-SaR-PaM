#include "Shared.h"
#include "Network.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

// Setup AP ssid and password
const char *ssid = "iphone bryan";
const char *password = "bryan123";
WebSocketsServer webSocket = WebSocketsServer(80); // Set port for websocket

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
        Serial.printf("[%u] Connected!\n", num);
        break;
    case WStype_TEXT:
    {
        String payload_str = String((char *)payload).substring(0, length);
        DynamicJsonDocument doc(256);
        deserializeJson(doc, payload_str);
        String command = doc["command"];
        bool record = doc["record"];

        if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
        {
            if (record && !isCurrentlyRecording)
            {
                recordedCommands.clear();
                Serial.println("Recording Started");
            }
            isCurrentlyRecording = record;

            if (isCurrentlyRecording && command != "PLAY" && command != "NONE")
            {
                recordedCommands.push_back(command); // Record command to vector
            }
            xSemaphoreGive(dataMutex);
        }

        RobotCommand pkg;
        bool sendToQueue = true;

        if (command == "FORWARD")
            pkg = {1, -4.0};
        else if (command == "REVERSE")
            pkg = {1, 4.0};
        else if (command == "LEFT")
            pkg = {2, 30.0};
        else if (command == "RIGHT")
            pkg = {2, -30.0};
        else if (command == "STOP")
            pkg = {0, 0};
        else if (command == "PLAY")
        {
            sendToQueue = false;
            if (xSemaphoreTake(dataMutex, portMAX_DELAY) == pdTRUE)
            {
                if (!recordedCommands.empty())
                {
                    isPlaying = true;
                    playbackIndex = 0;
                    xTimerStart(playbackTimer, 0);
                }
                xSemaphoreGive(dataMutex);
            }
        }
        else
            sendToQueue = false;

        if (sendToQueue)
            xQueueSend(commandQueue, &pkg, 0);
        break;
    }
    }
}

// Initialize WiFi and Websocket
void initWiFi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
        delay(500);
    Serial.println("WiFi Connected");
    webSocket.begin();
    webSocket.onEvent(onWebSocketEvent);
}

// Task for websocket loop
void TaskWiFi(void *pvParameters)
{
    for (;;)
    {
        webSocket.loop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}