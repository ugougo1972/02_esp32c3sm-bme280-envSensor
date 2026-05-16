# 07_SOFTWARE_ARCHITECTURE

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるソフトウェア実装構造を整理するためのものである。  
上位文書で定義した状態遷移、ログ形式、電源制御方針を、**実際にコードへ落とし込める粒度** まで具体化する。

初号機では、まず **安定して測定・保存・Deep Sleep 周期動作が成立する構造** を優先し、過度な抽象化や複雑なクラス設計は採らない。

---

## 基本方針

- 実装は **Arduino IDE / XIAO ESP32S3 Plus** を前提とする
- 電源経路は **LiPo → TP4056 → AE-TPS63802 → XIAO**（**通電試験全項目PASS・3.304V確認**）✅ を前提とする
- **GPIO割当確定版**（**D0～D10・D6(GPIO43)・D3(GPIO4)・D7(GPIO44)確定**）✅ を採用する
- **メイン基板レイアウト確定版**（**95mm×72mm・配置座標確定**）✅ を採用する
- RGB LED制御は **MCP23017（I2C GPIO Expander、アドレス0x20）** 経由とする（**XIAO裏面JTAG ランド使用不可**）✅
- **Battery voltage monitor**は **D3(GPIO4)・ADC1_CH3・100kΩ×2分圧回路**を採用する（**座標(13,03)～(13,06)確定**）✅
- **Deep Sleep対応設計確定**（**GPIO43・R_EN・microSD CS処理**）✅ を採用する
- 初号機では、**1スケッチで全体成立** を優先する
- 処理は役割ごとに関数分割する
- `setup()` / `loop()` の責務を明確に分ける
- Deep Sleep logger 基準版では、**setup() 完結型** を採用する
- UI試験版・periodic logger 版では、`loop()` による non-blocking 管理を採用する
- 入力系は **ESP32Encoder + PCNT 方式** を採用する
- 文書上の列名・状態名・コード名は  
  - `03_LOG_FORMAT.md`
  - `04_ctx.md`
  - `05_head.md`
  - `02_STATE_MACHINE.md`
  と整合させる

---

## GPIO割当確定版（実装基準）

| XIAO端子 | GPIO | 用途 | 用途例 | 状態 |
|----------|------|------|--------|------|
| D0 | GPIO1 | Encoder_A | `ENC_A_PIN = 1` | ✅ 確定 |
| D1 | GPIO2 | Encoder_B | `ENC_B_PIN = 2` | ✅ 確定 |
| D2 | GPIO3 | Encoder_SW | `ENC_SW_PIN = 3` | ✅ 確定 |
| **D3** | **GPIO4** | **Battery_ADC(ADC1_CH3)** | **`PIN_BAT_ADC = 4`** | **✅ 確定** |
| D4 | GPIO5 | I2C_SDA | 全I2Cデバイス | ✅ 確定 |
| D5 | GPIO6 | I2C_SCL | 全I2Cデバイス | ✅ 確定 |
| **D6** | **GPIO43** | **TPS63802_EN** | **`PIN_DCDC_EN = 43`** | **✅ 確定** |
| **D7** | **GPIO44** | **SPI_CS(microSD)** | **`PIN_SPI_CS = 44`** | **✅ 確定** |
| D8 | GPIO7 | SPI_SCK | microSD | ✅ 確定 |
| D9 | GPIO8 | SPI_MISO | microSD | ✅ 確定 |
| D10 | GPIO9 | SPI_MOSI | microSD | ✅ 確定 |

### pins.h定義例（確定版）

```cpp
// Encoder
constexpr uint8_t PIN_ENC_A    = 1;   // D0 / GPIO1
constexpr uint8_t PIN_ENC_B    = 2;   // D1 / GPIO2
constexpr uint8_t PIN_ENC_SW   = 3;   // D2 / GPIO3

// ADC
constexpr uint8_t PIN_BAT_ADC  = 4;   // D3 / GPIO4 / ADC1_CH3

// I2C
constexpr uint8_t PIN_I2C_SDA  = 5;   // D4 / GPIO5
constexpr uint8_t PIN_I2C_SCL  = 6;   // D5 / GPIO6

// Power Control
constexpr uint8_t PIN_DCDC_EN  = 43;  // D6 / GPIO43 / TPS63802 EN

// SPI
constexpr uint8_t PIN_SPI_CS   = 44;  // D7 / GPIO44 / microSD CS
constexpr uint8_t PIN_SPI_SCK  = 7;   // D8 / GPIO7
constexpr uint8_t PIN_SPI_MISO = 8;   // D9 / GPIO8
constexpr uint8_t PIN_SPI_MOSI = 9;   // D10 / GPIO9
```

---

## メイン基板レイアウト確定版（実装基準）

| 項目 | 仕様 |
|------|------|
| **基板サイズ** | **95mm × 72mm**✅ |
| **有効面積** | **37穴 × 27穴**✅ |
| **XIAO配置** | **(16,02)～(22,08)**✅ |
| **分圧回路(R1/R2)配置** | **(13,03)～(13,06)**✅ **D3(GPIO4)へ** |
| **R_EN(100kΩ)配置** | **(12,08)～(14,08)**✅ **GPIO43 Hi-Z対策** |
| **MCP23017配置** | **(21,13)～(26,26)**✅ **I2C 0x20・RGB LED** |
| **4ライン端子台配置** | **(31,25)～(34,27)**✅ |

---

## 実装モードの分離方針

初号機ソフトは、1つの最終完成版に一気に寄せるのではなく、目的別に段階構成で扱う。

| モード | 目的 | 主な特徴 | 対象GPIO |
|------|------|------|------|
| UI試験版 | 表示・入力確認 | `loop()` 中心 | D0～D5・D8～D10 |
| periodic logger 版 | 継続測定・継続保存確認 | `loop()` 中心 | D0～D5・D7～D10 |
| Deep Sleep logger 基準版 | 周期起床・保存成立確認 | `setup()` 完結型 | **D0～D10・D6・D3** ✅ |
| Battery monitor 試験版 | 分圧回路・ADC確認 | `loop()` 中心 | **D3(GPIO4)** ✅ |

### 現時点の基準点

- 現時点の基準点は **Deep Sleep logger 基準版**
- 理由は、最終運用に近い形で  
  - RTC（DS3231）
  - BME280
  - LTR390
  - microSD（**D7/GPIO44 CS確定**）✅
  - **Deep Sleep（GPIO43・R_EN・CS処理確定）**✅
  が成立しているため

---

## ソフトウェア全体構造

### 推奨レイヤ構成

| 層 | 内容 | GPIO |
|------|------|------|
| アプリケーション層 | 状態遷移、周期制御、モード選択 | - |
| 機能制御層 | 測定、表示、保存、入力、Sleep 制御 | **D0～D10・D6・D3** ✅ |
| デバイス制御層 | BME280 / LTR390 / DS3231 / SSD1306 / SD / Encoder / ADC / MCP23017 | **I2C(D4/D5)・SPI(D7～D10)・GPIO** |

---

## ファイル構成方針

初号機では、まずは **単一 `.ino` スケッチ** で成立させてよい。  
必要に応じて `.h` / `.cpp` 分割へ移行する。

### 初期段階の推奨構成

```text
env_logger_main.ino       // メインスケッチ
pins.h                    // GPIO割当確定版（D0～D10・D6・D3・D7）
```

### 将来の分割候補

```text
env_logger_main.ino
config.h
pins.h                    // GPIO割当確定版
log_format.h
sensor_task.cpp           // BME280 / LTR390
rtc_task.cpp              // DS3231
display_task.cpp          // OLED
input_task.cpp            // Encoder (D0/D1/D2)
encoder_led_task.cpp      // RGB LED (MCP23017 I2C)
sleep_task.cpp            // Deep Sleep (D6/GPIO43・D7/GPIO44)
sd_task.cpp               // microSD (D7/D8/D9/D10)
battery_task.cpp          // Battery ADC (D3/GPIO4)
```

---

## データ構造方針

### 1. 測定データ構造体

```cpp
struct LogRecord {
  char date[11];          // YYYY-MM-DD
  char time[9];           // HH:MM:SS
  float temp_c;
  float hum_pct;
  float press_hpa;
  uint32_t als;
  uint32_t uvs;
  uint8_t context_code;   // 04_ctx.md 参照
  uint8_t head_code;      // 05_head.md 参照
  float vbat;             // D3(GPIO4) ADC値×2（100kΩ×2分圧）
};
```

### 2. UI状態（確定版）

```cpp
enum UiState {
  UI_VIEW,                // 通常画面
  UI_MENU,                // メニュー画面
  UI_CLOCK,               // 時刻表示
  UI_LOG,                 // ログ状態
  UI_SLEEP                // スリープ確認
};
```

### 3. MENU選択状態（確定版）

```cpp
enum MenuItem {
  MENU_VIEW,
  MENU_CLOCK,
  MENU_LOG,
  MENU_SLEEP
};
```

---

## 定数定義方針

### GPIO系（確定版）✅

```cpp
// Encoder (D0/D1/D2)
const uint8_t PIN_ENC_A   = 1;      // D0 / GPIO1
const uint8_t PIN_ENC_B   = 2;      // D1 / GPIO2
const uint8_t PIN_ENC_SW  = 3;      // D2 / GPIO3

// Battery ADC (D3)
const uint8_t PIN_BAT_ADC = 4;      // D3 / GPIO4 / ADC1_CH3

// I2C (D4/D5)
const uint8_t PIN_SDA     = 5;      // D4 / GPIO5
const uint8_t PIN_SCL     = 6;      // D5 / GPIO6

// Power EN (D6)
const uint8_t PIN_EN      = 43;     // D6 / GPIO43 / TPS63802

// SPI (D7/D8/D9/D10)
const uint8_t PIN_CS      = 44;     // D7 / GPIO44 / microSD CS
const uint8_t PIN_SCK     = 7;      // D8 / GPIO7
const uint8_t PIN_MISO    = 8;      // D9 / GPIO8
const uint8_t PIN_MOSI    = 9;      // D10 / GPIO9
```

### 周期系

```cpp
const uint32_t SENSOR_INTERVAL_MS = 1000;
const uint32_t DISPLAY_INTERVAL_MS = 200;
const uint32_t LOG_INTERVAL_MS = 5000;
const uint32_t OLED_SHOW_MS = 5000;
const uint64_t SLEEP_INTERVAL_US = 60ULL * 1000ULL * 1000ULL;
```

### 状態コード系

```cpp
// context_code (04_ctx.md参照)
const uint8_t CTX_UNKNOWN = 0;
const uint8_t CTX_INDOOR  = 1;
const uint8_t CTX_OUTDOOR = 2;
const uint8_t CTX_MOVING  = 3;
const uint8_t CTX_OFFICE  = 4;
const uint8_t CTX_HOME    = 5;

// head_code (05_head.md参照)
const uint8_t HEAD_UNKNOWN   = 0;
const uint8_t HEAD_NONE      = 1;
const uint8_t HEAD_MILD      = 2;
const uint8_t HEAD_MODERATE  = 3;
const uint8_t HEAD_SEVERE    = 4;
const uint8_t HEAD_RECOVERED = 5;
```

### Battery monitor系（確定版）✅

```cpp
const float VBAT_DIV_RATIO = 2.0f;      // 100kΩ : 100kΩ = 1:1
const uint8_t BAT_ADC_PIN = PIN_BAT_ADC; // D3 / GPIO4
```

### I2Cアドレス系

```cpp
const uint8_t I2C_ADDR_BME280   = 0x76;
const uint8_t I2C_ADDR_LTR390   = 0x53;
const uint8_t I2C_ADDR_OLED     = 0x3C;
const uint8_t I2C_ADDR_RTC      = 0x68;
const uint8_t I2C_ADDR_EEPROM   = 0x57;
const uint8_t I2C_ADDR_MCP23017 = 0x20; // RGB LED control
```

---

## グローバル変数方針

| 変数 | 用途 | 型 |
|------|------|------|
| `UiState uiState` | 現在画面 | enum |
| `MenuItem menuItem` | MENU選択位置 | enum |
| `LogRecord currentRecord` | 最新測定値 | struct |
| `unsigned long lastSensorMs` | センサ周期管理 | uint32_t |
| `unsigned long lastDisplayMs` | 表示周期管理 | uint32_t |
| `unsigned long lastLogMs` | ログ周期管理 | uint32_t |
| `uint8_t currentContextCode` | 現在の行動状態 | uint8_t |
| `uint8_t currentHeadCode` | 現在の頭痛状態 | uint8_t |
| `uint32_t bootCount` | 起動回数確認 | uint32_t |
| `int32_t encoderCount` | Encoder カウント | int32_t |
| `bool encoderPressed` | 押下状態（D2/GPIO3） | bool |

---

## 関数分割方針

### 初期化系

| 関数名例 | 役割 | GPIO |
|------|------|------|
| `initWire()` | I2C初期化 | D4/D5 |
| `initRtc()` | DS3231初期化 | I2C 0x68 |
| `initBme280()` | BME280初期化 | I2C 0x76 |
| `initLtr390()` | LTR390初期化 | I2C 0x53 |
| `initDisplay()` | OLED初期化 | I2C 0x3C |
| `initSd()` | SPI / microSD初期化 | D7/D8/D9/D10 |
| `initEncoder()` | Encoder入力初期化 | D0/D1/D2 |
| **`initBatteryAdc()`** | **Battery ADC 初期化** | **D3/GPIO4** ✅ |
| **`initMcp23017()`** | **MCP23017 RGB LED初期化** | **I2C 0x20** ✅ |

### 読出し系

| 関数名例 | 役割 | GPIO |
|------|------|------|
| `readRtcToRecord(LogRecord &rec)` | 日付・時刻取得 | I2C 0x68 |
| `readBme280ToRecord(LogRecord &rec)` | 温度・湿度・気圧取得 | I2C 0x76 |
| `readLtr390ToRecord(LogRecord &rec)` | ALS / UVS取得 | I2C 0x53 |
| **`readBatteryToRecord(LogRecord &rec)`** | **`vbat`取得（分圧値×2）** | **D3/GPIO4/ADC1_CH3** ✅ |

### UI系

| 関数名例 | 役割 | GPIO |
|------|------|------|
| `updateEncoder()` | Encoder状態更新 | D0/D1/D2 |
| `handleUiInput()` | 状態遷移処理 | D0/D1/D2 |
| **`setEncoderLed()`** | **RGB LED 制御（MCP23017経由）** | **I2C 0x20・GPA0/1/2** ✅ |
| `drawViewScreen()` | VIEW画面描画 | I2C 0x3C |
| `drawMenuScreen()` | MENU画面描画 | I2C 0x3C |
| `drawClockScreen()` | CLOCK画面描画 | I2C 0x3C |
| `drawLogScreen()` | LOG画面描画 | I2C 0x3C |
| `drawSleepScreen()` | SLEEP画面描画 | I2C 0x3C |

### ログ系

| 関数名例 | 役割 | GPIO |
|------|------|------|
| `buildCsvLine(const LogRecord &rec)` | CSV1行生成 | - |
| `ensureLogFileHeader()` | ヘッダ確認・作成 | D7/D8/D9/D10 |
| `appendLogRecord(const LogRecord &rec)` | 1行追記 | D7/D8/D9/D10 |

### 電源系（DeepSleep対応確定版）✅

| 関数名例 | 役割 | GPIO |
|------|------|------|
| **`prepareForSleep()`** | **Sleep前処理（CS=HIGH・SPI.end()）** | **D7/GPIO44・D6/GPIO43** ✅ |
| **`enterDeepSleep()`** | **Deep Sleep移行（GPIO43→LOW）** | **D6/GPIO43** ✅ |
| `printWakeupReason()` | 起床要因表示 | Serial |

---

## `setup()` / `loop()` の責務

### UI試験版 / periodic logger 版

#### setup()

- Serial開始
- I2C / SPI 初期化
- RTC / BME280 / LTR390 / OLED / SD 初期化
- Encoder初期化（D0/D1/D2）
- **MCP23017初期化（I2C 0x20）**✅
- **Battery ADC初期化（D3/GPIO4）**✅
- 初期状態設定
- ヘッダ確認

#### loop()

- Encoder読取り（D0/D1/D2）
- UI状態遷移
- 周期ごとにセンサ読出し
- 周期ごとに表示更新
- 周期ごとにCSV保存

### Deep Sleep logger 基準版（DeepSleep対応確定版）✅

#### setup()

- Serial開始
- 起床要因確認
- bootCount更新
- I2C / SPI初期化
- RTC / BME280 / LTR390 / **MCP23017** / SD 初期化✅
- **Battery ADC初期化（D3/GPIO4）**✅
- RTC妥当性確認と必要なら復旧
- 測定値取得
- **`vbat`読出し**✅
- CSV保存
- 必要なら短時間表示
- **microSD CS=HIGH・SPI.end()処理**✅
- **GPIO43→LOW で電源遮断・Deep Sleep移行**✅

#### loop()

```cpp
void loop() {
  // Deep Sleep logger 基準版では未使用
}
```

---

## Battery monitor実装構造（確定版）✅

### 回路前提

```text
TP4056 OUT+ -- 100kΩ (R1) --+-- D3(GPIO4)
                            |   ADC1_CH3
                          100kΩ (R2)
                            |
TP4056 OUT- ----------------+-- GND
```

### メイン基板配置

- **R1/R2座標：(13,03)～(13,06)**✅
- **出力：D3(GPIO4)・ADC1_CH3**✅

### 読出し方針

- ADCで読んだ値を **2倍** して `vbat` とする
- 初回はテスター値との一致確認を優先する
- `analogReadMilliVolts()` を優先利用する

### 実装例

```cpp
void readBatteryToRecord(LogRecord &rec) {
  uint32_t vadc_mv = analogReadMilliVolts(PIN_BAT_ADC);  // D3/GPIO4
  rec.vbat = (vadc_mv / 1000.0f) * VBAT_DIV_RATIO;      // ×2
}
```

---

## CSV生成方針（確定版）✅

### 推奨列順

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

### 文字列生成例

```cpp
String buildCsvLine(const LogRecord &rec) {
  String line = "";
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

---

## 実装順序（確定版）✅

1. **ピン定義固定（pins.h）**✅ **D0～D10・D6・D3・D7確定**
2. 定数・enum・struct 定義
3. RTC読出し関数
4. BME280読出し関数
5. LTR390読出し関数
6. **Battery ADC読出し関数（D3/GPIO4）**✅
7. CSV生成関数
8. SD追記関数（D7/GPIO44 CS確定）✅
9. **Deep Sleep logger 基準版完成（GPIO43・R_EN・CS処理確定）**✅
10. periodic logger 版完成
11. UI状態遷移追加
12. **RGB Encoder 入力・LED追加（MCP23017 I2C 0x20）**✅
13. 画面描画整理
14. `context_code` / `head_code` 入力反映
15. `vbat` 追加確認

---

## 初号機で保留とする実装

- クラスベース全面再設計
- 複数ファイルへの過度な細分化
- イベント駆動フレームワーク導入
- FreeRTOS タスク分割
- Wi-Fi時刻同期
- センサ値からの状態自動推定
- 電池残量アルゴリズム本格実装

---

## ステータス

- [ACTIVE] 初号機ソフトウェア実装構造として有効
- [COMPLETE] **電源基板通電試験全項目PASS（3.304V確認）により基盤確保** ✅
- [COMPLETE] **メイン基板配置座標確定（95mm×72mm）** ✅
- [COMPLETE] **GPIO割当確定版（D0～D10・D6・D3・D7）** ✅
- [COMPLETE] **MCP23017採用確定（I2C 0x20・RGB LED制御）** ✅
- [COMPLETE] **Battery voltage monitor確定（D3/GPIO4・ADC1_CH3・100kΩ×2分圧）** ✅
- [COMPLETE] **Deep Sleep対応設計確定（R_EN・GPIO43・microSD CS処理）** ✅
- [ACTIVE] Deep Sleep logger 基準版を基準構造とする
- [ACTIVE] 単一スケッチ + 関数分割方針を採用
- [ACTIVE] `context_code` / `head_code` / `vbat` を含む LogRecord 構造を採用
- [ACTIVE] **RGB LED付きスイッチ付き Rotary Encoder（MCP23017経由）前提へ更新済み**✅
- [IN PROGRESS] **メイン基板実装進行中（分圧/R_EN/MCP23017）**
- [PENDING] `vbat` 実装確認（分圧回路追加後）
- [PENDING] `context_code` / `head_code` 入力UI実装
- [PENDING] **電池駆動での Deep Sleep logger 再確認（メイン基板完了後）**
- [PENDING] 将来のファイル分割は未確定
