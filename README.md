# A2test

These are sensor test sketches for the second Arduino board (**Arduino Uno R3**). You build and upload them with **arduino-cli** from VS Code.

## Pin map

All sketches use the same pins, so you can leave every part wired at once.

| Part | Arduino pin | Notes |
|---|---|---|
| DHT11 / DHT22 | D2 | Change `DHT_TYPE` to `DHT22` in the code if you use a DHT22 |
| RGB LED R / G / B | D3 / D5 / D6 | 220Ω resistor on each leg. For common anode, set `COMMON_ANODE = true` |
| HC-SR04 TRIG / ECHO | D7 / D8 | |
| Servo signal | D9 | Use external 5V power if it jitters (share GND) |
| Dot matrix (MAX7219) CS / DIN / CLK | D10 / D11 / D13 | |
| LCD I2C SDA / SCL | A4 / A5 | Default address `0x27` (some modules use `0x3F`) |
| Light sensor (CdS / HL-01 module) | A0 (AO), D4 (DO, optional) | Bare CdS needs a 10k voltage divider |
| Potentiometer | A1 | Wiper (middle pin) |

> The Servo library blocks PWM on D9 and D10, so the RGB LED uses D3/D5/D6.

## Sketches

| Folder | What it does |
|---|---|
| `sketches/dht_test` | Prints temperature, humidity, and heat index |
| `sketches/ultrasonic_test` | Prints distance (cm) |
| `sketches/analog_test` | CdS and potentiometer values (best viewed in the serial plotter) |
| `sketches/rgb_test` | PWM color/brightness control from the serial monitor (`255 100 0`, `r`/`g`/`b`/`w`/`off`, `bri 128`, `fire` flame effect, `speed 50`, `fade`, `demo`) |
| `sketches/servo_test` | Sweeps once, then moves to an angle (0–180) you type in the serial monitor |
| `sketches/dotmatrix_test` | Lights every dot one by one, then shows a smiley and a heart |
| `sketches/lcd_i2c_test` | Shows text and uptime |
| `sketches/i2c_scanner` | Finds the LCD's I2C address |
| `sketches/all_sensors` | Integrated test with everything connected (temp/humidity/distance on the LCD, RGB color by distance, servo follows the potentiometer, dot matrix shows light level) |

## Setup (once)

1. Install [arduino-cli](https://arduino.github.io/arduino-cli/latest/installation/) and add it to your PATH
2. Open this folder in VS Code and install the recommended extensions
3. `Ctrl+Shift+P` → **Tasks: Run Task** → **Arduino: 환경 설치 (코어 + 라이브러리)** (installs the core and libraries)

Or run it directly in a terminal:

```bash
arduino-cli core update-index
arduino-cli core install arduino:avr
arduino-cli lib install "DHT sensor library" "Adafruit Unified Sensor" "Servo" "LedControl" "LiquidCrystal I2C"
```

## How to test

1. Connect the board over USB → **Arduino: 포트 목록** (port list) task to find the port
2. **Arduino: 컴파일 + 업로드** (compile + upload) → pick a sketch → enter the port
3. **Arduino: 시리얼 모니터** (serial monitor) → check the output (9600 baud)

From a terminal:

```bash
arduino-cli compile --fqbn arduino:avr:uno --upload --port COM3 sketches/dht_test
arduino-cli monitor --port COM3 --config baudrate=9600
```

> Close the serial monitor before uploading (the port must be free).

## Troubleshooting

- **LCD is blank**: find the address with `i2c_scanner` and change `LCD_ADDR`. Adjust contrast with the trimmer on the back.
- **DHT reads NaN**: check the type (DHT11/22) and wiring. Bare sensors without a module need a 10k pull-up.
- **Ultrasonic says Out of range**: check the TRIG/ECHO wiring. The range is about 2cm–4m.
- **Board resets when the servo moves**: not enough power. Use an external 5V supply.
