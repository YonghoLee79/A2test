// 아날로그 센서 테스트 (조도 센서 CdS + 가변저항)
// 배선:
//   조도 모듈(HL-01 등 LM393 4핀): VCC -> 5V, GND -> GND, AO -> A0, DO -> D4(선택)
//   CdS 단품: 5V - CdS - A0 - 10k - GND (분압회로)
//   가변저항: 양 끝 -> 5V / GND, 가운데(와이퍼) -> A1
// 시리얼 플로터(Serial Plotter)로 보면 그래프로 확인 가능
#define LIGHT_PIN    A0
#define POT_PIN      A1
#define LIGHT_DO_PIN 4  // 모듈의 DO: 파란 가변저항으로 정한 기준보다 밝은지/어두운지 (0/1)

void setup() {
  Serial.begin(9600);
  pinMode(LIGHT_DO_PIN, INPUT_PULLUP);  // DO 미연결 시 1로 고정
}

void loop() {
  int light = analogRead(LIGHT_PIN);
  int pot = analogRead(POT_PIN);
  int lightDo = digitalRead(LIGHT_DO_PIN);

  // 시리얼 플로터 형식: label:value
  Serial.print(F("light:"));
  Serial.print(light);
  Serial.print(F(",light_DO:"));
  Serial.print(lightDo);
  Serial.print(F(",pot:"));
  Serial.print(pot);
  Serial.print(F(",pot_V:"));
  Serial.println(pot * 5.0 / 1023.0);
  delay(100);
}
