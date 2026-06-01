# patch-Z.MPQ 탈것 모델 연결 기획

## 현재 확인 결과

- `patch-ZZ.MPQ`는 원복 차원에서 제거했습니다.
- `patch-Z.MPQ`에는 `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`가 들어 있지만 `900001~900184` 커스텀 display/model ID는 없습니다.
- 현재 DB의 `910001~910084`, `930001~930084`는 `900xxx` display ID를 사용하고 있으며, 이 `900xxx` 연결은 현재 `patch-koKR-4.MPQ` 쪽 DBC에 의존합니다.
- `patch-koKR-4.MPQ`의 현재 커스텀 display `900001~900184`가 참조하는 모델 파일 중 `patch-Z.MPQ`에 같은 경로가 존재하는 것은 `AlliancePVPMount` 계열뿐입니다.
- 따라서 기존 `patch-koKR-4` 모델을 그대로 `patch-Z`로 바꾸는 것이 아니라, `patch-Z`에 실제 존재하는 탈것 모델로 새 매핑을 만들어야 합니다.

## 추출 파일

- `E:\server\tmp_mpq_model_overlap_check\patchZ_mount_like_m2_files.tsv`: `patch-Z.MPQ` 안의 탈것 후보 M2 파일 전체
- `E:\server\tmp_mpq_model_overlap_check\patchZ_mount_like_m2_by_folder.tsv`: 폴더 기준 탈것 후보 요약
- `E:\server\tmp_mpq_model_overlap_check\patchZ_mount_like_dbc_models.tsv`: `patch-Z.MPQ` DBC에 이미 연결된 탈것 후보 model/display ID
- `E:\server\tmp_mpq_model_overlap_check\custom_900_display_patchZ_availability.tsv`: 현재 `900xxx` 커스텀 display가 `patch-Z`에 같은 모델 파일을 가지고 있는지 비교한 결과

## 작업 방향

1. `patch-koKR-4.MPQ`의 커스텀 모델/DBC를 기준으로 삼지 않습니다.
2. `patch-Z.MPQ`에 존재하는 탈것 후보 목록에서 사용할 모델을 확정합니다.
3. 확정된 모델에 대해 `CreatureModelData.dbc`와 `CreatureDisplayInfo.dbc`의 커스텀 ID 대역을 새로 구성합니다.
4. 구성된 DBC는 `patch-Z.MPQ` 기준으로 관리합니다. `patch-koKR-4.MPQ`에는 같은 커스텀 display/model 연결을 남기지 않는 방향으로 정리합니다.
5. DB의 `creature_template_model`은 새 display ID로 연결합니다.
6. 서버 DataDir DBC와 클라이언트 MPQ DBC가 같은 내용을 가지도록 동기화합니다.

## 주의 사항

- `patch-Z.MPQ`에 모델 파일만 있고 DBC display 연결이 없는 모델이 많습니다. 이 경우 단순 DB 수정만으로는 보이지 않고 DBC row 생성이 필요합니다.
- 기존 `910xxx`, `930xxx` 이름과 `patch-Z` 모델 이름이 1:1로 맞지 않습니다. 어떤 탈것을 어떤 크리처에 연결할지 매핑표가 필요합니다.
- 기존 `patch-koKR-4.MPQ` 백업은 `E:\server\data\karazhan\20260106\backup_creature_display_split_20260531_154114`에 있습니다.

## 다음 단계

1. `patchZ_mount_like_m2_by_folder.tsv`를 기준으로 사용할 탈것 후보를 확정합니다.
2. 확정된 후보를 `910001~910084`, `930001~930084` 크리처에 매핑합니다.
3. 새 매핑표 기준으로 DBC와 `creature_template_model` 수정 SQL을 생성합니다.
4. `patch-koKR-4.MPQ`의 커스텀 DBC 의존을 제거하고 `patch-Z.MPQ` 기준으로만 테스트합니다.
