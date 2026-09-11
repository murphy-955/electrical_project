/*
 * 实验九 任务一：声控灯装置
 *
 * 题目要求：
 *   设计声控灯装置，要求当声音达到一定强度时候，点亮 LED 灯。
 *   并保持点亮状态 8 秒钟后熄灭。
 *
 * 实现思路：
 *   - 麦克风（声音传感器）接模拟口 A0，实时读取声音强度；
 *   - 当声音强度超过阈值 SOUND_THRESHOLD 时，点亮 LED；
 *   - 点亮后记录时间，8 秒内若再次检测到声音则重新计时；
 *   - 8 秒内没有新的声音触发，则熄灭 LED。
 */

#include <Arduino.h>

const int micPin = A0;                 // 声音传感器（麦克风）接 A0
const int ledPin = 13;                 // LED 接 13 号引脚

const int SOUND_THRESHOLD = 256;       // 声音强度阈值（0~1023），可按环境调整
const unsigned long KEEP_ON_MS = 8000; // 点亮保持时间：8 秒

unsigned long lastTriggerTime = 0;     // 最近一次被声音触发的时间
bool ledOn = false;                    // LED 当前状态

void setup()
{
    pinMode(ledPin, OUTPUT);
}

void loop()
{
    int soundValue = analogRead(micPin); // 读取声音强度：0~1023

    // 声音达到一定强度，触发点亮 / 刷新保持时间
    if (soundValue >= SOUND_THRESHOLD)
    {
        ledOn = true;
        lastTriggerTime = millis();
        digitalWrite(ledPin, HIGH);
    }

    // 点亮状态下，8 秒内没有新的声音触发，则熄灭
    if (ledOn && millis() - lastTriggerTime >= KEEP_ON_MS)
    {
        ledOn = false;
        digitalWrite(ledPin, LOW);
    }
}
