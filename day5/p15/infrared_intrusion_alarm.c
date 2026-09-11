/*
 * 实验十五 任务一：红外入侵检测装置
 *
 * 题目要求：
 *   将红外入侵检测装置放置在一间无人房间，无人时 RGB 灯显示绿色；
 *   当有人走入房间，被检测到有人进来，RGB 显示红色，并增加一个蜂鸣器
 *   示警功能。
 *
 * 实现思路：
 *   - E18-D80NK 红外光电开关（NPN 输出）：红线接 5V，蓝线接 GND，
 *     黑线（信号）接 2 号引脚；NPN 为开集电极输出，检测到物体时输出
 *     拉低为 LOW，无人时靠内部上拉为 HIGH，检测距离 3~80cm 可用传感器
 *     背面的电位器调节；
 *   - RGB LED 模块（WS2812 可寻址，单引脚控制）：数据线 DIN 接 13 号引脚，
 *     VCC 接 5V，GND 接 GND；蜂鸣器接 8 号引脚；
 *   - 无人：绿灯常亮，蜂鸣器安静；
 *   - 有人入侵：红灯点亮 + 蜂鸣器间歇鸣响示警。
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

const int pirPin    = 2;  // 人体红外（PIR）传感器
const int buzzerPin = 8;  // 蜂鸣器
const int rgbPin    = 13; // RGB LED 模块（WS2812）数据线

// WS2812 RGB LED 模块：1 颗灯珠，GRB 字节序
Adafruit_NeoPixel rgbLed(1, rgbPin, NEO_GRB + NEO_KHZ800);

const int TONE_FREQ = 1000;                // 报警音调：1kHz
const unsigned long BEAT_PERIOD_MS = 400;  // 报警节拍周期
const unsigned long BEEP_ON_MS = 200;      // 每个节拍内发声时长

// 设置 RGB 灯颜色（WS2812 模块，单线协议）
void setRgb(bool r, bool g, bool b)
{
    rgbLed.setPixelColor(0, rgbLed.Color(r ? 255 : 0, g ? 255 : 0, b ? 255 : 0));
    rgbLed.show();
}

void setup()
{
    pinMode(pirPin, INPUT_PULLUP); // NPN 开集电极输出，需上拉
    pinMode(buzzerPin, OUTPUT);
    rgbLed.begin();  // 初始化 WS2812
    setRgb(false, false, false); // 初始熄灭
}

void loop()
{
    // E18-D80NK 检测到人体时输出 LOW（NPN 开集电极，平时上拉为 HIGH）
    bool someone = (digitalRead(pirPin) == LOW);

    if (someone)
    {
        // 有人入侵：红灯点亮 + 蜂鸣器间歇鸣响示警
        setRgb(true, false, false);
        unsigned long phase = millis() % BEAT_PERIOD_MS; // 当前节拍内位置
        if (phase < BEEP_ON_MS)
            tone(buzzerPin, TONE_FREQ);
        else
            noTone(buzzerPin);
    }
    else
    {
        // 无人：绿灯常亮，蜂鸣器安静
        setRgb(false, true, false);
        noTone(buzzerPin);
    }
}
