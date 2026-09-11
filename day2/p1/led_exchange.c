const int buttonPin = 2; // 按钮接 2 号引脚
const int redPin = 3; // 红灯接 3 号引脚
const int greenPin = 4; // 绿灯接 4 号引脚

// ==================== 防抖相关变量 ====================
int lastButtonReading = LOW; // 上一次的原始读数
int stableButtonState = LOW; // 经防抖确认后的稳定状态
unsigned long lastDebounceTime = 0; // 上次读数变化的时间戳
const unsigned long DEBOUNCE_MS = 20; // 防抖延时：20 毫秒

// ==================== 手势识别变量 ====================
bool isPressed = false; // 当前按钮是否处于按下状态
unsigned long pressStartTime = 0; // 本次按下的开始时刻（毫秒）
bool longPressTriggered = false; // 长按动作是否已经触发过（防止重复）

bool waitingForSecondClick = false; // 是否在等待第二次点击（双击窗口期）
unsigned long clickWindowStart = 0; // 双击窗口开始时刻
bool doubleClickInProgress = false; // 已检测到双击意图，等待第二次释放

// ==================== 阈值参数 ====================
const unsigned long LONG_PRESS_MS = 800; // 长按阈值：按住超过 800ms 视为长按
const unsigned long DOUBLE_CLICK_MS = 400; // 双击窗口：两次释放间隔小于 400ms 视为双击

void setup()
{
    pinMode(buttonPin, INPUT);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);

    allOff(); // 初始状态：所有灯熄灭
    Serial.begin(9600);
    Serial.println("系统启动，等待按键...");
}

// ==================== LED 控制辅助函数 ====================
// 说明：以下函数假设 LED 为共阴极接法（HIGH = 亮，LOW = 灭）。
// 如果你的电路是共阳极接法（LOW = 亮），请将 HIGH 和 LOW 互换。
void ledRed(bool on)
{
    digitalWrite(redPin, on ? HIGH : LOW);
}

void ledGreen(bool on)
{
    digitalWrite(greenPin, on ? HIGH : LOW);
}

void allOff()
{
    ledRed(false);
    ledGreen(false);
}

void loop()
{
    // ---------- 1. 读取按钮原始状态 ----------
    int reading = digitalRead(buttonPin);

    // ---------- 2. 软件防抖 ----------
    // 如果读数发生变化，重置防抖计时器
    if (reading != lastButtonReading)
    {
        lastDebounceTime = millis();
    }

    // 只有当状态持续稳定超过 DEBOUNCE_MS，才认为是有效变化
    if ((millis() - lastDebounceTime) > DEBOUNCE_MS)
    {
        if (reading != stableButtonState)
        {
            stableButtonState = reading;

            // 产生边沿事件
            if (stableButtonState == HIGH)
            {
                onButtonPress(); // 按下边沿（假设常态 LOW，按下 HIGH）
            }
            else
            {
                onButtonRelease(); // 释放边沿
            }
        }
    }

    lastButtonReading = reading; // 保存本次原始读数

    // ---------- 3. 长按检测（按下期间持续检查，不等待释放） ----------
    if (isPressed && !longPressTriggered)
    {
        if (millis() - pressStartTime >= LONG_PRESS_MS)
        {
            onLongPress();
            longPressTriggered = true;
            // 长按触发后，取消所有待处理的单击/双击状态，避免冲突
            waitingForSecondClick = false;
            doubleClickInProgress = false;
        }
    }

    // ---------- 4. 单击确认（双击窗口超时处理） ----------
    // 只有在按钮释放且处于等待第二次点击的状态时，才检查是否超时
    if (!isPressed && waitingForSecondClick)
    {
        if (millis() - clickWindowStart >= DOUBLE_CLICK_MS)
        {
            onSingleClick();
            waitingForSecondClick = false;
        }
    }

    // ---------- 5. 释放后重置长按标记 ----------
    if (!isPressed)
    {
        longPressTriggered = false;
    }
}

// ==================== 边沿事件处理 ====================

// 按下边沿：记录时间，并检查是否构成双击
void onButtonPress()
{
    isPressed = true;
    pressStartTime = millis();

    // 如果在双击窗口内再次按下，说明用户意图是双击
    if (waitingForSecondClick)
    {
        waitingForSecondClick = false; // 退出单击等待状态
        doubleClickInProgress = true; // 标记双击进行中
    }
}

// 释放边沿：区分短按（单击/双击）和长按
void onButtonRelease()
{
    isPressed = false;
    unsigned long pressDuration = millis() - pressStartTime;

    // 如果本次按下已经达到长按阈值，则释放时不处理（长按已在按下时触发）
    if (pressDuration >= LONG_PRESS_MS)
    {
        return;
    }

    // 如果双击已确认，这是第二次释放，执行双击动作
    if (doubleClickInProgress)
    {
        onDoubleClick();
        doubleClickInProgress = false;
    }
    // 否则这是第一次短按释放，启动双击等待窗口
    else
    {
        waitingForSecondClick = true;
        clickWindowStart = millis();
    }
}

// ==================== 动作执行函数 ====================

// 单击：红灯亮，绿灯灭
void onSingleClick()
{
    ledRed(true);
    ledGreen(false);
    Serial.println("【单击】红灯点亮，绿灯熄灭");
}

// 双击：绿灯亮，红灯灭
void onDoubleClick()
{
    ledRed(false);
    ledGreen(true);
    Serial.println("【双击】绿灯点亮，红灯熄灭");
}

// 长按：所有灯熄灭
void onLongPress()
{
    allOff();
    Serial.println("【长按】所有 LED 熄灭");
}
