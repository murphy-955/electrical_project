/*
 * 实验十三 任务二：超声波倒车雷达
 *
 * 题目要求：
 *   请设计一个超声波倒车雷达，要求当距离目标 50cm 时候，蜂鸣器报警，
 *   而且频率随距离有变化，稍远处报警声音平缓，靠近处报警声音急促。
 *   串口监视器能实时显示测距结果。
 *
 * 实现思路：
 *   - HC-SR04 超声波模块：Trig 接 6 号引脚，Echo 接 7 号引脚；
 *   - 蜂鸣器接 8 号引脚；
 *   - 距离 >= 50cm：安全区域，不报警；
 *   - 距离 10~50cm：间歇蜂鸣，距离越近蜂鸣间隔越短（声音越急促），
 *     间隔由 map(距离, 10~50cm, 100~600ms) 得到，音调固定 800Hz；
 *   - 距离 < 10cm：连续长鸣，表示已经贴得很近；
 *   - 串口每隔 200ms 输出一次测距结果（cm）。
 */

#include <Arduino.h>

const int trigPin   = 6; // 超声波 Trig
const int echoPin   = 7; // 超声波 Echo
const int buzzerPin = 8; // 蜂鸣器

const long ALARM_DIST_CM = 50;      // 报警距离阈值：小于 50cm 报警
const long CONTINUOUS_CM = 10;      // 小于该距离连续长鸣
const int TONE_FREQ = 800;          // 报警音调：800Hz
const unsigned long SLOW_MS = 600;  // 最远报警距离处的蜂鸣间隔（平缓）
const unsigned long FAST_MS = 100;  // 最近报警距离处的蜂鸣间隔（急促）
const unsigned long BEEP_ON_MS = 60; // 每次蜂鸣发声时长
const unsigned long MEASURE_MS = 100;  // 测距间隔
const unsigned long PRINT_MS = 200;    // 串口输出间隔

long distanceCm = -1;              // 最近一次测距结果（-1 表示超出量程）
unsigned long lastMeasureTime = 0; // 上一次测距的时刻
unsigned long lastPrintTime = 0;   // 上一次串口输出的时刻

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

// 根据当前距离控制蜂鸣器报警节奏
void runAlarm()
{
    if (distanceCm < 0 || distanceCm >= ALARM_DIST_CM)
    {
        noTone(buzzerPin); // 安全区域，不报警
        return;
    }

    if (distanceCm < CONTINUOUS_CM)
    {
        tone(buzzerPin, TONE_FREQ); // 距离很近，连续长鸣
        return;
    }

    // 距离越近，蜂鸣间隔越短：10cm -> 100ms（急促），50cm -> 600ms（平缓）
    unsigned long interval = map(distanceCm, CONTINUOUS_CM, ALARM_DIST_CM,
                                 FAST_MS, SLOW_MS);
    unsigned long phase = millis() % interval; // 当前间隔周期内位置
    if (phase < BEEP_ON_MS)
        tone(buzzerPin, TONE_FREQ);
    else
        noTone(buzzerPin);
}

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(buzzerPin, OUTPUT);
    Serial.begin(9600);
}

void loop()
{
    // 每隔 MEASURE_MS 测一次距
    if (millis() - lastMeasureTime >= MEASURE_MS)
    {
        distanceCm = readDistanceCm();
        lastMeasureTime = millis();
    }

    runAlarm(); // 按距离控制报警节奏

    // 串口实时显示测距结果
    if (millis() - lastPrintTime >= PRINT_MS)
    {
        lastPrintTime = millis();
        Serial.print("distance = ");
        if (distanceCm < 0)
            Serial.println("out of range");
        else
        {
            Serial.print(distanceCm);
            Serial.println(" cm");
        }
    }
}
