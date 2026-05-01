# 01_SOFTWARE_OVERVIEW

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるソフトウェア全体構成を整理するための概要資料である。  
初号機では、まず **安定して記録できること** を最優先とし、その上で表示、入力、Deep Sleep、電池監視を段階的に統合する。

## ソフトウェア方針

- 最終構成候補MCUは **Seeed Studio XIAO ESP32S3 Plus**
- Bring-up は **ESP32-WROOM系開発ボード** と **XIAO ESP32S3 Plus** を使い分けて進める
- ソフトは、以下の機能を分離して設計する  
  - センサ読出し  
  - RTC時刻取得  
  - 画面更新  
  - 入力読取り  
  - ログ保存  
  - Deep Sleep 制御  
  - Battery voltage monitor
- UI表示中でもセンサ読出しを止めないため、**non-blocking ベース** を基本とする
- Wi-Fi は通常運用で使用しない
- Bluetooth は通常運用で使用しない
- RTC調整は **PC → USBシリアル → MCU → DS3231** で行う
- 初号機では、機能成立を優先し、消費電流最適化は完成後評価とする
- CSV列名・コード定義は、**`03_LOG_FORMAT.md` / `04_ctx.md` / `05_head.md` と整合**させる
- 電源系は **LiPo → TP4056 → AE-TPS63802 → XIAO** の最小構成成立を基準点とする
  - **電源基板通電試験 全項目PASS**（無負荷・USB-C単独・LiPo単独・USB-C+LiPo同時で3.304V確認）
  - **AE-TPS63802 実装完了**（XC9306から変更）
- Battery voltage monitor は **`TP4056 OUT+ / OUT-` を対象にした分圧回路** を前提に後段追加する

## ソフトウェアの役割

本ソフトウェアは、以下を担当する。

1. BME280 から温度・湿度・気圧を取得する
2. LTR390 から ALS / UVS を取得する
3. DS3231 から現在時刻を取得する
4. 取得値を OLED に表示する
5. RGB LED付きスイッチ付き Rotary Encoder で画面遷移や操作を行う
6. 必要に応じて Encoder RGB LED を制御する
7. 取得値を CSV として microSD に保存する
8. 一定周期動作後に Deep Sleep に入る
9. 起床後に各モジュールを再初期化して記録を継続する
10. 将来、分圧回路経由で電池電圧 `vbat` を取得する

## ソフトウェア全体構成

### 1. ハードウェア制御層

各デバイスの初期化・読出しを行う層。

| 区分 | 対象 | 通信 |
|------|------|------|
| Sensor | BME280 | I2C |
| Sensor | LTR390 | I2C |
| RTC | DS3231 | I2C |
| Display | SSD1306 OLED | I2C |
| Storage | microSD | SPI |
| Input | Rotary Encoder | GPIO / PCNT |
| UI LED | Encoder RGB LED | GPIO |
| Power Monitor | Battery divider | ADC |

### 2. 機能制御層

各部品を組み合わせて、装置としての動作を構成する層。

| 機能 | 内容 |
|------|------|
| sensor_task | センサ読出し |
| rtc_task | 時刻取得・妥当性確認 |
| display_task | 画面更新 |
| input_task | Encoder入力読取り |
| led_task | Encoder RGB LED 制御 |
| log_task | CSV生成・保存 |
| power_task | `vbat` 読出し（後段） |
| sleep_task | Deep Sleep移行制御 |

### 3. アプリケーション層

画面状態、ログ周期、運用ルールなど、製品としての振る舞いを定義する層。

| 機能 | 内容 |
|------|------|
| UI state | VIEW / MENU / CLOCK / LOG / SLEEP |
| logger mode | periodic logger / sleep logger |
| error handling | RTC異常 / SD異常 / 初期化失敗処理 |
| recovery | lostPower 時の復旧 |
| battery monitor | 分圧回路入力からの `vbat` 算出（後段） |

## 主要機能概要

### センサ読出し

#### BME280

- 温度
- 湿度
- 気圧

#### LTR390

- ALS
- UVS

設計上の注意:

- LTR390 は ALS / UVS 切替直後に待ち時間が必要
- periodic logger 初版で発生した ALS固定問題は、安定版読出し方式で解消済み
- UI処理と干渉しないよう、測定処理はブロッキングを避ける

### RTC処理

#### DS3231

- 現在時刻取得
- 時刻書込み
- lostPower 検出
- 妥当性判定
- 自動再設定

設計上の注意:

- `lostPower()` 検出時は復旧処理を行う
- 不正時刻を検出した場合も自動再設定対象とする
- 初期の `2000-01-01` 系時刻は無効扱いとする

### 表示処理

OLED SSD1306 に以下を表示する。

#### 通常画面

- 上段: 時刻欄
- 右上: 電池アイコン仮表示
- 本文:
  - 温度 / 湿度
  - 気圧
  - ALS / UV
- 下段: `PUSH: MENU`

#### メニュー画面

- VIEW
- CLOCK
- LOG
- SLEEP

#### CLOCK画面

- RTC時刻表示

#### LOG画面

- ログ保存状態
- ファイル名や保存結果の簡易表示
- `context_code` / `head_code` / `vbat` の表示余地を残す

設計上の注意:

- 表示更新は周期管理し、必要以上に全画面再描画しない
- センサ読出し・入力読取りを止めないことを優先する

### 入力処理

RGB LED付きスイッチ付き Rotary Encoder を用いて UI を操作する。

#### 入力内容

- 回転
- 押し込み
- RGB LED制御

#### 実装方針

- **ESP32Encoder + PCNT 方式**
- 右回転 = CW、左回転 = CCW
- 5ノッチ = 5イベント
- PUSH / RELEASE は正常確認済み
- RGB LED は個別制御可能
- `context_code` / `head_code` の最終入力UIは未確定

### ログ保存処理

microSD に CSV 形式で保存する。

#### 推奨保存対象

- `date`
- `time`
- `temp_c`
- `hum_pct`
- `press_hpa`
- `als`
- `uvs`
- `context_code`
- `head_code`
- `vbat`（後段実装）

#### 運用方針

- ファイル存在確認
- 無ければヘッダ作成
- あれば追記
- Deep Sleep 復帰後も同一形式で継続追記
- `context_code` は `04_ctx.md` に従う
- `head_code` は `05_head.md` に従う

#### 実績

- `/test.txt` 新規作成 / 読出し / 追記
- `/log.csv` 作成 / 追記
- `/log_env.csv` 保存
- `/log_env_loop.csv` 5秒周期保存
- `/log_sleep_env.csv` Deep Sleep 周期保存

#### 列構成に関する注意

- 開発途中では簡略列のログを許容する
- 運用版では列順を固定する
- `context` ではなく **`context_code`**
- `head` ではなく **`head_code`**
- `lux` / `uv` ではなく、現時点の実装実績に合わせて **`als` / `uvs`** を優先する

### Deep Sleep処理

#### 方針

- setup() 完結型
- 起床ごとに RTC / BME280 / LTR390 / microSD を再初期化
- 記録後に Deep Sleep へ移行

#### 実績

- 60秒周期 PASS
- 30秒周期 PASS
- 初回 `OTHER`、以後 `TIMER`
- `bootCount` 連続増加
- CSV 連続追記

#### 判定基準

- Deep Sleep 中は USB CDC の COM 切断を異常扱いしない
- Serial Monitor より CSV追記結果を優先して成立判定する

### 電池監視処理（後段）

#### 方針

- 測定対象は **TP4056 `OUT+ / OUT-`**
- 抵抗2本の **1:1 分圧回路** を前提とする
- 初期値は **100kΩ + 100kΩ**
- 必要に応じて ADC中点-GND 間へ **0.1µF MLCC** を追加する

#### 実装注意

- `vbat` 未実装時は `0`
- `vbat` を推測で埋めない
- 初回はテスター値との一致確認を優先する

## 初号機ソフトウェアの優先順位

| 優先度 | 項目 | 方針 |
|--------|------|------|
| 1 | 記録成立 | CSV を安定して保存できること |
| 2 | RTC成立 | 正しい時刻で記録できること |
| 3 | Deep Sleep成立 | 周期起床で継続記録できること |
| 4 | UI成立 | 画面遷移と基本表示 |
| 5 | RGB Encoder成立 | 回転・押下・LED制御 |
| 6 | `context_code` / `head_code` 反映 | 入力方式は後段で確定 |
| 7 | 電池監視 | 後段で追加 |
| 8 | 低消費電力最適化 | 完成後評価 |

## エラー処理方針

### RTC異常

- `lostPower()` を検出した場合は再設定する
- 不正時刻の場合は再設定する
- 復旧できない場合は無効時刻として扱う

### microSD異常

- 初期化失敗時は再試行余地を残す
- `CS High → SPI.begin → SD.begin(CS, SPI, 1000000)` を候補とする
- 保存失敗時は、少なくとも異常を判別できる表示またはシリアル出力を行う

### センサ異常

- 個別初期化失敗を切り分けられるようにする
- 一部センサ失敗時も、可能な範囲で他機能を継続できる構成を目指す
- ただし Deep Sleep logger 基準版では、まず全体成立を優先する

### 入力状態未確定

- `context_code` 未入力時は `0`
- `head_code` 未入力時は `0`
- 推測値を自動設定しない

## 開発段階のソフト構成

本プロジェクトでは、以下の順にソフトを積み上げる。

1. MCU単体確認
2. BME280 単体確認
3. LTR390 単体確認
4. OLED 単体確認
5. I2C統合確認
6. DS3231 統合確認
7. Rotary Encoder 統合確認
8. microSD 単体確認
9. RTC + microSD 確認
10. RTC + BME280 + microSD 確認
11. RTC + BME280 + LTR390 + microSD 確認
12. periodic logger 確認
13. Deep Sleep logger 確認
14. UI統合確認
15. RGB Encoder 置換確認
16. 電源系統合確認
17. `context_code` / `head_code` 反映
18. `vbat` 反映

## ファイル分割方針

ソフトウェア関連文書は、以下の役割で分ける。

| ファイル | 役割 |
|----------|------|
| `01_SOFTWARE_OVERVIEW.md` | ソフト全体概要 |
| `02_STATE_MACHINE.md` | 画面・状態遷移 |
| `03_LOG_FORMAT.md` | CSV形式定義 |
| `04_ctx.md` | `context_code` 運用定義 |
| `05_head.md` | `head_code` 入力定義 |
| `06_POWER_CONTROL.md` | Deep Sleep / 電源制御方針 |
| `07_SOFTWARE_ARCHITECTURE.md` | 実装構造 |

## 現時点で未確定の項目

- 電池電圧監視入力の実装
- `context_code` の最終入力方法
- `head_code` の最終入力方法
- `head_code` を含む運用版CSVの本実装反映
- ファイルローテーション
- 長期運用時の異常復旧方針詳細
- 完成後の低消費電力最適化内容
- RGB LED の運用ルール（状態表示 / 通知 / 消灯方針）

## ステータス

- [ACTIVE] 初号機ソフトウェア概要として有効
- [COMPLETE] USB給電ベースの統合ソフト構成は成立済み
- [ACTIVE] Deep Sleep logger 基準版を現時点の基準点とする
- [ACTIVE] RGB LED付きスイッチ付き Rotary Encoder 前提へ更新済み
- [COMPLETE] **LiPo → TP4056 → AE-TPS63802 → XIAO の最小電源経路完全成立**
- [COMPLETE] **電源基板通電試験全項目PASS**
- [COMPLETE] **メイン基板配置座標確定**
- [COMPLETE] **MCP23017採用確定**（LED制御用I2C GPIO Expander）
- [IN PROGRESS] メイン基板実装進行中
- [PENDING] 電池駆動での Deep Sleep logger 成立確認
- [PENDING] 電池電圧監視実装
- [PENDING] `context_code` / `head_code` の最終入力UI実装
