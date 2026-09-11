const int buttonPin = 7; // 按键
const int ledPins[] = {8, 9, 10}; // 从低位到高位：bit0, bit1, bit2
const int ledCount = 3;

int count = 0; // 当前计数 0~8

int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup()
{
    pinMode(buttonPin, INPUT_PULLUP); // 启用内部上拉
    for (int i = 0; i < ledCount; i++)
    {
        pinMode(ledPins[i], OUTPUT);
    }
    showBinary(count);
}

void loop()
{
    int reading = digitalRead(buttonPin);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        static int buttonState = HIGH;

        if (reading != buttonState)
        {
            buttonState = reading;

            if (buttonState == LOW)
            {
                // 按下瞬间
                count++;
                if (count > 7)
                {
                    // 第8次按下，归零
                    count = 0;
                }
                showBinary(count);
            }
        }
    }

    lastButtonState = reading;
}

// 把数值按二进制显示到 3 盏 LED 上
void showBinary(int value)
{
    for (int i = 0; i < ledCount; i++)
    {
        digitalWrite(ledPins[i], (value >> i) & 1 ? HIGH : LOW);
    }
}
