# patch-Z 원본 DBC 복구 및 신규 탈것 행 병합

- 기준 원본: `E:\server\data\karazhan\backup_mount_expand_20260531_174124\mpq\patch-Z.MPQ`
- 적용 대상: `E:\server\3.3.5\Data\patch-Z.MPQ`
- 깨진 상태 백업: `E:\server\data\karazhan\backup_patchz_broken_before_restore_20260531_204418`

## 원인

기존 작업에서 `CreatureModelData.dbc`, `CreatureDisplayInfo.dbc`를 클라이언트 기본 DBC 기준으로 재생성하면서, 원래 `patch-Z.MPQ`가 가지고 있던 기존 display/model 연결값이 대량으로 바뀌었다.

비교 결과:

- `CreatureModelData.dbc`: 기존 행 361개 변경, 기존 행 131개 누락
- `CreatureDisplayInfo.dbc`: 기존 행 24,430개 변경, 기존 행 24개 누락

이 상태에서는 일반 NPC가 원래 `patch-Z`에서 기대하던 모델/텍스처 연결과 맞지 않아 외형이 깨질 수 있다.

## 조치

원래 정상 기준으로 사용하던 `patch-Z.MPQ`의 Creature DBC를 기준으로 되돌리고, 신규 탈것에 필요한 행만 추가했다.

- `CreatureModelData.dbc`: 기존 행 변경 0개, 누락 0개, 신규 89개 추가
- `CreatureDisplayInfo.dbc`: 기존 행 변경 0개, 누락 0개, 신규 89개 추가
- 신규 display ID 범위: `900185~900273`
- 신규 model ID 범위: `900050~900138`

## 검증

- `patch-Z.MPQ` 내부 `CreatureModelData.dbc`가 병합 결과물과 해시 일치
- `patch-Z.MPQ` 내부 `CreatureDisplayInfo.dbc`가 병합 결과물과 해시 일치
- 원래 `patch-Z`에 존재하던 기존 DBC 행은 변경 없이 보존됨
