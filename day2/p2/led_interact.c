const int redButton = 2; // 红按钮（A）- 切换方向
const int greenButton = 3; // 绿按钮（B）- 调整速度

const int ledPins[] = {4, 5, 6, 7, 8}; // 5 盏 LED 引脚：4~8 号
const int numLeds = 5;

// 速度档位（毫秒）：快、中、慢 —— 可根据实际效果调整
const unsigned long speeds[] = {150, 400, 800};
const int numSpeeds = 3;

// ==================== 流水灯状态变量 ====================
int currentIndex = 0; // 当前点亮的 LED 索引（0~4 对应 4~8 号引脚）
int direction = 1; // 流动方向：1 = 左→右（4→8），-1 = 右→左（8→4）
int speedIndex = 1; // 当前速度档位：0=快, 1=中, 2=慢
unsigned long lastUpdateTime = 0; // 上次流水灯更新的时间戳
// ==================== 红按钮防抖变量 ====================
int lastRedReading = HIGH;
int stableRedState = HIGH;
unsigned long debounceTimeRed = 0;

// ==================== 绿按钮防抖变量 ====================
int lastGreenReading = HIGH;
int stableGreenState = HIGH;
unsigned long debounceTimeGreen = 0;

const unsigned long DEBOUNCE_MS = 20; // 软件防抖阈值：20 毫秒

void setup()
{
    pinMode(redButton, INPUT);
    pinMode(greenButton, INPUT);

    // 初始化所有 LED 引脚为输出并熄灭
    for (int i = 0; i < numLeds; i++)
    {
        pinMode(ledPins[i], OUTPUT);
        digitalWrite(ledPins[i], HIGH);
    }

    // 初始点亮第一盏灯（4 号引脚）
    digitalWrite(ledPins[currentIndex], LOW);

    Serial.begin(9600);
    Serial.println("交互式流水灯启动");
    Serial.println("红按钮(A)：切换方向 | 绿按钮(B)：切换速度（快→中→慢循环）");
}

void loop()
{
    // ---------- 1. 检测红按钮：切换方向 ----------
    if (checkButtonPress(redButton, lastRedReading, stableRedState, debounceTimeRed))
    {
        direction = -direction; // 方向取反：1 变 -1，-1 变 1
        String dirStr = (direction == 1) ? "左→右 (4→5→6→7→8)" : "右→左 (8→7→6→5→4)";
        Serial.println("【红按钮】方向切换为：" + dirStr);
    }

    // ---------- 2. 检测绿按钮：循环切换速度 ----------
    if (checkButtonPress(greenButton, lastGreenReading, stableGreenState, debounceTimeGreen))
    {
        speedIndex = (speedIndex + 1) % numSpeeds; // 循环：0→1→2→0...

        String speedStr;
        switch (speedIndex)
        {
        case 0: speedStr = "快 (150ms)";
            break;
        case 1: speedStr = "中 (400ms)";
            break;
        case 2: speedStr = "慢 (800ms)";
            break;
        }
        Serial.println("【绿按钮】速度切换为：" + speedStr);

        // 平滑过渡设计：不重置 lastUpdateTime。
        // 当前亮着的灯继续保持，下一盏灯在新的时间间隔后自然亮起，
        // 避免速度切换时出现闪烁、跳变或两灯同时亮的情况。
    }

    // ---------- 3. 非阻塞式流水灯更新 ----------
    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime >= speeds[speedIndex])
    {
        // 3.1 熄灭当前 LED
        digitalWrite(ledPins[currentIndex], HIGH);

        // 3.2 索引按方向移动
        currentIndex += direction;

        // 3.3 边界循环处理：到达首尾后自动回到另一端，形成闭环流水
        if (currentIndex >= numLeds)
        {
            currentIndex = 0; // 超过最右端，回到最左端（4号）
        }
        else if (currentIndex < 0)
        {
            currentIndex = numLeds - 1; // 超过最左端，回到最右端（8号）
        }

        // 3.4 点亮新的 LED
        digitalWrite(ledPins[currentIndex], LOW);

        // 3.5 更新时间戳
        lastUpdateTime = currentTime;
    }
}

// ==================== 通用按钮检测函数（带软件防抖） ====================
/**
 * 检测单个按钮的有效"按下"边沿事件
 * @param pin          按钮引脚号
 * @param lastReading  上次原始读数（引用，函数内更新）
 * @param stableState  防抖后的稳定状态（引用，函数内更新）
 * @param debounceTime 上次变化时间戳（引用，函数内更新）
 * @return true = 检测到一次有效按下；false = 无事件
 */
bool checkButtonPress(int pin, int &lastReading, int &stableState, unsigned long &debounceTime)
{
    int reading = digitalRead(pin);
    bool pressed = false;

    // 步骤 1：读数发生变化时，重置防抖计时器
    if (reading != lastReading)
    {
        debounceTime = millis();
    }

    // 步骤 2：状态持续稳定超过 DEBOUNCE_MS，才确认有效
    if ((millis() - debounceTime) > DEBOUNCE_MS)
    {
        // 步骤 3：稳定后的状态与之前记录的不同时，说明发生真实跳变
        if (reading != stableState)
        {
            stableState = reading;

            // 步骤 4：检测"按下"边沿（假设接下拉电阻，常态 LOW，按下 HIGH）
            // 若使用 INPUT_PULLUP（常态 HIGH，按下 LOW），改为 == LOW
            if (stableState == LOW)
            {
                pressed = true;
            }
        }
    }

    lastReading = reading; // 保存本次原始读数，供下次比较
    return pressed;
}
