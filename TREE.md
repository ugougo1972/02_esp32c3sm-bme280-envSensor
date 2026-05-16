# 提案ツリー構成

```text
02_esp32c3sm-bme280-envSensor/
├─ README.md
├─ CURRENT_STATUS.md
├─ CHANGELOG.md
├─ 10_HARDWARE/
│  ├─ 02_PARTS_LIST.md
│  └─ 03_POWER_DESIGN.md
├─ 20_SOFTWARE/
├─ 30_LOG/
├─ 40_DEV/
├─ 50_DECISIONS/
│  └─ 2026-05-16_adopt_bme680.md
└─ 90_HISTORY/
   └─ 2026-05-16_bme280_retired.md
```

## 適用方針

- `README.md` は入口文書のまま維持する。
- `CURRENT_STATUS.md` を現行状態の唯一の集約点とする。
- `10_HARDWARE/02_PARTS_LIST.md` は純粋な部品表へ近づける。
- `10_HARDWARE/03_POWER_DESIGN.md` は電源仕様専用へ近づける。
- BME280は現行仕様から外し、BME680を現行環境センサとして扱う。
- BME280関連の旧実績は `90_HISTORY/` へ隔離する。
```
