// 아날로그 센서 테스트 (조도 센서 CdS + 가변저항)
// 배선:
//   CdS: 5V - CdS - A0 - 10k - GND (분압회로, 모듈이면 AO -> A0)
//   가변저항: 양 끝 -> 5V / GND, 가운데(와이퍼) -> A1
// 시리얼 플로터(Serial Plotter)로 보면 그래프로 확인 가능
#define LIGHT_PIN A0
#define POT_PIN   A1

void setup() {
  Serial.begin(9600);
}

void loop() {
  int light = analogRead(LIGHT_PIN);
  int pot = analogRead(POT_PIN);

  // 시리얼 플로터 형식: label:value
  Serial.print(F("light:"));
  Serial.print(light);
  Serial.print(F(",pot:"));
  Serial.print(pot);
  Serial.print(F(",pot_V:"));
  Serial.println(pot * 5.0 / 1023.0);
  delay(100);
}
