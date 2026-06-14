# 01_HARDWARE_OVERVIEW

## 目的

本ファイルは、携帯型環境センサーロガー初号機のハードウェア構成全体を整理するための概要資料です。  
本プロジェクトでは、頭痛発生と環境要因の相関記録を目的とし、気圧・温度・湿度・照度・UV・行動情報を記録対象とします。

初号機では、専用基板化を急がず、ユニバーサル基板 + 既製モジュール実装を前提に、各機能ブロックを段階的に成立させます。

---

## ハードウェア方針

- MCUは Seeed Studio XIAO ESP32S3 Plus を採用する。
- 電源基板と本体基板を分離せず、TP4056 + TPS63802をメイン基板へ統合する。
- BME280は故障により廃止扱いとし、秋月AE-BME680へ置換する。
- BME680は当面、ガス測定を無効化し、温度・湿度・気圧のみ使用する。
- microSDはSPI接続とし、基板配線性を優先してD7〜D10を再配置する。
- I2C外付けプルアップ抵抗は今回実装しない。
- MCP23017はRGB LED制御用として使用する。
- DeepSleep運用は維持するが、実機安定性確認後に最終判断する。
- 電流測定はDT4256およびUSB-C電流計で実施する。

---

## システム全体構成

本機は、以下の機能ブロックで構成します。

1. MCUブロック
2. 環境センサブロック
3. 時刻保持ブロック
4. 表示ブロック
5. 入力ブロック
6. 記録ブロック
7. 電源ブロック
8. 電池電圧監視ブロック
9. GPIO拡張ブロック

---

## 1. MCUブロック

### 採用品

- Seeed Studio XIAO ESP32S3 Plus

### 役割

- センサ読出し
- RTC時刻取得
- OLED表示
- ロータリーエンコーダ入力
- microSD CSV記録
- Battery ADC読取り
- TPS63802 EN制御
- DeepSleep制御

### GPIO割当

| XIAO端子 | GPIO | 用途 | 備考 |
|---|---:|---|---|
| D0 | GPIO1 | RotaryEncoder_A | 回転入力 |
| D1 | GPIO2 | RotaryEncoder_B | 回転入力 |
| D2 | GPIO3 | RotaryEncoder_SW | 押下入力 |
| D3 | GPIO4 | Battery_ADC | 100kΩ + 100kΩ分圧中点 |
| D4 | GPIO5 | I2C_SDA | 共通I2C |
| D5 | GPIO6 | I2C_SCL | 共通I2C |
| D6 | GPIO43 | TPS63802_EN | EN制御 |
| D7 | GPIO44 | SPI_MISO | microSD |
| D8 | GPIO7 | SPI_SCK | microSD |
| D9 | GPIO8 | SPI_MOSI | microSD |
| D10 | GPIO9 | SPI_CS | microSD |

---

## 2. 環境センサブロック

### AE-BME680

BME280故障により、温湿度・気圧センサをAE-BME680へ置換します。

| 項目 | 内容 |
|---|---|
| 接続 | I2C |
| 電源 | 3.3V |
| 使用機能 | 温度・湿度・気圧 |
| 当面無効 | ガス測定 |
| ヒーター | OFF運用予定 |
| 期待I2Cアドレス | 0x76 または 0x77 |

### LTR390

| 項目 | 内容 |
|---|---|
| 接続 | I2C |
| 電源 | 3.3V |
| 接続先 | JST2 |
| 用途 | 照度・UV |
| 期待I2Cアドレス | 0x53 |

### I2C外付けプルアップ

今回の基板では、I2C外付けプルアップ抵抗を実装しません。  
各I2Cモジュール内蔵プルアップを前提とし、I2Cスキャン不安定時のみ後付け検討とします。

---

## 3. 時刻保持ブロック

### DS3231 + AT24C32

| 項目 | 内容 |
|---|---|
| RTC | DS3231 |
| EEPROM | AT24C32 |
| 接続 | I2C |
| RTCアドレス | 0x68 |
| EEPROMアドレス | 0x57 |
| SQW | 未使用 |
| 32K | 未使用 |

### 運用方針

- RTCをログ時刻の基準とする。
- 電源断後も時刻保持する。
- PCからUSBシリアル経由で時刻調整する。
- DeepSleep復帰後もRTCから時刻を取得する。

---

## 4. 表示ブロック

### OLED SSD1306

| 項目 | 内容 |
|---|---|
| 接続 | I2C |
| 接続先 | JST1 |
| 電源 | 3.3V |
| 期待I2Cアドレス | 0x3C |

### 表示対象

- 時刻
- 温度
- 湿度
- 気圧
- 照度
- UV
- 電池状態
- メニュー状態
- ログ状態

---

## 5. 入力ブロック

### RGB LED付きロータリーエンコーダ

| 信号 | 接続先 |
|---|---|
| Enc_A | XIAO D0 |
| Enc_B | XIAO D1 |
| Enc_SW | XIAO D2 |
| LED_R | MCP23017 GPA2 → 100Ω |
| LED_G | MCP23017 GPA1 → 100Ω |
| LED_B | MCP23017 GPA0 → 100Ω |
| VCC | LED極性により決定 |
| GND | GND共通 |

### 方針

- 回転・押下はXIAOへ直接接続する。
- RGB LEDはMCP23017経由で制御する。
- LED極性は実物確認後に確定する。
- 各LEDラインには100Ω抵抗を入れる。

---

## 6. GPIO拡張ブロック

### MCP23017

| 項目 | 内容 |
|---|---|
| 接続 | I2C |
| アドレス | 0x20 |
| A0/A1/A2 | GND |
| RESET | 10kΩで3.3Vプルアップ |
| VDD | 3.3V |
| VSS | GND |
| バイパス | VDD-VSS間 0.1uF |

### 実機確認

MCP23017は再配線後、I2Cスキャナにより0x20で安定認識済みです。

### GPIO割当

| MCP23017 | 用途 |
|---|---|
| GPA0 | RGB_B |
| GPA1 | RGB_G |
| GPA2 | RGB_R |
| GPA3〜GPA7 | 未使用 |
| GPB0〜GPB7 | 未使用 |
| INTA / INTB | 未使用 |

---

## 7. 記録ブロック

### microSD

| 項目 | 内容 |
|---|---|
| 接続 | SPI |
| 電源 | 3.3V |
| 用途 | CSVログ保存 |
| CS | D10 |
| MOSI | D9 |
| SCK | D8 |
| MISO | D7 |

### SPI再配置の理由

XIAOのD7〜D10物理配列とmicroSDモジュールのピン配列を直線化し、配線交差を避けるため、SPI割当を再配置しました。

### スケッチ定義

```cpp
#define SD_CS    D10
#define SD_MOSI  D9
#define SD_SCK   D8
#define SD_MISO  D7

SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
SD.begin(SD_CS, SPI);
```

---

## 8. 電源ブロック

### 構成

| 部品 | 役割 |
|---|---|
| LiPo | バッテリー |
| TP4056 | 充電・保護 |
| TPS63802 | 3.3V昇降圧 |
| XIAO USB | 書込み・シリアル確認 |
| JST0 | LiPo接続 |

### 電源経路

```text
LiPo JST0
  ↓
TP4056 B+/B-
  ↓
TP4056 OUT+/OUT-
  ↓
TPS63802 VIN/GND
  ↓
TPS63802 VOUT/GND
  ↓
3.3Vバス / GNDバス
```

### TPS63802

| ピン | 接続 |
|---|---|
| EN | XIAO D6 + 100kΩプルダウン |
| VOUT | 3.3Vバス |
| GND | GNDバス |
| VIN | TP4056 OUT+ |

### EN制御

現行方針では、ENノードは100kΩでGNDへプルダウンします。  
D6(GPIO43)からHIGHを出すことでTPS63802を有効化します。

---

## 9. 電池電圧監視ブロック

### 構成

```text
LiPo/TP4056 B+系 ----[100kΩ]----+---- XIAO D3(GPIO4)
                                |
                              [100kΩ]
                                |
GND ----------------------------+
```

### 方針

- 分圧比は1:1。
- ADC入力はD3(GPIO4)。
- 満充電付近でもADC入力は3.3V未満。
- 必要時のみADC中点-GND間へ0.1uFを追加する。

---

## 10. 受動部品一覧

| 項番 | 部品 | 値 | 接続 | 用途 |
|---:|---|---:|---|---|
| 1 | 抵抗 | 100kΩ | LiPo/TP4056 B+系 → Battery ADC中点 | 分圧上側 |
| 2 | 抵抗 | 100kΩ | Battery ADC中点 → GND | 分圧下側 |
| 3 | 抵抗 | 100kΩ | TPS63802 EN → GND | ENプルダウン |
| 4 | 抵抗 | 10kΩ | MCP23017 RESET → 3.3V | RESETプルアップ |
| 5 | 抵抗 | 100Ω | MCP23017 GPA2 → JST3 LED_R | RGB_R |
| 6 | 抵抗 | 100Ω | MCP23017 GPA1 → JST3 LED_G | RGB_G |
| 7 | 抵抗 | 100Ω | MCP23017 GPA0 → JST3 LED_B | RGB_B |
| 8 | コンデンサ | 0.1uF | MCP23017 VDD-VSS間 | バイパス |
| 9 | コンデンサ | 22〜47uF | TPS63802 VOUT-GND間 | 3.3V安定化 |
| 10 | コンデンサ | 0.1uF | XIAO 3V3-GND間 | デカップリング |
| 11 | コンデンサ | 10uF | XIAO 3V3-GND間 | バルク |
| 12 | コンデンサ | 0.1uF | I2C 3.3Vバス近傍-GND間 | 任意安定化 |

---

## 11. 2026-06-14版配置

### 基板

- 横X: 31穴
- 縦Y: 26穴
- 短辺両端にバス用ランドあり

### 部品配置

| 部品 | FootPrint |
|---|---|
| TP4056 | (01,01)～(06,12) |
| TPS63802 | (01,16)～(04,23) |
| XIAO ESP32S3 Plus | (17,02)～(23,08) |
| DS3231 + AT24 | (17,12)～(31,20) |
| microSD | (23,04)～(31,09) |
| BME680 | (28,22)～(31,25) |
| MCP23017 | (09,02)～(12,15) |
| JST1 OLED | (25,22)～(25,25) |
| JST2 LTR390 | (22,22)～(22,25) |
| JST3 RotaryEncoder | (14,02)～(14,09) |

---

## 12. Bring-up方針

1. 導通確認
2. USB給電で3.3V確認
3. XIAO起動確認
4. MCP23017認識確認
5. MCP23017 GPIO出力確認
6. DS3231 + AT24確認
7. BME680確認
8. microSD確認
9. Battery ADC確認
10. TPS63802 EN確認
11. OLED確認
12. LTR390確認
13. RotaryEncoder確認
14. LiPo運用確認
15. 消費電流測定

---

## 13. 本ファイルの位置付け

本ファイルは、ハードウェア全体像を整理する概要ファイルです。  
配線詳細は `05_WIRING_DIAGRAM.md`、進捗は `CURRENT_STATUS.md` を参照してください。

---

## 14. ステータス

- [COMPLETE] XIAO ESP32S3 Plus採用確定
- [COMPLETE] BME680置換確定
- [COMPLETE] TP4056 + TPS63802統合基板方針確定
- [COMPLETE] microSD SPI再配置確定
- [COMPLETE] MCP23017採用確定
- [COMPLETE] MCP23017 I2C 0x20認識確認済み
- [ACTIVE] 統合基板実装中
- [PENDING] microSD新SPI割当確認
- [PENDING] BME680実機確認
- [PENDING] LiPo駆動統合試験

