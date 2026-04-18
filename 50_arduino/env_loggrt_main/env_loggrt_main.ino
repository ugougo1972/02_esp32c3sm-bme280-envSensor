#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// === Optional libraries (install before full implementation) ===
// #include <Adafruit_BME280.h>
// #include <Adafruit_LTR390.h>
// #include <RTClib.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// ============================================================
// env_logger_main.ino
// 初号機用骨格コード
// - Deep Sleep logger 基準版を主基準とする
// - setup() 完結型で 1サイクル実行して Deep Sleep へ移行
// - UI試験版 / periodic logger 版へ拡張しやすい骨格にする
// ============================================================

// =========================
// Pin assignment
// =========================
static const int PIN_I2C_SDA   = 5;   // D4
static const int PIN_I2C_SCL   = 6;   // D5

static const int PIN_SD_CS     = 3;   // D2
static const int PIN_SD_SCK    = 7;   // D8
static const int PIN_SD_MISO   = 8;   // D9
static const int PIN_SD_MOSI   = 9;   // D10

static const int PIN_ENC_A     = 1;   // D0
static const int PIN_ENC_B     = 2;   // D1
static const int PIN_ENC_SW    = 4;   // D3

static const int PIN_VBAT_ADC  = 10;  // D16 (future)

// =========================
// File paths / CSV format
// =========================
static const char *LOG_PATH = "/log_sleep_env.csv";
static const char *CSV_HEADER =
  "date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat";

// =========================
// Timing
// =========================
static const uint64_t SLEEP_INTERVAL_US = 60ULL * 1000ULL * 1000ULL; // 60 sec
static const uint32_t OLED_SHOW_MS = 5000;

// =========================
// Context / head codes
// =========================
static const uint8_t CTX_UNKNOWN = 0;
static const uint8_t CTX_INDOOR  = 1;
static const uint8_t CTX_OUTDOOR = 2;
static const uint8_t CTX_MOVING  = 3;
static const uint8_t CTX_OFFICE  = 4;
static const uint8_t CTX_HOME    = 5;

static const uint8_t HEAD_UNKNOWN   = 0;
static const uint8_t HEAD_NONE      = 1;
static const uint8_t HEAD_MILD      = 2;
static const uint8_t HEAD_MODERATE  = 3;
static const uint8_t HEAD_SEVERE    = 4;
static const uint8_t HEAD_RECOVERED = 5;

// =========================
// UI state (future use)
// =========================
enum UiState {
  UI_VIEW,
  UI_MENU,
  UI_CLOCK,
  UI_LOG,
  UI_SLEEP
};

enum MenuItem {
  MENU_VIEW,
  MENU_CLOCK,
  MENU_LOG,
  MENU_SLEEP
};

// =========================
// Log record
// =========================
struct LogRecord {
  char date[11];         // YYYY-MM-DD
  char time[9];          // HH:MM:SS
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
// Globals
// =========================
RTC_DATA_ATTR uint32_t bootCount = 0;

UiState uiState = UI_VIEW;
MenuItem menuItem = MENU_VIEW;

LogRecord currentRecord = {};

uint8_t currentContextCode = CTX_UNKNOWN;
uint8_t currentHeadCode = HEAD_UNKNOWN;

// Replace with actual library instances later.
// Adafruit_BME280 bme;
// Adafruit_LTR390 ltr;
// RTC_DS3231 rtc;
// Adafruit_SSD1306 display(128, 64, &Wire, -1);

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

bool isRtcTimeValid();
bool recoverRtcIfNeeded();

bool readRtcToRecord(LogRecord &rec);
bool readBme280ToRecord(LogRecord &rec);
bool readLtr390ToRecord(LogRecord &rec);
bool readBatteryToRecord(LogRecord &rec);

void setDefaultCodesIfNeeded(LogRecord &rec);

String buildCsvLine(const LogRecord &rec);
bool ensureLogFileHeader(const char *path, const char *header);
bool appendLine(const char *path, const String &line);

void showStatusBriefly(const LogRecord &rec);
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
  Serial.println("env_logger_main start");
  Serial.print("bootCount = ");
  Serial.println(bootCount);

  printWakeupReason();
  initPins();

  if (!initWireBus()) {
    Serial.println("initWireBus failed");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initRtc()) {
    Serial.println("initRtc failed");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!recoverRtcIfNeeded()) {
    Serial.println("recoverRtcIfNeeded failed");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initBme280()) {
    Serial.println("initBme280 failed");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initLtr390()) {
    Serial.println("initLtr390 failed");
    prepareForSleep();
    enterDeepSleep();
  }

  if (!initSd()) {
    Serial.println("initSd failed");
    prepareForSleep();
    enterDeepSleep();
  }

  // OLED is optional in Deep Sleep logger baseline.
  // Failure should not block logging.
  if (!initDisplay()) {
    Serial.println("initDisplay skipped or failed");
  }

  memset(&currentRecord, 0, sizeof(currentRecord));

  if (!readRtcToRecord(currentRecord)) {
    Serial.println("readRtcToRecord failed");
  }

  if (!readBme280ToRecord(currentRecord)) {
    Serial.println("readBme280ToRecord failed");
  }

  if (!readLtr390ToRecord(currentRecord)) {
    Serial.println("readLtr390ToRecord failed");
  }

  if (!readBatteryToRecord(currentRecord)) {
    Serial.println("readBatteryToRecord skipped or failed");
  }

  setDefaultCodesIfNeeded(currentRecord);

  if (!ensureLogFileHeader(LOG_PATH, CSV_HEADER)) {
    Serial.println("ensureLogFileHeader failed");
  }

  const String csvLine = buildCsvLine(currentRecord);
  Serial.print("csv = ");
  Serial.println(csvLine);

  if (!appendLine(LOG_PATH, csvLine)) {
    Serial.println("appendLine failed");
  } else {
    Serial.println("appendLine OK");
  }

  showStatusBriefly(currentRecord);
  prepareForSleep();
  enterDeepSleep();
}

// =========================
// loop
// =========================
void loop() {
  // Deep Sleep logger 基準版では未使用
}

// =========================
// Init functions
// =========================
void initPins() {
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
}

bool initWireBus() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("Wire.begin OK");
  return true;
}

bool initRtc() {
  // TODO: replace with actual RTC init
  // if (!rtc.begin()) return false;
  Serial.println("rtc.begin placeholder OK");
  return true;
}

bool initBme280() {
  // TODO: replace with actual BME280 init
  // if (!bme.begin(0x76)) return false;
  Serial.println("bme.begin placeholder OK");
  return true;
}

bool initLtr390() {
  // TODO: replace with actual LTR390 init
  // if (!ltr.begin()) return false;
  Serial.println("ltr.begin placeholder OK");
  return true;
}

bool initDisplay() {
  // TODO: replace with actual OLED init
  // return display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  return false;
}

bool initSd() {
  pinMode(PIN_SD_CS, OUTPUT);
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
bool isRtcTimeValid() {
  // TODO: replace with actual RTC read & validation
  // Example rule:
  // year >= 2024 && year <= 2099
  return true;
}

bool recoverRtcIfNeeded() {
  // TODO: implement lostPower() / invalid time recovery
  // For now, always accept.
  Serial.println("RTC recovery placeholder OK");
  return true;
}

// =========================
// Read functions
// =========================
bool readRtcToRecord(LogRecord &rec) {
  // TODO: replace with actual RTC read
  strncpy(rec.date, "2026-04-05", sizeof(rec.date));
  rec.date[sizeof(rec.date) - 1] = '\0';

  strncpy(rec.time, "12:00:00", sizeof(rec.time));
  rec.time[sizeof(rec.time) - 1] = '\0';

  return true;
}

bool readBme280ToRecord(LogRecord &rec) {
  // TODO: replace with actual sensor read
  rec.temp_c = 22.50f;
  rec.hum_pct = 62.30f;
  rec.press_hpa = 1006.20f;
  return true;
}

bool readLtr390ToRecord(LogRecord &rec) {
  // TODO: replace with actual LTR390 read
  // Keep ALS/UVS names aligned with documents.
  rec.als = 565;
  rec.uvs = 0;
  return true;
}

bool readBatteryToRecord(LogRecord &rec) {
  // TODO: implement later
  rec.vbat = 0.0f;
  return true;
}

void setDefaultCodesIfNeeded(LogRecord &rec) {
  rec.context_code = currentContextCode; // default = unknown
  rec.head_code = currentHeadCode;       // default = unknown
}

// =========================
// CSV / SD functions
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
// Display / status
// =========================
void showStatusBriefly(const LogRecord &rec) {
  (void)rec;
  // TODO: implement OLED summary screen if needed
  delay(50);
}

// =========================
// Sleep control
// =========================
void prepareForSleep() {
  // TODO:
  // - OLED off
  // - close peripherals if needed
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
