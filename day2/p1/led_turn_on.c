const int buttonPin = 2; // 按钮引脚
const int ledPin = 3; // LED 引脚（题目要求3号）

int ledState = LOW; // LED 当前状态（LOW=灭，HIGH=亮）
int buttonState; // 按钮稳定后的状态
int lastButtonState = LOW; // 上一次的按钮状态

unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 20; // 防抖时间 20ms

void setup()
{
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, ledState); // 初始化LED状态
    Serial.begin(9600);
}

void loop()
{
    int reading = digitalRead(buttonPin);

    // 1. 如果读取值变化，重置防抖计时器
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // 2. 状态持续稳定超过防抖时间，才确认有效
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // 3. 确认后的状态与之前记录的不同时，说明发生了真实跳变
        if (reading != buttonState)
        {
            buttonState = reading;

            // 4. 检测"按下"边沿（假设按下为 HIGH，常态为 LOW）
            //    如果电路相反（常态HIGH，按下LOW），改为 if (buttonState == LOW)
            if (buttonState == HIGH)
            {
                ledState = !ledState; // 翻转LED状态
                digitalWrite(ledPin, ledState); // 输出到LED

                if (ledState == HIGH)
                {
                    Serial.println("LED 点亮");
                }
                else
                {
                    Serial.println("LED 熄灭");
                }
            }
        }
    }

    lastButtonState = reading; // 保存本次原始读数，供下次比较
}