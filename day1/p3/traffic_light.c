// 十字路口双向交通灯 + 6灯倒计时流水灯
// 1号灯 R=2, Y=3, G=4 ； 2号灯 R=5, Y=6, G=7
// 倒计时LED：8, 9, 10, 11, 12, 13（6个）
// 接线说明：LOW=点亮，HIGH=熄灭（共阳极）

int light1[3] = {2, 3, 4};            // 依次：红、黄、绿
int light2[3] = {5, 6, 7};            // 依次：红、黄、绿
int countdown_leds[6] = {8, 9, 10, 11, 12, 13};  // 6个倒计时LED

#define RED    0
#define YELLOW 1
#define GREEN  2

const int GREEN_TIME = 6000;    // 绿灯 6 秒
const int RED_TIME   = 6000;    // 红灯 6 秒
const int YELLOW_BLINK = 3;     // 黄灯闪烁 3 次
const int BLINK_INTERVAL = 500; // 黄灯闪烁间隔 500ms

void setup() {
  Serial.begin(115200);

  // 初始化交通灯
  for (int i = 0; i < 3; i++) {
    pinMode(light1[i], OUTPUT);
    pinMode(light2[i], OUTPUT);
  }

  // 初始化倒计时 LED
  for (int i = 0; i < 6; i++) {
    pinMode(countdown_leds[i], OUTPUT);
  }

  allLightsOff();
  countdownAllOff();
  Serial.println("系统启动，交通灯+倒计时流水灯开始运行...");
}

void loop() {
  // ========== 阶段1：方向1绿灯 | 方向2红灯 + 倒计时 ==========
  Serial.println("\n>>> 阶段1：方向1 绿灯 | 方向2 红灯");
  setTrafficLight(1, GREEN);
  setTrafficLight(2, RED);
  countdownDisplay(6);  // 6秒倒计时，6灯逐秒熄灭

  // 黄灯过渡（倒计时LED全灭）
  yellowTransition();

  // ========== 阶段2：方向1红灯 | 方向2绿灯 + 倒计时 ==========
  Serial.println("\n>>> 阶段2：方向1 红灯 | 方向2 绿灯");
  setTrafficLight(1, RED);
  setTrafficLight(2, GREEN);
  countdownDisplay(6);

  // 黄灯过渡
  yellowTransition();
}

// 倒计时显示：从6灯全亮开始，每过1秒熄灭1盏，正好6秒
void countdownDisplay(int seconds) {
  // 6个LED全部点亮
  for (int i = 0; i < 6; i++) {
    digitalWrite(countdown_leds[i], LOW);
  }
  Serial.println("  [倒计时] 6灯全亮");

  // 每秒熄灭一盏
  for (int i = 0; i < seconds; i++) {
    delay(1000);
    digitalWrite(countdown_leds[i], HIGH);  // 熄灭一盏

    Serial.print("  [倒计时] 剩余 ");
    Serial.print(seconds - i - 1);
    Serial.println(" 秒，熄灭一盏");
  }
}

// 黄灯闪烁过渡：两个方向同时黄闪，倒计时LED保持全灭
void yellowTransition() {
  Serial.println(">>> 黄灯闪烁过渡（倒计时LED全灭）...");

  countdownAllOff();  // 确保倒计时灯不亮
  allLightsOff();
  delay(200);

  for (int i = 0; i < YELLOW_BLINK; i++) {
    digitalWrite(light1[YELLOW], LOW);  // 黄灯亮
    digitalWrite(light2[YELLOW], LOW);
    delay(BLINK_INTERVAL);

    digitalWrite(light1[YELLOW], HIGH); // 黄灯灭
    digitalWrite(light2[YELLOW], HIGH);
    delay(BLINK_INTERVAL);

    Serial.print("  黄灯闪烁第 ");
    Serial.print(i + 1);
    Serial.println(" 次");
  }
}

// 设置指定方向的单一灯色
void setTrafficLight(int direction, int color) {
  int *light = (direction == 1) ? light1 : light2;
  for (int i = 0; i < 3; i++) {
    digitalWrite(light[i], HIGH);
  }
  digitalWrite(light[color], LOW);
}

// 熄灭所有交通灯
void allLightsOff() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(light1[i], HIGH);
    digitalWrite(light2[i], HIGH);
  }
}

// 熄灭所有倒计时LED
void countdownAllOff() {
  for (int i = 0; i < 6; i++) {
    digitalWrite(countdown_leds[i], HIGH);
  }
}