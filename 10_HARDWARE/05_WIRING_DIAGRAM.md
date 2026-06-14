# 05_WIRING_DIAGRAM

## 目的

本ファイルは、携帯型環境センサーロガー初号機における配線方針と、各モジュール間の接続関係を整理するためのものです。  
CADによる正式回路図ではなく、ユニバーサル基板実装時の基準となる配線単位の接続表として扱います。

---

## 前提

- MCUは XIAO ESP32S3 Plus。
- 基板は2026-06-14版 Env-Sensor Mappingを基準とする。
- 電源基板とメイン基板は分離せず、TP4056 + TPS63802をメイン基板へ統合する。
- BME280は使用せず、AE-BME680を使用する。
- BME680のガス測定は当面無効とする。
- I2C外付けプルアップ抵抗は今回実装しない。
- microSDのSPI配線はD7〜D10再配置版とする。
- MCP23017は0x20で認識確認済み。

---

## 1. 全体配線系統

配線は以下に分けて管理します。

1. 電源系
2. XIAO GPIO
3. I2Cバス
4. SPI microSD
5. MCP23017
6. RotaryEncoder / RGB LED
7. Battery ADC
8. JST外部接続

---

## 2. XIAO ESP32S3 Plus 配線

| XIAO端子 | 座標 | 信号 | 接続先 |
|---|---|---|---|
| D0 | (17,02) | RotaryEncoder_A | JST3 Enc_A |
| D1 | (17,03) | RotaryEncoder_B | JST3 Enc_B |
| D2 | (17,04) | RotaryEncoder_SW | JST3 Enc_SW |
| D3 | (17,05) | Battery_ADC | 分圧中点 |
| D4 | (17,06) | I2C_SDA | I2C SDAバス |
| D5 | (17,07) | I2C_SCL | I2C SCLバス |
| D6 | (17,08) | TPS63802_EN | TPS63802 EN |
| D7 | (23,08) | SPI_MISO | microSD MISO |
| D8 | (23,07) | SPI_SCK | microSD CLK |
| D9 | (23,06) | SPI_MOSI | microSD MOSI |
| D10 | (23,05) | SPI_CS | microSD CS |
| 3V3 | (23,04) | 3.3V | 3.3Vバス |
| GND | (23,03) | GND | GNDバス |
| VUSB | (23,02) | 未使用 | 原則未接続 |

---

## 3. 電源系配線

### 3.1 TP4056

| TP4056端子 | 座標 | 接続先 |
|---|---|---|
| IN- | (01,02) | USB充電入力GND |
| IN+ | (06,02) | USB充電入力5V |
| OUT- | (01,11) | GNDバス / TPS63802 GND |
| B- | (02,11) | JST0 B- |
| B+ | (05,11) | JST0 B+ |
| OUT+ | (06,11) | TPS63802 VIN / Battery ADC分圧上側 |

### 3.2 JST0 LiPo

| JST0端子 | 座標 | 接続先 |
|---|---|---|
| B- | (03,14) | TP4056 B- |
| B+ | (04,14) | TP4056 B+ |

### 3.3 TPS63802

| TPS63802端子 | 座標 | 接続先 |
|---|---|---|
| EN | (01,23) | XIAO D6 / 100kΩ → GND |
| VOUT | (02,23) | 3.3Vバス / 22〜47uF → GND |
| GND | (03,23) | GNDバス |
| VIN | (04,23) | TP4056 OUT+ |

### 3.4 電源安定化部品

| 部品 | 値 | 接続 |
|---|---:|---|
| C_TPS_OUT | 22〜47uF | TPS63802 VOUT-GND間 |
| C_XIAO_1 | 0.1uF | XIAO 3V3-GND間 |
| C_XIAO_2 | 10uF | XIAO 3V3-GND間 |

---

## 4. Battery ADC配線

### 回路

```text
TP4056 OUT+ / LiPo B+系 ----[R1 100kΩ]----+---- XIAO D3(GPIO4)
                                          |
                                        [R2 100kΩ]
                                          |
GND --------------------------------------+
```

### 接続表

| 項番 | 接続元 | 接続先 | 値 |
|---:|---|---|---:|
| B-1 | TP4056 OUT+ / LiPo B+系 | R1上端 | 100kΩ |
| B-2 | R1下端 | XIAO D3(GPIO4) | - |
| B-3 | XIAO D3(GPIO4) | R2上端 | - |
| B-4 | R2下端 | GND | 100kΩ |

### 注意

- TP4056 OUT+をD3へ直結しない。
- ADC中点の測定は必ずGND基準で行う。
- 必要時のみADC中点-GND間に0.1uFを追加する。

---

## 5. I2Cバス配線

### I2Cマスター

| XIAO | GPIO | 信号 |
|---|---:|---|
| D4 | GPIO5 | SDA |
| D5 | GPIO6 | SCL |

### I2C接続対象

| デバイス | SDA | SCL | VCC | GND | アドレス |
|---|---|---|---|---|---|
| MCP23017 | D4 | D5 | 3.3V | GND | 0x20 |
| DS3231 | D4 | D5 | 3.3V | GND | 0x68 |
| AT24C32 | D4 | D5 | 3.3V | GND | 0x57 |
| BME680 | D4 | D5 | 3.3V | GND | 0x76/0x77 |
| OLED SSD1306 | D4 | D5 | 3.3V | GND | 0x3C |
| LTR390 | D4 | D5 | 3.3V | GND | 0x53 |

### I2C外付けプルアップ

今回なし。

理由：

- 各モジュール内蔵プルアップ前提。
- 外付けを追加すると並列合成で抵抗値が下がりすぎる可能性がある。
- 不安定時のみ後付け検討とする。

---

## 6. MCP23017配線

### ピン配線

| MCP23017 | 座標 | 接続先 | 備考 |
|---|---|---|---|
| GPA7 | (09,02) | 未使用 | - |
| GPA6 | (09,03) | 未使用 | - |
| GPA5 | (09,04) | 未使用 | - |
| GPA4 | (09,05) | 未使用 | - |
| GPA3 | (09,06) | 未使用 | - |
| GPA2 | (09,07) | 100Ω → JST3 LED_R | RGB_R |
| GPA1 | (09,08) | 100Ω → JST3 LED_G | RGB_G |
| GPA0 | (09,09) | 100Ω → JST3 LED_B | RGB_B |
| INTA | (09,10) | 未使用 | - |
| INTB | (09,11) | 未使用 | - |
| RESET | (09,12) | 10kΩ → 3.3V | RESETプルアップ |
| A2 | (09,13) | GND | アドレス設定 |
| A1 | (09,14) | GND | アドレス設定 |
| A0 | (09,15) | GND | アドレス設定 |
| GPB0 | (12,02) | 未使用 | - |
| GPB1 | (12,03) | 未使用 | - |
| GPB2 | (12,04) | 未使用 | - |
| GPB3 | (12,05) | 未使用 | - |
| GPB4 | (12,06) | 未使用 | - |
| GPB5 | (12,07) | 未使用 | - |
| GPB6 | (12,08) | 未使用 | - |
| GPB7 | (12,09) | 未使用 | - |
| VDD | (12,10) | 3.3V | 0.1uFをVSS間に配置 |
| VSS | (12,11) | GND | - |
| NC | (12,12) | 未使用 | - |
| SCK/SCL | (12,13) | XIAO D5 | I2C SCL |
| SDA | (12,14) | XIAO D4 | I2C SDA |
| NC | (12,15) | 未使用 | - |

### アドレス

A0/A1/A2をGNDに接続するため、MCP23017のI2Cアドレスは `0x20` です。

### 確認済み結果

```text
I2C device found at 0x20
Found devices: 1
```

---

## 7. DS3231 + AT24C32配線

| 端子 | 座標 | 接続先 |
|---|---|---|
| GND | (17,14) | GND |
| VCC | (17,15) | 3.3V |
| SDA | (17,16) | XIAO D4 |
| SCL | (17,17) | XIAO D5 |
| SQW | (17,18) | 未使用 |
| 32K | (17,19) | 未使用 |

期待アドレス：

- DS3231: 0x68
- AT24C32: 0x57

---

## 8. microSD配線

### 現行割当

| microSD端子 | 座標 | XIAO接続 | 備考 |
|---|---|---|---|
| 3V3 | (25,04) | 3.3V | 電源 |
| CS | (25,05) | D10 | SPI_CS |
| MOSI | (25,06) | D9 | SPI_MOSI |
| CLK | (25,07) | D8 | SPI_SCK |
| MISO | (25,08) | D7 | SPI_MISO |
| GND | (25,09) | GND | GND |

### Arduino定義

```cpp
#define SD_CS    D10
#define SD_MOSI  D9
#define SD_SCK   D8
#define SD_MISO  D7

SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

if (!SD.begin(SD_CS, SPI)) {
  Serial.println("SD init failed");
} else {
  Serial.println("SD init OK");
}
```

### 注意

- `SCL`ではなく`SCK`または`CLK`表記へ統一する。
- 配線は短くする。
- CSは起動時HIGHを基本とする。
- DeepSleep前はCS=HIGH、SPI.end()を実行する。

---

## 9. BME680配線

| BME680端子 | 座標 | 接続先 |
|---|---|---|
| VIN | (28,22) | 3.3V |
| SDA | (28,23) | XIAO D4 |
| SCL | (28,24) | XIAO D5 |
| GND | (28,25) | GND |

方針：

- 温度・湿度・気圧のみ使用。
- ガス測定は当面無効。
- ヒーターOFF運用を基本とする。
- I2Cアドレスは実機スキャンで0x76/0x77を確認する。

---

## 10. OLED JST1配線

| JST1端子 | 座標 | 接続先 |
|---|---|---|
| VIN | (25,22) | 3.3V |
| SDA | (25,23) | XIAO D4 |
| SCL | (25,24) | XIAO D5 |
| GND | (25,25) | GND |

期待アドレス：

- SSD1306: 0x3C

---

## 11. LTR390 JST2配線

| JST2端子 | 座標 | 接続先 |
|---|---|---|
| VIN | (22,22) | 3.3V |
| SDA | (22,23) | XIAO D4 |
| SCL | (22,24) | XIAO D5 |
| GND | (22,25) | GND |

期待アドレス：

- LTR390: 0x53

---

## 12. RotaryEncoder JST3配線

| JST3端子 | 座標 | 接続先 |
|---|---|---|
| GND | (14,02) | GND |
| VCC | (14,03) | 3.3VまたはLED共通端子 | LED極性に依存 |
| Enc_A | (14,04) | XIAO D0 |
| Enc_B | (14,05) | XIAO D1 |
| Enc_SW | (14,06) | XIAO D2 |
| LED_R | (14,07) | MCP23017 GPA2 → 100Ω |
| LED_G | (14,08) | MCP23017 GPA1 → 100Ω |
| LED_B | (14,09) | MCP23017 GPA0 → 100Ω |

注意：

- LED共通端子の極性は実物確認する。
- 共通カソードならVCCではなくGND共通となる可能性がある。
- LED点灯試験前にテスターで極性確認する。

---

## 13. 受動部品配線

| 項番 | 部品 | 値 | 接続元 | 接続先 | 用途 |
|---:|---|---:|---|---|---|
| 1 | R_BAT_H | 100kΩ | LiPo/TP4056 B+系 | D3中点 | 分圧上側 |
| 2 | R_BAT_L | 100kΩ | D3中点 | GND | 分圧下側 |
| 3 | R_EN_PD | 100kΩ | TPS63802 EN | GND | ENプルダウン |
| 4 | R_RESET | 10kΩ | MCP23017 RESET | 3.3V | RESETプルアップ |
| 5 | R_LED_R | 100Ω | MCP23017 GPA2 | JST3 LED_R | 電流制限 |
| 6 | R_LED_G | 100Ω | MCP23017 GPA1 | JST3 LED_G | 電流制限 |
| 7 | R_LED_B | 100Ω | MCP23017 GPA0 | JST3 LED_B | 電流制限 |
| 8 | C_MCP | 0.1uF | MCP23017 VDD | MCP23017 VSS | バイパス |
| 9 | C_TPS_OUT | 22〜47uF | TPS63802 VOUT | GND | 出力安定化 |
| 10 | C_XIAO_DECAP | 0.1uF | XIAO 3V3 | GND | デカップリング |
| 11 | C_XIAO_BULK | 10uF | XIAO 3V3 | GND | バルク |
| 12 | C_I2C | 0.1uF | I2C 3.3Vバス近傍 | GND | 任意安定化 |

---

## 14. 導通確認項目

通電前に以下を確認します。

| 項目 | 期待結果 |
|---|---|
| 3.3V-GND間 | 短絡なし |
| LiPo B+-B-間 | 短絡なし |
| TP4056 OUT+-OUT-間 | 短絡なし |
| TPS63802 VOUT-GND間 | 短絡なし |
| XIAO 3V3-GND間 | 短絡なし |
| MCP23017 VDD-GND間 | 短絡なし |
| SDA-GND間 | 短絡なし |
| SCL-GND間 | 短絡なし |
| SPI各線-GND間 | 短絡なし |

---

## 15. Bring-up試験順序

1. LiPo未接続で導通確認
2. USB給電でXIAO起動
3. 3.3V測定
4. MCP23017単体I2Cスキャン
5. MCP23017 GPIO出力確認
6. DS3231 + AT24C32追加
7. I2Cスキャン
8. BME680追加
9. I2Cスキャン
10. microSD追加
11. SD認識確認
12. Battery ADC確認
13. TPS63802 EN確認
14. OLED接続
15. LTR390接続
16. RotaryEncoder接続
17. LiPo駆動確認
18. USB-C電流計・DT4256による消費電流測定

---

## 16. 期待I2Cスキャン結果

### MCP23017のみ

```text
0x20
```

### DS3231追加後

```text
0x20
0x57
0x68
```

### BME680追加後

```text
0x20
0x57
0x68
0x76 または 0x77
```

### OLED/LTR390追加後

```text
0x20
0x3C
0x53
0x57
0x68
0x76 または 0x77
```

---

## 17. 本ファイルの位置付け

本ファイルはユニバーサル基板実装用の配線基準です。  
ハードウェア全体像は `01_HARDWARE_OVERVIEW.md`、進捗は `CURRENT_STATUS.md` を参照してください。

---

## 18. ステータス

- [COMPLETE] D7〜D10 SPI再配置反映
- [COMPLETE] 統合基板構成反映
- [COMPLETE] MCP23017配線反映
- [COMPLETE] BME680置換反映
- [COMPLETE] 2026-06-14版マッピング反映
- [COMPLETE] I2C外付けプルアップ不採用反映
- [ACTIVE] 実装・Bring-up中

