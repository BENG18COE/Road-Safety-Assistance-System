#include <LiquidCrystal.h>
LiquidCrystal lcd(5,8,10,11,12,13);

const int s1 = A1, s2 = A2, s3 = A3, gas = 2, brake = 3, brake_lights = 4, 
          wheels = 6, alcohol_sensor = A4, buzzer = 7;
int speed_limit = 0, counter = 0;
volatile int speed = 0;
volatile bool brake_pedal_pressed = false;

void concatenate(String* str, int columns){
  *str = str->substring(0, columns);
}
void print_lcd(String str, int row) {
  static const int COLUMNS = 16;
  if(str.length() > COLUMNS) concatenate(&str,COLUMNS);
  int pre_spaces = (COLUMNS - str.length()) / 2;
  lcd.setCursor(0, row); lcd.print(String(' ', COLUMNS));
  lcd.setCursor(pre_spaces, row); lcd.print(str);
}
void print_lcd(String str0, String str1) {
  static const int COLUMNS = 16;
  if(str0.length() > COLUMNS) concatenate(&str0,COLUMNS);
  if(str1.length() > COLUMNS) concatenate(&str1,COLUMNS);
  int pre_spaces0 = (COLUMNS - str0.length()) / 2;
  int pre_spaces1 = (COLUMNS - str1.length()) / 2;
  lcd.clear();
  lcd.setCursor(pre_spaces0, 0); lcd.print(str0);
  lcd.setCursor(pre_spaces1, 1); lcd.print(str1);
}
void default_lcd_text(){
  print_lcd("RF CAR","SPEED LIMIT");
}
void accelerate(){
  speed += 10;
  if(speed > speed_limit) speed = speed_limit;
}
void deccelerate(){
  brake_pedal_pressed = true;
  speed -= 10;
  if(speed < 0) speed = 0;
}
void emergency_deccelerate(){
  print_lcd("ENGAGING AUTO","DECCELERATE");
  digitalWrite(brake_lights,1);
  delay(1000);
  digitalWrite(brake_lights,0);
  speed = speed_limit;
}
bool alcohol_high() {
  int analog_value = analogRead(alcohol_sensor);
  if(analog_value > 102) return true;
  return false;
}
void beep_buzzer() {
  digitalWrite(buzzer, 1);
  delay(500);
  digitalWrite(buzzer, 0);
}
void soft_slow_down() {
  for(; speed != 0; speed -= 10) {
    analogWrite(wheels, speed * 3);
    print_lcd("LIMIT: " + (String)speed_limit, "SPEED: " + (String)speed);
    delay(300);
  }
}
void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);
  pinMode(s1,INPUT_PULLUP);
  pinMode(s2,INPUT_PULLUP);
  pinMode(s3,INPUT_PULLUP);
  pinMode(gas,INPUT);
  pinMode(brake,INPUT);
  pinMode(brake_lights,OUTPUT);
  pinMode(wheels,OUTPUT);
  pinMode(buzzer,OUTPUT);
  pinMode(alcohol_sensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(gas), accelerate, FALLING);
  attachInterrupt(digitalPinToInterrupt(brake), deccelerate, FALLING);
  default_lcd_text();
  delay(500);
  //analogWrite(wheels,250); while(1);
}
void loop() {
  if(alcohol_high()){
    print_lcd("WARNING!","too much alcohol");
    if(counter == 0) {
      Serial.println("driver number 0023 has taken too much alcohol");
      counter = 3;
    }
    else counter--;
    delay(500);
    if(speed > 0) {
      beep_buzzer();
      soft_slow_down();
    }
  }
  else {
    if(!digitalRead(s1)) speed_limit = 30;
    else if(!digitalRead(s2)) speed_limit = 50;
    else if(!digitalRead(s3)) speed_limit = 80;
    else speed_limit = 80;
    if(speed > speed_limit) emergency_deccelerate();
    print_lcd("LIMIT: " + (String)speed_limit, "SPEED: " + (String)speed);
    analogWrite(wheels, speed * 3);

    //delay loop
    unsigned long t = millis();
    while( (millis() - t) < 500 ){
      if(brake_pedal_pressed){
        digitalWrite(brake_lights, 1);
        delay(200);
        digitalWrite(brake_lights,0);
        brake_pedal_pressed = false;
        break;
      }
    }
    counter = 0;
  }
}
