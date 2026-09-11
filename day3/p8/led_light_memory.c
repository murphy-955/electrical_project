#include <Arduino.h>

const int potPin = A0;
const int ledPin = 3; // PWM
const int buttonPin = 4;

int brightness = 0; // 当前输出到 LED 的亮度
int targetBrightness = 0; // 按键按下时记忆的亮度（恢复目标）
bool recovering = false; // 是否处于"恢复中"状态
unsigned long recoverStart = 0;
const unsigned long recoverDuration = 1500; // 恢复时长 1.5 秒（>1秒）

unsigned long lastSerialTime = 0;
const unsigned long serialInterval = 100; // 串口打印间隔

// 按键消抖
int lastButtonReading = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup()
{
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP);
    Serial.begin(9600);
}

void loop()
{
    // ---------- 1. 按键检测 ----------
    int reading = digitalRead(buttonPin);
    if (reading != lastButtonReading) lastDebounceTime = millis();

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        static int buttonState = HIGH;
        if (reading != buttonState)
        {
            buttonState = reading;
            if (buttonState == LOW && !recovering)
            {
                // 按下瞬间，且不在恢复中
                targetBrightness = brightness; // 记忆当前亮度
                brightness = 0; // 瞬间降至最低
                analogWrite(ledPin, brightness);
                recovering = true;
                recoverStart = millis();
            }
        }
    }
    lastButtonReading = reading;

    // ---------- 2. 亮度更新 ----------
    if (recovering)
    {
        unsigned long elapsed = millis() - recoverStart;
        if (elapsed >= recoverDuration)
        {
            brightness = targetBrightness; // 恢复完成
            recovering = false;
        }
        else
        {
            // 线性恢复：1.5 秒内从 0 爬升到目标亮度
            brightness = map(elapsed, 0, recoverDuration, 0, targetBrightness);
        }
        analogWrite(ledPin, brightness);
    }
    else
    {
        // 常态：旋钮直接控制（狗头映射，两端抗抖动）
        int sensorValue = analogRead(potPin);
        brightness = constrain(map(sensorValue, 0, 1023, 0, 256), 0, 255);
        analogWrite(ledPin, brightness);
    }

    // ---------- 3. 串口监视 ----------
    if (millis() - lastSerialTime >= serialInterval)
    {
        lastSerialTime = millis();
        Serial.print("当前亮度: ");
        Serial.print(brightness);
        Serial.print("    目标亮度: ");
        Serial.println(recovering ? targetBrightness : brightness);
    }
}
