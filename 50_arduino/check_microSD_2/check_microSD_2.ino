#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>

// ==================================================
// pin assignment
// ==================================================

// I2C
static const int PIN_I2C_SDA = 5; // XIAO D4 = GPIO5
static const int PIN_I2C_SCL = 6; // XIAO D5 = GPIO6

// microSD SPI
static const int PIN_SD_CS   = 3; // XIAO D2  = GPIO3
static const int PIN_SD_SCK  = 7; // XIAO D8  = GPIO7
static const int PIN_SD_MISO = 8; // XIAO D9  = GPIO8
static const int PIN_SD_MOSI = 9; // XIAO D10 = GPIO9

// sensor addresses
static const uint8_t BME280_ADDR = 0x76;

// file path
const char *LOG_PATH = "/log_env_loop.csv";

// RTC validity threshold
static const int RTC_VALID_YEAR_MIN = 2024;

// SD clock
static const uint32_t SD_CLOCK_HZ = 4000000;

// periodic logger
static const unsigned long LOG_INTERVAL_MS = 5000;
static const unsigned long START_DELAY_MS  = 2000;

// LTR390
static const unsigned long LTR390_TIMEOUT_MS = 500;
static const unsigned long LTR390_SETTLE_MS  = 120;

// ==================================================
// global instances
// ==================================================

RTC_DS3231 rtc;
Adafruit_BME280 bme;
Adafruit_LTR390 ltr = Adafruit_LTR390();
SPIClass spi(FSPI);

// ==================================================
// logger state
// ==================================================

unsigned long nextLogMs = 0;

// ==================================================
// utility
// ==================================================

bool fileExists(const char *path) {
  File f = SD.open(path, FILE_READ);
  if (f) {
    f.close();
    return true;
  }
  return false;
}

bool ensureHeader(const char *path) {
  if (fileExists(path)) {
    Serial.println("log_env_loop.csv already exists");
    return true;
  }

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("failed to create log_env_loop.csv");
    return false;
  }

  file.println("timestamp,temp_c,hum_pct,press_hpa,als,uvs");
  file.close();

  Serial.println("header created");
  return true;
}

String formatDateTime(const DateTime &dt) {
  char buf[24];
  snprintf(
    buf, sizeof(buf),
    "%04d-%02d-%02d %02d:%02d:%02d",
    dt.year(), dt.month(), dt.day(),
    dt.hour(), dt.minute(), dt.second()
  );
  return String(buf);
}

bool isRtcTimeValid(const DateTime &dt) {
  if (dt.year() < RTC_VALID_YEAR_MIN) return false;
  if (dt.month() < 1 || dt.month() > 12) return false;
  if (dt.day() < 1 || dt.day() > 31) return false;
  if (dt.hour() > 23) return false;
  if (dt.minute() > 59) return false;
  if (dt.second() > 59) return false;
  return true;
}

bool recoverRtcWithCompileTime() {
  Serial.println("rtc recovery start");

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  delay(50);

  DateTime now = rtc.now();

  Serial.print("rtc adjusted to compile time: ");
  Serial.println(formatDateTime(now));

  if (!isRtcTimeValid(now)) {
    Serial.println("rtc recovery failed");
    return false;
  }

  Serial.println("rtc recovery OK");
  return true;
}

bool appendCsvRow(
  const char *path,
  const String &timestamp,
  float tempC,
  float humPct,
  float pressHpa,
  uint32_t als,
  uint32_t uvs
) {
  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("failed to open log_env_loop.csv for append");
    return false;
  }

  file.print(timestamp);
  file.print(",");
  file.print(tempC, 2);
  file.print(",");
  file.print(humPct, 2);
  file.print(",");
  file.print(pressHpa, 2);
  file.print(",");
  file.print(als);
  file.print(",");
  file.println(uvs);

  file.close();
  return true;
}

void printCsvRow(
  const String &timestamp,
  float tempC,
  float humPct,
  float pressHpa,
  uint32_t als,
  uint32_t uvs
) {
  Serial.print("logged: ");
  Serial.print(timestamp);
  Serial.print(",");
  Serial.print(tempC, 2);
  Serial.print(",");
  Serial.print(humPct, 2);
  Serial.print(",");
  Serial.print(pressHpa, 2);
  Serial.print(",");
  Serial.print(als);
  Serial.print(",");
  Serial.println(uvs);
}

// ==================================================
// LTR390 stable read
// ==================================================

bool waitLtrReady(unsigned long timeoutMs) {
  unsigned long startMs = millis();

  while (!ltr.newDataAvailable()) {
    if (millis() - startMs > timeoutMs) {
      return false;
    }
    delay(5);
  }

  return true;
}

bool readLtr390AlsUvsStable(uint32_t &als, uint32_t &uvs) {
  // ALS
  ltr.setMode(LTR390_MODE_ALS);
  delay(LTR390_SETTLE_MS);

  if (!waitLtrReady(LTR390_TIMEOUT_MS)) {
    Serial.println("LTR390 ALS timeout");
    return false;
  }
  als = ltr.readALS();

  // UVS
  ltr.setMode(LTR390_MODE_UVS);
  delay(LTR390_SETTLE_MS);

  if (!waitLtrReady(LTR390_TIMEOUT_MS)) {
    Serial.println("LTR390 UVS timeout");
    return false;
  }
  uvs = ltr.readUVS();

  return true;
}

// ==================================================
// one log action
// ==================================================

bool logOneRow() {
  if (rtc.lostPower()) {
    Serial.println("rtc lostPower detected during loop");
    if (!recoverRtcWithCompileTime()) {
      return false;
    }
  }

  DateTime now = rtc.now();

  if (!isRtcTimeValid(now)) {
    Serial.println("rtc time invalid during loop");
    if (!recoverRtcWithCompileTime()) {
      return false;
    }

    now = rtc.now();
    if (!isRtcTimeValid(now)) {
      Serial.println("rtc still invalid after recovery");
      return false;
    }
  }

  String timestamp = formatDateTime(now);

  float tempC = bme.readTemperature();
  float humPct = bme.readHumidity();
  float pressHpa = bme.readPressure() / 100.0f;

  if (isnan(tempC) || isnan(humPct) || isnan(pressHpa)) {
    Serial.println("BME280 value invalid");
    return false;
  }

  uint32_t als = 0;
  uint32_t uvs = 0;
  if (!readLtr390AlsUvsStable(als, uvs)) {
    return false;
  }

  if (!appendCsvRow(LOG_PATH, timestamp, tempC, humPct, pressHpa, als, uvs)) {
    Serial.println("append NG");
    return false;
  }

  printCsvRow(timestamp, tempC, humPct, pressHpa, als, uvs);
  return true;
}

// ==================================================
// setup
// ==================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("RTC + BME280 + LTR390 + microSD periodic logger start");

  // I2C
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Serial.println("Wire.begin OK");

  // RTC
  if (!rtc.begin()) {
    Serial.println("rtc.begin failed");
    return;
  }
  Serial.println("rtc.begin OK");

  bool rtcNeedRecovery = false;

  if (rtc.lostPower()) {
    Serial.println("rtc lostPower detected");
    rtcNeedRecovery = true;
  } else {
    Serial.println("rtc lostPower = false");
  }

  DateTime now = rtc.now();
  Serial.print("rtc raw time = ");
  Serial.println(formatDateTime(now));

  if (!isRtcTimeValid(now)) {
    Serial.println("rtc time invalid");
    rtcNeedRecovery = true;
  } else {
    Serial.println("rtc time valid");
  }

  if (rtcNeedRecovery) {
    if (!recoverRtcWithCompileTime()) {
      Serial.println("set RTC first, then retry");
      return;
    }
  }

  // BME280
  if (!bme.begin(BME280_ADDR, &Wire)) {
    Serial.println("bme.begin failed");
    return;
  }
  Serial.println("bme.begin OK");

  // LTR390
  if (!ltr.begin()) {
    Serial.println("ltr.begin failed");
    return;
  }
  Serial.println("ltr.begin OK");

  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);

  // microSD
  spi.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, spi, SD_CLOCK_HZ)) {
    Serial.println("SD.begin failed");
    return;
  }
  Serial.println("SD.begin OK");

  if (!ensureHeader(LOG_PATH)) {
    Serial.println("header NG");
    return;
  }

  nextLogMs = millis() + START_DELAY_MS;

  Serial.print("log interval ms = ");
  Serial.println(LOG_INTERVAL_MS);

  Serial.println("periodic logger running");
}

// ==================================================
// loop
// ==================================================

void loop() {
  unsigned long nowMs = millis();

  if ((long)(nowMs - nextLogMs) >= 0) {
    bool ok = logOneRow();

    if (ok) {
      Serial.println("append OK");
    } else {
      Serial.println("append skipped");
    }

    nextLogMs += LOG_INTERVAL_MS;
  }
}