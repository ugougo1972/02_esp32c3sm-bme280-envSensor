# 02_携帯型環境センサーロガー作成プロジェクト

ESP32系MCUを用いた携帯型環境センサーロガーの設計・実装・評価リポジトリです。

頭痛発生と環境要因の相関分析を目的として、環境データ・行動情報・状態情報を継続記録します。

---

## 最重要参照先

最新状態は以下を最優先で参照してください。

- `CURRENT_STATUS.md`

READMEは概要・入口文書です。  
詳細仕様、進捗、履歴、判断理由は各専用文書へ分離管理します。

---

## プロジェクト目的

記録対象：

- 温度
- 湿度
- 気圧
- ガス抵抗値または空気質関連値
- 照度
- UV
- 行動状態
- 頭痛状態
- 電池電圧
- 時刻情報

---

## 基本方針

- 初号機はユニバーサル基板＋既製モジュールで成立確認を優先
- 電源系とメイン系を2基板へ分離
- Deep Sleep中心の低消費電力運用
- README肥大化を避け、責務分離を維持
- 最新状態は `CURRENT_STATUS.md` に集約
- 決定理由を `50_DECISIONS/` に保存
- 旧仕様・廃止案を `90_HISTORY/` に隔離

---

## リポジトリ構成

```text
02_esp32c3sm-bme280-envSensor/
├─ README.md
├─ CURRENT_STATUS.md
├─ CHANGELOG.md
├─ TREE.md
├─ 10_HARDWARE/
├─ 20_SOFTWARE/
├─ 30_LOG/
├─ 40_DEV/
├─ 50_DECISIONS/
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
| 50_DECISIONS | 設計判断と理由 |
| 90_HISTORY | 廃止仕様・旧履歴 |

---

## 現在の主要構成

- MCU: Seeed Studio XIAO ESP32S3 Plus
- RTC: DS3231
- 環境センサ: BME680 / LTR390
- 記録: microSD
- UI: OLED SSD1306
- 入力: RGB LED付きロータリーエンコーダ
- GPIO拡張: MCP23017
- 電源: LiPo + TP4056 + AE-TPS63802

詳細は `CURRENT_STATUS.md` および `10_HARDWARE/` を参照してください。

---

## 管理方針

- 最新仕様と履歴を混在させない
- READMEへ詳細仕様を戻さない
- 変更時は `CURRENT_STATUS.md` と該当仕様文書を更新する
- 設計変更時は `50_DECISIONS/` に判断理由を追加する
- 廃止仕様は削除せず `90_HISTORY/` に隔離する

---

## 注意事項

本リポジトリ内には、過去構想・旧設計・評価途中情報が含まれる可能性があります。

AI解析・人手確認の双方において、まず `CURRENT_STATUS.md` を参照してください。
