int led_arr[5] = {2, 3, 4, 5, 6};   // 5个LED引脚，对应位置：左-左中-中-右中-右
int delay_time = 300;                // 每步间隔，单位毫秒

void setup() {
  Serial.begin(115200);
  // 初始化所有LED为输出，并默认熄灭（HIGH）
  for (int i = 0; i < 5; i++) {
    pinMode(led_arr[i], OUTPUT);
    digitalWrite(led_arr[i], HIGH);
  }
}

void loop() {
  Serial.println("=== 模式：两端向中间汇聚点亮 ===");
  convergeToCenter();   // 从两边向中间逐个点亮

  delay(800);           // 全亮后保持片刻

  Serial.println("=== 模式：从中间向两端扩散熄灭 ===");
  spreadFromCenter();   // 从中间向两边逐个熄灭

  delay(800);           // 全灭后停顿，再开始下一轮
}

// ==================== 汇聚点亮 ====================
// 从左右两端同时向中间逐个点亮，已点亮的保持
void convergeToCenter() {
  // 5个灯，从外向内共需3步（第0步两端，第1步次两端，第2步正中）
  for (int step = 0; step < 3; step++) {
    int leftIndex  = step;           // 左侧要点亮的索引：0 → 1 → 2
    int rightIndex = 4 - step;       // 右侧要点亮的索引：4 → 3 → 2

    digitalWrite(led_arr[leftIndex], LOW);   // 点亮左侧（共阴/共阳视接线，LOW=亮）

    if (leftIndex != rightIndex) {
      digitalWrite(led_arr[rightIndex], LOW); // 点亮右侧（对称位置）
      Serial.print("点亮: 左[");
      Serial.print(led_arr[leftIndex]);
      Serial.print("] 和 右[");
      Serial.print(led_arr[rightIndex]);
      Serial.println("]");
    } else {
      // 最后一步左右汇聚到同一点（正中间）
      Serial.print("点亮: 正中[");
      Serial.print(led_arr[leftIndex]);
      Serial.println("] —— 全部点亮！");
    }

    delay(delay_time);
  }
}

// ==================== 扩散熄灭 ====================
// 从正中间开始向左右两端逐个熄灭，已熄灭保持
void spreadFromCenter() {
  // 从中间向两边，共需3步
  for (int step = 0; step < 3; step++) {
    int centerLeft  = 2 - step;      // 从中心向左扩散：2 → 1 → 0
    int centerRight = 2 + step;      // 从中心向右扩散：2 → 3 → 4

    digitalWrite(led_arr[centerLeft], HIGH);  // 熄灭左侧

    if (centerLeft != centerRight) {
      digitalWrite(led_arr[centerRight], HIGH); // 熄灭右侧
      Serial.print("熄灭: 左[");
      Serial.print(led_arr[centerLeft]);
      Serial.print("] 和 右[");
      Serial.print(led_arr[centerRight]);
      Serial.println("]");
    } else {
      // 第一步只熄灭正中间
      Serial.print("熄灭: 正中[");
      Serial.print(led_arr[centerLeft]);
      Serial.println("] —— 开始扩散...");
    }

    delay(delay_time);
  }
  Serial.println("全部熄灭！");
}