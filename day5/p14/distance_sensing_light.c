/*
 * 实验十四 任务二：超声波距离感应彩灯
 *
 * 题目要求：
 *   在任务一基础上，设计一个超声波距离感应彩灯，要求液晶显示器显示实时
 *   距离，同时根据距离变化，控制 RGB 灯的颜色：距离 30cm~50cm 时，灯为
 *   蓝色；距离在 20~30cm 时，灯为绿色；距离小于 20cm 时，灯为红色并伴随
 *   蜂鸣器报警，且距离越近声音越急促。当检测到物体移开时（>50cm），灯
 *   恢复为熄灭状态。
 *
 * 实现思路：
 *   - HC-SR04 超声波模块：Trig 接 6 号引脚，Echo 接 7 号引脚；
 *   - 蜂鸣器接 8 号引脚；RGB LED 模块（WS2812 可寻址，单引脚控制）：
 *     数据线 DIN 接 13 号引脚，VCC 接 5V，GND 接 GND；
 *   - 1602 液晶显示器（I2C，ST7032 驱动芯片）：SDA 接 A4，SCL 接 A5，
 *     I2C 地址 0x3E；本模块不是 PCF8574 方案，需按 ST7032 时序驱动；
 *     背光由 PCA9632（地址 0x60）控制，初始化为白色背光；
 *   - 距离分档：>50cm 灯灭且安静；30~50cm 蓝灯；20~30cm 绿灯；
 *     <20cm 红灯 + 蜂鸣器间歇报警，间隔 = map(距离, 3~20cm, 80~500ms)，
 *     距离越近间隔越短、声音越急促；
 *   - 液晶第一行显示实时距离，第二行显示当前档位提示。
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

const int trigPin   = 6; // 超声波 Trig
const int echoPin   = 7; // 超声波 Echo
const int buzzerPin = 8; // 蜂鸣器
const int rgbPin    = 13; // RGB LED 模块（WS2812）数据线

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

const long FAR_CM = 50;   // 超过该距离：灯灭
const long BLUE_CM = 30;  // 30~50cm：蓝灯
const long GREEN_CM = 20; // 20~30cm：绿灯；小于 20cm：红灯 + 报警
const long NEAR_CM = 3;   // 最近距离标定点
const int TONE_FREQ = 800;         // 报警音调：800Hz
const unsigned long SLOW_MS = 500; // 20cm 处的蜂鸣间隔（平缓）
const unsigned long FAST_MS = 80;  // 3cm 处的蜂鸣间隔（急促）
const unsigned long BEEP_ON_MS = 60;  // 每次蜂鸣发声时长
const unsigned long MEASURE_MS = 200; // 测距/刷新间隔

float distanceCm = -1;             // 最近一次测距结果（-1 表示超出量程）
unsigned long lastMeasureTime = 0; // 上一次测距的时刻

// 读取 HC-SR04 超声波测距，返回距离（cm），超时无回波返回 -1
float readDistanceCm()
{
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 30000UL); // 超时 30ms，约 5m
    if (duration == 0)
        return -1;                      // 超出量程
    return duration / 58.0;             // 声速换算：58us ≈ 1cm
}

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

// 红灯档（<20cm）：蜂鸣器间歇报警，距离越近间隔越短、声音越急促
void runAlarm()
{
    unsigned long interval = map(constrain((long)distanceCm, NEAR_CM, GREEN_CM),
                                 NEAR_CM, GREEN_CM, FAST_MS, SLOW_MS);
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
    rgbLed.setBrightness(50);
    rgbLed.begin();  // 初始化 WS2812
    rgbLed.show();   // 初始熄灭
    Wire.begin();
    lcdInit();
    lcdBacklightOn(); // 打开白色背光
}

void loop()
{
    if (millis() - lastMeasureTime >= MEASURE_MS)
    {
        lastMeasureTime = millis();
        distanceCm = readDistanceCm();

        // 液晶第一行：实时距离
        if (distanceCm < 0)
            lcdPrintLine(0, "Dist: out range");
        else
        {
            char buf[17];
            dtostrf(distanceCm, 0, 1, buf); // 保留 1 位小数
            char line[17];
            snprintf(line, sizeof(line), "Dist: %s cm", buf);
            lcdPrintLine(0, line);
        }

        // 距离分档控制 RGB 灯与蜂鸣器，液晶第二行显示当前档位
        if (distanceCm < 0 || distanceCm > FAR_CM)
        {
            setRgb(false, false, false); // 物体移开，灯恢复熄灭
            noTone(buzzerPin);
            lcdPrintLine(1, "Safe: light off");
        }
        else if (distanceCm > BLUE_CM)
        {
            setRgb(false, false, true);  // 30~50cm：蓝色
            noTone(buzzerPin);
            lcdPrintLine(1, "30-50cm: Blue");
        }
        else if (distanceCm > GREEN_CM)
        {
            setRgb(false, true, false);  // 20~30cm：绿色
            noTone(buzzerPin);
            lcdPrintLine(1, "20-30cm: Green");
        }
        else
        {
            setRgb(true, false, false);  // <20cm：红色 + 蜂鸣器报警
            lcdPrintLine(1, "<20cm: Red Alarm");
        }
    }

    // 红灯档才需要蜂鸣器按距离急促报警（放在分档外，保证节拍不被测距间隔限制）
    if (distanceCm >= 0 && distanceCm <= GREEN_CM)
        runAlarm();
    else
        noTone(buzzerPin);
}
