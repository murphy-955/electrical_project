/*
 * 实验十二 任务二：声光节拍装置
 *
 * 题目要求：
 *   在蜂鸣器发出的单纯的声音提示加上 LED 灯闪烁效果。闪烁频率和蜂鸣器
 *   音调成比例，按键按一下，声光闪烁频率为一秒一下。双击按键，声光闪烁
 *   频率提高为一秒 2 下。长按按键，声光闪烁频率再加快一倍（一秒 4 下）。
 *
 * 实现思路：
 *   - 按键接 4 号引脚，识别单击 / 双击 / 长按（按键识别逻辑同实验十一）；
 *   - 蜂鸣器接 8 号引脚，LED 接 13 号引脚，二者同步闪烁/发声；
 *   - 单击：1 下/秒（音调 440Hz）；双击：2 下/秒（音调 880Hz）；
 *     长按（>2 秒）：4 下/秒（音调 1760Hz），闪烁频率越高音调越高；
 *   - 每个节拍的前半周期 LED 亮 + 蜂鸣器发声，后半周期熄灭 + 静音。
 */

#include <Arduino.h>

const int buttonPin = 4;  // 按键
const int buzzerPin = 8;  // 蜂鸣器
const int ledPin    = 13; // LED

const unsigned long LONG_PRESS_MS   = 2000; // 长按判定时间：>2 秒
const unsigned long DOUBLE_CLICK_MS = 400;  // 双击判定间隔

// 节拍档位：0=停止，1=每秒 1 下，2=每秒 2 下，3=每秒 4 下
int tempoLevel = 0;
const int TONE_FREQ[4]   = {0, 440, 880, 1760};   // 各档位音调（Hz），与闪烁频率成比例
const int BEAT_PERIOD[4] = {0, 1000, 500, 250};   // 各档位节拍周期（ms）

bool beatOn = false;               // 当前是否处于节拍的前半周期（亮/响）
unsigned long lastToggleTime = 0;  // 上一次切换节拍的时刻

// 按键状态机变量
bool lastButtonState = HIGH;
unsigned long pressStartTime = 0;   // 本次按下的起始时间
unsigned long firstReleaseTime = 0; // 第一次短按松开的时间
bool waitingSecondClick = false;    // 是否在等待第二次点击

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    pinMode(ledPin, OUTPUT);
}

// 按键事件识别：返回 0=无事件, 1=单击, 2=双击, 3=长按
int scanButton()
{
    bool state = digitalRead(buttonPin);
    int event = 0;

    // 按下瞬间（下降沿）
    if (lastButtonState == HIGH && state == LOW)
    {
        pressStartTime = millis();
    }
    // 松开瞬间（上升沿）
    else if (lastButtonState == LOW && state == HIGH)
    {
        unsigned long pressDuration = millis() - pressStartTime;
        if (pressDuration >= LONG_PRESS_MS)
        {
            event = 3;                 // 长按
            waitingSecondClick = false;
        }
        else
        {
            // 短按：判断是否构成双击
            if (waitingSecondClick && millis() - firstReleaseTime <= DOUBLE_CLICK_MS)
            {
                event = 2;             // 双击
                waitingSecondClick = false;
            }
            else
            {
                waitingSecondClick = true; // 等待第二次点击
                firstReleaseTime = millis();
            }
        }
    }
    lastButtonState = state;

    // 等待第二次点击超时，判定为单击
    if (waitingSecondClick && millis() - firstReleaseTime > DOUBLE_CLICK_MS)
    {
        waitingSecondClick = false;
        event = 1; // 单击
    }

    return event;
}

// 输出一个节拍半周期：亮+发声 或 灭+静音
void applyBeat(bool on)
{
    beatOn = on;
    digitalWrite(ledPin, on ? HIGH : LOW);
    if (on)
        tone(buzzerPin, TONE_FREQ[tempoLevel]);
    else
        noTone(buzzerPin);
}

// 切换节拍档位，并立即进入新节拍的前半周期
void setTempo(int level)
{
    tempoLevel = level;
    applyBeat(true);
    lastToggleTime = millis();
}

// 按当前档位产生节拍：前半周期亮+响，后半周期灭+静
void runBeat()
{
    if (tempoLevel == 0)
        return;

    if (millis() - lastToggleTime >= (unsigned long)(BEAT_PERIOD[tempoLevel] / 2))
    {
        applyBeat(!beatOn);
        lastToggleTime = millis();
    }
}

void loop()
{
    // 处理按键事件，切换节拍档位
    switch (scanButton())
    {
    case 1: // 单击：一秒 1 下
        setTempo(1);
        break;
    case 2: // 双击：一秒 2 下
        setTempo(2);
        break;
    case 3: // 长按（>2 秒）：再加快一倍，一秒 4 下
        setTempo(3);
        break;
    default:
        break;
    }

    runBeat(); // 按当前档位输出声光节拍
}
