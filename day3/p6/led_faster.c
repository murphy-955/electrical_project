const int ledPin = 3; // LED 接 3 号引脚（PWM）

int brightness = 0; // 当前亮度 0~255
int step = 5; // 亮度步长
unsigned long delayTime = 30; // 初始每次变化的间隔(ms)，控制呼吸频率

unsigned long minDelay = 2; // 最快时的间隔
unsigned long maxDelay = 30; // 初始（最慢）间隔
unsigned long stepDown = 2; // 每轮加快的幅度
unsigned long lastTime = 0;

void setup()
{
    pinMode(ledPin, OUTPUT);
}

void loop()
{
    // 非阻塞延时：到达间隔时间才更新亮度
    if (millis() - lastTime >= delayTime)
    {
        lastTime = millis();

        analogWrite(ledPin, brightness);
        brightness += step;

        // 到达边界则反向
        if (brightness <= 0 || brightness >= 255)
        {
            step = -step;
            brightness = constrain(brightness, 0, 255);

            // 每完成一轮呼吸（一个完整周期），加快一点
            delayTime -= stepDown;

            // 快到一定程度后，恢复初始频率，开始新一轮循环
            if (delayTime <= minDelay)
            {
                delayTime = maxDelay;
            }
        }
    }
}
