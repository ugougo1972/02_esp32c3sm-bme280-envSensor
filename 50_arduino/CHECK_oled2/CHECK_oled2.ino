#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>
#include <U8g2lib.h>

// ========================================
// XIAO ESP32S3 Plus pin assignment
// ========================================
static const int PIN_I2C_SDA = 5;   // D4
static const int PIN_I2C_SCL = 6;   // D5

static const int PIN_ENC_A   = 2;   // D1
static const int PIN_ENC_B   = 3;   // D2
static const int PIN_ENC_SW  = 4;   // D3

// ========================================
// I2C address
// ========================================
static const uint8_t ADDR_BME280 = 0x76;

// ========================================
// Objects
// ========================================
Adafruit_BME280 bme;
Adafruit_LTR390 ltr;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// ========================================
// Device status
// ========================================
bool bmeOk  = false;
bool ltrOk  = false;
bool oledOk = false;

// ========================================
// Screen mode
// ========================================
enum ScreenMode {
  SCREEN_MAIN,
  SCREEN_MENU
};

ScreenMode screenMode = SCREEN_MAIN;

// ========================================
// Menu
// ========================================
const char* menuItems[] = {
  "VIEW",
  "CLOCK",
  "LOG",
  "SLEEP"
};
static const int MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);
int menuIndex = 0;

// ========================================
// Encoder state
// ========================================
int lastSw = HIGH;
unsigned long lastSwMs = 0;
const unsigned long SW_DEBOUNCE_MS = 100;

// encoder decode
uint8_t prevEncState = 0;
int8_t encAccum = 0;
unsigned long lastEncStepMs = 0;
const unsigned long ENC_STEP_GUARD_MS = 40;

// ========================================
// Timing control
// ========================================
unsigned long lastBmeMs = 0;
unsigned long lastDisplayMs = 0;

const unsigned long BME_INTERVAL_MS     = 500;
const unsigned long DISPLAY_INTERVAL_MS = 100;

// ========================================
// Sensor cache
// ========================================
float gTempC = 0.0f;
float gHumPct = 0.0f;
float gPressHpa = 0.0f;
uint32_t gAls = 0;
uint32_t gUvs = 0;

// ========================================
// LTR390 state machine
// ========================================
enum LtrState {
  LTR_IDLE,
  LTR_WAIT_ALS,
  LTR_WAIT_UVS
};

LtrState ltrState = LTR_IDLE;
unsigned long ltrStateMs = 0;
const unsigned long LTR_WAIT_MS = 120;

// ========================================
// Utility: I2C scan
// ========================================
void scanI2C() {
  Serial.println("---- I2C scan start ----");

  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("Found I2C device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }

  Serial.print("Total devices found: ");
  Serial.println(found);
  Serial.println("---- I2C scan end ----");
}

// ========================================
// Utility: time text (temporary elapsed time)
// ========================================
void makeTimeText(char* buf, size_t len) {
  unsigned long totalSec = millis() / 1000;
  unsigned int mm = (totalSec / 60) % 100;
  unsigned int ss = totalSec % 60;
  snprintf(buf, len, "%02u:%02u", mm, ss);
}

// ========================================
// Utility: battery icon (temporary fixed level)
// ========================================
void drawBatteryIcon(int x, int y, int level) {
  // level: 0 to 4
  u8g2.drawFrame(x, y, 18, 8);
  u8g2.drawBox(x + 18, y + 2, 2, 4);

  if (level >= 1) u8g2.drawBox(x + 2,  y + 2, 3, 4);
  if (level >= 2) u8g2.drawBox(x + 6,  y + 2, 3, 4);
  if (level >= 3) u8g2.drawBox(x + 10, y + 2, 3, 4);
  if (level >= 4) u8g2.drawBox(x + 14, y + 2, 2, 4);
}

// ========================================
// Draw: boot screen
// ========================================
void drawBootScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0, 12, "XIAO ESP32S3 sensor test");
  u8g2.drawStr(0, 28, bmeOk  ? "BME280 : OK" : "BME280 : NG");
  u8g2.drawStr(0, 42, ltrOk  ? "LTR390 : OK" : "LTR390 : NG");
  u8g2.drawStr(0, 56, oledOk ? "OLED   : OK" : "OLED   : NG");

  u8g2.sendBuffer();
}

// ========================================
// Draw: error screen
// ========================================
void drawErrorScreen(const char* msg1, const char* msg2) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0, 14, "INIT ERROR");
  u8g2.drawStr(0, 32, msg1);
  u8g2.drawStr(0, 48, msg2);

  u8g2.sendBuffer();
}

// ========================================
// Draw: main screen
// ========================================
void drawMainScreen(float tempC, float humPct, float pressHpa, uint32_t als, uint32_t uvs) {
  char line1[32];
  char line2[32];
  char line3[32];
  char timeText[16];

  // センサー測定数値は同一フォント・同一サイズ
  snprintf(line1, sizeof(line1), "T:%5.1fC   H:%4.1f%%", tempC, humPct);
  snprintf(line2, sizeof(line2), "P:%7.1fhPa", pressHpa);
  snprintf(line3, sizeof(line3), "A:%6lu   U:%6lu", (unsigned long)als, (unsigned long)uvs);

  makeTimeText(timeText, sizeof(timeText));

  u8g2.clearBuffer();

  // Header
  u8g2.setFont(u8g2_font_6x12_tf);
  u8g2.drawFrame(0, 0, 128, 12);
  u8g2.drawStr(2, 10, timeText);
  drawBatteryIcon(106, 2, 4);  // temporary fixed full level

  // Sensor values
  u8g2.drawStr(0, 24, line1);
  u8g2.drawStr(0, 38, line2);
  u8g2.drawStr(0, 52, line3);

  // Footer hint
  u8g2.drawBox(0, 54, 128, 10);
  u8g2.setDrawColor(0);
  u8g2.drawStr(36, 62, "PUSH: MENU");
  u8g2.setDrawColor(1);

  u8g2.sendBuffer();
}

// ========================================
// Draw: menu screen
// ========================================
void drawMenuScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tf);

  // Header
  u8g2.drawFrame(0, 0, 128, 12);
  u8g2.drawStr(4, 10, "MENU");

  // Menu items
  for (int i = 0; i < MENU_COUNT; i++) {
    int y = 24 + i * 10;

    if (i == menuIndex) {
      u8g2.drawBox(0, y - 9, 128, 10);
      u8g2.setDrawColor(0);
      u8g2.drawStr(6, y, menuItems[i]);
      u8g2.setDrawColor(1);
    } else {
      u8g2.drawStr(6, y, menuItems[i]);
    }
  }

  // Footer
  u8g2.drawFrame(0, 54, 128, 10);
  u8g2.drawStr(8, 62, "ROT:MOVE PUSH:BACK");

  u8g2.sendBuffer();
}

// ========================================
// Encoder update
// ========================================
void updateEncoder() {
  // -----------------------------
  // Rotary encoder robust decode
  // -----------------------------
  uint8_t a = digitalRead(PIN_ENC_A);
  uint8_t b = digitalRead(PIN_ENC_B);
  uint8_t currState = (a << 1) | b;

  if (currState != prevEncState) {
    uint8_t transition = (prevEncState << 2) | currState;

    // Gray-code transition table
    // CW candidates
    if (transition == 0b0001 ||
        transition == 0b0111 ||
        transition == 0b1110 ||
        transition == 0b1000) {
      encAccum++;
    }
    // CCW candidates
    else if (transition == 0b0010 ||
             transition == 0b1011 ||
             transition == 0b1101 ||
             transition == 0b0100) {
      encAccum--;
    }

    prevEncState = currState;

    // 4 transitions = 1 detent
    if (millis() - lastEncStepMs >= ENC_STEP_GUARD_MS) {
      if (encAccum >= 2) {
        menuIndex++;
        if (menuIndex >= MENU_COUNT) menuIndex = 0;
        encAccum = 0;
        lastEncStepMs = millis();
        Serial.println("ENC: CW");
      } else if (encAccum <= -2) {
        menuIndex--;
        if (menuIndex < 0) menuIndex = MENU_COUNT - 1;
        encAccum = 0;
        lastEncStepMs = millis();
        Serial.println("ENC: CCW");
      }
    }
  }

  // -----------------------------
  // Push button
  // -----------------------------
  int sw = digitalRead(PIN_ENC_SW);
  if (lastSw == HIGH && sw == LOW) {
    if (millis() - lastSwMs > SW_DEBOUNCE_MS) {
      if (screenMode == SCREEN_MAIN) {
        screenMode = SCREEN_MENU;
        Serial.println("SCREEN: MENU");
      } else {
        screenMode = SCREEN_MAIN;
        Serial.println("SCREEN: MAIN");
      }
      lastSwMs = millis();
    }
  }
  lastSw = sw;
}
// ========================================
// Periodic BME280 update
// ========================================
void updateBME280Periodic() {
  if (millis() - lastBmeMs >= BME_INTERVAL_MS) {
    lastBmeMs = millis();

    gTempC = bme.readTemperature();
    gHumPct = bme.readHumidity();
    gPressHpa = bme.readPressure() / 100.0f;

    Serial.print("Temp = ");
    Serial.print(gTempC, 1);
    Serial.print(" C, Hum = ");
    Serial.print(gHumPct, 1);
    Serial.print(" %, Press = ");
    Serial.print(gPressHpa, 1);
    Serial.print(" hPa, ALS = ");
    Serial.print(gAls);
    Serial.print(", UVS = ");
    Serial.println(gUvs);
  }
}

// ========================================
// Non-blocking LTR390 update
// ========================================
void updateLTR390NonBlocking() {
  switch (ltrState) {
    case LTR_IDLE:
      ltr.setMode(LTR390_MODE_ALS);
      ltrStateMs = millis();
      ltrState = LTR_WAIT_ALS;
      break;

    case LTR_WAIT_ALS:
      if (millis() - ltrStateMs >= LTR_WAIT_MS) {
        gAls = ltr.readALS();
        ltr.setMode(LTR390_MODE_UVS);
        ltrStateMs = millis();
        ltrState = LTR_WAIT_UVS;
      }
      break;

    case LTR_WAIT_UVS:
      if (millis() - ltrStateMs >= LTR_WAIT_MS) {
        gUvs = ltr.readUVS();
        ltrState = LTR_IDLE;
      }
      break;
  }
}

// ========================================
// Setup
// ========================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("XIAO ESP32S3 integrated sensor + menu test start");

  // Encoder pins
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);

  lastSw = digitalRead(PIN_ENC_SW);
  prevEncState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);

  // I2C begin
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 400000);
  delay(100);

  scanI2C();

  // OLED init
  u8g2.begin();
  oledOk = true;

  // BME280 init
  bmeOk = bme.begin(ADDR_BME280, &Wire);
  Serial.print("BME280 init: ");
  Serial.println(bmeOk ? "OK" : "NG");

  // LTR390 init
  // Adafruit LTR390 Library 1.1.2 compatible call
  ltrOk = ltr.begin(&Wire);
  Serial.print("LTR390 init: ");
  Serial.println(ltrOk ? "OK" : "NG");

  if (ltrOk) {
    ltr.setGain(LTR390_GAIN_3);
    ltr.setResolution(LTR390_RESOLUTION_18BIT);
    ltrState = LTR_IDLE;
  }

  drawBootScreen();
  delay(1500);

  if (!bmeOk || !ltrOk || !oledOk) {
    drawErrorScreen(
      bmeOk ? "BME280: OK" : "BME280: NG",
      ltrOk ? "LTR390: OK" : "LTR390: NG"
    );
  }
}

// ========================================
// Loop
// ========================================
void loop() {
  updateEncoder();

  if (!bmeOk || !ltrOk || !oledOk) {
    delay(5);
    return;
  }

  // sensor update continues even in menu screen
  updateBME280Periodic();
  updateLTR390NonBlocking();

  // display update
  if (millis() - lastDisplayMs >= DISPLAY_INTERVAL_MS) {
    lastDisplayMs = millis();

    if (screenMode == SCREEN_MAIN) {
      drawMainScreen(gTempC, gHumPct, gPressHpa, gAls, gUvs);
    } else {
      drawMenuScreen();
    }
  }
}