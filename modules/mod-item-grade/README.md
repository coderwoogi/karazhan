# mod-item-grade

AzerothCore (WotLK 3.3.5a) 모듈 — **아이템 등급(Grade) 스탯 스케일링**.

모두가 같은 베이스 아이템을 드랍하지만, 인스턴스마다 등급(S/A/B/C/D)이 부여되어
`item_template` 의 **기본 스탯이 등급 배수로 가감(+/-)** 된다.

## 동작 원리 (접근 A + 저장형)

- 등급은 **몬스터(크리처) 또는 상자/오브젝트(GameObject)를 루팅한 아이템에만**
  부여된다(`OnPlayerLootItem` 에서 `lootguid.IsCreature() || IsGameObject()` 판정).
  상점/제작/퀘스트/우편 등으로 얻은 아이템은 등급이 없다(= 스케일 안 함).
- 부여된 등급은 별도 테이블 `mod_item_grade(item_guid, grade)` 에 저장되고
  메모리에 캐싱된다. 테이블은 서버 기동 시 자동 생성된다(`CREATE TABLE IF NOT EXISTS`).
  `item_template` / `item_instance` 스키마는 **수정하지 않는다.**
- 장착 시 `Player::_ApplyItemBonuses` 의 `OnPlayerApplyItemModsBefore` 훅에서
  기본 옵션값(`ItemStat[].ItemStatValue`)에 등급 배수를 곱한다. 등급이 고정 저장
  값이므로 장착/해제 시 항상 동일하게 적용된다(능력치 오염 없음).

### 적용 범위

- 대상: **몬스터/상자 드랍 + 모든 품질(흰/회/초록/파랑/보라/주황)의 무기·방어구·
  장신구.** 스탯 유무·세트 여부와 무관(`Class == WEAPON || ARMOR`).
- 제외: 소비/재료/잡템/퀘스트 아이템, 비-루팅 획득(상점/제작/퀘스트보상/우편 등).
- 스케일 대상:
  - **기본 스탯**(`ItemStat[]`) — `OnPlayerApplyItemModsBefore`
  - **랜덤 속성/접미사 스탯**("곰의~" 등 초록템) — `OnPlayerApplyEnchantmentItemModsBefore`
    의 PROP 슬롯만(마부/보석 제외)
- 스케일 안 됨: 방어구 수치(Armor)·무기 데미지(DPS)·저항·세트 보너스·발동 효과.
  → 이런 값만 있는 무스탯 아이템은 등급 라벨은 보이되 수치 변동은 없다.

### 강화/마부 호환

- **마부(인챈트)**: 별도 경로(`ApplyEnchantment`)라 등급 영향 없음. GUID 불변이라
  등급 그대로 유지.
- **GUID 유지형 강화**(예: mod-custom-changes): 등급 유지.
- **GUID 교체형 강화**(mod-item-karazhan): 강화 시 새 아이템이 생기므로,
  karazhan 재생성 지점에서 `ItemGradeBridge::OnItemRecreated()` 를 호출해
  **기존 등급을 새 아이템으로 이전**한다(코드 연동 적용됨).
- 삭제된 아이템의 등급 행은 `OnItemDelFromDB` 에서 정리된다(고아행 방지).

## 클라이언트 표시 (선택)

- 3.3.5 클라는 기본 스탯/이름을 itemEntry 단위로 캐싱하므로, **기본 툴팁만으로는
  인스턴스별 변동 수치를 표시할 수 없다.**
- 동봉 애드온 `ItemGrade`(클라 `Interface/AddOns/ItemGrade`)를 설치하면, 서버가
  푸시하는 등급 정보를 받아 툴팁에 **등급 라벨 + 스케일된 스탯 수치**를 표시한다.
- 서버는 로그인 / 아이템 장착 / 신규 아이템 획득 시 클라 좌표(가방·슬롯)별 등급을
  애드온 메시지(prefix `IGRADE`)로 전송한다.
- **다른 플레이어의 아이템도 표시**(코어 무수정):
  - **살펴보기(Inspect)**: `ServerScript::CanPacketReceive` 로 `CMSG_INSPECT` 를
    가로채, 인스펙터에게 대상 장비 등급을 전송(`ICLR/IADD/IEND`). 애드온은 살펴보기
    툴팁(`SetInventoryItem(상대unit, slot)`)에 표시.
  - **거래(Trade)**: `OnPlayerCanSetTradeItem` 훅에서 거래 상대에게 내가 올린 아이템
    등급을 전송(`TSET/TDEL`). 애드온은 거래창 툴팁(`SetTradeTargetItem`)에 표시.
  - **보는 사람**(인스펙터/거래 상대)이 ItemGrade 애드온을 설치해야 보인다.
    아이템 주인은 애드온이 없어도 됨(서버가 등급을 보는 사람에게 전달).

## GM 명령어

GM 등급(`SEC_GAMEMASTER`) 이상에서 사용:

- `.itemgrade add <itemId> <S/A/B/C/D>` — 지정 등급으로 아이템을 생성해 본인에게 지급.
  등급은 글자(S~D) 또는 숫자(0=D ~ 4=S)로 입력. 예: `.itemgrade add 49623 S`
- `.itemgrade info` — 현재 장착 장비의 등급·배수와 각 스탯의 **기본 → 적용값**을
  출력(스탯 반영 검증용). 초록 접미사("곰의~") 스탯은 기본 스탯이 아니므로 여기엔
  안 나오고 캐릭터 시트에서 확인.

## 설정

`conf/mod_item_grade.conf.dist` → `mod_item_grade.conf` 참고. 등급별 확률·배수,
획득 알림, 애드온 푸시 여부를 조정할 수 있다.

## 빌드/설치

1. 본 디렉터리를 `modules/` 아래에 둔 채로 CMake 재구성 + 빌드(static/dynamic).
2. `mod_item_grade.conf.dist` 를 서버 conf 디렉터리에 `mod_item_grade.conf` 로 복사.
3. (선택) 클라이언트에 `ItemGrade` 애드온 설치.

## 표시 동기화

- 서버가 등급을 클라 좌표(가방/슬롯)별로 푸시하므로, 아이템을 이동·재배치하면
  슬롯 매핑이 잠시 어긋날 수 있다. 이를 막기 위해 서버가 **인벤토리 변화를
  주기적으로 감지(`ItemGrade.SyncIntervalMs`, 기본 1초)해 자동 재전송**한다.
  로그인/장착/획득/인벤토리 이동 시에는 즉시 갱신된다.
- 같은 아이템을 여러 개 보유해도 각 인스턴스의 등급이 슬롯별로 올바르게 표시된다.

## 알려진 한계

- 애드온 미설치 시 기본 툴팁에는 변동 수치가 보이지 않는다(서버 능력치는 정상 적용).
- 비교 툴팁(장착 중 아이템과 나란히 뜨는 ShoppingTooltip)·채팅 링크·경매장
  툴팁에는 등급이 표시되지 않는다(가방/장비 호버에만 표시).
- `ItemGrade.Mult.*` 를 운영 중 변경하면, 변경 전 장착 아이템은 재접속 전까지
  능력치가 어긋날 수 있다.

자세한 설계는 `doc/item_grade_scaling_plan_ko.md` 참고.
