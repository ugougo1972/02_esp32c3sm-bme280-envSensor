# 08_TASK_SCHEDULER

## 目的

本ファイルは携帯型環境センサーロガーにおけるタスク実行順序および周期管理方針を定義する。

---

## 基本方針

- 初号機は FreeRTOS を使用しない
- loop() ベースの協調動作を採用する
- ブロッキング処理を極力避ける
- タスク毎に実行周期を定義する
- DeepSleep版では setup() 完結型へ移行可能とする

---

## タスク一覧

| タスク | 役割 | 周期 |
|----------|----------|----------|
| TaskInput | Encoder読取り | 20ms |
| TaskDisplay | OLED更新 | 200ms |
| TaskSensor | BME680/LTR390取得 | 1秒 |
| TaskBattery | Battery ADC取得 | 5秒 |
| TaskRtc | RTC同期取得 | 1秒 |
| TaskLogger | CSV保存 | 60秒 |
| TaskStatusLed | RGB LED更新 | 200ms |

---

## 実行優先順位

1. TaskInput
2. TaskSensor
3. TaskBattery
4. TaskRtc
5. TaskLogger
6. TaskDisplay
7. TaskStatusLed

---

## タイマ管理例

```cpp
uint32_t lastSensorMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastLogMs = 0;
```

---

## TaskSensor

取得対象

- BME680
- LTR390

記録先

```cpp
LogRecord currentRecord;
```

---

## TaskBattery

取得対象

```cpp
D3(GPIO4)
```

換算

```cpp
vbat = adc_voltage * 2.0
```

---

## TaskLogger

保存先

```text
log_env.csv
```

保存内容

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

---

## DeepSleep版

DeepSleep版では以下へ集約する。

```text
Wake
↓
TaskRtc
↓
TaskSensor
↓
TaskBattery
↓
TaskLogger
↓
Sleep
```

---

## ステータス

- ACTIVE
- 初号機採用
