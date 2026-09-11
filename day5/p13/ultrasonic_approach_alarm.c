/*
 * 实验十三 任务一：超声波近视报警器
 *
 * 题目要求：
 *   请设计一个超声波近视报警器，当距离小于 50cm 左右发出中低频
 *   节拍为 1/2 的蜂鸣声。
 *
 * 实现思路：
 *   - HC-SR04 超声波模块：Trig 接 6 号引脚，Echo 接 7 号引脚；
 *   - 蜂鸣器接 8 号引脚；
 *   - 循环测距，距离小于 50cm 时蜂鸣器以 1/2（每 0.5 秒一个节拍）的
 *     节奏报警：每个节拍前 100ms 发出 300Hz 中低频短音，其余时间静音；
 *   - 距离大于等于 50cm 或超出量程时保持安静。
 */

#include <Arduino.h>

const int trigPin   = 6; // 超声波 Trig
const int echoPin   = 7; // 超声波 Echo
const int buzzerPin = 8; // 蜂鸣器

const long ALARM_DIST_CM = 50;    // 报警距离阈值：小于 50cm 报警
const int TONE_FREQ = 300;        // 报警音调：300Hz 中低频
const unsigned long BEAT_PERIOD_MS = 500; // 节拍周期：0.5 秒（节拍 1/2）
const unsigned long BEEP_ON_MS = 100;     // 每个节拍内发声时长
const unsigned long MEASURE_MS = 100;     // 测距间隔

long distanceCm = -1;              // 最近一次测距结果（-1 表示超出量程）
unsigned long lastMeasureTime = 0; // 上一次测距的时刻
bool buzzerOn = false;             // 蜂鸣器当前是否在发声（用于调试输出）

// 读取 HC-SR04 超声波测距，返回距离（cm），超时无回波返回 -1
long readDistanceCm()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 30000UL); // 超时 30ms，约 5m
    if (duration == 0)
        return -1;                    // 超出量程
    return duration / 58;             // 声速换算：58us ≈ 1cm
}

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);

    Serial.begin(9600); // 串口调试，波特率 9600
    Serial.println(F("=== 超声波近视报警器 调试模式 ==="));
    Serial.println(F("串口监视器请设置为 9600 波特率"));
}

void loop()
{
    // 每隔 MEASURE_MS 测一次距
    if (millis() - lastMeasureTime >= MEASURE_MS)
    {
        distanceCm = readDistanceCm();
        lastMeasureTime = millis();

        // 串口输出测距结果，便于判断蜂鸣器不响是测距问题还是发声问题
        Serial.print(F("距离: "));
        if (distanceCm < 0)
            Serial.println(F("超出量程 (无回波)"));
        else
        {
            Serial.print(distanceCm);
            Serial.println(F(" cm"));
        }
    }

    // 距离小于 50cm：按 1/2 节拍发出中低频蜂鸣声
    if (distanceCm >= 0 && distanceCm < ALARM_DIST_CM)
    {
        unsigned long phase = millis() % BEAT_PERIOD_MS; // 当前节拍内位置
        if (phase < BEEP_ON_MS)
        {
            tone(buzzerPin, TONE_FREQ);
            if (!buzzerOn) // 状态变化时打印一次，避免刷屏
            {
                buzzerOn = true;
                Serial.println(F(">>> 报警中：蜂鸣器发声 (300Hz)"));
            }
        }
        else
        {
            noTone(buzzerPin);
            if (buzzerOn)
            {
                buzzerOn = false;
                Serial.println(F(">>> 报警中：节拍静音"));
            }
        }
    }
    else
    {
        noTone(buzzerPin); // 距离安全，保持安静
        if (buzzerOn)
        {
            buzzerOn = false;
            Serial.println(F(">>> 距离安全，蜂鸣器关闭"));
        }
    }
}
