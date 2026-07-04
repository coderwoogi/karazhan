# 스킬(스펠) 강화 컨텐츠 기획서

> 상태: 기획 (개발 전) · 작성일: 2026-06-17 · 대상: AzerothCore WotLK 3.3.5a

---

## 1. 목표

- 와우 기본 스킬을 **아이템 강화처럼 동적으로 강화**한다.
- **강화된 스펠을 미리 만들어 두지 않는다** (DBC에 강화판 스펠을 양산하지 않음).
- 플레이어별로 특정 스킬의 강화 레벨을 보유하고, 레벨에 따라 그 스킬의 **위력(데미지/힐 등)이 상승**한다.
- UX는 mod-item-karazhan(아이템 강화)과 유사: NPC/명령어로 스킬 선택 → 비용 지불 → 강화 레벨 상승(성공/실패).

---

## 2. 핵심 기술 제약 (먼저 이해할 것)

### 2.1 스펠 위력은 어디서 계산되나

- 기본값: `SpellInfo.Effects[].BasePoints` → `Unit::CalculateSpellDamage` → 시전자 보너스
  `Unit::SpellDamageBonusDone/SpellHealingBonusDone`(주문력·계수 반영) → 최종 데미지.
- 최종 가감 지점(전역 훅): `UnitScript`
  - `ModifySpellDamageTaken(target, attacker, int32& damage, SpellInfo const*)` — 직접 데미지
  - `ModifyHealReceived(target, healer, uint32& heal, SpellInfo const*)` — 힐
  - `ModifyPeriodicDamageAurasTick(target, attacker, uint32& damage, SpellInfo const*)` — DoT 틱
  - 모두 **모든 스펠/모든 시전자**에 대해 호출되며 `SpellInfo`로 어떤 스펠인지 구분 가능.

### 2.2 클라이언트 툴팁 문제 (아이템과 동일 구조)

- 클라이언트는 스펠 툴팁을 **로컬 Spell.dbc로 계산**해 그린다.
- 따라서 서버에서 데미지를 키워도(2.1 훅) **스펠북 툴팁 수치는 기본값 그대로** 표시된다.
- 예외: **SpellModifier 시스템**(탤런트 방식)은 `SMSG_SET_FLAT/PCT_SPELL_MODIFIER`
  패킷을 클라에 보내 **툴팁이 자동 재계산**된다. 단 제약이 큼(§3.2).

### 2.3 Spell.dbc 런타임 수정은 per-player 불가

- `SpellInfoCorrections`로 SpellInfo를 런타임 수정할 수 있으나 **전역(모든 플레이어 공유)**.
  특정 플레이어만 강화하는 데는 쓸 수 없다.

---

## 3. 접근 방식 비교

### 3.1 접근 A — 서버 시전-시점 스케일 (+ 애드온 표시) · **권장**

- 플레이어별 스킬 강화 레벨을 **별도 테이블**에 저장(메모리 캐시).
- `ModifySpellDamageTaken` / `ModifyHealReceived` / `ModifyPeriodicDamageAurasTick`
  에서, **시전자가 그 스킬의 강화를 보유**하면 `damage/heal × 배수`.
- 표시는 **커스텀 애드온**(mod-item-grade의 ItemGrade 애드온과 동일 방식)이 스펠
  툴팁에 "강화 +N (+X%)"를 덧붙인다.
- 장점: **임의의 단일 스킬을 정확히** 강화(스펠ID로 매칭), 완전 동적, DBC 무수정,
  강화판 스펠 미생성 → 목표에 정확히 부합.
- 단점: 기본 스펠 툴팁엔 네이티브로 반영 안 됨(애드온으로 보완). 위력(데미지/힐) 위주.

### 3.2 접근 B — SpellModifier 오라 (네이티브 툴팁) · 제약 큼

- `SPELL_AURA_ADD_PCT_MODIFIER` 오라로 스펠을 % 강화 → 클라 툴팁 자동 갱신.
- 장점: 네이티브 툴팁 반영(애드온 불필요), 데미지뿐 아니라 지속시간/마나/쿨다운 등
  `SpellModOp` 종류별 수정 가능.
- 단점(치명적):
  - 대상 매칭이 **SpellFamilyName + SpellFamilyFlags(96비트)** 기반 → **단일 스킬만
    콕 집어 강화하기 어렵다**(같은 family-bit를 공유하는 스펠들이 함께 강화됨).
  - 강화 강도별 **오라 스펠을 DBC에 미리 만들어야** 함(소량이지만 "스펠 사전 생성"에 해당).
- 결론: "임의의 스킬을 개별 강화"라는 목표와 충돌. 보조 수단으로만 고려.

### 3.3 접근 C — Spell.dbc 런타임 전역 수정 · **불가**

- per-player가 안 됨(§2.3). 제외.

> **권장: 접근 A.** 아이템 등급 시스템(서버 스케일 + 애드온)과 동일한 철학·구조라
> 운영/유지보수가 일관된다.

---

## 4. 권장 설계 (접근 A) 상세

### 4.1 강화 적용 (위력 스케일)

```
시전 → 데미지/힐 계산 → UnitScript 훅
   ModifySpellDamageTaken(target, attacker, damage, spellInfo):
     if attacker가 플레이어 && 강화레벨 = GetLevel(attacker, base(spellInfo)) > 0:
        damage = round(damage × 배수(강화레벨))
   ModifyHealReceived / ModifyPeriodicDamageAurasTick 동일
```

- `base(spellInfo)` = `sSpellMgr->GetFirstSpellInChain(spellInfo->Id)` →
  **스킬 랭크 통합**: Fireball 1~16랭크를 하나의 "스킬"로 보고 강화 공유.
- 배수: 강화 레벨 L → `1 + L × step`(예 step=2% → L10이면 +20%). config로 조정.

### 4.2 데이터 모델

별도 테이블(코어/Spell.dbc 무수정):

```sql
CREATE TABLE `player_spell_enhancement` (
    `owner_guid` INT UNSIGNED NOT NULL,
    `spell_id`   INT UNSIGNED NOT NULL,   -- 스킬 체인 첫 랭크 ID
    `level`      TINYINT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`owner_guid`, `spell_id`)
) ENGINE=InnoDB;
```

- 로그인 시 본인 데이터를 메모리 맵으로 로드(시전 훅에서 빠른 조회).
- 강화/캐릭터 삭제 시 갱신·정리(mod-item-karazhan 패턴 재사용).

### 4.3 강화 UX (mod-item-karazhan과 동형)

- NPC(또는 `.skillforge` 명령어)로 진입 → **배운 스킬 중 강화 가능 목록** 표시.
- 스킬 선택 → 비용(골드/재료) 지불 → 강화 시도:
  - 성공: 레벨 +1, 실패: 레벨 유지/하락(정책 선택), 비용 소모.
  - 레벨 상한(예: 0~10), 레벨별 성공률·비용 곡선(config).
- 진행 UI는 기존 karazhan UI 애드온과 유사하게 구성 가능.

### 4.4 표시 (애드온)

- 서버가 플레이어의 강화 스킬 목록(spellId→레벨/배수)을 애드온 메시지로 push.
- 애드온이 스펠북/액션바 툴팁(`GameTooltip:SetSpellByID` 등)에 "강화 +N (+X%)"와
  스케일된 예상 수치를 덧붙인다. (ItemGrade 애드온과 동일 골격)

---

## 5. 적용 범위 (효과 종류)

| 효과 | 훅 | 강화 가능 |
|------|----|:--:|
| 직접 데미지 | `ModifySpellDamageTaken` | ✅ |
| 지속 데미지(DoT) | `ModifyPeriodicDamageAurasTick` | ✅ |
| 직접 힐 | `ModifyHealReceived` | ✅ |
| 지속 힐(HoT) | 주기 힐 경로(확인 필요) | △ |
| 흡수막(보호막) | 전용 훅 없음 | ❌(별도 작업) |
| CC 지속시간·유틸리티 | 위력 개념 없음 | ❌(SpellMod 필요) |

- 우선 **데미지 + 힐(DoT 포함)** 을 1차 범위로. 흡수막/유틸은 추후.
- **강화 대상 스킬은 큐레이션 권장**: 강화 가능한 spellId 목록(또는 클래스/카테고리)을
  config/DB로 관리 → 발동/소환/아이템부여 스펠 등 부작용 큰 대상 제외.

---

## 6. 엣지 케이스 / 주의

- **랭크**: 체인 첫 랭크로 키 통일(§4.1) → 모든 랭크에 강화 적용.
- **PvP 밸런스**: 플레이어 스펠 데미지 상승은 PvP에 직접 영향. PvP 적용 여부/별도 배수
  정책 필요.
- **발동·트리거 스펠**: 같은 spellId로 트리거되는 효과까지 스케일될 수 있음 → 큐레이션으로 통제.
- **펫/소환수 스펠**: 시전자가 펫이면 주인 강화를 적용할지 결정.
- **기존 버프/탤런트와의 합연산**: 강화는 최종 배수로 적용(곱연산) → 과중첩 주의.
- **표시 불일치**: 애드온 미설치자는 기본 툴팁(미반영)을 봄. 서버 실제 위력은 정상.

---

## 7. 미결정 사항 (개발 착수 전 확정 필요)

1. **표시 방식**: 접근 A(서버 스케일 + 애드온) 확정? 아니면 일부 스킬은 접근 B로 네이티브 툴팁?
2. **강화 효과 범위**: 위력(데미지/힐/DoT)만? 쿨다운/마나 감소도 포함?(이건 SpellMod/별도)
3. **강화 대상**: 전체 학습 스킬 vs 큐레이션 목록(권장).
4. **강화 곡선**: 레벨 상한, 레벨당 증가율, 성공률·비용 곡선.
5. **PvP 적용**: 적용/미적용/별도 배수.
6. **진입 방식**: NPC vs 명령어, 성공/실패 정책(실패 시 레벨 하락 여부).

---

## 8. 참고 코드 위치

- 데미지/힐 수정 훅: `src/server/game/Scripting/ScriptDefines/UnitScript.h/.cpp`
  (`ModifySpellDamageTaken`, `ModifyHealReceived`, `ModifyPeriodicDamageAurasTick`)
- 호출 지점: `src/server/game/Entities/Unit/Unit.cpp`(데미지 1524 부근, 힐 8359 부근)
- 스펠 데미지 계산: `Unit::SpellDamageBonusDone` (`Unit.cpp:8723~`),
  `Unit::CalculateSpellDamage` (`Unit.cpp:11592~`)
- 랭크 체인: `SpellMgr::GetFirstSpellInChain`
- 스펠 보유 확인: `Player::HasSpell` (`Player.cpp:3881`)
- SpellModifier(접근 B): `Player::AddSpellMod`(`Player.cpp:9890`),
  `SpellInfo::IsAffectedBySpellMod`(`SpellInfo.cpp:1344`),
  `SpellAuraEffects.cpp:694`(오라→spellmod)
- 저장/UX 참고: `modules/mod-item-karazhan/` (강화레벨 저장·성공실패·UI 패턴)
