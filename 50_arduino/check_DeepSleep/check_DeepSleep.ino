#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>
#include "esp_sleep.h"

// ===== XIAO ESP32S3 Plus 仮配線 =====
static const int PIN_I2C_SDA = 5;   // D4
static const int PIN_I2C_SCL = 6;   // D5

static const int PIN_SD_CS   = 3;   // D2
static const int PIN_SD_SCK  = 7;   // D8
static const int PIN_SD_MISO = 8;   // D9
static const int PIN_SD_MOSI = 9;   // D10

// ===== 設定値 =====
static const uint32_t WAKE_INTERVAL_SEC = 30;   // 最初は60秒推奨
static const uint8_t  BME280_ADDR = 0x76;
static const char* LOG_FILE = "/log_sleep_env.csv";

// LTR390待機設定
static const uint32_t LTR_TIMEOUT_MS = 500;     // newDataAvailable待ち上限
static const uint32_t LTR_SWITCH_GUARD_MS = 20; // モード切替直後の最低待機

RTC_DS3231 rtc;
Adafruit_BME280 bme;
Adafruit_LTR390 ltr;

RTC_DATA_ATTR uint32_t bootCount = 0;

String wakeupCauseToString(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:    return "TIMER";
    case ESP_SLEEP_WAKEUP_EXT0:     return "EXT0";
    case ESP_SLEEP_WAKEUP_EXT1:     return "EXT1";
    case ESP_SLEEP_WAKEUP_TOUCHPAD: return "TOUCH";
    case ESP_SLEEP_WAKEUP_ULP:      return "ULP";
    default:                        return "OTHER";
  }
}

void printDateTime(const DateTime& dt) {
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d",
                dt.year(), dt.month(), dt.day(),
                dt.hour(), dt.minute(), dt.second());
}

bool rtcTimeLooksValid(const DateTime& dt) {
  if (dt.year() < 2024 || dt.year() > 2035) return false;
  return true;
}

void ensureRtcValid() {
  DateTime now = rtc.now();

  if (!rtcTimeLooksValid(now)) {
    Serial.println("RTC time invalid -> adjust to compile time");
    rtc.adjust(DateTime(__DATE__, __TIME__));
    delay(50);
    now = rtc.now();
  }

  Serial.print("RTC current time = ");
  printDateTime(now);
  Serial.println();
}

void ensureLogHeader() {
  if (!SD.exists(LOG_FILE)) {
    File f = SD.open(LOG_FILE, FILE_WRITE);
    if (!f) {
      Serial.println("ERROR: header create failed");
      while (true) delay(1000);
    }
    f.println("timestamp,temp_c,hum_pct,press_hpa,als,uvs,boot_count,wakeup_cause");
    f.close();
    Serial.println("CSV header created");
  }
}

bool waitLtrDataReady(uint32_t timeoutMs) {
  uint32_t startMs = millis();
  while ((millis() - startMs) < timeoutMs) {
    if (ltr.newDataAvailable()) {
      return true;
    }
    delay(5);
  }
  return false;
}

bool readLtr390Once(uint32_t &als, uint32_t &uvs) {
  // ALS
  ltr.setMode(LTR390_MODE_ALS);
  delay(LTR_SWITCH_GUARD_MS);
  if (!waitLtrDataReady(LTR_TIMEOUT_MS)) {
    Serial.println("ERROR: LTR390 ALS data timeout");
    return false;
  }
  als = ltr.readALS();

  // UVS
  ltr.setMode(LTR390_MODE_UVS);
  delay(LTR_SWITCH_GUARD_MS);
  if (!waitLtrDataReady(LTR_TIMEOUT_MS)) {
    Serial.println("ERROR: LTR390 UVS data timeout");
    return false;
  }
  uvs = ltr.readUVS();

  return true;
}

void appendLogLine(const DateTime& now,
                   float tempC,
                   float humPct,
                   float pressHpa,
                   uint32_t als,
                   uint32_t uvs,
                   const String& wakeCause) {
  char ts[24];
  snprintf(ts, sizeof(ts), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year(), now.month(), now.day(),
           now.hour(), now.minute(), now.second());

  File f = SD.open(LOG_FILE, FILE_APPEND);
  if (!f) {
    Serial.println("ERROR: append open failed");
    while (true) delay(1000);
  }

  char line[200];
  snprintf(line, sizeof(line), "%s,%.2f,%.2f,%.2f,%lu,%lu,%lu,%s",
           ts,
           tempC,
           humPct,
           pressHpa,
           (unsigned long)als,
           (unsigned long)uvs,
           (unsigned long)bootCount,
           wakeCause.c_str());

  f.println(line);
  f.close();

  Serial.print("logged: ");
  Serial.println(line);
}

void goDeepSleep() {
  Serial.printf("sleep %lu sec...\n", (unsigned long)WAKE_INTERVAL_SEC);
  Serial.flush();

  esp_sleep_enable_timer_wakeup((uint64_t)WAKE_INTERVAL_SEC * 1000000ULL);
  delay(100);
  esp_deep_sleep_start();
}

void initSdCard() {
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);
  delay(20);

  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  delay(20);

  if (!SD.begin(PIN_SD_CS, SPI, 1000000)) {
    Serial.println("ERROR: SD.begin failed");
    while (true) delay(1000);
  }

  Serial.println("SD.begin OK");
}

void setup() {
  Serial.begin(115200);
  delay(800);

  bootCount++;
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  String wakeCause = wakeupCauseToString(cause);

  Serial.println();
  Serial.println("=== deep sleep env logger start ===");
  Serial.printf("bootCount   = %lu\n", (unsigned long)bootCount);
  Serial.printf("wakeupCause = %s\n", wakeCause.c_str());

  // I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("Wire.begin OK");

  // RTC
  if (!rtc.begin()) {
    Serial.println("ERROR: rtc.begin failed");
    while (true) delay(1000);
  }
  Serial.println("rtc.begin OK");
  ensureRtcValid();

  // BME280
  if (!bme.begin(BME280_ADDR, &Wire)) {
    Serial.println("ERROR: bme.begin failed");
    while (true) delay(1000);
  }
  Serial.println("bme.begin OK");

  bme.setSampling(
    Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1,   // temp
    Adafruit_BME280::SAMPLING_X1,   // press
    Adafruit_BME280::SAMPLING_X1,   // hum
    Adafruit_BME280::FILTER_OFF,
    Adafruit_BME280::STANDBY_MS_0_5
  );

  delay(50);
  if (!bme.takeForcedMeasurement()) {
    Serial.println("ERROR: takeForcedMeasurement failed");
    while (true) delay(1000);
  }

  float tempC    = bme.readTemperature();
  float humPct   = bme.readHumidity();
  float pressHpa = bme.readPressure() / 100.0f;

  // LTR390
  if (!ltr.begin(&Wire)) {
    Serial.println("ERROR: ltr.begin failed");
    while (true) delay(1000);
  }
  Serial.println("ltr.begin OK");

  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);

  uint32_t als = 0;
  uint32_t uvs = 0;
  if (!readLtr390Once(als, uvs)) {
    Serial.println("ERROR: readLtr390Once failed");
    while (true) delay(1000);
  }

  // SD
  initSdCard();

  ensureLogHeader();

  DateTime now = rtc.now();
  appendLogLine(now, tempC, humPct, pressHpa, als, uvs, wakeCause);

  goDeepSleep();
}

void loop() {
  // Deep Sleep最小試験では未使用
}