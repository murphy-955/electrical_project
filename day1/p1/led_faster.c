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
  for(int i =0;i<5;i++){
    int inner_delay = 1000 - 150*i;
    led_flash(inner_delay);
  led_turn_off(inner_delay);
  led_turn_on(inner_delay);
  }
}

void led_flash(int inner_delay){
   int i;
   for(int i = 0;i<5;i++){
    digitalWrite(led_arr[i],LOW);
    delay(inner_delay);
    digitalWrite(led_arr[i],HIGH);
   }
}

void led_turn_off(int inner_delay){
  int i;
   for(int i = 0;i<5;i++){
    digitalWrite(led_arr[i],HIGH);
    delay(inner_delay);
   }
}

void led_turn_on(int inner_delay){
  int i;
   for(int i = 0;i<5;i++){
    digitalWrite(led_arr[i],LOW);
    delay(inner_delay);
   }
}