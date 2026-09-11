/*
 * 实验十 任务一：楼道灯装置（双控灯：声控 + 光控）
 *
 * 题目要求：
 *   设计楼道灯装置，要求是双控灯，即声控和光控。只有同时满足
 *   声音大和光线弱这两个条件，才亮灯；其余情况，一律灭灯。
 *
 * 实现思路：
 *   - 麦克风（声音传感器）接模拟口 A0，实时读取声音强度；
 *   - 光敏电阻（光线传感器）接模拟口 A2，实时读取光线强度；
 *     （光敏分压电路：环境越暗，A2 读数越小）
 *   - 只有当 声音强度 >= SOUND_THRESHOLD 且 光线强度 <= LIGHT_THRESHOLD
 *     两个条件同时满足时，点亮 LED；
 *   - 其余任何情况，一律熄灭 LED。
 */

#include <Arduino.h>

const int micPin   = A0;   // 声音传感器（麦克风）接 A0
const int lightPin = A2;   // 光敏电阻（光线传感器）接 A2
const int ledPin   = 13;   // LED 接 13 号引脚

const int SOUND_THRESHOLD = 300 ;// 声音强度阈值（0~1023），可按环境调整
const int LIGHT_THRESHOLD = 50 ;// 光线强度阈值（0~1023），低于此值视为光线弱

void setup()
{
    pinMode(ledPin, OUTPUT);
}

void loop()
{
    int soundValue = analogRead(micPin);   // 读取声音强度：0~1023
    int lightValue = analogRead(lightPin); // 读取光线强度：0~1023

    // 双控逻辑：声音大 且 光线弱，两个条件同时满足才亮灯
    if (soundValue >= SOUND_THRESHOLD && lightValue < LIGHT_THRESHOLD)
    {
        digitalWrite(ledPin, HIGH); // 亮灯
    }
    else
    {
        digitalWrite(ledPin, LOW);  // 其余情况一律灭灯
    }
}
