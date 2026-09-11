/*
 * 实验十一 任务二：多模式环境适应灯装置
 *
 * 题目要求：
 *   a) 常态：电位器手动控制亮度。
 *   b) 单击按键，进入声控模式，检测到声音大于某阈值时提升亮度至 100%。
 *   c) 长按按键（>2 秒）进入自动光控模式：根据光敏电阻自动调节亮度
 *      （环境越暗亮度越高）。
 *   在上述两种模式下可双击按键返回手动模式。装置运行期间用红绿黄
 *   （三种颜色 LED）提供模式状态 LED 指示，绿色代表手动模式，
 *   红色代表声控模式，黄色代表光控模式。
 *
 * 引脚分配：
 *   - 按键：4 号引脚（INPUT_PULLUP，按下为 LOW）
 *   - 麦克风（声控）：A0
 *   - 电位器（手动调光）：A1
 *   - 光敏电阻（光控）：A2（环境越暗读数越小）
 *   - 主照明 LED：3 号引脚（PWM 调光）
 *   - 状态指示 LED：红 8、绿 9、黄 10
 *
 * 按键识别：
 *   - 单击：短按后 400ms 内无第二次按下
 *   - 双击：400ms 内连续两次短按
 *   - 长按：按住超过 2000ms 后松开
 */

#include <Arduino.h>

const int buttonPin  = 4;  // 按键
const int micPin     = A0; // 麦克风（声控）
const int potPin     = A1; // 电位器（手动调光）
const int lightPin   = A2; // 光敏电阻（光控）
const int lampPin    = 3;  // 主照明 LED（PWM）
const int redLedPin    = 8;  // 红色 LED：声控模式指示
const int greenLedPin  = 9;  // 绿色 LED：手动模式指示
const int yellowLedPin = 10; // 黄色 LED：光控模式指示

// 状态 LED 点亮电平：普通接法（引脚→电阻→LED→GND）置 false，输出 HIGH 点亮；
// 共阳极接法（LED 正极接 5V）置 true，输出 LOW 点亮
const bool LED_ACTIVE_LOW = false;

// 声控参数：麦克风静止时有直流偏置（约 512），直接和固定阈值比较会一直成立，
// 因此改为检测“波动幅度”（峰值-谷值），只对真实声音响应
const int SOUND_AMPLITUDE_THRESHOLD = 300;    // 声音波动幅度阈值，可按环境调整
const unsigned long SOUND_KEEP_MS = 1500;    // 触发后保持最亮的时间
unsigned long lastSoundTrigger = 0;          // 最近一次声音触发时间

// 光控参数：光敏电阻实际读数范围通常远小于 0~1023，直接映射会饱和，
// 用下面两个常量标定实际范围（环境越暗读数越小），可通过串口观察后调整
const int LIGHT_MIN = 100;  // 最暗环境对应的读数
const int LIGHT_MAX = 800; // 最亮环境对应的读数

unsigned long lastPrintTime = 0; // 串口调试输出计时
const unsigned long LONG_PRESS_MS   = 2000; // 长按判定时间：>2 秒
const unsigned long DOUBLE_CLICK_MS = 400;  // 双击判定间隔

// 工作模式
enum Mode
{
    MODE_MANUAL, // 手动模式（电位器调光）
    MODE_SOUND,  // 声控模式
    MODE_LIGHT   // 光控模式
};

Mode mode = MODE_MANUAL; // 默认常态为手动模式

// 按键状态机变量
bool lastButtonState = HIGH;
unsigned long pressStartTime = 0;   // 本次按下的起始时间
unsigned long firstReleaseTime = 0; // 第一次短按松开的时间
bool waitingSecondClick = false;    // 是否在等待第二次点击

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(lampPin, OUTPUT);
    pinMode(redLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(yellowLedPin, OUTPUT);
    Serial.begin(9600); // 串口调试：观察传感器实际读数，便于调整阈值

    // 上电自检：红绿黄三盏灯依次点亮 0.5 秒，用于检查接线是否正确
    ledWrite(redLedPin, true);
    delay(500);
    ledWrite(redLedPin, false);
    ledWrite(greenLedPin, true);
    delay(500);
    ledWrite(greenLedPin, false);
    ledWrite(yellowLedPin, true);
    delay(500);
    ledWrite(yellowLedPin, false);
}

// 按 LED_ACTIVE_LOW 的设置输出点亮/熄灭电平
void ledWrite(int pin, bool on)
{
    digitalWrite(pin, (on != LED_ACTIVE_LOW) ? HIGH : LOW);
}

// 根据当前模式更新状态指示 LED
void updateModeLeds()
{
    ledWrite(greenLedPin,  mode == MODE_MANUAL);
    ledWrite(redLedPin,    mode == MODE_SOUND);
    ledWrite(yellowLedPin, mode == MODE_LIGHT);
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

// 各模式下的主照明 LED 亮度控制
void runLamp()
{
    int brightness = 0;
    switch (mode)
    {
    case MODE_MANUAL:
        // 电位器手动控制亮度：0~1023 映射为 0~255
        brightness = map(analogRead(potPin), 0, 1023, 0, 255);
        break;

    case MODE_SOUND:
    {
        // 声控模式：检测声音的波动幅度（峰值-谷值），超过阈值时亮度提升至 100%
        int mn = 1023, mx = 0;
        for (int i = 0; i < 50; i++) // 连续采样一小段时间
        {
            int v = analogRead(micPin);
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
        int amplitude = mx - mn;
        if (amplitude >= SOUND_AMPLITUDE_THRESHOLD)
            lastSoundTrigger = millis(); // 刷新触发时间
        brightness = (millis() - lastSoundTrigger < SOUND_KEEP_MS) ? 255 : 0;

        if (millis() - lastPrintTime >= 200) // 串口输出波动幅度，便于调整阈值
        {
            lastPrintTime = millis();
            Serial.print("sound amplitude = ");
            Serial.println(amplitude);
        }
        break;
    }

    case MODE_LIGHT:
    {
        // 光控模式：环境越暗亮度越高（光敏读数越小越暗），
        // 先把读数限制在实际范围内再映射，避免亮度饱和
        int v = constrain(analogRead(lightPin), LIGHT_MIN, LIGHT_MAX);
        brightness = 255 - map(v, LIGHT_MIN, LIGHT_MAX, 0, 255);

        if (millis() - lastPrintTime >= 200) // 串口输出光敏读数，便于标定范围
        {
            lastPrintTime = millis();
            Serial.print("light value = ");
            Serial.print(analogRead(lightPin));
            Serial.print("  brightness = ");
            Serial.println(brightness);
        }
        break;
    }
    }
    analogWrite(lampPin, brightness);
}

void loop()
{
    // 处理按键事件，切换工作模式
    switch (scanButton())
    {
    case 1: // 单击：进入声控模式
        mode = MODE_SOUND;
        break;
    case 2: // 双击：声控/光控模式下返回手动模式
        if (mode != MODE_MANUAL)
            mode = MODE_MANUAL;
        break;
    case 3: // 长按（>2 秒）：进入光控模式
        mode = MODE_LIGHT;
        break;
    default:
        break;
    }

    updateModeLeds(); // 更新模式状态指示 LED
    runLamp();        // 按当前模式控制主照明 LED 亮度
}
