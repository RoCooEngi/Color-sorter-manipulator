#include <Arduino.h>
#include <Servo.h>

Servo servo_push; //forward-back: 160 - 75
Servo servo_raise; //up - down: 150 - 60
Servo servo_rotate; //rotate left - right: 162 - 0
Servo servo_catch; //catch open - close: 70 - 170

String esp32value;
int rotation;

void slow_servo(Servo servo, int target, int speed = 15) {
  // Rotates the servo at a slow rate
  int current = servo.read();
  while (current != target) {
      if (current < target) current++;
      else current--;
      servo.write(current);
      delay(speed);
  }
}

void servo_return() {
  // Returns the servos to their original position
  servo_catch.write(70);
  slow_servo(servo_raise, 70);
  slow_servo(servo_push, 110);
  slow_servo(servo_rotate, 162);
}

void cube_sorting(int rotation) {
  // Rotates servo drives during sorting
  servo_catch.write(170);
  slow_servo(servo_raise, 110);
  slow_servo(servo_push, 70);
  slow_servo(servo_rotate, rotation);
  slow_servo(servo_push, 110);
  slow_servo(servo_raise, 70);
  servo_catch.write(70);
}

void setup() {
  Serial.begin(115200);
  // Starting position
  servo_catch.write(70);
  servo_raise.write(70);
  servo_push.write(110);
  servo_rotate.write(162);
  delay(100);

  // Fixing to PWM pins
  servo_push.attach(3);
  servo_raise.attach(7);
  servo_catch.attach(9);
  servo_rotate.attach(5);
  delay(100);
}

void loop() {
  if (Serial.available()) {
    // Getting data from ESP32 CAM
    esp32value = Serial.readStringUntil('\n');
    esp32value.trim();
    if (esp32value != "RED" && esp32value != "GREEN" && esp32value != "PURPLE"){
      return;
    }
  }
  else {
    Serial.println(F("UART disconnected"));
    return;
  }
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