const int potPin = A0; // 旋钮接 A0
const int ledPin = 3; // LED 接 3 号引脚（PWM）

void setup()
{
    pinMode(ledPin, OUTPUT);
}

void loop()
{
    int sensorValue = analogRead(potPin); // 读取旋钮：0~1023
    int brightness = map(sensorValue, 0, 1023, 0, 255); // 映射到 PWM：0~255
    analogWrite(ledPin, brightness); // 输出 PWM 控制亮度
}
