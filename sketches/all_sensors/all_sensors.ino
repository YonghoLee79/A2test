// 통합 테스트: 모든 센서/부품을 동시에 연결해서 동작 확인
//   DHT      -> LCD 1번째 줄 (온도/습도)
//   HC-SR04  -> LCD 2번째 줄 + RGB LED 색 (가까움=빨강, 중간=초록, 멀리=파랑)
//   가변저항 -> 서보 각도 (USE_POT = false 면 거리에 따라 서보 각도)
//   CdS 조도 -> 도트 매트릭스 막대 그래프
// 핀 배치는 README.md 의 핀 맵 참고
//
// 서보가 움직일 때 전압이 순간적으로 떨어지면 LCD 글자가 깨지고 도트 매트릭스(MAX7219)가
// 꺼짐 모드로 리셋될 수 있어서, 두 부품을 주기적으로 다시 초기화함.
// 근본 해결: 서보에 외부 5V 전원(GND 공통) 또는 서보 전원에 470uF 이상 콘덴서 연결
#include <DHT.h>
#include <Servo.h>
#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHT_PIN   2
#define DHT_TYPE  DHT11  // DHT22 사용 시 변경
#define RED_PIN   3
#define GREEN_PIN 5
#define BLUE_PIN  6
#define TRIG_PIN  7
#define ECHO_PIN  8
#define SERVO_PIN 9
#define CS_PIN    10
#define DIN_PIN   11
#define CLK_PIN   13
#define LIGHT_PIN A0
#define POT_PIN   A1
#define LCD_ADDR  0x27

const bool COMMON_ANODE = false;
// 가변저항을 A1 에 연결했으면 true. A1 이 비어 있으면 값이 흔들려 서보가 계속 움직이므로 false
const bool USE_POT = false;
const byte MATRIX_INTENSITY = 2;  // 0~15, 낮을수록 전류 적게 사용
const int SERVO_DEADBAND = 3;     // 이 각도 미만의 변화는 무시 (서보 떨림 방지)
const unsigned long ECHO_TIMEOUT_US = 30000;

DHT dht(DHT_PIN, DHT_TYPE);
Servo servo;
LedControl matrix(DIN_PIN, CLK_PIN, CS_PIN, 1);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

float temperature = NAN;
float humidity = NAN;
float distanceCm = -1;
int light = 0;
int pot = 0;

unsigned long lastDht = 0;
unsigned long lastFast = 0;
unsigned long lastPrint = 0;
unsigned long lastReinit = 0;
int servoAngle = 90;

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

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1;
  float cm = duration * 0.0343 / 2.0;
  if (cm < 2 || cm > 400) return -1;  // HC-SR04 측정 범위 밖은 오류로 처리
  return cm;
}

float median3(float a, float b, float c) {
  if ((a <= b && b <= c) || (c <= b && b <= a)) return b;
  if ((b <= a && a <= c) || (c <= a && a <= b)) return a;
  return c;
}

// 최근 3번 측정값의 중간값: 한 번씩 튀는 값(0.05cm, 199cm 등)을 걸러냄
float filteredDistanceCm() {
  static float history[3] = {-1, -1, -1};
  static byte index = 0;
  history[index] = readDistanceCm();
  index = (index + 1) % 3;
  return median3(history[0], history[1], history[2]);
}

// 온도도 전원 잡음으로 한 번씩 튀므로(22.5 -> 26.1 등) 최근 3번의 중간값 사용
float filteredTemperature(float t) {
  static float history[3] = {NAN, NAN, NAN};
  static byte index = 0;
  if (isnan(history[0])) history[0] = history[1] = history[2] = t;  // 첫 측정값으로 채움
  history[index] = t;
  index = (index + 1) % 3;
  return median3(history[0], history[1], history[2]);
}

void updateRgb() {
  if (distanceCm < 0)        setColor(0, 0, 0);
  else if (distanceCm < 10)  setColor(255, 0, 0);
  else if (distanceCm < 30)  setColor(0, 255, 0);
  else                       setColor(0, 0, 255);
}

void updateMatrix() {
  // 밝을수록 아래에서부터 더 많은 줄이 켜짐 (0~8줄)
  initMatrix();
  byte rows = map(light, 0, 1023, 0, 8);
  for (byte row = 0; row < 8; row++) {
    matrix.setRow(0, 7 - row, row < rows ? B11111111 : B00000000);
  }
}

void updateLcd() {
  char line[17];
  char tBuf[6], hBuf[6], dBuf[6];

  if (isnan(temperature)) {
    snprintf(line, sizeof(line), "T: --  H: --    ");
  } else {
    dtostrf(temperature, 4, 1, tBuf);
    dtostrf(humidity, 4, 1, hBuf);
    snprintf(line, sizeof(line), "T:%sC H:%s%%  ", tBuf, hBuf);
  }
  lcd.setCursor(0, 0);
  lcd.print(line);

  if (distanceCm < 0) {
    snprintf(line, sizeof(line), "D: ---  L:%4d  ", light);
  } else {
    dtostrf(distanceCm, 5, 1, dBuf);
    snprintf(line, sizeof(line), "D:%scm L:%4d", dBuf, light);
  }
  lcd.setCursor(0, 1);
  lcd.print(line);
}

void printSerial() {
  Serial.print(F("T="));
  Serial.print(temperature);
  Serial.print(F("C H="));
  Serial.print(humidity);
  Serial.print(F("% D="));
  Serial.print(distanceCm);
  Serial.print(F("cm light="));
  Serial.print(light);
  Serial.print(F(" pot="));
  Serial.print(pot);
  Serial.print(F(" servo="));
  Serial.println(servoAngle);
}

// MAX7219 는 리셋되면 꺼짐 모드 + 1줄만 표시로 돌아가므로 매번 설정을 다시 보냄 (명령 몇 개라 부담 없음)
void initMatrix() {
  matrix.setScanLimit(0, 7);
  matrix.shutdown(0, false);
  matrix.setIntensity(0, MATRIX_INTENSITY);
}

void updateServo() {
  int target;
  if (USE_POT) {
    target = map(pot, 0, 1023, 0, 180);
  } else if (distanceCm >= 0) {
    // 5cm -> 0도, 50cm -> 180도
    target = map(constrain((int)distanceCm, 5, 50), 5, 50, 0, 180);
  } else {
    return;
  }
  if (abs(target - servoAngle) >= SERVO_DEADBAND) {
    servoAngle = target;
    servo.write(servoAngle);
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();
  servo.write(servoAngle);
  servo.attach(SERVO_PIN);

  initMatrix();
  matrix.clearDisplay(0);

  lcd.init();
  lcd.backlight();
  lcd.print("A2test all");
  delay(1000);
  lcd.clear();

  Serial.println(F("All sensors test"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastDht >= 2000) {
    lastDht = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      humidity = h;
      temperature = filteredTemperature(t);
    }
  }

  if (now - lastFast >= 100) {
    lastFast = now;
    distanceCm = filteredDistanceCm();
    light = analogRead(LIGHT_PIN);
    pot = analogRead(POT_PIN);

    updateServo();
    updateRgb();
    updateMatrix();
  }

  // 전압 강하로 리셋/통신 오류가 난 경우를 대비해 주기적으로 다시 초기화
  if (now - lastReinit >= 10000) {
    lastReinit = now;
    lcd.init();
    lcd.backlight();
  }

  if (now - lastPrint >= 500) {
    lastPrint = now;
    updateLcd();
    printSerial();
  }
}
