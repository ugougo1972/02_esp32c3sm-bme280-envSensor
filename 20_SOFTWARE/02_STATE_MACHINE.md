# 02_STATE_MACHINE

## 目的

本ファイルは、携帯型環境センサーロガー初号機における画面状態および動作状態の遷移を整理するためのものである。  
初号機では、まず **MENU / CLOCK / LOG / SLEEP を含む基本UI遷移** と、**periodic logger / Deep Sleep logger の基本動作状態** を表形式で定義する。

## 基本方針

- 状態遷移は **単純・可読・切り分けしやすい構成** を優先する
- センサ読出し、画面更新、入力読取りは分離し、UI状態遷移が測定処理を止めないようにする
- UI は **RGB LED付きスイッチ付き Rotary Encoder（回転 + 押し込み）** で操作する
- 初号機では、複雑な多段メニューは導入せず、最小構成で成立させる
- Deep Sleep logger 基準版は、**setup() 完結型** を前提に扱う
- Encoder の回転検出は **ESP32Encoder + PCNT 方式** を前提とする

## 状態分類

本システムでは、状態を以下の2種類に分けて扱う。

1. 画面状態（UI State）
2. 動作状態（Operation State）

---

## 1. 画面状態（UI State）

### 画面状態一覧

| 状態ID | 状態名 | 内容 |
|--------|--------|------|
| UI-01 | VIEW | 通常画面。時刻、温度、湿度、気圧、ALS、UV、将来は電池状態を表示 |
| UI-02 | MENU | メニュー選択画面 |
| UI-03 | CLOCK | RTC時刻表示画面 |
| UI-04 | LOG | 記録状態表示画面 |
| UI-05 | SLEEP | スリープ移行確認画面またはスリープ移行状態 |

### 初期状態

- 起動直後の初期画面状態は **VIEW**
- Deep Sleep logger 基準版では、UIを経由せず記録処理へ進む構成も許容する
- UI試験版では、起動後に VIEW を表示し、操作入力を受け付ける

### 画面状態遷移表

| 現在状態 | 入力/条件 | 次状態 | 備考 |
|----------|-----------|--------|------|
| VIEW | Encoder PUSH | MENU | 通常画面からメニューへ移行 |
| VIEW | Encoder CW/CCW | VIEW | 値表示のみ、画面遷移なし |
| MENU | Encoder CW | MENU | 選択項目を次へ |
| MENU | Encoder CCW | MENU | 選択項目を前へ |
| MENU | PUSH on VIEW | VIEW | 通常画面へ戻る |
| MENU | PUSH on CLOCK | CLOCK | 時刻表示へ移行 |
| MENU | PUSH on LOG | LOG | 記録状態画面へ移行 |
| MENU | PUSH on SLEEP | SLEEP | スリープ画面へ移行 |
| CLOCK | Encoder PUSH | MENU | メニューへ戻る |
| CLOCK | Timeout | VIEW | 一定時間後に通常画面へ戻す構成も可 |
| LOG | Encoder PUSH | MENU | メニューへ戻る |
| LOG | Timeout | VIEW | 一定時間後に通常画面へ戻す構成も可 |
| SLEEP | Encoder PUSH | MENU | スリープキャンセル時 |
| SLEEP | Timeout / Confirm | VIEW または Sleep処理 | 実装方式に依存 |

### MENU選択状態

MENU 画面では、内部的に選択カーソル状態を持つ。

| 選択番号 | 表示名 | 遷移先 |
|----------|--------|--------|
| M-01 | VIEW | VIEW |
| M-02 | CLOCK | CLOCK |
| M-03 | LOG | LOG |
| M-04 | SLEEP | SLEEP |

### Rotary Encoder入力ルール

| 入力 | 動作 |
|------|------|
| CW | 次項目へ移動 |
| CCW | 前項目へ移動 |
| PUSH | 決定 / MENU遷移 |
| 長押し | 未使用（現時点） |

### 入力判定方式

- **ESP32Encoder + PCNT 方式**
- 右回転 = CW
- 左回転 = CCW
- **5ノッチ = 5イベント**
- PUSH / RELEASE 判定は成立済み
- RGB LED は状態表示用途として後段利用可能

---

## 2. 動作状態（Operation State）

### 動作状態一覧

| 状態ID | 状態名 | 内容 |
|--------|--------|------|
| OP-01 | BOOT | 電源投入またはリセット直後 |
| OP-02 | INIT | 各モジュール初期化 |
| OP-03 | IDLE | 通常待機状態 |
| OP-04 | MEASURE | センサ測定状態 |
| OP-05 | LOGGING | CSV生成・microSD保存 |
| OP-06 | DISPLAY | 表示更新 |
| OP-07 | PRE_SLEEP | Deep Sleep移行前処理 |
| OP-08 | SLEEPING | Deep Sleep中 |
| OP-09 | WAKEUP | Deep Sleep復帰直後 |
| OP-10 | ERROR | 異常検出状態 |

### 動作状態の基本遷移

| 現在状態 | 条件 | 次状態 | 備考 |
|----------|------|--------|------|
| BOOT | 起動 | INIT | 電源投入直後 |
| INIT | 初期化成功 | IDLE | UI版では待機へ |
| INIT | 初期化失敗 | ERROR | 切り分け対象 |
| IDLE | 測定周期到来 | MEASURE | periodic logger 等 |
| IDLE | 画面更新周期到来 | DISPLAY | non-blocking 更新 |
| IDLE | Sleep条件成立 | PRE_SLEEP | Deep Sleep移行前 |
| MEASURE | 測定完了 | LOGGING | RTC + センサ値確定後 |
| MEASURE | 測定失敗 | ERROR または LOGGING | 実装方針に依存 |
| LOGGING | 保存成功 | IDLE または PRE_SLEEP | モードによる |
| LOGGING | 保存失敗 | ERROR または IDLE | 再試行余地あり |
| DISPLAY | 描画完了 | IDLE | 表示のみ更新 |
| PRE_SLEEP | 前処理完了 | SLEEPING | `esp_deep_sleep_start()` |
| SLEEPING | Timer Wakeup | WAKEUP | 周期起床 |
| WAKEUP | 復帰 | INIT | setup() 完結型では再初期化 |
| ERROR | 復旧成功 | IDLE または INIT | 異常内容による |
| ERROR | 復旧不可 | ERROR | 停止または簡易表示 |

---

## 3. periodic logger の状態遷移

periodic logger は、USB給電ベースで継続測定・継続保存する試験用構成とする。

### periodic logger 遷移表

| 順序 | 状態 | 内容 |
|------|------|------|
| P-01 | INIT | RTC / BME280 / LTR390 / microSD 初期化 |
| P-02 | IDLE | 周期待ち |
| P-03 | MEASURE | RTC読出し、BME280読出し、LTR390読出し |
| P-04 | LOGGING | CSV行生成、追記保存 |
| P-05 | DISPLAY | 必要なら表示更新 |
| P-06 | IDLE | 次周期まで待機 |

### periodic logger の特徴

- Deep Sleep は使用しない
- Serial Monitor で継続確認しやすい
- センサ値安定化や CSV 形式確認に向く
- `/log_env_loop.csv` への 5秒周期追記実績あり

---

## 4. Deep Sleep logger の状態遷移

Deep Sleep logger は、低消費電力運用を前提とした基準版である。

### Deep Sleep logger 遷移表

| 順序 | 状態 | 内容 |
|------|------|------|
| D-01 | BOOT | 起床直後 |
| D-02 | INIT | RTC / BME280 / LTR390 / microSD 再初期化 |
| D-03 | MEASURE | RTC、BME280、LTR390 読出し |
| D-04 | LOGGING | CSV追記 |
| D-05 | PRE_SLEEP | 次回起床条件設定 |
| D-06 | SLEEPING | Deep Sleep 移行 |
| D-07 | WAKEUP | Timer wakeup |
| D-08 | INIT | 再初期化して繰り返し |

### Deep Sleep logger の特徴

- **setup() 完結型**
- 起床ごとに各モジュールを再初期化する
- 判定は Serial より **CSV追記結果を優先**
- 30秒周期、60秒周期の成立実績あり
- 初回 `wakeupCause = OTHER`、以後 `TIMER` を想定する

### 電池駆動との関係

- **LiPo → TP4056 → XC9306 → XIAO** の最小電源経路は成立済み
- LiPo駆動で **Blink** は正常動作確認済み
- ただし、**電池駆動での Deep Sleep logger 成立は未確認** とする

---

## 5. エラー状態の扱い

### エラー分類

| 種別 | 例 | 基本方針 |
|------|----|----------|
| RTCエラー | `lostPower()`、不正時刻 | 再設定または無効時刻扱い |
| SDエラー | `SD.begin()` 失敗 | 再試行余地を残す |
| センサエラー | 初期化失敗、読出し失敗 | 切り分け優先 |
| UIエラー | 入力暴走、画面更新不整合 | UI機能を一時縮退可能 |
| 電源系エラー | 再起動ループ、起動失敗 | 配線・電源前段へ戻る |
| ADCエラー | `vbat` 異常値 | 分圧回路 / ADC設定を確認 |

### ERROR状態の扱い方針

- 初号機では、異常を隠さず **切り分けしやすい状態表示・シリアル出力** を優先する
- Deep Sleep logger 基準版では、無理な自動復旧よりも **記録成立可否の確認** を優先する
- 復旧不能時は、停止または簡易表示で異常を明示する

---

## 6. タイマ・周期管理の考え方

### 基本方針

- `delay()` 連打ではなく、`millis()` ベースで周期管理する
- 表示更新、入力読取り、センサ読出しの周期を分離する
- Deep Sleep logger では、起床 → 処理 → Sleep の1サイクルで完結させる

### 管理対象例

| 項目 | 内容 |
|------|------|
| sensorInterval | センサ読出し周期 |
| displayInterval | 画面更新周期 |
| logInterval | CSV保存周期 |
| sleepInterval | Deep Sleep周期 |
| ltrWaitTime | LTR390 切替後待ち時間 |
| batteryInterval | `vbat` 読出し周期（後段） |

---

## 7. 初号機で未使用または保留の状態

| 項目 | 状況 |
|------|------|
| 長押し分岐 | 未使用 |
| 設定変更モード | 未実装 |
| 頭痛入力画面 | 未実装 |
| context詳細編集画面 | 未実装 |
| 電池残量警告画面 | 未実装 |
| エラー履歴画面 | 未実装 |
| RGB LED 状態表示ルール | 未確定 |

---

## 8. 実装上の優先順位

| 優先度 | 状態管理項目 | 方針 |
|--------|--------------|------|
| 1 | Deep Sleep logger の状態遷移 | 基準点として固定 |
| 2 | VIEW / MENU / CLOCK / LOG / SLEEP | UI基本遷移成立 |
| 3 | RGB Encoder 入力 | 回転・押下を安定化 |
| 4 | エラー時の切り分け | シリアル出力優先 |
| 5 | Timeoutによる自動復帰 | 必要に応じ追加 |
| 6 | 設定変更系状態 | 後段追加 |
| 7 | `vbat` 状態反映 | 後段追加 |

---

## ステータス

- [ACTIVE] 初号機状態遷移定義として有効
- [ACTIVE] MENU / CLOCK / LOG / SLEEP 基本遷移を定義
- [ACTIVE] periodic logger / Deep Sleep logger の基本動作状態を定義
- [ACTIVE] RGB LED付きスイッチ付き Rotary Encoder 前提へ更新済み
- [COMPLETE] **電源基板通電試験全項目PASS により安定性確保**
- [COMPLETE] **メイン基板配置座標確定**
- [PENDING] 電池駆動での Deep Sleep logger 成立確認
- [PENDING] SLEEP画面の確定挙動は実装方式により調整余地あり
- [PENDING] エラー状態の最終復旧仕様は未確定
- [PENDING] 頭痛入力・context編集状態は未実装
