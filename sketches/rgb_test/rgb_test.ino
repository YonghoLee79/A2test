// RGB LED PWM 출력 조절 테스트
// 배선: R -> D3, G -> D5, B -> D6 (각각 220Ω 저항), 공통핀 -> GND(캐소드) 또는 5V(애노드)
//       (선택) 가변저항 가운데 -> A1 로 전체 밝기 조절: USE_POT = true 로 변경
// D9/D10은 Servo 라이브러리가 PWM을 막으므로 사용하지 않음
//
// 시리얼 모니터 명령 (9600 baud, 줄바꿈 포함):
//   255 100 0   -> R G B 값(0~255) 직접 지정
//   r / g / b / w / off -> 빨강 / 초록 / 파랑 / 흰색 / 끄기
//   bri 128     -> 전체 밝기(0~255) 지정
//   fade        -> 색상환 페이드 (아무 키나 입력하면 정지)
//   demo        -> R, G, B, 흰색 순서로 표시
#define RED_PIN   3
#define GREEN_PIN 5
#define BLUE_PIN  6
#define POT_PIN   A1

// 공통 애노드(+) RGB LED면 true 로 변경
const bool COMMON_ANODE = false;
// 가변저항으로 전체 밝기를 조절할지 여부 (A1 이 비어 있으면 값이 흔들리므로 false)
const bool USE_POT = false;

byte red = 0, green = 0, blue = 0;
byte brightness = 255;
bool fading = false;
int fadeStep = 0;

void writePwm(byte r, byte g, byte b) {
  r = (unsigned int)r * brightness / 255;
  g = (unsigned int)g * brightness / 255;
  b = (unsigned int)b * brightness / 255;
  if (COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

void setColor(byte r, byte g, byte b) {
  red = r;
  green = g;
  blue = b;
  writePwm(red, green, blue);
}

void printColor() {
  Serial.print(F("RGB = "));
  Serial.print(red);
  Serial.print(' ');
  Serial.print(green);
  Serial.print(' ');
  Serial.print(blue);
  Serial.print(F("  brightness = "));
  Serial.println(brightness);
}

void demo() {
  setColor(255, 0, 0);     Serial.println(F("Red"));   delay(700);
  setColor(0, 255, 0);     Serial.println(F("Green")); delay(700);
  setColor(0, 0, 255);     Serial.println(F("Blue"));  delay(700);
  setColor(255, 255, 255); Serial.println(F("White")); delay(700);
  setColor(0, 0, 0);
}

void handleCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();
  if (cmd.length() == 0) return;
  fading = false;

  if (cmd == "r")        setColor(255, 0, 0);
  else if (cmd == "g")   setColor(0, 255, 0);
  else if (cmd == "b")   setColor(0, 0, 255);
  else if (cmd == "w")   setColor(255, 255, 255);
  else if (cmd == "off") setColor(0, 0, 0);
  else if (cmd == "demo") demo();
  else if (cmd.startsWith("bri ")) {
    brightness = constrain(cmd.substring(4).toInt(), 0, 255);
    writePwm(red, green, blue);
  }
  else if (cmd == "fade") {
    fading = true;
    Serial.println(F("Fading... (type anything to stop)"));
    return;
  } else {
    int r, g, b;
    if (sscanf(cmd.c_str(), "%d %d %d", &r, &g, &b) == 3) {
      setColor(constrain(r, 0, 255), constrain(g, 0, 255), constrain(b, 0, 255));
    } else {
      Serial.println(F("Unknown command. Use: R G B (0-255), r, g, b, w, off, bri N, fade, demo"));
      return;
    }
  }
  printColor();
}

void updateFade() {
  static unsigned long last = 0;
  if (millis() - last < 10) return;
  last = millis();

  byte phase = fadeStep / 256;
  byte v = fadeStep % 256;
  if (phase == 0)      setColor(255 - v, v, 0);
  else if (phase == 1) setColor(0, 255 - v, v);
  else                 setColor(v, 0, 255 - v);
  fadeStep = (fadeStep + 4) % 768;
}

void updateBrightness() {
  static unsigned long last = 0;
  if (!USE_POT || millis() - last < 50) return;
  last = millis();

  byte newBrightness = analogRead(POT_PIN) / 4;  // 0~1023 -> 0~255
  // 노이즈로 값이 계속 바뀌는 것 방지
  if (abs((int)newBrightness - (int)brightness) > 3) {
    brightness = newBrightness;
    writePwm(red, green, blue);
    printColor();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  if (USE_POT) brightness = analogRead(POT_PIN) / 4;

  Serial.println(F("RGB PWM test"));
  demo();
  Serial.println(F("Commands: R G B (0-255), r, g, b, w, off, bri N, fade, demo"));
  if (USE_POT) Serial.println(F("Turn the potentiometer (A1) to change brightness"));
}

void loop() {
  if (Serial.available()) {
    handleCommand(Serial.readStringUntil('\n'));
  }
  if (fading) updateFade();
  updateBrightness();
}
