# 아이디어 / 미완료 작업 TODO (카라잔)

> 작성 2026-06-30 · 기획·감사만 하고 **적용을 멈춘** 항목들을 모아둔 백로그.
> 우선순위: 🔴 위험(크래시/악용) > 🟠 플레이 영향 > 🟡 개선/선택.

---

## 1. 🔴 애드온 부재 + 채팅 모듈 보안 하드닝  — *감사 완료, 적용 대기 (이번에 멈춘 부분)*

클라 애드온 미설치/구버전/비활성, 그리고 웹 입력 처리에서 **월드서버 크래시·골드 복제·플레이 잠금** 위험을 감사함. 4개 묶음으로 정리.

### 묶음 A — Lua 폴백  (✅ 재빌드 불필요, `.reload eluna`)
- `operate/lua_scripts/현자석.lua` `OnGossipHello`(L762) : 애드온 없으면 메뉴 접근 불가 → **네이티브 gossip 폴백** 추가(`HandleAction` 재사용).
- `operate/lua_scripts/이동술사.lua` `OnGossipHello`(L317) : 동일 → gossip 폴백 + 구버전 안내 메시지(L255 dead-click).
- `operate/lua_scripts/신규캐릭터_테스트지원.lua` L44 / L76·L80 : 미정의 변수(`err`/`itemGuidOrError`) → 콘솔 에러·오작동 버그 수정.

### 묶음 B — 크래시 차단  (⚠️ 재빌드)
- `modules/mod-item-karazhan/src/npc_item_karazhan.cpp:368,374,388` : `std::stoul()` 가 비숫자/초과 입력에 예외 → 서버 다운. **안전 파서(`Acore::StringTo`)+범위검사**.
- `modules/mod-web-chat/src/mod_web_chat.cpp` `ProcessOutgoing`(L191)·`ProcessGoldOps`(L224) : **행 처리 try/catch 없음** → 한 행 예외로 서버 다운. 행별 try/catch.
- `mod_web_chat.cpp` `ApplyGoldOp`(L245) : add/sub **비원자적** → 크래시 시 재처리로 **골드 복제**. 적용 전 `processing` 선마킹 또는 트랜잭션. `mode` 화이트리스트.

### 묶음 C — 플레이 잠금 폴백  (⚠️ 재빌드)
- `modules/mod-solo-arena/src/SoloArena.cpp` `OnGossipHello`(:7698) : 시련 입장/티켓/포기/보상이 애드온 전용 → **NPC gossip 폴백**(단계선택·티켓·포기).
- `modules/mod-instance-bonus-mission/src/InstanceBonusMission.cpp` 투표(:2310) : 미션이 애드온 투표로만 시작 → **`.bonus yes/no` 명령 또는 타임아웃 자동승인**.

### 묶음 D — 안정성/손실/스푸핑  (⚠️ 재빌드)
- `mod-item-karazhan` `ItemKarazhan.cpp` : `DO` 연타 시 큐 1000 초과분 무음 폐기 → **재료/골드만 소모, 환불 없음**. 1인 쿨다운/중복가드. 또 `const_cast`로 공유 아이템명 변조(:666) → 로컬 변수로.
- `mod-item-grade` `ItemGrade.cpp:625` : `CMSG_INSPECT` `copy >> guid` 예외 가능 → `ByteBufferException` 가드. 미배포 시 `AddonPush=0`/`ForceResyncIntervalMs↑` 권장.
- `mod-web-chat` : Process* **비동기화**(현재 월드스레드 동기 쿼리로 틱 스톨), 주입 **메시지 길이 캡**, `gm_mark`/`sender` **스푸핑 게이트**(GM 태그·이름 사칭), "channel"이 전 접속자에 송출(:330) → 채널 멤버 한정.

**다음 단계**: A는 즉시 적용 가능. B/C/D는 재빌드 시점에 일괄 패치. (권장 순서 B→C→A→D)

---

## 2. 🟡 ItemGrade 등급 차이 개선  — *옵션만 제시, 미선택*
- 증상: 기본 스탯 작은 아이템은 B(×1.04)·A(×1.06) 보너스가 정수 반올림으로 둘 다 +1.
- 옵션 A: 배수 간격 확대(conf, 무재빌드) / B: 등급별 **고정 보너스**(재빌드, 작은스탯도 차이 보장) / C: 무기 데미지도 스케일(재빌드).
- 다음 단계: 방향 선택 필요.

## 3. 🟡 "This server runs on AzerothCore" 제거  — *원인 규명, 수정 미적용*
- 메시지는 코어 런타임 생성. 차단 모듈 `mod-login-info-filter` 존재(`LoginInfoFilter.cpp`, `Enable=1`)하나 검사 opcode가 `SMSG_MESSAGECHAT/GM/NOTIFICATION/AREA_TRIGGER`만이라 **`SMSG_MOTD` 등 다른 경로는 못 막음**.
- 다음 단계: 실제 발신 opcode 특정 → 필터에 opcode/패턴 추가(재빌드).

## 4. 🟡 현자대사 보상 버프 "끈 상태 영구 기억"  — *제안만 함*
- 현재: 칭호 보유자 로그인 시 22818 재적용(끄면 세션 내 유지, 재접속 시 다시 켜짐).
- 아이디어: 캐릭터별 on/off 선호 저장(커스텀 플래그/테이블) → 껐으면 재접속해도 유지.

## 5. 🟡 시련 입장권(600022) 포인트 상점 판매 추가  — *제안만 함*
- 현재 보스 드랍 전용. 원하면 `update.point_shop_items`에 `item_type=game, item_entry=600022` 추가로 웹 구매 가능.

---

## 6. 🟠 GM 계정 업적 차단 (RBAC)  — *기획됨, 미적용*
- 목표: GM 계정은 와우 업적이 쌓이지 않게.
- 방법: `RBAC_PERM_CANNOT_EARN_ACHIEVEMENTS`(코어 보유 권한)를 GM 보안등급에 부여.
- 다음 단계: 어떤 보안등급/계정 범위에 적용할지 결정 후 `rbac_*` 반영.

## 7. 🟡 전문기술/스킬 강화 구현  — *plan 문서 존재*
- `doc/skill_enhancement_plan_ko.md`, `doc/item_grade_scaling_plan_ko.md` 참고. 구현 미착수.

## 8. 🟠 대규모 전투 (플레이어 vs 봇 PvP)  — *기획서 작성됨, 개발 미착수*
- 기획서: `E:\server\data\대규모 전투\` (개요~로드맵 6종).
- 신규 모듈 `mod-mass-battle` (또는 mod-solo-arena 자산 재사용), MVP 8~11주.
- 다음 단계: 봇 엔진 방향(A playerbots / B 크리처확장 / C 하이브리드) 확정 후 착수.

## 9. 🟢 환생 (Reincarnation / Prestige)  — *신규 아이디어*

**개요**: 만렙 도달 후 환생 → 1레벨로 초기화. 다시 키워 만렙을 재달성할 때마다 **영구 누적 보너스를 3택1**로 획득. 아이템·스펠·특성은 유지. 반복할수록 캐릭터가 영구 성장(로그라이크식 prestige).

**사용자 사양**
1. 만렙 달성 시 특정 NPC에게 환생 신청.
2. 환생 시 1레벨로 초기화.
3. 다시 만렙 달성 시 화면에 3가지 옵션이 랜덤 출력.
4. 3택1 선택 → 영구적으로 증가.
5. 재환생 시 선택했던 옵션은 그대로 두고 다시 1레벨.
6. 아이템·스펠·특성은 그대로.
7. 반복하면 큰 성장.

**동작 흐름(안)**
- 환생 NPC(gossip) → 만렙 확인 → 확인창 → `환생 카운트++`, 레벨 1로 초기화(스펠/특성/글리프/아이템/가방 유지).
- 만렙 재달성 감지(`OnLevelChanged` == MaxLevel & 환생카운트>0 & 이번 사이클 보상 미수령) → 보너스 풀에서 3개 랜덤 추첨 → UI 표시.
- 선택 → 누적 저장 → 즉시 + 매 로그인 시 영구 적용.

**기술 스케치(구현 시)**
- 신규 모듈 `mod-reincarnation` (PlayerScript: `OnLevelChanged`/`OnLogin`, CreatureScript: 환생 NPC gossip).
- DB: `character_reincarnation`(guid, count, pending_choice), `character_reincarnation_bonus`(guid, bonus_id, stacks).
- 보너스 적용: 로그인 시 누적 보너스를 stat/aura로 재적용(서버 권위적).
- 레벨 초기화: `SetLevel(1)` + XP 0. ⚠️ **주의**: 특성/글리프/자동학습 스펠이 레벨 하향에 영향받을 수 있음 → 환생 시 특성·스펠·글리프 보존/복원 처리 필수(레벨1에서 80렙 특성 보유 = 비표준, 검증 필요).

**결정 필요**
- 보너스 풀 구성(예: 전 스탯 %, 최대체력 %, 치명타, 공격/주문력, 이동속도 등) + 종류별 상한·스택 한도.
- 환생 횟수 제한 여부 / 만렙 기준(80).
- 초기화 범위: 레벨만? (골드·평판·전문기술·퀘스트는 유지?) — 사용자는 아이템·스펠·특성 유지만 명시.
- 3택 UI: 기본 gossip vs 전용 애드온 — 애드온이면 **부재 폴백 필요**(위 1번 하드닝 원칙과 동일).
- 만렙 재달성 보상 트리거가 "최초 만렙"과 겹치지 않도록(환생 카운트>0 조건).

---

### 메모
- C++ 변경(B/C/D, 3·6·8 일부)은 **재빌드 필요** — 빌드 가능 시점에 묶어서 진행.
- 모든 적용 완료 시 작업 로그(Google Sheet) 기록.
