# 03_LOG_FORMAT

## 目的

本ファイルは、携帯型環境センサーロガーで記録するログデータのCSV形式を定義する。  
ログは後から Excel、Python、R 等で解析することを前提とし、列名・単位・欠損値・開発段階ログと運用ログの扱いを明確にする。

---

## 1. 現行前提

本ファイルは、2026-06-14時点の現行設計を基準とする。

- MCU: XIAO ESP32S3 Plus
- 温湿度・気圧: AE-BME680
- ガス測定: 当面無効、ヒーターOFF運用
- 照度・UV: LTR390
- RTC: DS3231 + AT24C32
- 記録媒体: microSD
- 電池監視: D3(GPIO4) + 100kΩ + 100kΩ分圧
- SPI割当:
  - D10 = CS
  - D9 = MOSI
  - D8 = SCK
  - D7 = MISO

---

## 2. 基本方針

- ログはCSV形式とする。
- 保存先はmicroSDとする。
- 保存方式は追記保存（append）とする。
- 新規ファイル作成時のみヘッダ行を書き込む。
- 文字列状態は直接保存せず、数値コードで保存する。
- `context_code` は `04_ctx.md` の定義に従う。
- `head_code` は `05_head.md` の定義に従う。
- 未入力・不明・症状なしを混同しない。
- 推測値は記録しない。
- 初号機では、まず安定して追記できることを優先する。

---

## 3. ログファイル基本仕様

| 項目 | 内容 |
|---|---|
| ファイル形式 | CSV |
| 文字コード | UTF-8 |
| 改行コード | LF |
| 保存媒体 | microSD |
| 保存方式 | append |
| 基本ファイル名 | `log_env.csv` |
| ヘッダ行 | あり |
| 区切り文字 | カンマ |
| 小数点 | `.` |
| タイムゾーン | RTC時刻基準、原則ローカル時刻 |

---

## 4. ファイル名運用

| ファイル名 | 用途 | 状態 |
|---|---|---|
| `test.txt` | microSD単体確認 | 開発用 |
| `log.csv` | 初期CSV確認 | 開発用 |
| `log_env.csv` | 通常運用ログ | 推奨 |
| `log_env_loop.csv` | periodic logger試験 | 開発用 |
| `log_sleep_env.csv` | DeepSleep logger試験 | 開発用 |

運用版では `log_env.csv` を基本とする。  
将来、日次分割や月次分割を導入する場合は本ファイルを更新する。

---

## 5. 推奨CSV列構成

運用版の推奨ヘッダは以下とする。

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

| 列番号 | 項目名 | 内容 | 単位・型 | 取得元 | 備考 |
|---:|---|---|---|---|---|
| 1 | `date` | 日付 | `YYYY-MM-DD` | DS3231 | RTC時刻 |
| 2 | `time` | 時刻 | `HH:MM:SS` | DS3231 | RTC時刻 |
| 3 | `temp_c` | 温度 | ℃ / float | BME680 | ガス測定とは独立 |
| 4 | `hum_pct` | 湿度 | % / float | BME680 | 相対湿度 |
| 5 | `press_hpa` | 気圧 | hPa / float | BME680 | 解析上重要 |
| 6 | `als` | 照度センサ値 | count / uint32 | LTR390 | ALS生値 |
| 7 | `uvs` | UVセンサ値 | count / uint32 | LTR390 | UVS生値 |
| 8 | `context_code` | 行動状態 | code / uint8 | UI入力 | `04_ctx.md`準拠 |
| 9 | `head_code` | 頭痛状態 | code / uint8 | UI入力 | `05_head.md`準拠 |
| 10 | `vbat` | 電池系電圧 | V / float | D3(GPIO4) ADC | 100kΩ+100kΩ分圧換算後 |

---

## 6. 段階別ヘッダ

開発段階では、以下の簡略ヘッダを許容する。

### 6.1 RTC + BME680 + microSD

```text
date,time,temp_c,hum_pct,press_hpa
```

### 6.2 RTC + BME680 + LTR390 + microSD

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs
```

### 6.3 context_code追加版

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code
```

### 6.4 運用版

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

注意：

- 段階ログと運用ログを混在解析しない。
- 運用版に入った後は列順を固定する。
- 列名の変更は破壊的変更として扱う。

---

## 7. 記録例

### 7.1 運用版例

```text
2026-06-14,08:00:00,24.31,55.20,1007.42,418,0,5,1,4.102
2026-06-14,08:01:00,24.33,55.10,1007.39,420,0,5,1,4.098
2026-06-14,08:02:00,24.35,55.00,1007.37,419,0,5,2,4.094
```

### 7.2 未入力を含む例

```text
2026-06-14,08:03:00,24.36,54.98,1007.36,421,0,0,0,4.090
```

この例では、

- `context_code = 0`: 未入力・不明
- `head_code = 0`: 未入力・不明

を意味する。

### 7.3 vbat未実装段階の例

```text
2026-06-14,08:04:00,24.37,54.91,1007.35,420,0,0,0,0
```

`vbat = 0` は、未実装または未取得を意味する。

---

## 8. `context_code`

`context_code` は `04_ctx.md` を正とする。

| code | 状態名 | 意味 |
|---:|---|---|
| 0 | unknown | 不明・未入力・未設定 |
| 1 | indoor | 屋内 |
| 2 | outdoor | 屋外 |
| 3 | moving | 移動中 |
| 4 | office | 職場 |
| 5 | home | 自宅 |

運用ルール：

- 既定値は `0`。
- 未入力時は `0`。
- センサ値から自動推定しない。
- 屋内・屋外等を自動補完しない。

---

## 9. `head_code`

`head_code` は `05_head.md` を正とする。

| code | 状態名 | 意味 |
|---:|---|---|
| 0 | unknown | 不明・未入力・記録なし |
| 1 | none | 頭痛なし |
| 2 | mild | 軽い頭痛 |
| 3 | moderate | 中程度の頭痛 |
| 4 | severe | 強い頭痛 |
| 5 | recovered | 軽快 |

運用ルール：

- 既定値は `0`。
- 未入力時は `0`。
- 推測で `1` を自動設定しない。
- 「未入力」と「頭痛なし」は区別する。

---

## 10. `vbat`

### 10.1 定義

`vbat` は、Battery ADCで取得した値を分圧比で換算した電池系電圧とする。

### 10.2 回路前提

```text
LiPo/TP4056 B+系 ----[100kΩ]----+---- XIAO D3(GPIO4)
                                |
                              [100kΩ]
                                |
GND ----------------------------+
```

### 10.3 換算式

100kΩ + 100kΩの1:1分圧のため、ADC入力値を2倍して元電圧に換算する。

```cpp
float vbat = analogReadMilliVolts(PIN_BAT_ADC) / 1000.0f * 2.0f;
```

### 10.4 記録ルール

| 状態 | 記録 |
|---|---|
| 正常取得 | 実測換算値 |
| 未実装 | `0` |
| 読取失敗 | `0` または `-1` |
| 明らかな異常値 | 実装側でエラー扱い |

初回実装後、日置DT4256の実測値と比較して確認する。

---

## 11. 欠損値・異常値

| 項目 | 欠損時 | 備考 |
|---|---:|---|
| `temp_c` | `-999` | センサ異常値候補 |
| `hum_pct` | `-1` | 湿度範囲外として扱いやすい |
| `press_hpa` | `-1` | 気圧範囲外として扱いやすい |
| `als` | `0` | 暗所と区別困難なためエラーフラグ列は将来検討 |
| `uvs` | `0` | 同上 |
| `context_code` | `0` | unknown |
| `head_code` | `0` | unknown |
| `vbat` | `0` | 未実装・未取得 |

現時点ではエラーフラグ列は追加しない。  
必要になった場合は `err_flags` 列の追加を検討する。

---

## 12. microSD保存ルール

### 12.1 SPI割当

| 信号 | XIAO端子 |
|---|---|
| CS | D10 |
| MOSI | D9 |
| SCK | D8 |
| MISO | D7 |

### 12.2 初期化例

```cpp
#define SD_CS    D10
#define SD_MOSI  D9
#define SD_SCK   D8
#define SD_MISO  D7

SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

if (!SD.begin(SD_CS, SPI)) {
  Serial.println("SD init failed");
}
```

### 12.3 保存処理

| 処理 | 内容 |
|---|---|
| 起動時 | microSD初期化 |
| ファイルなし | 新規作成 + ヘッダ書込み |
| ファイルあり | 追記 |
| 書込み後 | flush / close |
| Sleep前 | CS=HIGH、SPI.end() |
| 復帰後 | SPI/microSD再初期化 |

---

## 13. ログ周期

| 種別 | 周期 | 用途 |
|---|---:|---|
| 試験短周期 | 5秒 | 機能試験 |
| Bring-up | 30秒 | 統合確認 |
| 通常候補 | 60秒 | 初期運用 |
| 将来 | 300秒以上 | 消費電流評価後 |

初号機では60秒周期を基準候補とする。

---

## 14. ファイルサイズ見積り

1行あたり約60〜100バイトを想定する。

| 期間 | 行数（60秒周期） | 概算サイズ |
|---|---:|---:|
| 1日 | 1,440 | 約150KB |
| 1週間 | 10,080 | 約1MB |
| 1ヶ月 | 43,200 | 約4MB |
| 1年 | 525,600 | 約50MB |

microSD容量としては8GB以上で十分余裕がある。

---

## 15. 解析時の注意

- `context_code = 0` は「unknown」であり「屋内」ではない。
- `head_code = 0` は「unknown」であり「頭痛なし」ではない。
- `vbat = 0` は未取得扱い。
- 開発段階ログと運用版ログはヘッダを確認して分ける。
- BME680のガス値は現行運用では記録対象外。
- 温湿度・気圧はBME680由来だが、旧BME280ログとの比較ではセンサ変更を考慮する。

---

## 16. 将来拡張候補

| 候補列 | 内容 | 状態 |
|---|---|---|
| `gas_ohm` | BME680ガス抵抗 | 当面不採用 |
| `err_flags` | エラー状態ビット列 | 候補 |
| `boot_count` | 起動回数 | 候補 |
| `wakeup_reason` | 起床要因 | 候補 |
| `sd_ms` | SD書込み時間 | 候補 |
| `note_code` | 補足状態 | 候補 |

---

## 17. ステータス

- [COMPLETE] BME680前提へ更新
- [COMPLETE] microSD新SPI割当へ更新
- [COMPLETE] vbat定義更新
- [COMPLETE] context_code / head_code 方針維持
- [ACTIVE] CSV定義として有効
- [PENDING] 実機vbat値確認
- [PENDING] microSD新割当での実保存確認
