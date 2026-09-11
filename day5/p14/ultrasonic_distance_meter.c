/*
 * 实验十四 任务一：超声波测距仪
 *
 * 题目要求：
 *   请设计一个超声波测距仪，要求带板载显示器，能实时显示测距结果。
 *
 * 实现思路：
 *   - HC-SR04 超声波模块：Trig 接 6 号引脚，Echo 接 7 号引脚；
 *   - 1602 液晶显示器（I2C，ST7032 驱动芯片）：SDA 接 A4，SCL 接 A5，
 *     I2C 地址 0x3E；注意本模块不是 PCF8574 方案，不能用
 *     LiquidCrystal_I2C 库驱动，需按 ST7032 时序用 Wire 直接驱动；
 *     背光由 PCA9632（地址 0x60）控制，初始化为白色背光；
 *   - 每 200ms 测一次距，第一行显示标题，第二行实时显示距离（cm）；
 *   - 超出量程（无回波）时显示 "out of range"。
 */

#include <Arduino.h>
#include <Wire.h>

const int trigPin = 6; // 超声波 Trig
const int echoPin = 7; // 超声波 Echo

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

const unsigned long MEASURE_MS = 200; // 测距/刷新间隔

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

void setup()
{
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    Wire.begin();
    lcdInit();
    lcdBacklightOn(); // 打开白色背光
    lcdPrintLine(0, "Distance Meter");
}

void loop()
{
    if (millis() - lastMeasureTime >= MEASURE_MS)
    {
        lastMeasureTime = millis();
        float distanceCm = readDistanceCm();

        // 第二行实时显示测距结果
        if (distanceCm < 0)
        {
            lcdPrintLine(1, "Dist: out range");
        }
        else
        {
            char buf[17];
            dtostrf(distanceCm, 0, 1, buf); // 保留 1 位小数
            char line[17];
            snprintf(line, sizeof(line), "Dist: %s cm", buf);
            lcdPrintLine(1, line);
        }
    }
}
