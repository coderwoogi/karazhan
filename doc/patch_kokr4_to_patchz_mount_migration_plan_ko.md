# 4toz 기준 patch-koKR-4 탈것 자료 이전 계획

## 결론

- `patch-koKR-4.MPQ`와 `patch-Z.MPQ`에 모두 `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`가 있어 탈것 display 연결 추적이 어려웠습니다.
- `4toz` 자료 기준으로 `patch-koKR-4`의 커스텀 `900xxx` 탈것 모델 폴더는 44종이었습니다.
- 그중 `AlliancePVPMount`만 `patch-Z`에 이미 존재했고, 나머지 43종은 `patch-Z`에 없었습니다.
- 추가 검증 중 `CreatureModelData`의 `900001`이 `AllianceLionMount`를 참조하는 것을 확인하여 이 폴더도 함께 이전했습니다.
- 최종적으로 `patch-koKR-4`의 커스텀 탈것 모델과 DBC row를 `patch-Z`로 이전하고, `patch-koKR-4`의 중복 Creature DBC 의존을 제거했습니다.

## 자료 위치

- `E:\server\data\karazhan\4toz\patch-koKR-4`: 기존 커스텀 탈것 모델 추출본
- `E:\server\data\karazhan\4toz\patch-Z`: 이전 대상 패치 추출본
- `E:\server\data\karazhan\4toz\patch-koKR-4.MPQ`: 정리 후 koKR 패치
- `E:\server\data\karazhan\4toz\patch-Z.MPQ`: 병합 후 Z 패치

## 적용 결과

- `patch-Z.MPQ`에 `900001~900184` `CreatureDisplayInfo.dbc` row를 병합했습니다.
- `patch-Z.MPQ`에 `900001~900049` `CreatureModelData.dbc` row를 병합했습니다.
- `patch-koKR-4`에 있던 커스텀 탈것 모델 폴더를 `patch-Z`로 이전했습니다.
- `CreatureModelData`의 `900001`에서 참조하는 `AllianceLionMount`도 추가로 이전했습니다.
- `patch-koKR-4.MPQ`에서는 `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`, 이전된 탈것 모델 폴더를 제거했습니다.
- `Item.dbc`, `Spell.dbc`, `SpellItemEnchantment.dbc`, `ItemRandomProperties.dbc` 등 기존 커스텀 DBC는 유지했습니다.
- 서버 DBC 위치인 `E:\server\operate\data\dbc`와 작업 기준 폴더 `E:\server\data\karazhan\20260106`의 Creature DBC도 동일하게 동기화했습니다.

## 백업 위치

- `E:\server\data\karazhan\4toz\backup_mount_migration_20260531_162513`

## 최종 파일

- `E:\server\3.3.5\Data\patch-Z.MPQ`
- `E:\server\3.3.5\Data\koKR\patch-koKR-4.MPQ`
- `E:\server\data\karazhan\4toz\patch-Z.MPQ`
- `E:\server\data\karazhan\4toz\patch-koKR-4.MPQ`

## 검증 결과

- `patch-Z.MPQ` 안에 `CreatureDisplayInfo.dbc` `900001~900184` 168개가 존재합니다.
- `patch-Z.MPQ` 안에 `CreatureModelData.dbc` `900001~900049` 49개가 존재합니다.
- `patch-koKR-4.MPQ` 안에는 `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`가 없습니다.
- `patch-koKR-4.MPQ` 안에는 이전된 커스텀 탈것 모델 폴더가 없습니다.
- DB 연결은 `910001~910084 -> 900001~900084`, `930001~930084 -> 900101~900184` 상태입니다.

## 테스트 방법

1. 와우 클라이언트를 완전히 종료합니다.
2. `E:\server\3.3.5\Cache` 폴더를 삭제합니다.
3. 월드서버를 재시작합니다.
4. `910001`, `930001` 계열 크리처 또는 해당 탈것 기능을 테스트합니다.
