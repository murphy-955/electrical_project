const int redButton = 2; // 红色按钮接 2 号引脚
const int greenButton = 3; // 绿色按钮接 3 号引脚
const int ledPin = 4; // LED 接 4 号引脚

// ---------- 红色按钮防抖变量 ----------
int lastReadingRed = LOW; // 上次原始读数
int stableStateRed = LOW; // 防抖后的稳定状态
unsigned long debounceTimeRed = 0; // 上次变化时间戳

// ---------- 绿色按钮防抖变量 ----------
int lastReadingGreen = LOW; // 上次原始读数
int stableStateGreen = LOW; // 防抖后的稳定状态
unsigned long debounceTimeGreen = 0; // 上次变化时间戳

const unsigned long DEBOUNCE_MS = 20; // 防抖阈值 20ms

int ledState = LOW; // LED 当前状态（LOW=灭，HIGH=亮）

void setup()
{
    pinMode(redButton, INPUT);
    pinMode(greenButton, INPUT);
    pinMode(ledPin, OUTPUT);

    digitalWrite(ledPin, ledState); // 初始化 LED（默认熄灭）
    Serial.begin(9600);
    Serial.println("双控开关系统启动，等待按键...");
}

void loop()
{
    // 分别检测两个按钮，若任一按钮产生有效"按下"边沿，则翻转 LED
    if (checkButtonPress(redButton, lastReadingRed, stableStateRed, debounceTimeRed))
    {
        toggleLed();
        Serial.println("【红色按钮】触发，LED 状态翻转");
    }

    if (checkButtonPress(greenButton, lastReadingGreen, stableStateGreen, debounceTimeGreen))
    {
        toggleLed();
        Serial.println("【绿色按钮】触发，LED 状态翻转");
    }
}

// ==================== 辅助函数 ====================

/**
 * 检测单个按钮的有效按下事件（带软件防抖）
 *
 * @param pin           按钮引脚号
 * @param lastReading   该按钮上次的原始读数（引用，函数内会更新）
 * @param stableState   该按钮防抖后的稳定状态（引用，函数内会更新）
 * @param debounceTime  该按钮上次变化的时间戳（引用，函数内会更新）
 * @return true 表示检测到一次有效的"按下"边沿；false 表示无事件
 */
bool checkButtonPress(int pin, int &lastReading, int &stableState, unsigned long &debounceTime)
{
    int reading = digitalRead(pin); // 读取当前原始电平
    bool pressed = false; // 返回值：是否检测到有效按下

    // 1. 如果读数发生变化，重置防抖计时器
    if (reading != lastReading)
    {
        debounceTime = millis();
    }

    // 2. 状态持续稳定超过 DEBOUNCE_MS，才确认有效
    if ((millis() - debounceTime) > DEBOUNCE_MS)
    {
        // 3. 稳定后的状态与之前记录的不同时，说明发生了真实跳变
        if (reading != stableState)
        {
            stableState = reading;

            // 4. 检测"按下"边沿（假设接下拉电阻，常态 LOW，按下为 HIGH）
            //    如果电路相反（内部上拉 INPUT_PULLUP），改为 if (stableState == LOW)
            if (stableState == HIGH)
            {
                pressed = true;
            }
        }
    }

    lastReading = reading; // 保存本次原始读数，供下次比较
    return pressed;
}

/**
 * 翻转 LED 状态
 */
void toggleLed()
{
    ledState = !ledState; // 逻辑取反：亮变灭，灭变亮
    digitalWrite(ledPin, ledState); // 输出到 LED 引脚
}