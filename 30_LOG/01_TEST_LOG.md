# 01_試験ログ

本ファイルは、動作確認・試験結果を管理する文書です。  
現行仕様は `CURRENT_STATUS.md`、部品表は `10_HARDWARE/02_PARTS_LIST.md` を参照してください。

---

## 1. 試験ログ管理方針

- 実施済み試験と結果を記録する。
- 未実施項目は予定として記録する。
- 仕様や採用理由は記載しない。
- 旧仕様の試験結果は、必要に応じて `90_HISTORY/` にも退避する。

---

## 2. 実施済み試験

### 2.1 XIAO ESP32S3 Plus

| 項目 | 結果 |
|---|---|
| USB接続COMポート認識 | PASS |
| スケッチ書込み | PASS |
| USB CDC On Boot / シリアル出力 | PASS |
| Lチカ | PASS |
| Deep Sleep周期起床 | PASS |
| LiPo駆動Blink | PASS |

### 2.2 旧BME280

| 項目 | 結果 | 現在の扱い |
|---|---|---|
| I2Cアドレス認識 | PASS | 旧仕様 |
| 温度・湿度・気圧読出し | PASS | 旧仕様 |
| RTC + microSD統合CSV保存 | PASS | 旧仕様 |
| Deep Sleep復帰後再読出し | PASS | 旧仕様 |
| その後の扱い | 故障判明 | 廃止 |

### 2.3 LTR390

| 項目 | 結果 |
|---|---|
| I2Cアドレス 0x53 認識 | PASS |
| ALS読出し | PASS |
| UVS読出し | PASS |
| ALS/UVS切替 | PASS |
| CSV保存 | PASS |
| Deep Sleep復帰後取得 | PASS |

### 2.4 OLED SSD1306

| 項目 | 結果 |
|---|---|
| I2Cアドレス 0x3C 認識 | PASS |
| 初期化 | PASS |
| 文字表示 | PASS |
| センサ値表示 | PASS |
| メイン画面レイアウト | PASS |

### 2.5 DS3231

| 項目 | 結果 |
|---|---|
| I2Cアドレス 0x68 認識 | PASS |
| EEPROM 0x57 認識 | PASS |
| 時刻読出し | PASS |
| 任意時刻書込み | PASS |
| 再起動後保持 | PASS |
| lostPower()検出時復旧 | PASS |

### 2.6 microSD

| 項目 | 結果 |
|---|---|
| SPI初期化 | PASS |
| カード認識 | PASS |
| ファイル作成 | PASS |
| CSV追記 | PASS |
| Deep Sleep周期起床ロガー | PASS |
| SD.begin低速初期化安定化 | PASS |

### 2.7 電源基板

| 項目 | 結果 |
|---|---|
| LiPo単体外観確認 | PASS |
| LiPo単体電圧確認 | PASS |
| TP4056充電動作 | PASS |
| AE-TPS63802 3.3V出力 | PASS |
| 無負荷3.3V出力 | PASS |
| USB-C単独給電 | PASS |
| LiPo単独給電 | PASS |
| USB-C + LiPo同時 | PASS |
| GND-3.3V短絡確認 | PASS |
| RAW-3.3V短絡確認 | PASS |

---

## 3. 未実施試験

| 項目 | 状態 |
|---|---|
| BME680 I2Cアドレス確認 | 未実施 |
| BME680温度・湿度・気圧読出し | 未実施 |
| BME680ガス値読出し | 未実施 |
| BME680ヒーターOFF運用 | 未実施 |
| BME680 Deep Sleep復帰後再初期化 | 未実施 |
| MCP23017 I2C統合 | 未実施 |
| AE-TPS63802 EN制御遮断 | 未実施 |
| Battery voltage monitor ADC校正 | 未実施 |
| LiPo駆動統合ロガー | 未実施 |
| 完成後消費電流測定 | 未実施 |
