// 十字路口双向交通灯
// 1号灯 R=2, Y=3, G=4 ； 2号灯 R=5, Y=6, G=7
// 接线说明：本代码使用共阳极接法，LOW=点亮，HIGH=熄灭

int light1[3] = {2, 3, 4};   // 依次：红、黄、绿
int light2[3] = {10,11,12};   // 依次：红、黄、绿

#define RED    0
#define YELLOW 1
#define GREEN  2

const int GREEN_TIME = 6000;    // 绿灯亮 6 秒
const int RED_TIME   = 6000;    // 红灯亮 6 秒
const int YELLOW_BLINK = 3;     // 黄灯闪烁 3 次
const int BLINK_INTERVAL = 500; // 黄灯闪烁间隔 500ms（亮500ms 灭500ms）

void setup() {
  Serial.begin(115200);

  // 初始化 6 个交通灯引脚
  for (int i = 0; i < 3; i++) {
    pinMode(light1[i], OUTPUT);
    pinMode(light2[i], OUTPUT);
  }

  allLightsOff();  // 初始全部熄灭
  Serial.println("系统启动，交通灯开始运行...");
}

void loop() {
  // ========== 阶段1：方向1通行（绿），方向2禁止（红）==========
  Serial.println("\n>>> 阶段1：方向1 绿灯 | 方向2 红灯");
  setTrafficLight(1, GREEN);
  setTrafficLight(2, RED);
  delay(GREEN_TIME);

  // 黄灯过渡
  yellowTransition();

  // ========== 阶段2：方向1禁止（红），方向2通行（绿）==========
  Serial.println("\n>>> 阶段2：方向1 红灯 | 方向2 绿灯");
  setTrafficLight(1, RED);
  setTrafficLight(2, GREEN);
  delay(RED_TIME);

  // 黄灯过渡
  yellowTransition();
}

// 设置指定方向的单一灯色（先熄灭该方向所有灯，再点亮目标灯）
void setTrafficLight(int direction, int color) {
  int *light = (direction == 1) ? light1 : light2;

  for (int i = 0; i < 3; i++) {
    digitalWrite(light[i], HIGH);  // 熄灭
  }
  digitalWrite(light[color], LOW); // 点亮目标灯
}

// 两个方向同时黄灯闪烁 3 次，作为切换过渡
void yellowTransition() {
  Serial.println(">>> 黄灯闪烁过渡...");
  allLightsOff();
  delay(200);  // 短暂全灭，避免灯色重叠

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

// 熄灭所有交通灯
void allLightsOff() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(light1[i], HIGH);
    digitalWrite(light2[i], HIGH);
  }
}