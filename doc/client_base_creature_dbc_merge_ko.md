# 클라이언트 기준 Creature DBC 병합

- 기준 MPQ: `E:\server\3.3.5\Data\koKR\patch-koKR-3.MPQ`
- 커스텀 CreatureModelData 행: 138
- 커스텀 CreatureDisplayInfo 행: 257
- patch-Z 신규 탈것 display 행: 89
- patch-Z 신규 탈것 텍스처 미지정 행: 0
- 최종 CreatureModelData 행: 1469
- 최종 CreatureDisplayInfo 행: 24519
- 클라이언트 `patch-Z.MPQ` 반영: 완료
- 서버 `E:\server\operate\data\dbc` 복사: 서버 프로세스가 파일을 사용 중이라 이번 실행에서는 건너뜀

## 처리 의도

기존 다른 NPC 외형이 이상해진 원인은 `patch-Z.MPQ`에 들어간 전체 Creature DBC가 현재 클라이언트 기준 DBC와 달라진 상태에서 덮어써졌기 때문으로 판단했다.

이번 작업은 클라이언트의 최신 koKR 기준 DBC인 `patch-koKR-3.MPQ`에서 `CreatureModelData.dbc`, `CreatureDisplayInfo.dbc`를 다시 추출한 뒤, `900001~900999` 커스텀 행만 병합했다. 따라서 기존 블리자드 NPC/크리처 행은 클라이언트 기준값으로 복구되고, 커스텀 탈것 행만 유지된다.

## 검증

- `patch-Z.MPQ` 내부 `CreatureModelData.dbc` 해시가 `E:\server\data\karazhan\20260106\CreatureModelData.dbc`와 일치한다.
- `patch-Z.MPQ` 내부 `CreatureDisplayInfo.dbc` 해시가 `E:\server\data\karazhan\20260106\CreatureDisplayInfo.dbc`와 일치한다.
- `CreatureModelData.dbc.csv`, `CreatureDisplayInfo.dbc.csv`에서 한글 깨짐 대체 문자는 발견되지 않았다.
