# patch-Z.MPQ 탈것 후보 목록

## 기준

- 대상 파일: `E:\server\3.3.5\Data\patch-Z.MPQ`
- 기준 DBC: `CreatureDisplayInfo.dbc`, `CreatureModelData.dbc`
- 조건: `Creature` 모델 경로이며 탈것으로 보이는 키워드가 있고, 실제 display ID가 연결된 항목

## 요약

- display ID가 연결된 탈것 후보 model row: `254개`
- 폴더 기준 탈것 후보: `134종`
- 전체 추출 목록: `E:\server\tmp_mpq_model_overlap_check\patchZ_mount_like_dbc_models.tsv`
- 폴더 요약 목록: `E:\server\tmp_mpq_model_overlap_check\patchZ_mount_like_m2_by_folder.tsv`

## 주요 후보 예시

| 분류 | 대표 ModelID | 대표 모델 경로 | DisplayID 예시 |
|---|---:|---|---|
| 곰 탈것 | 9638 | `Creature\amanibearmount2\amanibearmount2.mdx` | `22463,22464,22466,22467` |
| 비행 말 | 3358 | `Creature\CelestialHorse\CelestialHorse.mdx` | `31958` |
| 죽음의 기사 탈것 | 2810 | `Creature\DeathKnightMount\DeathKnightMount.mdx` | `25278,25279,25280` |
| 늑대 | 217 | `Creature\DireWolf\RidingDireWolf.mdx` | `207,247,1166` |
| 용 | 203 | `Creature\dragon2\dragon2fixed.mdx` | `1686,1687,2717` |
| 비룡 | 2858 | `Creature\drakemount2\drakemount2.mdx` | `24714,25803,25831` |
| 엘레크 | 2313 | `Creature\Elekk\elekkdraenormount.mdx` | `17063,17142,19869` |
| 그리핀 | 2376 | `Creature\elitegryphon\elitegryphonarmored.mdx` | `17698,17703,17717` |
| 와이번 | 2378 | `Creature\elitewyvern\elitewyvernarmored.mdx` | `17702,17719,17720` |
| 호랑이 | 1912 | `Creature\frostsabre2fast\nightsaber2mountarmored.mdx` | `14329,14330,14331` |
| 기계 탈것 | 2915 | `Creature\GoblinShredderMount\GoblinShredderMount.mdx` | `26558,26559` |
| 로켓 | 2195 | `Creature\GnomeRocketCar\GnomeRocketCar.mdx` | `2490` |

## 비고

- 이 문서는 전체 목록을 모두 본문에 넣지 않고, 검토용 요약만 유지합니다.
- 전체 후보를 확인할 때는 TSV 파일을 기준으로 보는 것이 안전합니다.
- 한글이 포함된 문서는 UTF-8로 저장해야 합니다.
