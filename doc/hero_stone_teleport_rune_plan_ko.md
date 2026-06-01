# 영웅석 순간이동 룬문자 기능 기획

## 목표

영웅석에 `순간이동` 기능을 추가한다.

순간이동 기능 자체는 구독 여부와 상관없이 모든 사용자가 사용할 수 있다. 다만 이동 가능한 목적지는 다음 정책을 따른다.

| 사용자 상태 | 영웅석 순간이동 목적지 |
|---|---|
| 비구독자 | 직접 룬문자 아이템을 사용해 새긴 목적지만 이동 가능 |
| 구독자 | 모든 활성 목적지가 이미 새겨진 것으로 간주되어 전체 이동 가능 |

룬문자 아이템은 특정 위치를 영웅석에 새기는 해금 아이템이다.

예시:

| 룬문자 아이템 | 해금 목적지 | 영웅석 표시명 |
|---|---|---|
| 오그리마 룬문자 | 오그리마 | 오그리마 |
| 언더시티 룬문자 | 언더시티 | 언더시티 |
| 실버문 룬문자 | 실버문 | 실버문 |

## 변경된 핵심 정책

| 항목 | 정책 |
|---|---|
| 순간이동 메뉴 사용 | 구독 여부와 상관없이 사용 가능 |
| 비구독자 목적지 | DB에 해금 기록이 있는 룬문자만 표시 |
| 구독자 목적지 | 모든 활성 목적지를 표시 |
| 구독 중 룬문자 아이템 사용 | 사용 불가 |
| 구독 중 룬문자 사용 실패 메시지 | `구독 중 룬문자 아이템 사용이 불가하며, 구독이 해지되면 사용이 가능 합니다. 룬문자 아이템을 버리면 안됩니다.` |
| 구독 해지 후 | 보유한 룬문자 아이템 사용 가능 |
| 해금 단위 | 계정 단위 |
| 목적지 정의 | 이동술사 데이터를 참고해 별도 영웅석 순간이동 테이블에 등록 |

## 현재 확인된 이동술사 데이터 구조

대상 Lua:

`E:\server\operate\lua_scripts\이동술사.lua`

이동술사 NPC:

| entry | name | gossip_menu_id |
|---:|---|---:|
| 190000 | 이동술사 | 47000 |

이동술사는 다음 DB를 읽는다.

| 테이블 | 용도 |
|---|---|
| `creature_template` | 이동술사 NPC의 `gossip_menu_id` 확인 |
| `gossip_menu_option` | 메뉴와 하위 메뉴, 목적지 이름/아이콘 |
| `smart_scripts` | 실제 순간이동 좌표 |

이동술사 좌표는 `smart_scripts`에 아래 조건으로 들어있다.

```sql
SELECT entryorguid,
       event_param1 AS menu_id,
       event_param2 AS option_id,
       action_type,
       action_param1 AS map_id,
       target_x,
       target_y,
       target_z,
       target_o
FROM smart_scripts
WHERE source_type = 0
  AND entryorguid = 190000
  AND event_type = 62
  AND action_type = 62;
```

예시로 확인된 대도시 메뉴:

| menu_id | option_id | map_id | x | y | z | o |
|---:|---:|---:|---:|---:|---:|---:|
| 47001 | 1 | 0 | -8814.46 | 626.123 | 94.1149 | 3.89435 |
| 47001 | 2 | 0 | -4804.74 | -1100.72 | 498.807 | 5.40619 |
| 47001 | 5 | 1 | 1632.66 | -4413.2 | 17.0302 | 3.12039 |
| 47001 | 6 | 0 | 1585.78 | 240.439 | -52.1503 | 0.00785 |
| 47001 | 8 | 530 | 9508.23 | -7345.6 | 14.3602 | 1.55116 |
| 47001 | 10 | 571 | 5804.23 | 639.307 | 647.773 | 0.901726 |

## 별도 테이블을 만드는 이유

이동술사 데이터는 `gossip_menu_option`과 `smart_scripts`에 흩어져 있다. 영웅석 룬문자 시스템은 아이템 해금, 구독 전체 해금, 정렬, 진영 제한, 활성/비활성 관리가 필요하므로 별도 테이블이 적합하다.

| 이동술사 원본 사용 | 별도 테이블 사용 |
|---|---|
| 원본 변경 시 영웅석도 같이 흔들림 | 영웅석 목적지를 독립 관리 가능 |
| 룬문자 item_entry 연결이 어려움 | 목적지마다 item_entry 직접 연결 가능 |
| 표시명/아이콘 정제가 필요 | 영웅석 전용 표시명/아이콘 관리 가능 |
| 구독 전체 해금 조건 추가가 복잡 | 단순 조회로 처리 가능 |

## DB 설계

### 1. 영웅석 순간이동 위치 테이블

권장 DB:

`acore_world`

테이블명:

`hero_stone_teleport_locations`

```sql
CREATE TABLE IF NOT EXISTS `hero_stone_teleport_locations` (
  `location_id` INT UNSIGNED NOT NULL,
  `rune_item_entry` INT UNSIGNED NOT NULL DEFAULT 0,
  `name_ko` VARCHAR(80) NOT NULL,
  `description_ko` VARCHAR(255) NOT NULL DEFAULT '',
  `icon` VARCHAR(160) NOT NULL DEFAULT 'Interface\\ICONS\\Spell_Arcane_TeleportDalaran',
  `category` VARCHAR(40) NOT NULL DEFAULT '기타',
  `source_menu_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `source_option_id` INT UNSIGNED NOT NULL DEFAULT 0,
  `map_id` INT UNSIGNED NOT NULL,
  `position_x` FLOAT NOT NULL,
  `position_y` FLOAT NOT NULL,
  `position_z` FLOAT NOT NULL,
  `orientation` FLOAT NOT NULL DEFAULT 0,
  `faction_mask` TINYINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '0=공용, 1=얼라이언스, 2=호드',
  `sort_order` INT NOT NULL DEFAULT 0,
  `is_active` TINYINT UNSIGNED NOT NULL DEFAULT 1,
  PRIMARY KEY (`location_id`),
  UNIQUE KEY `uk_hero_stone_teleport_locations_item` (`rune_item_entry`),
  KEY `idx_hero_stone_teleport_locations_source` (`source_menu_id`, `source_option_id`),
  KEY `idx_hero_stone_teleport_locations_sort` (`category`, `sort_order`, `location_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

컬럼 설명:

| 컬럼 | 설명 |
|---|---|
| `location_id` | 영웅석 내부 목적지 ID |
| `rune_item_entry` | 해당 위치를 해금하는 룬문자 아이템 ID |
| `name_ko` | 영웅석에 표시될 이름 |
| `description_ko` | UI 설명 |
| `icon` | 영웅석 UI 아이콘 |
| `category` | 대도시, 마을, PVP, 인던 등 |
| `source_menu_id` | 이동술사 원본 `gossip_menu_option.MenuID` |
| `source_option_id` | 이동술사 원본 `OptionID` |
| `map_id`, `position_x/y/z`, `orientation` | 실제 이동 좌표 |
| `faction_mask` | 진영 제한 |
| `is_active` | 비활성 목적지는 구독자에게도 표시하지 않음 |

### 2. 계정별 룬문자 해금 테이블

권장 DB:

`acore_characters`

테이블명:

`character_hero_stone_teleport_runes`

```sql
CREATE TABLE IF NOT EXISTS `character_hero_stone_teleport_runes` (
  `account_id` INT UNSIGNED NOT NULL,
  `character_guid` INT UNSIGNED NOT NULL DEFAULT 0,
  `location_id` INT UNSIGNED NOT NULL,
  `rune_item_entry` INT UNSIGNED NOT NULL,
  `unlocked_at` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`account_id`, `location_id`),
  KEY `idx_character_hero_stone_teleport_runes_character` (`character_guid`),
  KEY `idx_character_hero_stone_teleport_runes_item` (`rune_item_entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

정책:

구독자는 이 테이블에 모든 룬문자를 INSERT하지 않는다. 구독 상태일 때 조회 로직에서 모든 활성 목적지를 표시한다. 그래야 구독 해지 시 본인이 실제로 룬문자 아이템으로 해금한 목적지만 남는다.

## 이동술사 데이터 이관 방식

### 원본 목적지 추출 쿼리

`gossip_menu_option`의 목적지 이름과 `smart_scripts`의 좌표를 조합한다.

```sql
SELECT
  g.MenuID AS source_menu_id,
  g.OptionID AS source_option_id,
  g.OptionText AS raw_option_text,
  s.action_param1 AS map_id,
  s.target_x,
  s.target_y,
  s.target_z,
  s.target_o
FROM gossip_menu_option g
JOIN smart_scripts s
  ON s.entryorguid = 190000
 AND s.source_type = 0
 AND s.event_type = 62
 AND s.event_param1 = g.MenuID
 AND s.event_param2 = g.OptionID
 AND s.action_type = 62
WHERE g.MenuID IN (
  47001, 47002, 47003, 47004, 47005,
  47006, 47007, 47008, 47009, 47010,
  47011, 47012, 47013, 47014,
  47024, 47025
)
ORDER BY g.MenuID, g.OptionID;
```

### 이관 시 정리할 내용

| 원본 | 영웅석 테이블 |
|---|---|
| `OptionText` 안의 `|T...|t` | 아이콘 경로로 분리 |
| `OptionText` 안의 `|c...|r` | 색상 코드 제거 |
| 하위 메뉴 ID | `category`로 변환 |
| `smart_scripts.action_param1` | `map_id` |
| `target_x/y/z/o` | 좌표 |

### 카테고리 매핑 예시

| MenuID | 카테고리 |
|---:|---|
| 47001 | 대도시 |
| 47002 | 마을 |
| 47003 | PVP |
| 47004 | PVP |
| 47005 | 인던/공격대 |
| 47006 이상 | 세부 지역 |

정확한 카테고리는 `gossip_menu_option` 전체 구조를 보고 확정한다.

## 룬문자 아이템 설계

각 목적지마다 룬문자 아이템을 1개씩 가진다.

예시:

| rune_item_entry | 이름 | 연결 위치 |
|---:|---|---|
| 950010 | 룬문자 : 스톰윈드 | 스톰윈드 |
| 950011 | 룬문자 : 아이언포지 | 아이언포지 |
| 950014 | 룬문자 : 오그리마 | 오그리마 |

아이템 설명 예시:

`영웅석에 오그리마 순간이동 룬문자를 새깁니다.`

구독자 사용 차단 메시지:

`구독 중 룬문자 아이템 사용이 불가하며, 구독이 해지되면 사용이 가능 합니다. 룬문자 아이템을 버리면 안됩니다.`

중요:

구독 중 룬문자 아이템 사용이 차단되어야 하므로, 구독자에게는 아이템이 소비되면 안 된다. 아이템을 소비형으로 만들더라도 Lua 사용 이벤트에서 구독 상태를 먼저 확인하고 실패 처리해야 한다.

## 영웅석 UI 설계

### 메인 메뉴

`순간이동` 버튼은 구독 여부와 상관없이 항상 표시한다.

| 메뉴 | 구독자 | 비구독자 |
|---|---|---|
| 달라란 이동 | 표시 | 표시 |
| 순간이동 | 표시 | 표시 |
| 개인 은행 | 표시 | 미표시 |
| 잡화상인 | 표시 | 미표시 |
| 우편함 | 표시 | 미표시 |
| 무료 버프 | 미표시 | 표시 |

### 순간이동 화면

| 사용자 상태 | 화면 내용 |
|---|---|
| 구독자 | 모든 활성 목적지 표시 |
| 비구독자 + 해금 있음 | 해금된 목적지만 표시 |
| 비구독자 + 해금 없음 | `영웅석에 새겨진 룬문자가 없습니다.` 표시 |

### 액션 ID 설계

기존 영웅석은 숫자 action id 구조다. 이를 유지한다.

| action id | 의미 |
|---:|---|
| 120 | 순간이동 화면 열기 |
| 121 | 메인 화면으로 돌아가기 |
| 200000 + location_id | 해당 위치로 순간이동 |

## Lua 처리 설계

### 1. 메뉴 구성

`BuildMenuItems()`에서 `순간이동` 버튼을 항상 추가한다.

```lua
table.insert(items, {
    id = 120,
    label = "순간이동",
    desc = "영웅석에 새겨진 룬문자로 이동합니다.",
    icon = "Interface\\ICONS\\Spell_Arcane_TeleportDalaran",
})
```

### 2. 룬문자 아이템 사용

처리 순서:

1. 사용자가 구독 중인지 확인
2. 구독 중이면 메시지 출력 후 중단
3. item_entry로 `hero_stone_teleport_locations` 조회
4. 비활성 또는 존재하지 않는 룬문자면 중단
5. 이미 해금된 위치인지 확인
6. 미해금이면 `character_hero_stone_teleport_runes` INSERT
7. 성공 메시지 출력

구독 중 차단 메시지:

```text
구독 중 룬문자 아이템 사용이 불가하며, 구독이 해지되면 사용이 가능 합니다. 룬문자 아이템을 버리면 안됩니다.
```

이미 해금된 경우:

```text
이미 영웅석에 새겨진 룬문자입니다.
```

성공:

```text
영웅석에 오그리마 룬문자가 새겨졌습니다.
```

### 3. 순간이동 목록 조회

구독자:

```sql
SELECT location_id, name_ko, description_ko, icon,
       map_id, position_x, position_y, position_z, orientation, faction_mask
FROM acore_world.hero_stone_teleport_locations
WHERE is_active = 1
ORDER BY category, sort_order, location_id;
```

비구독자:

```sql
SELECT l.location_id, l.name_ko, l.description_ko, l.icon,
       l.map_id, l.position_x, l.position_y, l.position_z, l.orientation, l.faction_mask
FROM acore_characters.character_hero_stone_teleport_runes u
JOIN acore_world.hero_stone_teleport_locations l
  ON l.location_id = u.location_id
WHERE u.account_id = ?
  AND l.is_active = 1
ORDER BY l.category, l.sort_order, l.location_id;
```

### 4. 순간이동 실행 검증

구독자는 `hero_stone_teleport_locations.is_active=1`이면 이동 가능하다.

비구독자는 `character_hero_stone_teleport_runes`에 해금 기록이 있어야 이동 가능하다.

실행 전 검증:

| 조건 | 처리 |
|---|---|
| 전투 중 | 차단 |
| 사망 상태 | 차단 권장 |
| 전장/투기장 | 차단 권장 |
| 인스턴스 내부 | 차단 권장 |
| 진영 제한 | `faction_mask`로 확인 |
| 목적지 비활성 | 차단 |

## 구현 단계

### 1단계. 이동술사 목적지 추출

작업:

1. `gossip_menu_option` + `smart_scripts`에서 이동술사 목적지 전체 추출
2. `OptionText`에서 아이콘/표시명 정리
3. 카테고리 매핑
4. 룬문자 item_entry 부여

산출물:

`hero_stone_teleport_locations` INSERT SQL

### 2단계. DB 테이블 생성

작업:

1. `acore_world.hero_stone_teleport_locations` 생성
2. `acore_characters.character_hero_stone_teleport_runes` 생성
3. 위치 데이터 INSERT

### 3단계. 룬문자 아이템 생성

작업:

1. `item_template`에 룬문자 아이템 생성
2. 아이템 이름/설명 한글 등록
3. 사용 가능한 소비형 아이템으로 설정
4. 구독 중 사용 차단 시 아이템이 사라지지 않는지 테스트

### 4단계. `현자석.lua` 수정

작업:

1. `순간이동` 메뉴 항상 추가
2. 순간이동 목록 화면 추가
3. 구독자 전체 목적지 조회
4. 비구독자 해금 목적지 조회
5. 목적지 클릭 시 이동 처리
6. 뒤로가기 처리

### 5단계. 룬문자 아이템 Lua 등록

작업:

1. 활성 위치의 `rune_item_entry` 목록 로드
2. 각 item_entry에 `RegisterItemEvent` 등록
3. 사용 시 구독 여부 확인
4. 해금 기록 INSERT
5. 중복/실패 메시지 처리

### 6단계. 테스트

| 테스트 | 기대 결과 |
|---|---|
| 비구독자 영웅석 순간이동 클릭, 해금 없음 | 해금된 룬문자가 없다는 안내 |
| 비구독자 룬문자 사용 | 해금 기록 생성 |
| 비구독자 해금 후 영웅석 순간이동 클릭 | 해당 목적지 표시 |
| 비구독자 목적지 클릭 | 이동 성공 |
| 구독자 영웅석 순간이동 클릭 | 모든 활성 목적지 표시 |
| 구독자 룬문자 아이템 사용 | 사용 차단 메시지, 아이템 보존 |
| 구독 해지 후 룬문자 사용 | 정상 해금 |
| 구독 해지 후 영웅석 순간이동 | 직접 해금한 목적지만 표시 |
| 전투 중 이동 | 차단 |
| 비활성 목적지 | 표시/이동 불가 |

## 결정 필요 사항

| 항목 | 선택지 | 추천 |
|---|---|---|
| 해금 단위 | 캐릭터 / 계정 | 계정 |
| 구독자 전체 해금 저장 방식 | DB INSERT / 조회 시 전체 표시 | 조회 시 전체 표시 |
| 목적지 범위 | 대도시만 / 이동술사 전체 | 이동술사 전체를 별도 테이블로 이관 후 선택 활성화 |
| 진영 제한 | 적용 / 미적용 | 대도시는 적용, 중립 지역은 공용 |
| 인스턴스 내부 사용 | 허용 / 제한 | 제한 |
| 룬문자 아이템 소비 | 성공 시 소비 / 실패 시 보존 | 성공 시 소비, 구독 중 실패 시 보존 |

## 요약

순간이동 기능은 모든 사용자가 접근할 수 있다. 비구독자는 룬문자 아이템으로 직접 새긴 목적지만 이동 가능하고, 구독자는 모든 활성 목적지가 이미 새겨진 것으로 처리한다.

구독자는 룬문자 아이템을 사용할 수 없어야 하며, 아래 메시지를 반드시 출력한다.

```text
구독 중 룬문자 아이템 사용이 불가하며, 구독이 해지되면 사용이 가능 합니다. 룬문자 아이템을 버리면 안됩니다.
```

이동술사의 기존 위치 데이터는 `gossip_menu_option`과 `smart_scripts`에서 추출한 뒤, 영웅석 전용 `hero_stone_teleport_locations` 테이블에 별도로 등록하는 방향이 가장 안전하다.
