# 영웅석 룬문자 퀘스트 보상 기획안

## 목표

950010부터 950240까지 존재하는 영웅석 순간이동 룬문자 아이템을, 각 룬문자에 대응되는 지역의 퀘스트 완료 시 낮은 확률로 보상 지급한다.

룬문자 보상은 기존 퀘스트 보상 구조를 직접 덮어쓰지 않고, 별도 확률 보상 시스템으로 동작해야 한다. 기존 퀘스트 보상, 골드, 경험치, 아이템 보상에는 영향을 주지 않는다.

## 기본 정책

| 항목 | 정책 |
|---|---|
| 대상 아이템 | 950010 ~ 950240 룬문자 아이템 |
| 지급 시점 | 퀘스트 완료 보상 수령 직후 |
| 지급 방식 | 확률 판정 성공 시 가방 지급 또는 우편 지급 |
| 추천 지급 방식 | 우편 지급 |
| 중복 획득 | 이미 계정에 등록된 룬문자는 다시 지급하지 않음 |
| 구독자 처리 | 구독자는 모든 이동지가 해금 상태이므로 룬문자 지급 제외 권장 |
| 대상 퀘스트 | 룬문자 위치와 같은 지역의 일반 퀘스트 |
| 제외 퀘스트 | 반복 퀘스트, 이벤트 퀘스트, 내부 퀘스트, 직업 전용 퀘스트는 기본 제외 |

## 권장 구현 방식

DB의 `quest_template` 보상 컬럼을 직접 수정하지 않고, 별도 테이블과 Lua 또는 C++ 스크립트로 확률 지급한다.

직접 `quest_template`의 보상 아이템 칸에 룬문자를 넣는 방식은 권장하지 않는다. 이유는 다음과 같다.

| 방식 | 장점 | 문제점 |
|---|---|---|
| quest_template 보상 직접 수정 | 구현이 단순함 | 확률 보상이 어려움, 기존 보상 슬롯을 침범함, 231개 아이템 관리가 어려움 |
| SmartScript 사용 | 일부 조건 처리 가능 | 퀘스트/지역/계정 해금 조건 관리가 복잡함 |
| Lua OnQuestReward 사용 | 서버 재빌드 없이 적용 가능, 확률 처리 쉬움 | 성능을 위해 캐싱 필요 |
| C++ PlayerScript 사용 | 가장 안정적이고 빠름 | 코어 재빌드 필요 |

1차 구현은 Lua 방식이 적합하다. 기능 검증 후 안정화되면 C++ 모듈로 이전할 수 있다.

## 신규 테이블 설계

### 1. 룬문자 지역 보상 설정 테이블

```sql
CREATE TABLE IF NOT EXISTS `hero_stone_rune_quest_reward_rules` (
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `location_id` INT UNSIGNED NOT NULL,
  `rune_item_id` INT UNSIGNED NOT NULL,
  `zone_id` INT UNSIGNED NOT NULL,
  `chance` FLOAT NOT NULL DEFAULT 1,
  `min_level` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `max_level` TINYINT UNSIGNED NOT NULL DEFAULT 80,
  `enabled` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  `comment` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `idx_zone_id` (`zone_id`),
  KEY `idx_rune_item_id` (`rune_item_id`),
  KEY `idx_location_id` (`location_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

이 테이블은 특정 지역 퀘스트 완료 시 어떤 룬문자를 몇 퍼센트 확률로 지급할지 정의한다.

예시:

| location_id | rune_item_id | zone_id | chance | comment |
|---:|---:|---:|---:|---|
| 1 | 950010 | 14 | 1.0 | 듀로타 룬문자 |
| 2 | 950011 | 85 | 1.0 | 티리스팔 숲 룬문자 |
| 3 | 950012 | 1 | 0.5 | 던 모로 룬문자 |

### 2. 지급 로그 테이블

```sql
CREATE TABLE IF NOT EXISTS `character_hero_stone_rune_reward_log` (
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT UNSIGNED NOT NULL,
  `character_guid` INT UNSIGNED NOT NULL,
  `quest_id` INT UNSIGNED NOT NULL,
  `location_id` INT UNSIGNED NOT NULL,
  `rune_item_id` INT UNSIGNED NOT NULL,
  `zone_id` INT UNSIGNED NOT NULL,
  `result` VARCHAR(32) NOT NULL,
  `created_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_account_id` (`account_id`),
  KEY `idx_character_guid` (`character_guid`),
  KEY `idx_quest_id` (`quest_id`),
  KEY `idx_rune_item_id` (`rune_item_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

지급 여부, 확률 실패, 이미 보유한 룬문자 제외 등을 추적하기 위한 로그 테이블이다.

## 지역 매칭 방식

퀘스트가 어느 지역의 퀘스트인지 판단하는 기준은 아래 순서로 정한다.

1. `quest_template.ZoneOrSort` 값이 양수이면 해당 값을 지역 ID로 사용한다.
2. `ZoneOrSort`가 0이거나 신뢰하기 어려운 경우, 퀘스트 시작 NPC의 위치 지역을 사용한다.
3. 시작 NPC가 여러 명이면 가장 많이 등장하는 지역을 대표 지역으로 사용한다.
4. 그래도 판단이 어렵다면 수동 예외 테이블로 관리한다.

초기 구현은 `quest_template.ZoneOrSort` 기준으로 시작하는 것이 가장 단순하다.

## 확률 정책

기본 확률은 낮게 시작하는 것을 권장한다.

| 룬문자 가치 | 권장 확률 |
|---|---:|
| 일반 지역 | 1.0% |
| 주요 도시 | 0.5% |
| 희귀 이동지 | 0.2% ~ 0.5% |
| 이벤트성 이동지 | 비활성 또는 수동 지급 |

모든 룬문자에 동일 확률을 적용하면 주요 도시 룬문자의 가치가 낮아질 수 있다. 도시, 던전 입구, 희귀 지역은 별도 확률을 두는 편이 좋다.

## 중복 지급 방지

룬문자는 현재 계정 기준 해금 구조이므로, 지급 전 아래를 확인한다.

1. 플레이어의 `account_id` 확인
2. `character_hero_stone_teleport_runes`에서 `account_id + location_id` 존재 여부 확인
3. 이미 해금된 룬문자라면 지급하지 않음
4. 미해금 상태이고 확률 판정에 성공하면 룬문자 아이템 지급

아이템을 이미 가방에 가지고 있지만 아직 사용하지 않은 경우는 중복 지급 가능성이 있다. 이를 막으려면 지급 전 `player:HasItem(rune_item_id)`도 함께 확인한다.

## 구독자 처리

구독자는 영웅석 순간이동이 전체 해금 상태로 동작한다. 따라서 구독 중에는 룬문자 아이템 지급을 제외하는 것이 좋다.

정책 문구:

> 구독 중에는 모든 룬문자가 새겨진 상태로 이용되므로 퀘스트 보상 룬문자가 지급되지 않습니다.

구독 해지 후에는 기존에 직접 등록한 룬문자만 남고, 퀘스트를 통해 다시 룬문자를 획득할 수 있다.

## 지급 방식

### 권장: 우편 지급

가방이 가득 찬 상황을 피하고, 획득 기록을 명확하게 남길 수 있다.

우편 제목:

> 영웅석 룬문자 발견

우편 내용:

> 퀘스트를 완료하는 과정에서 새로운 순간이동 룬문자를 발견했습니다.
> 아이템을 사용하면 영웅석에 해당 위치가 새겨집니다.

첨부 아이템:

> 룬문자 : 위치 이름

### 대안: 즉시 가방 지급

가방 지급은 직관적이지만, 가방이 가득 찬 경우 처리 로직이 필요하다. 가방 지급 실패 시 우편으로 전환하는 방식도 가능하다.

## Lua 처리 흐름

```text
플레이어가 퀘스트 완료
        ↓
QuestReward 이벤트 발생
        ↓
퀘스트의 ZoneOrSort 확인
        ↓
hero_stone_rune_quest_reward_rules에서 zone_id 매칭
        ↓
활성화된 룰 중 확률 판정
        ↓
계정 기준 이미 해금된 룬문자인지 확인
        ↓
가방에 같은 룬문자 아이템이 있는지 확인
        ↓
조건 통과 시 우편으로 룬문자 지급
        ↓
character_hero_stone_rune_reward_log 기록
```

## 밸런스 주의사항

반복 퀘스트와 일일 퀘스트는 룬문자 파밍 수단이 될 수 있다. 초기에는 일반 퀘스트만 대상으로 잡는 것이 안전하다.

확률은 서버 경제와 이동 편의성에 직접 영향을 준다. 처음에는 0.5% ~ 1.0%로 낮게 시작하고, 실제 획득량을 보고 조정하는 방식이 좋다.

## 단계별 진행안

### 1단계: 데이터 매핑

1. `hero_stone_teleport_locations`의 231개 위치를 확인한다.
2. 각 위치를 지역 ID와 매칭한다.
3. 매칭된 데이터를 `hero_stone_rune_quest_reward_rules`에 추가한다.

### 2단계: Lua 확률 지급 구현

1. 퀘스트 완료 이벤트를 등록한다.
2. 퀘스트 지역을 확인한다.
3. 룰 테이블에서 해당 지역 룬문자를 조회한다.
4. 확률 판정 후 우편 지급한다.
5. 지급 로그를 남긴다.

### 3단계: 중복/구독 예외 처리

1. 계정 기준 해금 여부 확인
2. 구독자 지급 제외
3. 이미 가방에 있는 룬문자 지급 제외
4. 반복 퀘스트 제외

### 4단계: 운영 명령어 추가

운영 편의를 위해 아래 명령어를 추가할 수 있다.

| 명령어 | 기능 |
|---|---|
| `.herorune reload` | 룬문자 퀘스트 보상 룰 재로드 |
| `.herorune test <zone>` | 특정 지역 룬문자 지급 테스트 |
| `.herorune list <zone>` | 지역별 룬문자 룰 확인 |

### 5단계: 웹 관리 연동

웹에서 지역별 룬문자 드랍 확률을 수정할 수 있도록 관리 화면을 추가한다.

필요 기능:

| 기능 | 설명 |
|---|---|
| 지역별 룬문자 목록 | zone_id 기준 룬문자 표시 |
| 확률 수정 | chance 값 수정 |
| 활성/비활성 | enabled 값 수정 |
| 지급 로그 조회 | 어떤 계정이 어떤 퀘스트에서 획득했는지 확인 |

## 결론

가장 안전한 방식은 기존 퀘스트 보상 데이터를 수정하지 않고, 별도 룰 테이블과 Lua 퀘스트 완료 이벤트로 확률 보상을 추가하는 것이다.

이 방식은 기존 퀘스트 보상을 손상시키지 않고, 룬문자 지급 확률과 대상 지역을 운영 중에도 조정할 수 있다. 기능이 안정화되면 Lua 로직을 C++ 모듈로 이전해 성능과 관리성을 높일 수 있다.
