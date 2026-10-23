// 서보모터(SG90 등) 테스트
// 배선: 신호(주황/노랑) -> D9, VCC(빨강) -> 5V, GND(갈색) -> GND
// 서보가 떨리거나 보드가 리셋되면 외부 5V 전원 사용 (GND는 아두이노와 공통)
// 시리얼 모니터에 0~180 숫자를 입력하면 해당 각도로 이동
#include <Servo.h>

#define SERVO_PIN 9

Servo servo;

void sweep() {
  for (int a = 0; a <= 180; a += 2) { servo.write(a); delay(15); }
  for (int a = 180; a >= 0; a -= 2) { servo.write(a); delay(15); }
}

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  Serial.println(F("Servo test: sweeping once"));
  sweep();
  servo.write(90);
  Serial.println(F("Type an angle (0-180) and press Enter"));
}

void loop() {
  if (Serial.available()) {
    int angle = Serial.parseInt();
    while (Serial.available()) Serial.read();  // 남은 줄바꿈 제거
    angle = constrain(angle, 0, 180);
    servo.write(angle);
    Serial.print(F("Angle: "));
    Serial.println(angle);
  }
}
