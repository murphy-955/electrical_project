const int buttonPin = 2; // 按钮引脚
const int ledPin = 3; // LED 引脚（题目要求 3 号）

// ---------- 防抖相关变量 ----------
int lastButtonState = LOW; // 上一次的原始读取值
int buttonState = LOW; // 经防抖确认后的稳定状态
unsigned long lastDebounceTime = 0; // 上次状态变化的时间戳
const unsigned long debounceDelay = 20; // 防抖时间，单位 ms（一般 20~50ms）

// ---------- 延时灯相关变量 ----------
unsigned long ledOnTime = 0; // LED 点亮的起始时刻
bool ledTimerRunning = false; // 延时计时是否进行中（true=正在延时）

void setup()
{
    pinMode(buttonPin, INPUT);
    pinMode(ledPin, OUTPUT);

    digitalWrite(ledPin, HIGH); // 初始状态：LED 熄灭（根据电路，HIGH=灭，LOW=亮）
    Serial.begin(9600);
}

void loop()
{
    int reading = digitalRead(buttonPin); // 读取按钮当前原始电平

    // 1. 如果原始电平与上次不同，说明可能发生抖动或真实动作，重置防抖计时器
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // 2. 只有当电平持续稳定超过 debounceDelay，才认为是有效状态
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // 3. 如果稳定后的状态与之前记录的不同，说明发生了真实的按下/释放
        if (reading != buttonState)
        {
            buttonState = reading; // 更新稳定状态

            // 4. 检测"按下"边沿（假设按钮接下拉电阻，常态 LOW，按下为 HIGH）
            //    只在按下的瞬间执行一次，避免按住不放时重复触发
            if (buttonState == HIGH)
            {
                digitalWrite(ledPin, LOW); // 点亮 LED（LOW=亮）
                ledOnTime = millis(); // 记录当前时间，作为延时起点
                ledTimerRunning = true; // 标记延时计时开始
                Serial.println("按钮按下，LED 点亮，开始 6 秒延时");
            }
        }
    }

    // 5. 非阻塞式 6 秒延时处理：不卡死 loop，可随时响应其他任务
    if (ledTimerRunning)
    {
        // 检查是否已经过了 6 秒（6000 毫秒）
        if (millis() - ledOnTime >= 6000)
        {
            digitalWrite(ledPin, HIGH); // 熄灭 LED（HIGH=灭）
            ledTimerRunning = false; // 停止计时
            Serial.println("6 秒时间到，LED 自动熄灭");
        }
    }

    // 6. 保存本次原始读数，供下次循环比较
    lastButtonState = reading;
}