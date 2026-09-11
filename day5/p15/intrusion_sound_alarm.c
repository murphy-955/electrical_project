/*
 * 实验十五 任务二：红外 + 声音双传感器入侵报警装置
 *
 * 题目要求：
 *   在任务一基础上，增加声音传感器，当检测到异常声音时，也触发报警。
 *   声强报警颜色为紫色，同时，外接一个液晶屏 1602，显示不同的报警内容
 *   字符。红外入侵时，报警显示 "Infrared Intrusion Alarm" 红灯闪烁。
 *   异常声强时，报警显示 "Acoustic Anomaly Alarm" 紫灯闪烁。无人正常
 *   情况显示 "Normal condition"，绿灯常亮。为防止误触发声音报警，可用
 *   按钮控制声音报警功能是否切入。
 *
 * 实现思路：
 *   - E18-D80NK 红外光电开关（NPN 输出）：红线接 5V，蓝线接 GND，
 *     黑线（信号）接 2 号引脚；NPN 为开集电极输出，检测到物体时输出
 *     拉低为 LOW，无人时靠内部上拉为 HIGH；
 *   - 麦克风（声音传感器）接 A0，检测声音的波动幅度（峰值-谷值），
 *     避免直流偏置导致误触发（同实验十一）；
 *   - 按钮接 4 号引脚（INPUT_PULLUP，按下为 LOW），每按一下切换
 *     声音报警功能的切入/关闭；
 *   - RGB LED 模块（WS2812 可寻址，单引脚控制）：数据线 DIN 接 13 号
 *     引脚，VCC 接 5V，GND 接 GND；蜂鸣器接 8 号引脚；
 *   - 1602 液晶显示器（I2C，ST7032 驱动芯片）：SDA 接 A4，SCL 接 A5，
 *     I2C 地址 0x3E；背光由 PCA9632（地址 0x60）控制，初始化为白色背光；
 *   - 状态优先级：红外入侵 > 声音异常 > 正常；
 *     红外入侵：红灯闪烁 + 蜂鸣器急促鸣响，液晶显示 Infrared Intrusion Alarm；
 *     声音异常（且声音报警已切入）：紫灯（红+蓝）闪烁 + 蜂鸣器间歇鸣响，
 *     液晶显示 Acoustic Anomaly Alarm；
 *     正常：绿灯常亮，液晶显示 Normal condition，第二行显示声音报警开关状态。
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

const int pirPin    = 2;  // E18-D80NK 红外光电开关（NPN 输出，检测到低电平）
const int buttonPin = 4;  // 按钮：切换声音报警功能
const int buzzerPin = 8;  // 蜂鸣器
const int rgbPin    = 13; // RGB LED 模块（WS2812）数据线
const int micPin    = A0; // 麦克风（声音传感器）

// WS2812 RGB LED 模块：1 颗灯珠，GRB 字节序
Adafruit_NeoPixel rgbLed(1, rgbPin, NEO_GRB + NEO_KHZ800);

// 1602 液晶（I2C，ST7032 驱动芯片）：地址 0x3E，16 列 2 行
const byte LCD_ADDR = 0x3E;

// 向 ST7032 发送命令
void lcdCmd(byte cmd)
{
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x00); // 控制字节：Co=0, RS=0（命令）
    Wire.write(cmd);
    Wire.endTransmission();
    delayMicroseconds(100);
}

// 向 ST7032 发送显示数据
void lcdData(byte data)
{
    Wire.beginTransmission(LCD_ADDR);
    Wire.write(0x40); // 控制字节：Co=0, RS=1（数据）
    Wire.write(data);
    Wire.endTransmission();
    delayMicroseconds(100);
}

// 初始化 ST7032（5V 供电时序）
void lcdInit()
{
    delay(50);        // 等待上电稳定
    lcdCmd(0x38);     // Function set: 8 位数据长度, 2 行, IS=0
    lcdCmd(0x39);     // IS=1，进入扩展指令集
    lcdCmd(0x14);     // 内部振荡频率
    lcdCmd(0x74);     // 对比度低 4 位（5V 供电下 0x74 较合适）
    lcdCmd(0x54);     // Power/ICON/对比度高位：5V 供电，booster 关闭
    lcdCmd(0x6F);     // Follower 控制：5V 下 Rab=111
    delay(200);       // 等待电源稳定
    lcdCmd(0x38);     // IS=0，回到普通指令集
    lcdCmd(0x0C);     // 显示开，无光标，无闪烁
    lcdCmd(0x01);     // 清屏
    delay(2);
    lcdCmd(0x06);     // 输入方式：光标右移
}

// 打开 RGB 背光（PCA9632，地址 0x60）：三通道 PWM 全亮（白色）
void lcdBacklightOn()
{
    Wire.beginTransmission(0x60);
    Wire.write(0x00); Wire.write(0x00); // MODE1：正常工作
    Wire.endTransmission();
    Wire.beginTransmission(0x60);
    Wire.write(0x01); Wire.write(0x00); // MODE2
    Wire.endTransmission();
    for (byte reg = 0x02; reg <= 0x04; reg++) // PWM0~2 = 255
    {
        Wire.beginTransmission(0x60);
        Wire.write(reg); Wire.write(0xFF);
        Wire.endTransmission();
    }
    Wire.beginTransmission(0x60);
    Wire.write(0x08); Wire.write(0xFF); // LEDOUT：三路由各自 PWM 控制
    Wire.endTransmission();
}

// 设置光标位置：col 0~15，row 0~1
void lcdSetCursor(int col, int row)
{
    lcdCmd(0x80 | (row ? 0x40 : 0x00) | col);
}

const int SOUND_AMPLITUDE_THRESHOLD = 300;  // 声音波动幅度阈值，可按环境调整
const unsigned long SOUND_KEEP_MS = 1500;   // 声音触发后报警保持时间
const unsigned long BLINK_MS = 250;         // 报警灯闪烁半周期
const int INFRARED_TONE = 1000;             // 红外报警音调：1kHz（急促）
const int ACOUSTIC_TONE = 600;              // 声音报警音调：600Hz（稍缓）
const unsigned long DEBOUNCE_MS = 50;       // 按钮消抖时间

// 报警状态
enum State
{
    STATE_NORMAL,   // 正常
    STATE_INFRARED, // 红外入侵报警
    STATE_ACOUSTIC  // 声音异常报警
};

State state = STATE_NORMAL;
State lastShownState = STATE_NORMAL; // 上一次液晶显示的状态
bool soundAlarmOn = true;            // 声音报警功能是否切入
bool lastShownSoundOn = true;        // 上一次液晶显示的开关状态
unsigned long lastSoundTrigger = 0;  // 最近一次声音触发时间
bool lastButtonState = HIGH;         // 上一次的按钮电平

// 设置 RGB 灯颜色（WS2812 模块，单线协议）
void setRgb(bool r, bool g, bool b)
{
    rgbLed.setPixelColor(0, rgbLed.Color(r ? 255 : 0, g ? 255 : 0, b ? 255 : 0));
    rgbLed.show();
}

// 在指定行输出内容，不足 16 字符用空格补齐（清除残留字符）
void lcdPrintLine(int row, const char *text)
{
    lcdSetCursor(0, row);
    int i = 0;
    for (; text[i] != '\0' && i < 16; i++)
        lcdData(text[i]);
    for (; i < 16; i++)
        lcdData(' ');
}

// 按钮处理：按一下切换声音报警功能的切入/关闭
void handleButton()
{
    bool state = digitalRead(buttonPin);
    if (lastButtonState == HIGH && state == LOW) // 按下瞬间（下降沿）
    {
        delay(DEBOUNCE_MS); // 消抖后确认
        if (digitalRead(buttonPin) == LOW)
            soundAlarmOn = !soundAlarmOn;
    }
    lastButtonState = state;
}

// 检测声音波动幅度（峰值-谷值），返回是否超过阈值
bool detectSound()
{
    int mn = 1023, mx = 0;
    for (int i = 0; i < 50; i++) // 连续采样一小段时间
    {
        int v = analogRead(micPin);
        if (v < mn) mn = v;
        if (v > mx) mx = v;
    }
    return (mx - mn) >= SOUND_AMPLITUDE_THRESHOLD;
}

// 状态或开关变化时刷新液晶显示
void updateLcd()
{
    if (state == lastShownState
        && (state != STATE_NORMAL || soundAlarmOn == lastShownSoundOn))
        return;
    lastShownState = state;
    lastShownSoundOn = soundAlarmOn;

    switch (state)
    {
    case STATE_INFRARED:
        lcdPrintLine(0, "Infrared");
        lcdPrintLine(1, "Intrusion Alarm");
        break;
    case STATE_ACOUSTIC:
        lcdPrintLine(0, "Acoustic Anomaly");
        lcdPrintLine(1, "Alarm");
        break;
    default:
        lcdPrintLine(0, "Normal condition");
        lcdPrintLine(1, soundAlarmOn ? "Sound alarm: ON " : "Sound alarm: OFF");
        break;
    }
}

void setup()
{
    pinMode(pirPin, INPUT_PULLUP); // NPN 开集电极输出，需上拉
    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(buzzerPin, OUTPUT);
    rgbLed.begin();  // 初始化 WS2812
    setRgb(false, false, false); // 初始熄灭
    Wire.begin();
    lcdInit();
    lcdBacklightOn(); // 打开白色背光
}

void loop()
{
    handleButton(); // 按钮切换声音报警功能

    bool blink = (millis() / BLINK_MS) % 2 == 0; // 报警闪烁节拍

    // 声音异常检测（仅在声音报警切入时有效），触发后保持一段时间
    if (soundAlarmOn && detectSound())
        lastSoundTrigger = millis();

    // 确定当前状态：红外入侵 > 声音异常 > 正常
    // E18-D80NK 检测到人体时输出 LOW（NPN 开集电极，平时上拉为 HIGH）
    bool infrared = (digitalRead(pirPin) == LOW);
    bool acoustic = (millis() - lastSoundTrigger < SOUND_KEEP_MS);
    if (infrared)
        state = STATE_INFRARED;
    else if (acoustic)
        state = STATE_ACOUSTIC;
    else
        state = STATE_NORMAL;

    // 按状态输出声光报警
    switch (state)
    {
    case STATE_INFRARED: // 红外入侵：红灯闪烁 + 蜂鸣器急促鸣响
        setRgb(blink, false, false);
        if (blink) tone(buzzerPin, INFRARED_TONE); else noTone(buzzerPin);
        break;
    case STATE_ACOUSTIC: // 声音异常：紫灯（红+蓝）闪烁 + 蜂鸣器间歇鸣响
        setRgb(blink, false, blink);
        if (blink) tone(buzzerPin, ACOUSTIC_TONE); else noTone(buzzerPin);
        break;
    default:             // 正常：绿灯常亮
        setRgb(false, true, false);
        noTone(buzzerPin);
        break;
    }

    updateLcd(); // 刷新液晶显示
}
