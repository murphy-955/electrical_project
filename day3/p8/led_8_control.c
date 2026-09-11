#include <Arduino.h>

const int potPin = A0;
const int ledPins[] = {5, 6, 7, 8, 9, 10, 11, 12, 13};
const int ledCount = 9;

const int deadZone = 40; // 消隐死区：读数低于 40 视为 0

void setup()
{
    for (int i = 0; i < ledCount; i++)
    {
        pinMode(ledPins[i], OUTPUT);
    }
}

void loop()
{
    int sensorValue = analogRead(potPin);

    int numLit;
    if (sensorValue < deadZone)
    {
        numLit = 0; // 旋钮调到最小，全部熄灭
    }
    else
    {
        // 把有效量程 [deadZone, 1023] 重新映射为 [0, 8]
        numLit = map(sensorValue, deadZone, 1023, 0, ledCount);
        numLit = constrain(numLit, 0, ledCount);
    }

    for (int i = 0; i < ledCount; i++)
    {
        digitalWrite(ledPins[i], i < numLit ? HIGH : LOW);
    }
}
