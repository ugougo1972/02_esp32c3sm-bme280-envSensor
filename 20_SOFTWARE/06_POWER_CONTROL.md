# 06_POWER_CONTROL

## 目的

本ファイルは、携帯型環境センサーロガー初号機における電源制御および低消費電力動作の方針を整理するためのものである。  
初号機では、**まず Deep Sleep を用いた周期記録が成立すること** を最優先とし、厳密な消費電流最適化は完成後評価とする。

---

## 基本方針

- 動作モードは **USB給電試験モード** と **電池駆動想定モード** を分けて考える
- 現時点の基準点は **Deep Sleep logger 基準版**
- Deep Sleep logger は **setup() 完結型** を前提とする
- 起床ごとに RTC / BME280 / LTR390 / microSD を再初期化する
- Wi-Fi は通常運用で使用しない
- Bluetooth は通常運用で使用しない
- OLED は常時点灯せず、必要時のみ表示する
- 電流測定は完成まで実施しない
- 電池寿命の厳密見積りも後段で行う
- **電源経路は LiPo → TP4056 → AE-TPS63802 → XIAO を採用基準**✅ **確定**
  - **AE-TPS63802 実装完了**（XC9306から変更）✅
  - **EN端子付き（GPIO43制御）でDeep Sleep時の完全電源遮断が可能**✅ **確定**
  - **通電試験全項目PASS（3.3V=3.304V確認）**✅ **確定**
- **GPIO割当確定版**（**D6/GPIO43・D3/GPIO4・D7/GPIO44**）✅ を採用基準とする
- **メイン基板レイアウト確定版**（**95mm×72mm・配置座標確定**）✅ を採用基準とする
- 初号機では **電源基板** と **メイン基板** を分けて考える
- **Battery voltage monitor は D3(GPIO4)・ADC1_CH3・100kΩ×2分圧回路で確定**✅

---

## 電源制御の対象

本ファイルで扱う電源制御対象は以下とする。

| 項番 | 対象 | 内容 | GPIO |
|------|------|------|------|
| P-01 | MCU動作モード | Active / Deep Sleep | **D6(GPIO43)** ✅ |
| P-02 | OLED表示制御 | 点灯 / 消灯 | I2C |
| P-03 | センサ動作制御 | 読出し時のみ使用を基本 | I2C |
| P-04 | ログ保存制御 | 必要時のみ microSD 初期化・保存 | **D7(GPIO44) CS** ✅ |
| P-05 | 起床周期制御 | Timer Wakeup | - |
| P-06 | **Battery voltage monitor** | **分圧回路 + ADC（D3/GPIO4）** | **D3(GPIO4)** ✅ |
| P-07 | Encoder RGB LED | **UI状態表示（MCP23017経由）** | **I2C 0x20** ✅ |

---

## 動作モード定義

### 1. USB給電試験モード

開発・切り分け・シリアル確認を優先するモード。

| 項目 | 方針 |
|------|------|
| 給電 | USB（**安定供給確認済み**）✅ |
| Serial Monitor | 常用 |
| logger | periodic logger / UI試験版 |
| Deep Sleep | 必要時のみ使用 |
| OLED | 確認用に表示可 |
| 優先事項 | 動作可視化・切り分け |

### 2. 電池駆動想定モード

低消費電力運用を想定したモード。

| 項目 | 方針 |
|------|------|
| 給電 | **LiPo + TP4056 + AE-TPS63802**✅ **確定** |
| Serial Monitor | 常用しない |
| logger | Deep Sleep logger |
| Deep Sleep | **基本動作（GPIO43制御）**✅ **確定** |
| OLED | 必要時のみ短時間点灯 |
| 優先事項 | 周期記録成立 |

---

## 電源系成立状況

### 確認済み（全項目PASS）✅

| 項目 | 値 | 状態 |
|------|------|------|
| **LiPo単体電圧** | **4.002V** | ✅ 安定 |
| **TP4056 OUT電圧** | **3.7V～4.0V** | ✅ 正常 |
| **AE-TPS63802 VOUT（無負荷）** | **3.304V** | ✅ 安定 |
| **AE-TPS63802 VOUT（USB-C単独）** | **3.304V** | ✅ 安定 |
| **AE-TPS63802 VOUT（LiPo単独）** | **3.304V** | ✅ 安定 |
| **AE-TPS63802 VOUT（USB-C+LiPo）** | **3.304V** | ✅ 安定 |
| **XIAO 3V3-GND** | **3.288V** | ✅ 安定 |
| **電源基板GND-3.3V短絡** | **なし** | ✅ 確認済み |
| **LiPo駆動 Blink** | **正常動作** | ✅ PASS |

### 未確認

- **電池駆動での Deep Sleep logger 成立（メイン基板実装完了待ち）**🔄
- **AE-TPS63802 EN端子（GPIO43）制御によるシャットダウン確認**🔄 **配置確定**
- **`vbat` 実装後の ADC安定性確認**🔄
- 完成後の実消費電流

---

## Deep Sleep方針（GPIO43・R_EN確定版）✅

### 基本方針

- 1回の起床ごとに必要な処理だけ実行する
- 処理完了後は速やかに Deep Sleep に入る
- **Deep Sleep前に microSD CS=HIGH・SPI.end()を実施する**✅ **確定**
- **GPIO43を LOW で 3.3V電源遮断**✅ **確定**
- Deep Sleep 中の USB CDC 切断は異常扱いしない
- 成立判定は **Serial 出力より CSV追記結果を優先** する

### Deep Sleep 1サイクル（GPIO43制御対応版）✅

| 順序 | 処理 | 内容 | GPIO |
|------|------|------|------|
| D-01 | Wakeup | **Timer で起床・GPIO43確認** | **D6(GPIO43)** ✅ |
| D-02 | 初期化 | RTC / BME280 / LTR390 / microSD初期化 | I2C・SPI |
| D-03 | RTC確認 | 現在時刻取得、lostPower / 妥当性確認 | I2C |
| D-04 | 測定 | BME280 / LTR390 読出し | I2C |
| D-05 | Battery | **`vbat`読出し（分圧値×2）** | **D3(GPIO4)** ✅ |
| D-06 | 保存 | **CSV生成、microSD追記（CS=HIGH確認）** | **D7(GPIO44)** ✅ |
| D-07 | 表示 | 必要なら短時間表示 | I2C |
| D-08 | Pre-Sleep | **CS=HIGH・SPI.end()・GPIO43→LOW準備** | **D7・D6** ✅ |
| D-09 | Deep Sleep | `esp_deep_sleep_start()`・**GPIO43→LOW** | **D6(GPIO43)** ✅ |

### Deep Sleep基準実績

| 項目 | 状況 |
|------|------|
| 30秒周期 | PASS ✅ |
| 60秒周期 | PASS ✅ |
| wakeupCause 初回 | OTHER ✅ |
| wakeupCause 以後 | TIMER ✅ |
| bootCount | 連続増加確認済み ✅ |
| CSV追記 | 継続確認済み ✅ |
| **GPIO43制御（復帰時HIGH確認）** | **設計確定待ち** 🔄 |
| **R_EN(100kΩ)プルアップ対応** | **座標(12,08)～(14,08)確定** ✅ |

### 電池駆動での扱い

- LiPo駆動で **Blink** は成立済み ✅
- ただし、Deep Sleep logger をそのまま **電池駆動で成立したとはまだ扱わない**
- 次工程で **LiPo電源のまま Deep Sleep logger を再確認** する（メイン基板実装完了後）🔄

---

## 起床後再初期化方針

Deep Sleep 復帰後は、前回動作状態を保持前提にせず、毎回再初期化する。

### 再初期化対象

| 項番 | 対象 | 理由 | GPIO |
|------|------|------|------|
| I-01 | Wire / I2C | 通信再開のため | D4/D5 |
| I-02 | RTC (DS3231) | 時刻取得のため | I2C |
| I-03 | BME280 | 測定再開のため | I2C |
| I-04 | LTR390 | ALS / UVS 読出しのため | I2C |
| I-05 | **MCP23017** | **RGB LED初期化のため** | **I2C 0x20** ✅ |
| I-06 | SPI | microSD使用前に再設定 | D7/D8/D9/D10 |
| I-07 | microSD | 保存処理のため | D7(CS確定) |
| I-08 | **Battery ADC** | **`vbat`実装後に追加** | **D3(GPIO4)** ✅ |
| I-09 | OLED | 必要時のみ再初期化 | I2C |

### 再初期化順序（推奨・GPIO43対応版）✅

1. **起床確認・GPIO43→HIGH**（3.3V復帰）✅
2. 起床要因取得
3. I2C初期化
4. RTC初期化
5. RTC時刻妥当性確認
6. BME280初期化
7. LTR390初期化
8. **MCP23017初期化**✅
9. SPI初期化
10. microSD初期化
11. **Battery ADC準備（D3/GPIO4）**✅
12. 必要時のみ OLED初期化
13. 測定・保存
14. **microSD CS=HIGH・SPI.end()準備**✅
15. **GPIO43→LOW で電源遮断**✅
16. Deep Sleep 再移行

---

## RTC と時刻復旧方針

### 基本方針

- RTC は DS3231 を基準時刻源とする
- Wi-Fi による常時計時同期は行わない
- RTC調整は **PC → USBシリアル → MCU → DS3231** で行う

### 異常時処理

| 状態 | 処理 |
|------|------|
| `lostPower()` 検出 | 自動再設定対象 |
| 不正時刻検出 | 自動再設定対象 |
| `2000-01-01` 系時刻 | 無効扱い |
| 復旧不可 | 無効データ扱いで記録または停止を検討 |

---

## OLED制御方針

### 基本方針

- OLED は **常時点灯しない**
- 通常運用では、表示は短時間確認用とする
- Deep Sleep logger 基準版では、表示なし運用も許容する

### OLED表示ルール

| 状態 | OLED動作 |
|------|----------|
| USB給電試験中 | 表示あり可 |
| UI試験中 | 表示あり |
| periodic logger | 必要に応じ表示 |
| **Deep Sleep logger** | **原則短時間のみ（GPIO43制御対応）**✅ |
| **Deep Sleep直前** | **消灯→CS=HIGH・SPI.end()→GPIO43→LOW** ✅ |
| Deep Sleep 中 | 消灯（電源遮断） |

---

## センサ制御方針

### BME280

- 起床後に初期化
- 測定時に読出し
- 読出し後は保持不要

### LTR390

- 起床後に初期化
- ALS / UVS 切替時は待ち時間を入れる
- 読出し後は保持不要

### DS3231

- 起床後に読出し
- 必要時のみ書込み
- 通常は時刻取得専用

### microSD

- **保存前に初期化確認（D7/GPIO44 CS確定）**✅
- **Deep Sleep前に CS=HIGH・SPI.end()実施**✅ **確定**
- 電池駆動時の安定性は今後確認🔄

---

## microSD初期化方針（GPIO44 CS確定版）✅

### 基本方針

- Deep Sleep 復帰後は毎回初期化する
- `SD.begin()` の安定性を優先する
- 必要時は低速初期化を使用する

### 安定化手順候補（D7/GPIO44対応）✅

1. **CS(D7/GPIO44)を High に確定**✅
2. `SPI.begin(...)`
3. `SD.begin(PIN_SPI_CS, SPI, 1000000)` を試す
4. **Deep Sleep前に SPI.end()実施**✅

### Deep Sleep前処理（必須）✅

```cpp
// Deep Sleep移行直前
pinMode(PIN_SPI_CS, OUTPUT);      // D7(GPIO44)
digitalWrite(PIN_SPI_CS, HIGH);   // CS=HIGH固定
SPI.end();                          // SPI終了
```

---

## Battery voltage monitor方針（D3/GPIO4・100kΩ×2分圧確定版）✅

### 回路前提（確定版）✅

| 項目 | 仕様 |
|------|------|
| **測定対象** | **TP4056 `OUT+ / OUT-`** ✅ |
| **分圧回路** | **100kΩ + 100kΩ（1:1）** ✅ |
| **ADC入力** | **D3(GPIO4)・ADC1_CH3** ✅ |
| **メイン基板配置** | **(13,03)～(13,06)** ✅ |
| **ADC中点コンデンサー** | **必要時のみ 0.1µF** |

### ソフト方針

- **`vbat`未実装時は `0`**✅
- **実装後はテスター値と比較して補正確認**✅
- 低電圧警告や残量換算は後段

### 読出し実装例（D3/GPIO4対応）✅

```cpp
void readBatteryToRecord(LogRecord &rec) {
  uint32_t vadc_mv = analogReadMilliVolts(PIN_BAT_ADC);  // D3(GPIO4)
  rec.vbat = (vadc_mv / 1000.0f) * 2.0f;                  // 分圧値×2
}
```

---

## RGB LED制御方針（MCP23017確定版）✅

### 基本方針

- **RGB LED は MCP23017(I2C 0x20)経由で制御**✅ **確定**
- **GPA0=R / GPA1=G / GPA2=B**✅
- UI状態表示候補（詳細ルールは未確定）
- 常時点灯前提にしない

### LED点灯パターン（設計例）

- VIEW画面：Green
- MENU画面：Yellow
- CLOCK画面：Cyan
- LOG画面：Blue
- SLEEP画面：Red

---

## Sleep移行条件

### Deep Sleep logger基準版（GPIO43制御対応）✅

以下を満たしたら Sleep へ移行する。

1. RTC読出し完了
2. センサ読出し完了
3. **Battery ADC読出し完了（D3/GPIO4）**✅
4. CSV生成完了
5. **microSD保存完了（D7/GPIO44 CS確定）**✅
6. 必要な表示完了
7. **microSD CS=HIGH・SPI.end()完了**✅
8. 次回起床条件設定完了
9. **GPIO43→LOW で電源遮断準備完了**✅

---

## 起床条件

### 現時点の採用条件

| 項目 | 内容 |
|------|------|
| **主条件** | **Timer Wakeup** |
| **実績** | **30秒 / 60秒** ✅ |
| **判定** | 初回 OTHER、以後 TIMER を許容 ✅ |
| **GPIO43復帰** | **High確認待ち** 🔄 |

---

## 低消費電力化の考え方

### 現時点で採る方針

- Wi-Fi OFF
- Bluetooth OFF
- OLED常時消灯（**GPIO43制御対応）**✅
- 測定後すぐ Sleep
- **Deep Sleep時電源遮断（GPIO43制御）**✅
- USB給電試験と低消費電力運用を分けて考える
- **RGB LED は常時点灯前提にしない（MCP23017経由）**✅

### 後段で評価する項目

- CPUクロック最適化
- 未使用GPIO処理
- **Battery voltage monitor(D3/GPIO4)の追加負荷**✅
- OLED更新頻度最適化
- **microSD初期化回数の見直し（D7/GPIO44安定性確認）**✅
- 実消費電流測定
- **RGB LED の点灯ルール（MCP23017制御）**✅

---

## 電池運用に関する方針

- LiPo 1000mAh は **初期評価用の暫定容量**
- 電池容量は最終固定しない
- 容量変更可能な実装を優先する
- まずは機能成立を優先し、寿命の厳密評価は後回しとする

---

## 今後の確認対象

| 項目 | 状況 | 対応GPU |
|------|------|----------|
| **電池駆動での Deep Sleep logger 成立** | **メイン基板実装完了待ち** 🔄 | **D0～D10・D6・D3・D7** |
| **GPIO43(EN)制御の実装・確認** | **設計確定待ち** 🔄 | **D6(GPIO43)** |
| **R_EN(100kΩ)プルアップ実装確認** | **座標(12,08)～(14,08)確定** ✅ | **D6(GPIO43)** |
| **microSD CS=HIGH・SPI.end()処理確認** | **設計確定待ち** 🔄 | **D7(GPIO44)** |
| **Battery voltage monitor実装・確認** | **分圧回路実装待ち** 🔄 | **D3(GPIO4)** |
| **実消費電流測定** | 完成後評価 | - |

---

## ステータス

- [ACTIVE] 初号機電源制御方針として有効
- [COMPLETE] **電源基板通電試験全項目PASS（3.3V=3.304V確認）** ✅
- [COMPLETE] **AE-TPS63802 実装完了**（XC9306から変更）✅
- [COMPLETE] **LiPo → TP4056 → AE-TPS63802 → XIAO の最小電源経路完全成立** ✅
- [COMPLETE] **GPIO割当確定版（D6/GPIO43・D3/GPIO4・D7/GPIO44）** ✅
- [COMPLETE] **メイン基板レイアウト確定版（95mm×72mm・配置座標）** ✅
- [COMPLETE] **Battery voltage monitor確定版（D3/GPIO4・ADC1_CH3・100kΩ×2分圧・座標）** ✅
- [COMPLETE] **Deep Sleep対応設計確定（R_EN・GPIO43・microSD CS処理）** ✅
- [ACTIVE] Deep Sleep logger 基準版を基準点とする
- [ACTIVE] 起床後再初期化方針を採用
- [ACTIVE] OLED は常時点灯しない方針
- [ACTIVE] **RGB LED（MCP23017経由I2C 0x20制御）採用確定** ✅
- [IN PROGRESS] **メイン基板実装進行中（分圧/R_EN/MCP23017）**
- [IN PROGRESS] **GPIO43制御・microSD CS処理実装準備中** 🔄
- [PENDING] **電池駆動時の Deep Sleep logger 成立確認（メイン基板実装完了後）** 🔄
- [PENDING] **AE-TPS63802 EN端子（GPIO43）制御の実装・確認** 🔄
- [PENDING] 実消費電流測定
