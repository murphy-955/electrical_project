const int ledPin = 3; // LED 接 3 号引脚（PWM）
const int buttonPin = 7; // 按键接 7 号引脚

// 三档亮度值（0~255），可按手感调整
const int brightnessLevels[] = {0, 80, 160, 255};
int levelIndex = 0; // 当前档位：0=熄灭, 1=微亮, 2=更亮, 3=特亮

int lastButtonState = HIGH; // 上拉模式下未按下为 HIGH
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // 消抖时间

void setup()
{
    pinMode(ledPin, OUTPUT);
    pinMode(buttonPin, INPUT_PULLUP); // 启用内部上拉电阻
    analogWrite(ledPin, brightnessLevels[levelIndex]);
}

void loop()
{
    int reading = digitalRead(buttonPin);

    // 按键状态发生变化时，重置消抖计时器
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // 状态稳定超过消抖时间后，确认这是一次有效按键
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        static int buttonState = HIGH;

        if (reading != buttonState)
        {
            buttonState = reading;

            // 按下瞬间（低电平）切换档位
            if (buttonState == LOW)
            {
                levelIndex = (levelIndex + 1) % 4; // 0→1→2→3→0 循环
                analogWrite(ledPin, brightnessLevels[levelIndex]);
            }
        }
    }

    lastButtonState = reading;
}
