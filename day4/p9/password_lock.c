/*
 * 实验九 任务二：旋转编码器密码锁
 *
 * 题目要求：
 *   设计一个旋转编码器密码锁，输入传感器：旋钮＋按键+麦克风。逻辑：预设3
 *   位十进制密码（例如368），转动旋钮，随着转动角度逐渐加大，串口输出0~9数
 *   字，转到要选择数字时、短按按钮确认；第1位数字正确串口输出OK1，然后再转
 *   动旋钮，转到要选择数字时、短按按钮确认，第2位数字正确串口输出OK2，以此
 *   类推再输入第3位密码；如果数字错误，串口打印"error"，同时红色LED快速
 *   闪烁报警。3位数字都正确，对着麦克风说"开门"则绿色LED 常亮代表门已开，
 *   串口打印"Welcome"。长按按钮复位，恢复等待输入密码状态。
 *
 * 实现思路：
 *   - 旋钮（电位器）接 A1，转动时读取模拟值映射为 0~9 数字并串口输出；
 *   - 按键接 4 号引脚，短按确认当前数字，逐位与预设密码比对
 *     （按键空闲电平在上电时自动检测，高/低电平有效的按键模块均适用）；
 *   - 数字错误：串口打印 error，红色 LED 快速闪烁报警 2 秒后可重新输入该位；
 *   - 3 位全部正确后进入"等待开门"状态，麦克风检测到声音即开门；
 *   - 任意时刻长按按键（>=1.5秒）复位，回到等待输入密码状态。
 */

#include <Arduino.h>

// ---------------- 引脚定义 ----------------
const int KNOB_PIN    = A1;   // 旋钮（电位器）
const int BUTTON_PIN  = 4;    // 按键（空闲电平在 setup 中自动检测）
const int MIC_PIN     = A0;   // 麦克风（声音传感器）
const int RED_LED     = 12;   // 红色 LED（报警，低电平点亮）
const int GREEN_LED   = 13;   // 绿色 LED（门已开，低电平点亮）

// 红/绿 LED 均为低电平点亮（共阳接法）：输出 LOW 亮、HIGH 灭
const int RED_ON    = LOW;
const int RED_OFF   = HIGH;
const int GREEN_ON  = LOW;
const int GREEN_OFF = HIGH;

// ---------------- 参数定义 ----------------
const int PASSWORD[3] = {3, 6, 8};       // 预设 3 位十进制密码
const int SOUND_THRESHOLD = 256;         // 麦克风声音阈值（0~1023）
const unsigned long LONG_PRESS_MS = 1500; // 长按判定时间：1.5 秒
const unsigned long DEBOUNCE_MS = 50;    // 按键消抖时间

// ---------------- 运行状态 ----------------
int currentDigit = 0;       // 当前旋钮指向的数字（0~9）
int inputIndex = 0;         // 已确认的密码位数（0~3）
bool doorOpen = false;      // 门是否已打开

int buttonIdleLevel = HIGH;    // 按键空闲电平（setup 中自动检测）
bool buttonPressed = false;    // 消抖后的按键状态：true=按下
unsigned long buttonPressTime = 0;   // 按键按下时刻
bool longPressFired = false;         // 本次按下是否已触发长按

// 复位到等待输入密码状态
void resetLock()
{
    inputIndex = 0;
    doorOpen = false;
    digitalWrite(GREEN_LED, GREEN_OFF); // 绿灯关闭
    digitalWrite(RED_LED, RED_OFF);     // 红灯关闭
    Serial.println("reset");
}

// 红色 LED 快速闪烁报警 2 秒
void alarmBlink()
{
    for (int i = 0; i < 10; i++)   // 100ms 周期闪烁 10 次 = 2 秒
    {
        digitalWrite(RED_LED, RED_ON);
        delay(100);
        digitalWrite(RED_LED, RED_OFF);
        delay(100);
    }
}

// 读取旋钮（电位器），返回当前指向的 0~9 数字
void readKnob()
{
    int knobValue = analogRead(KNOB_PIN);       // 读取旋钮：0~1023
    int digit = map(knobValue, 0, 1023, 0, 9);  // 映射为 0~9
    digit = constrain(digit, 0, 9);

    if (digit != currentDigit)
    {
        currentDigit = digit;
        Serial.println(currentDigit);           // 串口输出当前数字
    }
}

// 短按确认当前数字
void confirmDigit()
{
    if (currentDigit == PASSWORD[inputIndex])
    {
        inputIndex++;
        Serial.print("OK");
        Serial.println(inputIndex);       // 输出 OK1 / OK2 / OK3
        if (inputIndex == 3)
            Serial.println("please say open"); // 提示对麦克风说"开门"
    }
    else
    {
        Serial.println("error");          // 数字错误
        alarmBlink();                     // 红色 LED 快速闪烁报警
    }
}

void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // 上电后红/绿灯均保持关闭：先写熄灭电平再设为输出，避免上电瞬间亮灯
    digitalWrite(RED_LED, RED_OFF);
    pinMode(RED_LED, OUTPUT);
    digitalWrite(GREEN_LED, GREEN_OFF);
    pinMode(GREEN_LED, OUTPUT);
    // 上电时按键应处于未按下状态，采样此时的电平作为空闲电平，
    // 之后电平与空闲电平相反即视为按下（兼容高/低电平有效的按键模块）
    buttonIdleLevel = digitalRead(BUTTON_PIN);
    Serial.begin(9600);
    Serial.println("please input password");
}

// 按键处理：短按松开确认数字，按住超过 1.5 秒复位
void handleButton()
{
    bool pressed = (digitalRead(BUTTON_PIN) != buttonIdleLevel);

    if (pressed != buttonPressed)     // 状态发生变化，消抖后确认
    {
        delay(DEBOUNCE_MS);
        pressed = (digitalRead(BUTTON_PIN) != buttonIdleLevel);
        if (pressed == buttonPressed)
            return;                   // 抖动，忽略

        buttonPressed = pressed;
        if (buttonPressed)            // 按下：记录时刻
        {
            buttonPressTime = millis();
            longPressFired = false;
        }
        else if (!longPressFired && !doorOpen)
        {
            confirmDigit();           // 短按松开：确认当前数字
        }
    }

    // 只有确认处于按下状态时，才进行长按计时判定
    if (buttonPressed && !longPressFired
        && millis() - buttonPressTime >= LONG_PRESS_MS)
    {
        longPressFired = true;
        resetLock();                  // 长按复位
    }
}

void loop()
{
    // 门已打开后只响应长按复位
    if (!doorOpen)
        readKnob();

    handleButton();

    // ---------------- 3 位密码正确后，等待麦克风"开门" ----------------
    if (inputIndex == 3 && !doorOpen)
    {
        int soundValue = analogRead(MIC_PIN);
        if (soundValue >= SOUND_THRESHOLD)   // 检测到声音，视为说了"开门"
        {
            doorOpen = true;
            digitalWrite(GREEN_LED, GREEN_ON);   // 绿色 LED 常亮，门已开
            Serial.println("Welcome");
        }
    }
}
