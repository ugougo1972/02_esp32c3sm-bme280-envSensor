#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============================================================
// env_logger_minimal_oled.ino
// 最小OLED版
// - XIAO ESP32S3 Plus 前提
// - DS3231 / BME280 / LTR390 / microSD / SSD1306 を実接続
// - setup() 完結型で 1回記録して Deep Sleep へ移行
// - OLED には LOG 成功/失敗と主要値だけを短時間表示
// - Encoder / context_code入力 / head_code入力 / vbat は未実装
// ============================================================

// =========================
// Pin assignment
// =========================
static const int PIN_I2C_SDA  = 5;  // D4
static const int PIN_I2C_SCL  = 6;  // D5

static const int PIN_SD_CS    = 3;  // D2
static const int PIN_SD_SCK   = 7;  // D8
static const int PIN_SD_MISO  = 8;  // D9
static const int PIN_SD_MOSI  = 9;  // D10

// =========================
// Device addresses / files
// =========================
static const uint8_t BME280_ADDR = 0x76;
static const uint8_t OLED_ADDR   = 0x3C;

static const char *LOG_PATH = "/log_sleep_env.csv";
static const char *CSV_HEADER =
  "date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat";

// =========================
// Sleep / display timing
// =========================
static const uint64_t SLEEP_INTERVAL_US = 60ULL * 1000ULL * 1000ULL;  // 60 sec
static const uint32_t OLED_SHOW_MS = 5000;

// =========================
// OLED spec
// =========================
static const int SCREEN_WIDTH  = 128;
static const int SCREEN_HEIGHT = 64;
static const int OLED_RESET    = -1;

// =========================
// Codes
// =========================
static const uint8_t CTX_UNKNOWN  = 0;
static const uint8_t HEAD_UNKNOWN = 0;

// =========================
// RTC persistent data
// =========================
RTC_DATA_ATTR uint32_t bootCount = 0;

// =========================
// Libraries
// =========================
RTC_DS3231 rtc;
Adafruit_BME280 bme;
Adafruit_LTR390 ltr;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// =========================
// Data structure
// =========================
struct LogRecord {
  char date[11];   // YYYY-MM-DD
  char time[9];    // HH:MM:SS
  float temp_c;
  float hum_pct;
  float press_hpa;
  uint32_t als;
  uint32_t uvs;
  uint8_t context_code;
  uint8_t head_code;
  float vbat;
};

// =========================
// Forward declarations
// =========================
void printWakeupReason();
void initPins();
bool initWireBus();
bool initRtc();
bool initBme280();
bool initLtr390();
bool initDisplay();
bool initSd();

bool isRtcTimeValid(const DateTime &dt);
bool recoverRtcIfNeeded();

bool readRtcToRecord(LogRecord &rec);
bool readBme280ToRecord(LogRecord &rec);
bool readLtr390ToRecord(LogRecord &rec);
bool readBatteryToRecord(LogRecord &rec);

String buildCsvLine(const LogRecord &rec);
bool ensureLogFileHeader(const char *path, const char *header);
bool appendLine(const char *path, const String &line);

void showBootScreen();
void showResultScreen(const LogRecord &rec, bool saveOk);
void safeDisplayMessage(const char *line1, const char *line2 = nullptr, const char *line3 = nullptr);

void prepareForSleep();
void enterDeepSleep();

// =========================
// setup
// =========================
void setup() {
  Serial.begin(115200);
  delay(200);

  ++bootCount;

  Serial.println();
  Serial.println("env_logger_minimal_oled start");
  Serial.print("bootCount = ");
  Serial.println(bootCount);

  printWakeupReason();
  initPins();

  if (!initWireBus()) {
    Serial.println("initWireBus failed");
    prepareForSleep();
    enterDeepSleep();
  }

  const bool displayOk = initDisplay();
  if (displayOk) {
    showBootScreen();
  }

  if (!initRtc()) {
    Serial.println("initRtc failed");
    safeDisplayMessage("RTC init NG", "sleep soon");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!recoverRtcIfNeeded()) {
    Serial.println("recoverRtcIfNeeded failed");
    safeDisplayMessage("RTC recover NG", "sleep soon");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initBme280()) {
    Serial.println("initBme280 failed");
    safeDisplayMessage("BME280 init NG", "sleep soon");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initLtr390()) {
    Serial.println("initLtr390 failed");
    safeDisplayMessage("LTR390 init NG", "sleep soon");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initSd()) {
    Serial.println("initSd failed");
    safeDisplayMessage("SD init NG", "sleep soon");
    prepareForSleep();
    enterDeepSleep();
  }

  LogRecord rec = {};
  if (!readRtcToRecord(rec)) {
    Serial.println("readRtcToRecord failed");
  }

  if (!readBme280ToRecord(rec)) {
    Serial.println("readBme280ToRecord failed");
  }

  if (!readLtr390ToRecord(rec)) {
    Serial.println("readLtr390ToRecord failed");
  }

  if (!readBatteryToRecord(rec)) {
    Serial.println("readBatteryToRecord failed");
  }

  rec.context_code = CTX_UNKNOWN;
  rec.head_code = HEAD_UNKNOWN;

  if (!ensureLogFileHeader(LOG_PATH, CSV_HEADER)) {
    Serial.println("ensureLogFileHeader failed");
  }

  const String line = buildCsvLine(rec);
  Serial.print("csv = ");
  Serial.println(line);

  const bool saveOk = appendLine(LOG_PATH, line);
  if (saveOk) {
    Serial.println("appendLine OK");
  } else {
    Serial.println("appendLine failed");
  }

  showResultScreen(rec, saveOk);
  prepareForSleep();
  enterDeepSleep();
}

void loop() {
  // Deep Sleep logger 基準版では未使用
}

// =========================
// Init
// =========================
void initPins() {
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
}

bool initWireBus() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  delay(10);
  Serial.println("Wire.begin OK");
  return true;
}

bool initRtc() {
  if (!rtc.begin()) {
    return false;
  }
  Serial.println("rtc.begin OK");
  return true;
}

bool initBme280() {
  if (!bme.begin(BME280_ADDR, &Wire)) {
    return false;
  }
  Serial.println("bme.begin OK");
  return true;
}

bool initLtr390() {
  if (!ltr.begin()) {
    return false;
  }

  ltr.setMode(LTR390_MODE_ALS);
  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_18BIT);
  Serial.println("ltr.begin OK");
  return true;
}

bool initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    return false;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  Serial.println("display.begin OK");
  return true;
}

bool initSd() {
  digitalWrite(PIN_SD_CS, HIGH);
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, SPI, 1000000)) {
    return false;
  }
  Serial.println("SD.begin OK");
  return true;
}

// =========================
// RTC validation / recovery
// =========================
bool isRtcTimeValid(const DateTime &dt) {
  if (dt.year() < 2024) return false;
  if (dt.year() > 2099) return false;
  if (dt.month() < 1 || dt.month() > 12) return false;
  if (dt.day() < 1 || dt.day() > 31) return false;
  if (dt.hour() > 23) return false;
  if (dt.minute() > 59) return false;
  if (dt.second() > 59) return false;
  return true;
}

bool recoverRtcIfNeeded() {
  bool needsRecover = false;

  if (rtc.lostPower()) {
    Serial.println("rtc lostPower = true");
    needsRecover = true;
  } else {
    Serial.println("rtc lostPower = false");
  }

  DateTime now = rtc.now();
  Serial.print("rtc raw time = ");
  Serial.print(now.year()); Serial.print("-");
  if (now.month() < 10) Serial.print("0");
  Serial.print(now.month()); Serial.print("-");
  if (now.day() < 10) Serial.print("0");
  Serial.print(now.day()); Serial.print(" ");
  if (now.hour() < 10) Serial.print("0");
  Serial.print(now.hour()); Serial.print(":");
  if (now.minute() < 10) Serial.print("0");
  Serial.print(now.minute()); Serial.print(":");
  if (now.second() < 10) Serial.print("0");
  Serial.println(now.second());

  if (!isRtcTimeValid(now)) {
    Serial.println("rtc time invalid");
    needsRecover = true;
  } else {
    Serial.println("rtc time valid");
  }

  if (!needsRecover) {
    return true;
  }

  DateTime compiled(F(__DATE__), F(__TIME__));
  rtc.adjust(compiled);
  Serial.println("rtc adjusted by compile time");

  DateTime check = rtc.now();
  return isRtcTimeValid(check);
}

// =========================
// Read functions
// =========================
bool readRtcToRecord(LogRecord &rec) {
  DateTime now = rtc.now();

  snprintf(
    rec.date, sizeof(rec.date),
    "%04d-%02d-%02d",
    now.year(), now.month(), now.day()
  );

  snprintf(
    rec.time, sizeof(rec.time),
    "%02d:%02d:%02d",
    now.hour(), now.minute(), now.second()
  );

  return true;
}

bool readBme280ToRecord(LogRecord &rec) {
  rec.temp_c = bme.readTemperature();
  rec.hum_pct = bme.readHumidity();
  rec.press_hpa = bme.readPressure() / 100.0f;
  return true;
}

bool readLtr390ToRecord(LogRecord &rec) {
  ltr.setMode(LTR390_MODE_ALS);
  delay(120);
  rec.als = ltr.readALS();

  ltr.setMode(LTR390_MODE_UVS);
  delay(120);
  rec.uvs = ltr.readUVS();

  return true;
}

bool readBatteryToRecord(LogRecord &rec) {
  rec.vbat = 0.0f;  // 未実装
  return true;
}

// =========================
// CSV / SD
// =========================
String buildCsvLine(const LogRecord &rec) {
  String line;
  line.reserve(96);

  line += rec.date;
  line += ",";
  line += rec.time;
  line += ",";
  line += String(rec.temp_c, 2);
  line += ",";
  line += String(rec.hum_pct, 2);
  line += ",";
  line += String(rec.press_hpa, 2);
  line += ",";
  line += String(rec.als);
  line += ",";
  line += String(rec.uvs);
  line += ",";
  line += String(rec.context_code);
  line += ",";
  line += String(rec.head_code);
  line += ",";
  line += String(rec.vbat, 3);

  return line;
}

bool ensureLogFileHeader(const char *path, const char *header) {
  if (SD.exists(path)) {
    Serial.println("log file already exists");
    return true;
  }

  File f = SD.open(path, FILE_WRITE);
  if (!f) {
    return false;
  }

  f.println(header);
  f.close();

  Serial.println("header created");
  return true;
}

bool appendLine(const char *path, const String &line) {
  File f = SD.open(path, FILE_APPEND);
  if (!f) {
    return false;
  }

  f.println(line);
  f.close();
  return true;
}

// =========================
// OLED
// =========================
void safeDisplayMessage(const char *line1, const char *line2, const char *line3) {
  if (!display.width()) return;

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  if (line1) display.println(line1);
  if (line2) display.println(line2);
  if (line3) display.println(line3);
  display.display();
}

void showBootScreen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("env logger");
  display.println("boot...");
  display.print("bootCount=");
  display.println(bootCount);
  display.display();
  delay(400);
}

void showResultScreen(const LogRecord &rec, bool saveOk) {
  if (!display.width()) return;

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println(saveOk ? "LOG: OK" : "LOG: NG");

  display.setCursor(0, 10);
  display.print(rec.time);

  display.setCursor(0, 22);
  display.print("T:");
  display.print(rec.temp_c, 1);
  display.print("C H:");
  display.print(rec.hum_pct, 0);
  display.print("%");

  display.setCursor(0, 34);
  display.print("P:");
  display.print(rec.press_hpa, 1);
  display.print("hPa");

  display.setCursor(0, 46);
  display.print("A:");
  display.print(rec.als);
  display.print(" U:");
  display.print(rec.uvs);

  display.display();
  delay(OLED_SHOW_MS);

  display.clearDisplay();
  display.display();
}

// =========================
// Sleep
// =========================
void prepareForSleep() {
  if (display.width()) {
    display.clearDisplay();
    display.display();
  }
  Serial.println("prepareForSleep");
  esp_sleep_enable_timer_wakeup(SLEEP_INTERVAL_US);
}

void enterDeepSleep() {
  Serial.println("enterDeepSleep");
  Serial.flush();
  esp_deep_sleep_start();
}

// =========================
// Wake reason
// =========================
void printWakeupReason() {
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  Serial.print("wakeupCause = ");
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("TIMER");
      break;
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("EXT0");
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("EXT1");
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("TOUCHPAD");
      break;
    case ESP_SLEEP_WAKEUP_ULP:
      Serial.println("ULP");
      break;
    default:
      Serial.println("OTHER");
      break;
  }
}
