# 07_SOFTWARE_ARCHITECTURE

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるソフトウェア実装構造を整理する。  
状態遷移、ログ形式、電源制御、デバイス初期化、タスク分割方針を、コードへ落とし込める粒度で定義する。

---

## 1. 現行前提

- 開発環境: Arduino IDE
- MCU: XIAO ESP32S3 Plus
- センサ: BME680 / LTR390
- RTC: DS3231 + AT24C32
- 表示: SSD1306 OLED
- 記録: microSD
- GPIO拡張: MCP23017
- 電源: LiPo + TP4056 + TPS63802
- Battery ADC: D3(GPIO4)
- SPI:
  - D10 = CS
  - D9 = MOSI
  - D8 = SCK
  - D7 = MISO

---

## 2. 設計方針

- 初号機では、過度な抽象化を避ける。
- まず単一 `.ino` + 関数分割で成立させる。
- 後から `.h` / `.cpp` 分割へ移行可能な構造にする。
- UI試験版、周期ロガー版、DeepSleep版を分けて考える。
- 最終的には同一構造体 `LogRecord` に測定値を集約する。
- 推測値は入れない。
- センサ失敗時も、可能な範囲で切り分け可能なログを残す。

---

## 3. GPIO定義

### 3.1 XIAO端子基準

| XIAO端子 | GPIO | 用途 |
|---|---:|---|
| D0 | GPIO1 | Encoder A |
| D1 | GPIO2 | Encoder B |
| D2 | GPIO3 | Encoder SW |
| D3 | GPIO4 | Battery ADC |
| D4 | GPIO5 | I2C SDA |
| D5 | GPIO6 | I2C SCL |
| D6 | GPIO43 | TPS63802 EN |
| D7 | GPIO44 | SPI MISO |
| D8 | GPIO7 | SPI SCK |
| D9 | GPIO8 | SPI MOSI |
| D10 | GPIO9 | SPI CS |

### 3.2 `pins.h` 定義例

```cpp
#pragma once

// Encoder
constexpr uint8_t PIN_ENC_A    = D0;
constexpr uint8_t PIN_ENC_B    = D1;
constexpr uint8_t PIN_ENC_SW   = D2;

// Battery ADC
constexpr uint8_t PIN_BAT_ADC  = D3;

// I2C
constexpr uint8_t PIN_I2C_SDA  = D4;
constexpr uint8_t PIN_I2C_SCL  = D5;

// Power Control
constexpr uint8_t PIN_DCDC_EN  = D6;

// SPI / microSD
constexpr uint8_t PIN_SD_MISO  = D7;
constexpr uint8_t PIN_SD_SCK   = D8;
constexpr uint8_t PIN_SD_MOSI  = D9;
constexpr uint8_t PIN_SD_CS    = D10;
```

D番号マクロが利用できない環境ではGPIO番号で定義する。

---

## 4. I2Cアドレス定義

```cpp
constexpr uint8_t I2C_ADDR_MCP23017 = 0x20;
constexpr uint8_t I2C_ADDR_OLED     = 0x3C;
constexpr uint8_t I2C_ADDR_LTR390   = 0x53;
constexpr uint8_t I2C_ADDR_EEPROM   = 0x57;
constexpr uint8_t I2C_ADDR_DS3231   = 0x68;
constexpr uint8_t I2C_ADDR_BME680_A = 0x76;
constexpr uint8_t I2C_ADDR_BME680_B = 0x77;
```

BME680は実機スキャンで0x76または0x77を確認する。

---

## 5. データ構造

### 5.1 LogRecord

```cpp
struct LogRecord {
  char date[11];          // YYYY-MM-DD
  char time[9];           // HH:MM:SS

  float temp_c;
  float hum_pct;
  float press_hpa;

  uint32_t als;
  uint32_t uvs;

  uint8_t context_code;
  uint8_t head_code;

  float vbat;
};
```

### 5.2 DeviceStatus

```cpp
struct DeviceStatus {
  bool rtc_ok;
  bool bme680_ok;
  bool ltr390_ok;
  bool oled_ok;
  bool sd_ok;
  bool mcp_ok;
  bool battery_ok;
};
```

### 5.3 UiState

```cpp
enum UiState {
  UI_VIEW,
  UI_MENU,
  UI_CLOCK,
  UI_LOG,
  UI_SLEEP,
  UI_ERROR
};
```

### 5.4 AppMode

```cpp
enum AppMode {
  MODE_BRINGUP,
  MODE_PERIODIC_LOGGER,
  MODE_UI_TEST,
  MODE_SLEEP_LOGGER
};
```

---

## 6. 推奨ファイル構成

### 6.1 初期実装

```text
env_logger_main.ino
pins.h
```

### 6.2 分割候補

```text
env_logger_main.ino
config.h
pins.h
types.h

device_i2c.cpp
device_rtc.cpp
device_bme680.cpp
device_ltr390.cpp
device_oled.cpp
device_sd.cpp
device_mcp23017.cpp

task_measure.cpp
task_log.cpp
task_display.cpp
task_input.cpp
task_power.cpp
task_sleep.cpp
```

初号機では、最初から複雑な分割を必須としない。

---

## 7. 初期化順序

推奨順序：

1. Serial開始
2. D6(TPS63802 EN)初期化
3. Battery ADC初期化
4. I2C初期化
5. MCP23017初期化
6. RTC初期化
7. BME680初期化
8. LTR390初期化
9. OLED初期化
10. SPI初期化
11. microSD初期化
12. Encoder初期化
13. ヘッダ確認

Bring-upでは、接続済みデバイスのみ初期化してよい。

---

## 8. デバイス初期化関数

| 関数 | 役割 |
|---|---|
| `initPowerControl()` | D6 EN設定 |
| `initBatteryAdc()` | D3 ADC設定 |
| `initI2c()` | Wire.begin(D4,D5) |
| `initMcp23017()` | MCP23017 0x20初期化 |
| `initRtc()` | DS3231初期化 |
| `initBme680()` | BME680初期化 |
| `initLtr390()` | LTR390初期化 |
| `initOled()` | SSD1306初期化 |
| `initSd()` | SPI.begin + SD.begin |
| `initEncoder()` | Encoder A/B/SW設定 |

---

## 9. 測定関数

| 関数 | 内容 |
|---|---|
| `readRtcToRecord(LogRecord &rec)` | date/time取得 |
| `readBme680ToRecord(LogRecord &rec)` | temp/hum/press取得 |
| `readLtr390ToRecord(LogRecord &rec)` | als/uvs取得 |
| `readBatteryToRecord(LogRecord &rec)` | vbat取得 |
| `fillDefaultCodes(LogRecord &rec)` | context/head既定値設定 |

---

## 10. Battery ADC実装

```cpp
float readVbat() {
  uint32_t mv = analogReadMilliVolts(PIN_BAT_ADC);
  return (mv / 1000.0f) * 2.0f;
}
```

注意：

- 分圧比は1:1。
- 初回はDT4256でB+系電圧とADC中点電圧を確認する。
- `vbat` に推測値を入れない。

---

## 11. microSD実装

### 11.1 初期化

```cpp
bool initSd() {
  pinMode(PIN_SD_CS, OUTPUT);
  digitalWrite(PIN_SD_CS, HIGH);

  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);

  return SD.begin(PIN_SD_CS, SPI);
}
```

### 11.2 ヘッダ

```cpp
const char* LOG_HEADER =
  "date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat";
```

### 11.3 CSV生成

```cpp
String buildCsvLine(const LogRecord &rec) {
  String line;
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
```

### 11.4 Sleep前処理

```cpp
void closeSdBeforeSleep() {
  digitalWrite(PIN_SD_CS, HIGH);
  SPI.end();
}
```

---

## 12. MCP23017実装

### 12.1 用途

MCP23017はRGB LED制御に使用する。

| MCP23017 | 用途 |
|---|---|
| GPA0 | LED_B |
| GPA1 | LED_G |
| GPA2 | LED_R |

### 12.2 初期化

```cpp
bool initMcp23017() {
  if (!mcp.begin_I2C(0x20)) {
    return false;
  }

  mcp.pinMode(0, OUTPUT); // GPA0 LED_B
  mcp.pinMode(1, OUTPUT); // GPA1 LED_G
  mcp.pinMode(2, OUTPUT); // GPA2 LED_R

  mcp.digitalWrite(0, LOW);
  mcp.digitalWrite(1, LOW);
  mcp.digitalWrite(2, LOW);

  return true;
}
```

LED極性によりHIGH/LOWの意味は反転する場合がある。

---

## 13. Encoder実装

| 信号 | XIAO |
|---|---|
| A | D0 |
| B | D1 |
| SW | D2 |

方針：

- ESP32Encoder + PCNT方式を優先する。
- 押下はチャタリング対策を入れる。
- context_code / head_code入力は後段で実装する。

---

## 14. 電源制御実装

### 14.1 EN初期化

```cpp
void initPowerControl() {
  pinMode(PIN_DCDC_EN, OUTPUT);
  digitalWrite(PIN_DCDC_EN, HIGH);
}
```

### 14.2 Sleep移行前

```cpp
void prepareForSleep() {
  closeSdBeforeSleep();
  // 必要に応じてOLED消灯、LED消灯
}
```

### 14.3 EN制御注意

D6をLOWにするとTPS63802が停止し、XIAO自身も停止する可能性がある。  
実機確認までは、DeepSleep前に必ずD6 LOWへする設計を固定しない。

---

## 15. `setup()` / `loop()` 方針

### 15.1 Bring-up版

`loop()` で状態をシリアル出力しながら確認する。

### 15.2 Periodic Logger版

`loop()` で周期管理する。

```cpp
void loop() {
  updateInput();

  if (millis() - lastMeasureMs >= MEASURE_INTERVAL_MS) {
    readAllSensors(currentRecord);
  }

  if (millis() - lastLogMs >= LOG_INTERVAL_MS) {
    appendLogRecord(currentRecord);
  }

  updateDisplayIfNeeded();
}
```

### 15.3 DeepSleep Logger版

`setup()` 完結型とする。

```cpp
void setup() {
  initAll();
  readAllSensors(currentRecord);
  appendLogRecord(currentRecord);
  prepareForSleep();
  esp_deep_sleep_start();
}

void loop() {}
```

---

## 16. エラー処理

| エラー | 方針 |
|---|---|
| RTC失敗 | 無効時刻または停止 |
| BME680失敗 | 欠損値を入れて継続候補 |
| LTR390失敗 | als/uvsを0または欠損値 |
| SD失敗 | ログ不可、表示またはシリアル出力 |
| MCP失敗 | LED制御なしで継続 |
| Battery ADC失敗 | vbat=0 |
| OLED失敗 | 表示なしで継続 |

初号機では、ログ保存ができない場合は重大エラー扱いとする。

---

## 17. 実装優先順位

1. pins.h確定
2. I2Cスキャン
3. MCP23017 GPIO試験
4. RTC試験
5. BME680試験
6. microSD新SPI割当試験
7. Battery ADC試験
8. CSV保存
9. OLED表示
10. LTR390
11. Encoder
12. DeepSleep
13. EN制御
14. 消費電流測定

---

## 18. 保留事項

- FreeRTOSタスク分割
- クラス全面設計
- Wi-Fi同期
- BLE連携
- ガス測定値ログ化
- ファイルローテーション
- エラーフラグ列
- 電池残量パーセント換算

---

## 19. ステータス

- [COMPLETE] BME680前提へ更新
- [COMPLETE] 新SPI割当へ更新
- [COMPLETE] Battery ADC構造更新
- [COMPLETE] MCP23017構造更新
- [ACTIVE] 初号機実装構造として有効
- [PENDING] 実機統合スケッチ作成
- [PENDING] DeepSleep版スケッチ作成
