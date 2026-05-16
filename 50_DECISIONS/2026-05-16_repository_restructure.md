# Repository Restructure Decision

## Decision

READMEを入口文書へ縮小し、最新状態・履歴・判断理由を分離管理する。

## Reason

READMEとハードウェア文書に大量の重複が存在し、以下の問題が発生していた。

- AIが旧仕様と現行仕様を混同する
- 仕様と進捗と履歴が混在する
- README肥大化
- 変更理由が追跡できない

## Adopted Structure

- README.md
- CURRENT_STATUS.md
- 50_DECISIONS/
- 90_HISTORY/

## Expected Effects

- AI解析精度向上
- 保守性向上
- 仕様の責務分離
- 履歴混線防止
