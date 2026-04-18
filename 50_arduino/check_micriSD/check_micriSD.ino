#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

// ===== I2C pins =====
static const int PIN_I2C_SDA = 5; // D4 = GPIO5
static const int PIN_I2C_SCL = 6; // D5 = GPIO6

// ===== microSD pins =====
static const int PIN_SD_CS   = 3; // D2  = GPIO3
static const int PIN_SD_SCK  = 7; // D8  = GPIO7
static const int PIN_SD_MISO = 8; // D9  = GPIO8
static const int PIN_SD_MOSI = 9; // D10 = GPIO9

const char *LOG_PATH = "/log.csv";

RTC_DS3231 rtc;
SPIClass spi(FSPI);

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
    Serial.println("log.csv already exists");
    return true;
  }

  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("failed to create log.csv");
    return false;
  }

  file.println("timestamp,message");
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

bool appendCsvRow(const char *path, const String &timestamp, const char *message) {
  File file = SD.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("failed to open log.csv for append");
    return false;
  }

  file.print(timestamp);
  file.print(",");
  file.println(message);
  file.close();
  return true;
}

void readFile(const char *path) {
  File file = SD.open(path, FILE_READ);
  if (!file) {
    Serial.println("failed to open file for read");
    return;
  }

  Serial.print("----- ");
  Serial.print(path);
  Serial.println(" -----");

  while (file.available()) {
    Serial.write(file.read());
  }

  Serial.println();
  Serial.println("----- end -----");

  file.close();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("RTC + microSD csv test start");

  // I2C init
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  if (!rtc.begin()) {
    Serial.println("rtc.begin failed");
    return;
  }
  Serial.println("rtc.begin OK");

  if (rtc.lostPower()) {
    Serial.println("rtc lostPower detected");
    Serial.println("set RTC first, then retry");
    return;
  }

  DateTime now = rtc.now();
  String timestamp = formatDateTime(now);

  Serial.print("timestamp = ");
  Serial.println(timestamp);

  // SPI / SD init
  spi.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  if (!SD.begin(PIN_SD_CS, spi, 4000000)) {
    Serial.println("SD.begin failed");
    return;
  }
  Serial.println("SD.begin OK");

  if (!ensureHeader(LOG_PATH)) {
    Serial.println("header NG");
    return;
  }

  if (!appendCsvRow(LOG_PATH, timestamp, "rtc_boot_test")) {
    Serial.println("append NG");
    return;
  }
  Serial.println("append OK");

  readFile(LOG_PATH);

  Serial.println("RTC + microSD csv test PASS");
}

void loop() {
}