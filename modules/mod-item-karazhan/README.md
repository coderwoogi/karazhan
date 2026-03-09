# Karazhan Item Enhancement System

아이템 강화 시스템 모듈입니다.

## 📋 개요

이 모듈은 WoW 아이템에 강화 시스템을 추가합니다. 플레이어는 NPC를 통해 장비를 최대 +10까지 강화할 수 있으며, 강화 레벨이 높아질수록 실패 확률이 증가하고 아이템이 파괴될 수도 있습니다.

## ✨ 주요 기능

- **장비 강화**: 대부분의 장비 슬롯 강화 가능 (무기, 방어구, 액세서리 등)
- **강화 레벨**: +1 ~ +10 단계별 강화
- **확률 시스템**: 레벨이 높아질수록 성공률 감소, 파괴 확률 증가
- **비용 시스템**: 골드 및 재료 소모
- **안전한 처리**: 대기 큐 시스템으로 아이템 손실 방지
- **통계 기록**: 성공/실패 횟수, 소모 골드 추적

## 🎯 강화 시스템

### 강화 확률표

| 강화 레벨 | 성공률 | 실패률 | 파괴률 | 골드 비용 |
|-----------|--------|--------|--------|-----------|
| +1 | 100% | 0% | 0% | 10G |
| +2 | 95% | 5% | 0% | 20G |
| +3 | 90% | 10% | 0% | 50G |
| +4 | 85% | 15% | 0% | 100G |
| +5 | 80% | 20% | 0% | 200G |
| +6 | 70% | 25% | 5% | 500G |
| +7 | 60% | 30% | 10% | 1,000G |
| +8 | 50% | 35% | 15% | 2,000G |
| +9 | 40% | 40% | 20% | 5,000G |
| +10 | 30% | 50% | 20% | 10,000G |

### 강화 결과

- **성공**: 강화 레벨 +1 증가
- **실패**: 강화 레벨 유지 (비용은 소모됨)
- **파괴**: 아이템 영구 소실 (비용도 소모됨)

## 🛠️ 설치 방법

### 1. 데이터베이스 설정

#### World Database

```sql
-- World DB에 실행
mysql -u root -p world < modules/mod-item-karazhan/data/sql/db-world/base/item_karazhan_tables.sql
```

생성되는 테이블:
- `karazhan_enchant_slots`: 강화 가능한 슬롯 정의
- `karazhan_enchant_config`: 강화 레벨별 설정

#### Character Database

```sql
-- Character DB에 실행 (필수!)
mysql -u root -p characters < modules/mod-item-karazhan/data/sql/db-characters/base/item_karazhan_character.sql
```

생성되는 테이블:
- `karazhan_item_enhance`: 아이템 강화 정보 저장

### 2. 모듈 컴파일

```bash
cd build
cmake ../
cmake --build . --config RelWithDebInfo --target install
```

### 3. NPC 스폰

강화 NPC를 스폰해야 합니다:

```sql
-- 예시: 스톰윈드에 NPC 스폰
INSERT INTO `creature` (`guid`, `id1`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`)
VALUES
(9900001, 190010, 0, -8842.09, 626.358, 94.0867, 3.61284, 300);
```

또는 GM 명령어:
```
.npc add 190010
```

## 💡 사용 방법

### 플레이어 관점

1. **강화 NPC 찾기**
   - NPC 이름: "카라잔 강화사" (또는 설정된 이름)

2. **아이템 강화**
   - NPC와 대화
   - 강화할 아이템 선택
   - 비용 및 확률 확인
   - 강화 시도

3. **결과 확인**
   - 성공: 아이템 강화 레벨 증가 (이름에 +1, +2 등 표시)
   - 실패: 강화 레벨 유지, 비용 소모
   - 파괴: 아이템 영구 소실

### 관리자 설정

#### 강화 가능 슬롯 변경

```sql
-- karazhan_enchant_slots 테이블 수정
UPDATE karazhan_enchant_slots 
SET can_enhance = 0, enabled = 0 
WHERE slot_id = 11;  -- 반지 강화 비활성화
```

#### 강화 확률 조정

```sql
-- karazhan_enchant_config 테이블 수정
UPDATE karazhan_enchant_config 
SET success_rate = 50.0, fail_rate = 40.0, gold_cost = 5000 
WHERE enchant_level = 9;  -- +9 강화 확률 및 비용 변경
```

#### 재료 추가

```sql
UPDATE karazhan_enchant_config 
SET material_1 = 34057,  -- 아제로스의 별 (예시)
    material_1_count = 5
WHERE enchant_level >= 6;  -- +6 이상은 특수 재료 필요
```

## 🔍 기술 사양

### 아키텍처

1. **ItemKarazhanMgr**: 핵심 매니저 클래스 (싱글톤)
2. **대기 큐 시스템**: 안전한 아이템 교체 보장
3. **DB 연동**: World DB (설정) + Character DB (아이템 정보)

### 주요 컴포넌트

#### ItemKarazhanMgr
- `Initialize()`: 설정 로드
- `Update()`: 대기 큐 처리
- `RequestEnhancement()`: 강화 요청
- `ProcessPendingEnhancement()`: 실제 강화 처리

#### NPC Script
- `npc_item_karazhan.cpp`: 강화 NPC AI
- Gossip 메뉴를 통한 인터페이스 제공

### 데이터 흐름

```
플레이어 → NPC 대화
  ↓
아이템 선택
  ↓
비용/재료 확인
  ↓
대기 큐 추가 (RequestEnhancement)
  ↓
WorldScript::Update() → ItemKarazhanMgr::Update()
  ↓
ProcessPendingEnhancement()
  ↓
결과 적용 (성공/실패/파괴)
  ↓
플레이어에게 결과 알림
```

## 🐛 문제 해결

### 테이블이 로드되지 않음

**증상**: "karazhan_enchant_slots table is empty!" 로그

**해결**:
```sql
-- World DB에 SQL 파일 실행 확인
SELECT * FROM karazhan_enchant_slots;
SELECT * FROM karazhan_enchant_config;
```

### 강화 정보가 저장되지 않음

**증상**: 강화 후 재접속 시 강화 레벨 초기화

**해결**:
```sql
-- Character DB에 테이블 생성 확인
USE characters;
SELECT * FROM karazhan_item_enhance;
```

### 아이템이 사라짐

**증상**: 강화 시도 후 아이템 소실

**원인**: 대기 큐 시스템이 제대로 작동하지 않음

**해결**:
1. 서버 로그 확인
2. `ProcessPendingEnhancement` 로직 확인
3. 인벤토리 공간 확보 후 재시도

## 📊 통계 조회

### 플레이어별 강화 통계

```sql
SELECT 
    owner_guid,
    COUNT(*) AS total_items,
    SUM(enhance_level) AS total_enhance_levels,
    SUM(total_success) AS total_success,
    SUM(total_fail) AS total_fail,
    SUM(total_gold_spent) / 10000 AS total_gold_spent
FROM karazhan_item_enhance
GROUP BY owner_guid
ORDER BY total_gold_spent DESC
LIMIT 10;
```

### 강화 레벨 분포

```sql
SELECT 
    enhance_level,
    COUNT(*) AS item_count
FROM karazhan_item_enhance
GROUP BY enhance_level
ORDER BY enhance_level;
```

## 📝 라이선스

Copyright (C) 2016+ AzerothCore <www.azerothcore.org>

---

## 📚 추가 문서

- **ItemKarazhan.cpp**: 핵심 로직 구현
- **npc_item_karazhan.cpp**: NPC 스크립트
- **item_karazhan_tables.sql**: 데이터베이스 스키마

## 🔄 업데이트 로그

- **v1.0**: 초기 릴리스
  - 기본 강화 시스템
  - +1 ~ +10 레벨
  - 대기 큐 시스템
