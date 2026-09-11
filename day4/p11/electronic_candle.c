/*
 * 实验十一 任务一：电子蜡烛装置
 *
 * 题目要求：
 *   设计电子蜡烛装置，要求关闭室内照明设备时，按下按键“点火”，
 *   LED 亮起并发出类似烛光摇曳闪烁效果。当对着 MIC 头吹气，LED 灯熄灭。
 *
 * 实现思路：
 *   - 按键接 4 号引脚（使用内部上拉 INPUT_PULLUP，按下为 LOW）；
 *   - 麦克风（声音传感器）接模拟口 A0，检测吹气产生的强声音；
 *   - LED 接 13 号引脚；
 *   - 熄灭状态下，按下按键“点火”，LED 点亮；
 *   - 点亮后以随机亮度/随机间隔快速变化（PWM 调光 + random），
 *     模拟烛光摇曳闪烁的效果；
 *   - 点亮状态下，检测到吹气（声音强度超过阈值）则熄灭 LED，
 *     回到等待点火状态。
 *
 * 注意：PWM 调光需要引脚支持 PWM，UNO 的 13 号引脚不支持 PWM，
 *       这里用“快速通断 + 随机占空比”的软件方式模拟亮度变化，
 *       利用人眼视觉暂留实现闪烁摇曳效果。
 */

#include <Arduino.h>

const int buttonPin = 4;   // 按键接 4 号引脚（另一端接地）
const int micPin    = A0;  // 麦克风（声音传感器）接 A0
const int ledPin    = 13;  // LED 接 13 号引脚

const int SOUND_THRESHOLD = 256; // 吹气检测阈值（0~1023），可按环境调整

bool candleOn = false; // 蜡烛是否处于点燃状态

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    randomSeed(analogRead(A1)); // 用悬空模拟口的噪声作随机种子
}

// 模拟烛光摇曳：随机亮度闪烁一段时间（约几十毫秒）
void flicker()
{
    // 随机占空比：亮 onTime 微秒，灭 offTime 微秒，重复若干次
    int cycles = random(10, 40);
    for (int i = 0; i < cycles; i++)
    {
        int onTime  = random(200, 2000);  // 亮的时间（微秒），随机变化模拟摇曳
        int offTime = random(100, 1200);  // 灭的时间（微秒）
        digitalWrite(ledPin, HIGH);
        delayMicroseconds(onTime);
        digitalWrite(ledPin, LOW);
        delayMicroseconds(offTime);
    }
}

void loop()
{
    if (!candleOn)
    {
        // 熄灭状态：等待按键“点火”
        if (digitalRead(buttonPin) == LOW)
        {
            delay(20); // 简单消抖
            if (digitalRead(buttonPin) == LOW)
            {
                candleOn = true;
                while (digitalRead(buttonPin) == LOW)
                    ; // 等待松开按键，避免重复触发
            }
        }
    }
    else
    {
        // 点燃状态：烛光摇曳闪烁
        flicker();

        // 检测吹气：声音强度超过阈值则熄灭
        int soundValue = analogRead(micPin);
        if (soundValue >= SOUND_THRESHOLD)
        {
            candleOn = false;
            digitalWrite(ledPin, LOW); // 熄灭蜡烛
        }
    }
}
