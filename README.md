# 02_携帯型環境センサーロガー作成プロジェクト

ESP32系MCUを用いた携帯型環境センサーロガーの設計・実装・評価リポジトリです。  
頭痛発生と環境要因の相関分析を目的として、環境データ・行動情報・状態情報を継続記録します。

---

## 最重要参照先

最新状態は以下を最優先で参照してください。

- `CURRENT_STATUS.md`

READMEはリポジトリ入口・概要文書です。  
詳細仕様、進捗、履歴、判断理由は各専用文書へ分離管理します。

---

## プロジェクト目的

本プロジェクトでは、携帯可能な環境センサーロガーを作成し、頭痛発生前後の環境要因を記録します。

記録対象：

- 温度
- 湿度
- 気圧
- 照度
- UV
- 行動状態
- 頭痛状態
- 電池電圧
- 時刻情報

BME680は搭載しますが、初期運用ではガス測定を無効化し、温度・湿度・気圧センサとして扱います。

---

## 現行基本方針

- 初号機はユニバーサル基板 + 既製モジュールで成立確認を優先する。
- 電源基板と本体基板は分離せず、TP4056 + TPS63802をメイン基板へ統合する。
- MCUは Seeed Studio XIAO ESP32S3 Plus を採用する。
- BME280は故障により廃止扱いとし、秋月AE-BME680へ置換する。
- BME680のガス測定は当面無効化し、ヒーターOFF運用を基本とする。
- I2C外付けプルアップ抵抗は今回実装しない。
- microSDのSPI配線は、基板上の直線配線性を優先してD7〜D10の割当を再配置する。
- MCP23017はRGB LED制御用として使用する。
- 最新状態は `CURRENT_STATUS.md` に集約する。
- 設計判断は `51_DECISIONS/` に保存する。
- 旧仕様・廃止案は `90_HISTORY/` に隔離する。

---

## 現在の主要構成

| 区分 | 採用品 | 接続 | 備考 |
|---|---|---|---|
| MCU | XIAO ESP32S3 Plus | - | 初号機メインMCU |
| 温湿度・気圧 | AE-BME680 | I2C | BME280故障により置換、ガス測定は当面無効 |
| 照度・UV | LTR390 | I2C | JST2接続 |
| RTC | DS3231 + AT24C32 | I2C | 基板搭載 |
| 記録媒体 | microSDモジュール | SPI | D10=CS, D9=MOSI, D8=SCK, D7=MISO |
| 表示 | OLED SSD1306 | I2C | JST1接続 |
| 入力 | RGB Rotary Encoder | GPIO + MCP23017 | 回転・押下はXIAO直結、RGBはMCP23017 |
| GPIO拡張 | MCP23017 | I2C | 0x20認識確認済み |
| 電源 | LiPo + TP4056 + TPS63802 | 統合基板 | TPS63802はEN端子付き |
| 電池監視 | 100kΩ + 100kΩ分圧 | D3(GPIO4) | Battery ADC |

---

## 現行GPIO概要

| XIAO端子 | 用途 |
|---|---|
| D0 | Rotary Encoder A |
| D1 | Rotary Encoder B |
| D2 | Rotary Encoder SW |
| D3 | Battery ADC 分圧中点 |
| D4 | I2C SDA |
| D5 | I2C SCL |
| D6 | TPS63802 EN |
| D7 | SPI MISO |
| D8 | SPI SCK |
| D9 | SPI MOSI |
| D10 | SPI CS |

---

## リポジトリ構成

```text
02_esp32c3sm-bme280-envSensor/
├─ README.md
├─ CURRENT_STATUS.md
├─ CHANGELOG.md
├─ TREE.md
├─ 10_HARDWARE/
│  ├─ 01_HARDWARE_OVERVIEW.md
│  └─ 05_WIRING_DIAGRAM.md
├─ 20_SOFTWARE/
├─ 30_LOG/
├─ 40_DEV/
├─ 51_DECISIONS/
└─ 90_HISTORY/
```

---

## 文書責務

| 領域 | 内容 |
|---|---|
| README.md | リポジトリ入口・概要 |
| CURRENT_STATUS.md | 最新状態の唯一の集約点 |
| CHANGELOG.md | 時系列の変更履歴 |
| TREE.md | 文書構成と配置方針 |
| 10_HARDWARE | 現行ハードウェア仕様 |
| 20_SOFTWARE | 現行ソフトウェア仕様 |
| 30_LOG | 測定・分析・試験記録 |
| 40_DEV | 実装・開発管理 |
| 51_DECISIONS | 設計判断と理由 |
| 90_HISTORY | 廃止仕様・旧履歴 |

---

## 管理方針

- 最新仕様と履歴を混在させない。
- READMEへ詳細仕様を戻しすぎない。
- 変更時は `CURRENT_STATUS.md` と該当仕様文書を更新する。
- ピン割当変更時は `01_HARDWARE_OVERVIEW.md` と `05_WIRING_DIAGRAM.md` を必ず同時更新する。
- 設計変更時は `51_DECISIONS/` に判断理由を追加する。
- 廃止仕様は削除せず `90_HISTORY/` に隔離する。

---

## 注意事項

本リポジトリ内には、過去構想・旧設計・評価途中情報が含まれる可能性があります。  
AI解析・人手確認の双方において、まず `CURRENT_STATUS.md` を参照してください。

