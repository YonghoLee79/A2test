// RGB LED 테스트
// 배선: R -> D3, G -> D5, B -> D6 (각각 220Ω 저항), 공통핀 -> GND(캐소드) 또는 5V(애노드)
// D9/D10은 Servo 라이브러리가 PWM을 막으므로 사용하지 않음
#define RED_PIN   3
#define GREEN_PIN 5
#define BLUE_PIN  6

// 공통 애노드(+) RGB LED면 true 로 변경
const bool COMMON_ANODE = false;

void setColor(byte r, byte g, byte b) {
  if (COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setup() {
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  Serial.println(F("RGB test: R, G, B, white, then fade"));
}

void loop() {
  setColor(255, 0, 0);   Serial.println(F("Red"));   delay(700);
  setColor(0, 255, 0);   Serial.println(F("Green")); delay(700);
  setColor(0, 0, 255);   Serial.println(F("Blue"));  delay(700);
  setColor(255, 255, 255); Serial.println(F("White")); delay(700);

  // 색상환 페이드
  Serial.println(F("Fade"));
  for (int i = 0; i < 768; i += 4) {
    byte phase = i / 256;
    byte v = i % 256;
    if (phase == 0)      setColor(255 - v, v, 0);
    else if (phase == 1) setColor(0, 255 - v, v);
    else                 setColor(v, 0, 255 - v);
    delay(10);
  }
}
