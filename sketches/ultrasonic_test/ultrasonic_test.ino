// HC-SR04 초음파 거리 센서 테스트
// 배선: TRIG -> D7, ECHO -> D8, VCC -> 5V, GND -> GND
#define TRIG_PIN 7
#define ECHO_PIN 8

// 약 5m 이상이면 타임아웃(측정 실패) 처리
const unsigned long ECHO_TIMEOUT_US = 30000;

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;  // 음속 343 m/s, 왕복이므로 /2
}

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  Serial.println(F("HC-SR04 test"));
}

void loop() {
  float cm = readDistanceCm();
  if (cm < 0) {
    Serial.println(F("Out of range"));
  } else {
    Serial.print(F("Distance: "));
    Serial.print(cm, 1);
    Serial.println(F(" cm"));
  }
  delay(200);
}
