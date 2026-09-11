int led_arr[5] = {2,3,4,5,6};
int long_singal = 1500;
int short_singal = 200;
void setup() {
  int i ;
  Serial.begin(115200);
  for(int i = 0;i<5;i++){
      pinMode(led_arr[i],OUTPUT);
      digitalWrite(led_arr[i],HIGH);
   }
}

void loop() {
  Serial.println("start");
  for (int i =0;i<3;i++){
    led_turn_on(short_singal);
    led_turn_off(short_singal);
  }

  for (int i =0;i<3;i++){
    led_turn_on(long_singal);
    led_turn_off(long_singal);
  }
}


void led_turn_on(int singal){
  int i;
   for(int i = 0;i<5;i++){
    digitalWrite(led_arr[i],LOW);
   }
   delay(singal);
}

void led_turn_off(int singal){
    int i;
   for(int i = 0;i<5;i++){
    digitalWrite(led_arr[i],HIGH);
   }
   delay(singal);
}