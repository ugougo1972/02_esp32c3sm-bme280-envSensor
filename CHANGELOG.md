# CHANGELOG

本ファイルは、リポジトリ内ドキュメントおよび設計変更の時系列記録です。  
現行状態の判断には `CURRENT_STATUS.md` を優先してください。

---

## 2026-05-16

### 変更

- BME280を現行構成から廃止し、BME680を正式採用品として反映。
- `CURRENT_STATUS.md` に更新ルールを追加。
- `CHANGELOG.md` を新規導入。
- `10_HARDWARE/02_PARTS_LIST.md` を純粋な部品表へ整理。
- `10_HARDWARE/03_POWER_DESIGN.md` を電源仕様専用文書へ整理。
- 実装進捗を `10_HARDWARE/07_HARDWARE_STATUS.md` へ分離。
- 試験記録を `30_LOG/01_TEST_LOG.md` へ分離。
- BME280旧仕様を `90_HISTORY/2026-05-16_bme280_retired.md` へ隔離。
- 文書責務分離の判断理由を `50_DECISIONS/2026-05-16_document_responsibility_rules.md` に記録。
- BME680採用判断を `50_DECISIONS/2026-05-16_adopt_bme680.md` に記録。

### 理由

- BME280破損により、GitHub上の現行仕様と実機状態が不一致になっていたため。
- 部品表、電源設計、進捗、試験結果、履歴が混在し、AI解析時の誤認識リスクが高かったため。
- 長期保守、GitHub運用、AI連携を前提に、最新仕様・履歴・判断理由を分離する必要があったため。

### 影響

- 以後、温湿度・気圧センサはBME680を現行として扱う。
- BME280に関する記述は、旧仕様または履歴としてのみ扱う。
- 現行状態の確認は `CURRENT_STATUS.md` を最優先とする。
