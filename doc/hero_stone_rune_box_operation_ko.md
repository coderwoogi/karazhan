# 영웅석 룬문자 지역 상자 운영 절차

## 현재 상태

룬문자 지역 상자 시스템은 안전을 위해 후보 좌표를 자동 활성화하지 않습니다.

| 항목 | 상태 |
|---|---:|
| 지역 그룹 | 66 |
| 룬문자 매핑 | 231 |
| 후보 좌표 | 315 |
| 활성 후보 좌표 | 0 |
| 실제 스폰된 상자 | 0 |

후보 좌표는 `hero_stone_teleport_locations` 주변을 기준으로 자동 생성됐지만, 벽 안/공중/접근 불가 위치 가능성이 있으므로 전부 `enabled=0`, `verified=0` 상태입니다.

## 관련 파일

| 파일 | 역할 |
|---|---|
| `data/sql/custom/2026_05_30_00_hero_stone_rune_box_system.sql` | 테이블, 지역, 룬문자 매핑, 후보 좌표 생성 |
| `data/sql/custom/2026_05_30_01_hero_stone_rune_box_spawn_from_enabled_candidates.sql` | 검수 완료 후보 좌표를 실제 gameobject/pool로 반영 |
| `E:\server\operate\lua_scripts\영웅석_룬문자지역상자.lua` | 게임오브젝트 가십 메뉴와 룬문자 지급 처리 |

## 후보 좌표 확인

특정 지역의 후보 좌표를 확인합니다.

```sql
SELECT
    candidate.spawn_id,
    region.region_name,
    candidate.source_location_id,
    location.name_ko AS source_location,
    candidate.map_id,
    candidate.position_x,
    candidate.position_y,
    candidate.position_z,
    candidate.orientation,
    candidate.enabled,
    candidate.verified,
    candidate.note
FROM hero_stone_rune_box_spawn_candidate AS candidate
INNER JOIN hero_stone_rune_box_region AS region
    ON region.region_id = candidate.region_id
LEFT JOIN hero_stone_teleport_locations AS location
    ON location.location_id = candidate.source_location_id
WHERE region.region_name = '여명의 설원'
ORDER BY candidate.spawn_id;
```

## 후보 좌표 검수

인게임에서 좌표가 접근 가능한지 확인합니다.

검수 기준:

| 기준 | 설명 |
|---|---|
| 접근 가능 | 일반 유저가 이동해서 도달 가능해야 함 |
| 발견 가능 | 너무 멀거나 지형 안쪽이면 안 됨 |
| 너무 쉬운 위치 제외 | NPC 바로 옆, 포탈 바로 앞, 마을 중앙은 피함 |
| 전투 위험도 | 저레벨 지역에서 과도하게 위험한 몬스터 밀집지는 피함 |
| 인스턴스 내부 제외 | 던전/레이드 룬문자는 입구 필드 지역에 배치 |

## 후보 좌표 활성화

검수 완료된 좌표만 활성화합니다.

예시:

```sql
UPDATE hero_stone_rune_box_spawn_candidate
SET enabled = 1,
    verified = 1,
    note = CONCAT(note, ' / 검수 완료')
WHERE spawn_id IN (1, 2, 3);
```

지역별 권장 활성 후보 수는 `hero_stone_rune_box_region.active_count`보다 많아야 합니다.

예시:

| 지역 룬문자 수 | 권장 검수 완료 후보 |
|---:|---:|
| 1 ~ 2개 | 최소 3개 |
| 3 ~ 5개 | 최소 5개 |
| 6 ~ 9개 | 최소 8개 |
| 10개 이상 | 최소 12개 |

## 실제 스폰 반영

검수 완료 후보를 실제 월드 오브젝트와 pool에 반영합니다.

```bash
mysql --default-character-set=utf8mb4 -uacore -pacore -e "SOURCE E:/server/azerothcore-wotlk/data/sql/custom/2026_05_30_01_hero_stone_rune_box_spawn_from_enabled_candidates.sql"
```

적용 후 월드서버 재시작이 가장 안전합니다.

## 적용 확인

```sql
SELECT COUNT(*) AS spawned_boxes
FROM gameobject
WHERE id BETWEEN 960100 AND 960165
  AND Comment LIKE 'HeroStoneRuneBox spawn_id=%';

SELECT COUNT(*) AS pools
FROM pool_template
WHERE entry BETWEEN 970100 AND 970165;

SELECT COUNT(*) AS pool_gameobjects
FROM pool_gameobject
WHERE pool_entry BETWEEN 970100 AND 970165;
```

## 동작 방식

1. 유저가 지역 상자를 클릭합니다.
2. 루팅창이 아니라 가십 메뉴가 열립니다.
3. `룬문자를 확인한다`를 누르면 구독 여부를 확인합니다.
4. 구독자는 지급되지 않고 안내 메시지만 출력됩니다.
5. 해당 계정에 이미 해금된 룬문자는 지급 후보에서 제외됩니다.
6. 가방에 같은 룬문자를 이미 가지고 있어도 중복 보유는 허용됩니다.
7. 확률 판정에 성공하면 해당 지역에 묶인 룬문자 중 하나가 지급됩니다.
8. 성공 또는 확률 실패 시 상자는 despawn되고 pool에 의해 다시 배치됩니다.

## 로그 확인

```sql
SELECT *
FROM acore_characters.character_hero_stone_rune_box_log
ORDER BY id DESC
LIMIT 50;
```

`result` 값:

| result | 의미 |
|---|---|
| `reward` | 룬문자 지급 성공 |
| `chance_fail` | 확률 실패 |
| `subscriber` | 구독자라 지급 제외 |
| `already_unlocked` | 해당 지역 룬문자를 모두 해금함 |
| `no_space` | 가방 공간 부족 |
| `no_candidate` | 지급 후보 없음 |

## 주의사항

자동 후보 좌표를 바로 전체 활성화하면 안 됩니다. 반드시 지역별로 인게임 검수 후 `enabled=1`, `verified=1`로 바꿔야 합니다.

`2026_05_30_01_hero_stone_rune_box_spawn_from_enabled_candidates.sql`은 기존 커스텀 룬문자 상자 스폰과 pool을 재생성합니다. 운영 중 적용할 때는 서버 재시작 전후로 상자 중복 여부를 확인해야 합니다.
