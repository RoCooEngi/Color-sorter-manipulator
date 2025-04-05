#include <Arduino.h>
#include <Servo.h>

#define LED A2

Servo servo_push; //forward-back: 160 - 75
Servo servo_raise; //up - down: 150 - 60
Servo servo_rotate; //rotate left - right: 162 - 0
Servo servo_catch; //catch open - close: 70 - 170

String esp32value;
int rotation;

unsigned long time = millis();
unsigned long current_time;
unsigned time_value = 100;
bool led_condition = false;

void slow_servo(Servo servo, int target, int speed = 5) {
  // Rotates the servo at a slow rate
  int current = servo.read();
  while (current != target) {
      if (current < target) current++;
      else current--;
      servo.write(current);
      delay(speed);
  }
  delay(100);
}

void servo_return() {
  // Returns the servos to their original position
  servo_catch.write(70);
  delay(100);
  slow_servo(servo_raise, 80);
  slow_servo(servo_push, 110);
  slow_servo(servo_rotate, 163);
}

void cube_sorting(int rotation) {
  // Rotates servo drives during sorting
  servo_catch.write(135);
  slow_servo(servo_raise, 110);
  slow_servo(servo_push, 70);
  slow_servo(servo_rotate, rotation);
  slow_servo(servo_push, 110);
  slow_servo(servo_raise, 30);
  slow_servo(servo_push, 140);  
}

void led_flickering() {
  current_time = millis();
  if (current_time - time > time_value) {
    led_condition = true;
    time = millis();
    digitalWrite(LED, HIGH);
  } else if (current_time - time > time_value / 2 && led_condition == true){
    digitalWrite(LED, LOW);
    led_condition = false;
  }
}

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  // Starting position
  servo_catch.write(70);
  delay(100);
  servo_raise.write(80);
  delay(100);
  servo_push.write(110);
  delay(100);
  servo_rotate.write(163);
  delay(100);

  // Fixing to PWM pins
  servo_push.attach(3);
  servo_raise.attach(6);
  servo_catch.attach(9);
  servo_rotate.attach(5);
  delay(100);
}

void loop() {
  if (Serial.available()) {
    // Getting data from ESP32 CAM
    led_flickering();
    esp32value = Serial.readStringUntil('\n');
    esp32value.trim();
    if (esp32value != "RED" && esp32value != "GREEN" && esp32value != "PURPLE"){
      return;
    }
  }
  else {
    led_flickering();
    Serial.println(F("UART disconnected"));
    return;
  }
  digitalWrite(LED, LOW);
  Serial.println(esp32value);
  // Performing sorting by color
  if (esp32value == "RED") {
    rotation = 108;
  }
  else if (esp32value == "GREEN") {
    rotation = 54;
  }
  else if (esp32value == "PURPLE") {
    rotation = 0;
  }
  cube_sorting(rotation);
  servo_return();
  delay(100);
  
  // Photo command
  Serial.println("go");
}