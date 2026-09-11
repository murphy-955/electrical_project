/*
 * 实验十二 任务一：模拟发声装置
 *
 * 题目要求：
 *   请设计模拟发声装置，要求按下按键后，等了几秒，蜂鸣器发声一段时间后停止。
 *
 * 实现思路：
 *   - 按键接 4 号引脚（INPUT_PULLUP，按下为 LOW）；
 *   - 蜂鸣器接 8 号引脚，用 tone() 输出固定音调；
 *   - 按下按键后进入"等待"状态，延迟 WAIT_MS（3 秒）；
 *   - 等待结束后蜂鸣器发声 SOUND_MS（2 秒）后自动停止，回到空闲状态；
 *   - 全程用 millis() 计时，不阻塞，等待/发声期间再次按键无效。
 */

#include <Arduino.h>


const int buttonPin = 4; // 按键
const int buzzerPin = 8; // 蜂鸣器

const unsigned long WAIT_MS = 3000; // 按下后等待时间：3 秒
const unsigned long SOUND_MS = 2000; // 发声持续时间：2 秒
const int TONE_FREQ = 1000; // 发声频率：1kHz

// 工作状态
enum State
{
    STATE_IDLE, // 空闲，等待按键
    STATE_WAITING, // 已按键，等待延迟结束
    STATE_SOUNDING // 蜂鸣器发声中
};

State state = STATE_IDLE;
unsigned long stateStartTime = 0; // 进入当前状态的时刻
bool lastButtonState = HIGH; // 上一次的按键电平

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
}

void loop()
{
    bool buttonState = digitalRead(buttonPin);

    // 空闲状态下检测按键按下（下降沿）
    if (state == STATE_IDLE && lastButtonState == HIGH && buttonState == LOW)
    {
        state = STATE_WAITING;
        stateStartTime = millis();
    }
    lastButtonState = buttonState;

    // 等待时间到，开始发声
    if (state == STATE_WAITING && millis() - stateStartTime >= WAIT_MS)
    {
        state = STATE_SOUNDING;
        stateStartTime = millis();
        tone(buzzerPin, TONE_FREQ);
    }
    // 发声时间到，停止发声，回到空闲
    else if (state == STATE_SOUNDING && millis() - stateStartTime >= SOUND_MS)
    {
        noTone(buzzerPin);
        state = STATE_IDLE;
    }
}
