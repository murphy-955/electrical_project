/*
 * LED 引脚测试程序：8、9、10 三盏灯每 0.5 秒一起亮/灭闪烁。
 * 用于排查状态指示 LED 不亮的问题。
 *
 * 判断方法：
 *   - 三盏灯一起闪烁 → 引脚和接线都正常，问题出在上传的主程序版本；
 *   - 不闪 → 上传未成功、引脚号接错、或板子/连线有问题（与主程序无关）。
 */

#include <Arduino.h>

void setup()
{
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
}

void loop()
{
    digitalWrite(8, HIGH);
    digitalWrite(9, HIGH);
    digitalWrite(10, HIGH);
    delay(500);
    digitalWrite(8, LOW);
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    delay(500);
}
