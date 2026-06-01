# AzerothCore 크리처 룻 테이블 초보자 가이드

이 문서는 AzerothCore에서 몬스터가 아이템을 드랍하는 구조를 비개발자도 이해할 수 있도록 설명합니다.

핵심만 먼저 말하면, 몬스터 룻은 단순히 "아이템 A가 10%로 나온다"처럼만 구성되지 않습니다. AzerothCore의 룻 시스템은 "어떤 몬스터가 어떤 룻 목록을 쓰는지", "각 아이템이 독립적으로 굴러가는지", "그룹 안에서 하나만 선택되는지", "다른 룻 묶음을 참조하는지"를 함께 봐야 정확히 이해할 수 있습니다.

## 1. 몬스터 룻은 어디서 시작되나

몬스터가 죽었을 때 어떤 룻을 줄지는 보통 `creature_template` 테이블의 `lootid` 값에서 시작합니다.

| 테이블 | 역할 |
|---|---|
| `creature_template` | 몬스터 기본 정보 |
| `creature_template.lootid` | 이 몬스터가 사용할 룻 목록 번호 |
| `creature_loot_template.Entry` | 실제 룻 목록 번호 |

예를 들어 어떤 몬스터의 `lootid`가 `12345`라면, 실제 룻 목록은 `creature_loot_template`에서 `Entry = 12345`인 행들을 봐야 합니다.

```sql
SELECT entry, name, lootid
FROM creature_template
WHERE entry = 몬스터ID;

SELECT *
FROM creature_loot_template
WHERE Entry = 룻ID
ORDER BY GroupId, Chance DESC;
```

## 2. creature_loot_template의 주요 컬럼

`creature_loot_template`는 몬스터가 어떤 아이템을 드랍할 수 있는지 정의하는 테이블입니다.

| 컬럼 | 쉬운 설명 |
|---|---|
| `Entry` | 룻 목록 번호입니다. 보통 `creature_template.lootid`와 연결됩니다. |
| `Item` | 드랍될 아이템 ID입니다. |
| `Reference` | 다른 룻 묶음을 가져올 때 사용하는 번호입니다. |
| `Chance` | 드랍 확률입니다. 단, 그룹 여부에 따라 의미가 달라집니다. |
| `QuestRequired` | 특정 퀘스트가 있어야 보이는 룻인지 여부입니다. |
| `LootMode` | 일반/영웅/난이도 같은 룻 모드 조건입니다. |
| `GroupId` | 같은 그룹 안에서 하나를 고르는 룻 묶음 번호입니다. |
| `MinCount` | 최소 드랍 수량입니다. |
| `MaxCount` | 최대 드랍 수량입니다. |
| `Comment` | 사람이 보기 위한 설명입니다. |

## 3. 가장 기본적인 룻 방식: 독립 드랍

`GroupId = 0`인 아이템은 보통 각각 따로 확률을 굴립니다.

예시:

| Item | Chance | GroupId | 의미 |
|---:|---:|---:|---|
| 1001 | 50 | 0 | 50% 확률로 따로 드랍 |
| 1002 | 20 | 0 | 20% 확률로 따로 드랍 |
| 1003 | 5 | 0 | 5% 확률로 따로 드랍 |

이 경우 세 아이템은 서로 독립입니다.

즉, 운이 좋으면 1001, 1002, 1003이 한 번에 모두 나올 수도 있고, 아무것도 안 나올 수도 있습니다.

## 4. 그룹 룻: 여러 개 중 하나만 고르는 방식

`GroupId`가 0보다 크면, 같은 `Entry`와 같은 `GroupId`를 가진 아이템들은 하나의 그룹으로 묶입니다.

쉽게 말하면 "이 목록 중 하나를 골라라"입니다.

예시:

| Item | Chance | GroupId | 의미 |
|---:|---:|---:|---|
| 2001 | 0 | 1 | 그룹 1 후보 |
| 2002 | 0 | 1 | 그룹 1 후보 |
| 2003 | 0 | 1 | 그룹 1 후보 |

이 경우 `Chance`가 모두 `0`이어도 드랍이 안 되는 것이 아닙니다.

그룹 안에서 세 아이템 중 하나가 선택됩니다.

| Item | 실제 느낌 |
|---:|---:|
| 2001 | 약 33.33% |
| 2002 | 약 33.33% |
| 2003 | 약 33.33% |

## 5. 0.0%인데 드랍되는 이유

DB에서 `Chance = 0.0`으로 보이는데 실제로 드랍되는 경우가 있습니다.

이것은 대부분 버그가 아니라 룻 시스템의 정상 동작입니다.

| 상황 | `0.0%`의 의미 |
|---|---|
| `GroupId > 0`이고 같은 그룹에 여러 아이템이 있음 | 남은 확률을 자동으로 나눠 가짐 |
| 같은 그룹의 아이템이 전부 `0.0%`임 | 서로 균등 확률로 선택됨 |
| `Reference`가 0이 아님 | 실제 확률은 다른 룻 테이블에서 계산됨 |
| `GroupId = 0`, `Reference = 0`, `Chance = 0` | 보통 직접 드랍 설정으로는 부자연스러움 |

예시:

| Item | Chance | GroupId |
|---:|---:|---:|
| 3001 | 20 | 1 |
| 3002 | 30 | 1 |
| 3003 | 0 | 1 |
| 3004 | 0 | 1 |

이 경우 3001은 20%, 3002는 30%입니다.

남은 50%를 3003과 3004가 나눠 가집니다.

| Item | 실제 확률 |
|---:|---:|
| 3001 | 20% |
| 3002 | 30% |
| 3003 | 25% |
| 3004 | 25% |

그래서 `0.0%`는 항상 "안 나옴"이 아닙니다.

그룹 안에서는 "남은 확률을 알아서 나눠 가져라"라는 의미로 쓰일 수 있습니다.

## 6. Reference 룻이란

`Reference`는 "다른 룻 묶음을 가져와서 사용하라"는 뜻입니다.

예를 들어 여러 몬스터가 같은 보석 묶음, 같은 전문기술 재료 묶음, 같은 월드 드랍 묶음을 공유할 때 사용합니다.

| 테이블 | 역할 |
|---|---|
| `creature_loot_template` | 몬스터가 직접 사용하는 룻 목록 |
| `reference_loot_template` | 여러 곳에서 재사용할 수 있는 공용 룻 묶음 |

예시:

| Entry | Item | Reference | Chance | 의미 |
|---:|---:|---:|---:|---|
| 12345 | 0 | 90000 | 10 | 10% 확률로 참조 룻 90000을 사용 |

이 경우 실제 아이템은 `creature_loot_template`에 직접 없을 수 있습니다.

대신 `reference_loot_template`에서 `Entry = 90000`인 목록을 다시 봐야 합니다.

```sql
SELECT *
FROM reference_loot_template
WHERE Entry = 90000
ORDER BY GroupId, Chance DESC;
```

## 7. 수량은 Chance가 아니라 MinCount와 MaxCount가 정한다

아이템이 몇 개 나오는지는 `Chance`가 아니라 `MinCount`, `MaxCount`가 정합니다.

| Chance | MinCount | MaxCount | 의미 |
|---:|---:|---:|---|
| 50 | 1 | 1 | 50% 확률로 1개 드랍 |
| 50 | 2 | 5 | 50% 확률로 2개에서 5개 드랍 |
| 100 | 3 | 3 | 항상 3개 드랍 |

즉, `Chance`는 "나올지 말지"이고, `MinCount`, `MaxCount`는 "나온다면 몇 개 나올지"입니다.

## 8. QuestRequired는 퀘스트 조건이다

`QuestRequired`가 1이면, 보통 관련 퀘스트를 가진 플레이어에게만 보이는 룻입니다.

| QuestRequired | 의미 |
|---:|---|
| 0 | 일반 룻 |
| 1 | 퀘스트 조건이 있을 때만 보이는 룻 |

그래서 어떤 아이템이 DB에는 있는데 일반 사냥에서는 안 보인다면, `QuestRequired`를 확인해야 합니다.

## 9. LootMode는 난이도나 모드 조건이다

`LootMode`는 해당 룻이 어떤 모드에서 적용되는지 정하는 값입니다.

예를 들어 일반 던전, 영웅 던전, 공격대 난이도 등에 따라 다른 룻을 쓰게 할 수 있습니다.

초보자 관점에서는 이렇게 이해하면 됩니다.

| LootMode | 쉬운 의미 |
|---:|---|
| 1 | 기본 룻 모드에서 사용 |
| 그 외 값 | 특정 난이도나 조건에서만 사용될 수 있음 |

정확한 의미는 코어 설정과 난이도 구조에 따라 달라질 수 있으므로, 특정 몬스터의 룻을 수정할 때는 기존 같은 몬스터나 같은 던전의 다른 룻 설정을 참고하는 것이 안전합니다.

## 10. 룻을 확인할 때 보는 순서

룻 문제를 확인할 때는 아래 순서로 보면 됩니다.

1. 몬스터의 `lootid`를 확인합니다.
2. `creature_loot_template`에서 해당 `Entry`를 찾습니다.
3. `GroupId`가 0인지 아닌지 확인합니다.
4. `Reference`가 0인지 아닌지 확인합니다.
5. `Reference`가 있으면 `reference_loot_template`도 확인합니다.
6. `Chance`, `MinCount`, `MaxCount`를 확인합니다.
7. `QuestRequired`, `LootMode` 조건을 확인합니다.

자주 쓰는 확인 쿼리:

```sql
-- 몬스터가 어떤 룻 ID를 쓰는지 확인
SELECT entry, name, lootid
FROM creature_template
WHERE entry = 몬스터ID;

-- 몬스터 룻 목록 확인
SELECT *
FROM creature_loot_template
WHERE Entry = 룻ID
ORDER BY GroupId, Chance DESC, Item;

-- 참조 룻 목록 확인
SELECT *
FROM reference_loot_template
WHERE Entry = 참조ID
ORDER BY GroupId, Chance DESC, Item;
```

## 11. 흔한 오해

| 오해 | 실제 |
|---|---|
| `0.0%`면 절대 안 나온다 | 그룹 룻에서는 정상적으로 나올 수 있습니다. |
| `Chance`만 보면 실제 확률을 알 수 있다 | `GroupId`, `Reference`까지 같이 봐야 합니다. |
| `Item`에 적힌 값이 항상 실제 드랍 아이템이다 | `Reference`가 있으면 실제 아이템은 참조 테이블에 있을 수 있습니다. |
| 수량은 확률과 관련 있다 | 수량은 `MinCount`, `MaxCount`가 정합니다. |
| DB에 있는데 안 나온다면 확률 문제다 | 퀘스트 조건, 룻 모드, 참조 테이블 문제일 수도 있습니다. |

## 12. 간단한 비유

룻 테이블은 뽑기 상자라고 보면 됩니다.

`GroupId = 0`은 각각 따로 뽑는 뽑기입니다.

예를 들어 사탕 50%, 초콜릿 20%, 쿠키 5%를 각각 따로 뽑습니다. 운이 좋으면 셋 다 나올 수 있습니다.

`GroupId > 0`은 한 상자 안에서 하나만 뽑는 방식입니다.

예를 들어 칼, 도끼, 지팡이가 같은 그룹이면 그중 하나만 선택됩니다.

`Reference`는 다른 상자를 가져오는 방식입니다.

예를 들어 몬스터 룻 상자 안에 "보석 상자 열기"가 들어 있고, 실제 보석 목록은 `reference_loot_template`에 따로 있는 구조입니다.

## 13. 실무에서 안전하게 수정하는 방법

룻을 수정할 때는 기존 구조를 유지하는 것이 안전합니다.

| 하고 싶은 일 | 권장 방식 |
|---|---|
| 특정 아이템을 독립 확률로 추가 | `GroupId = 0`, 원하는 `Chance` 지정 |
| 여러 아이템 중 하나만 나오게 하기 | 같은 `GroupId`로 묶기 |
| 같은 룻 묶음을 여러 몬스터가 쓰게 하기 | `reference_loot_template` 사용 |
| 무조건 드랍하게 하기 | `Chance = 100`, `MinCount`, `MaxCount` 설정 |
| 퀘스트 아이템으로 만들기 | `QuestRequired = 1` 여부 검토 |

`0.0%`가 보인다고 바로 잘못된 데이터로 판단하면 안 됩니다.

반드시 `GroupId`와 `Reference`를 같이 확인해야 합니다.
