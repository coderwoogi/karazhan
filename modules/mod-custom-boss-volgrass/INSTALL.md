# Volgrass Boss Module - 설치 가이드

## 📋 목차

1. [요구사항](#요구사항)
2. [설치 단계](#설치-단계)
3. [데이터베이스 설정](#데이터베이스-설정)
4. [설정 파일](#설정-파일)
5. [보스 스폰](#보스-스폰)
6. [테스트](#테스트)
7. [문제 해결](#문제-해결)

## 요구사항

- AzerothCore (WotLK 3.3.5a)
- C++17 이상
- MySQL/MariaDB
- CMake 3.16+

## 설치 단계

### 1. 모듈 파일 복사

모듈을 AzerothCore의 `modules` 디렉토리에 배치합니다:

```bash
# Linux/macOS
cp -r mod-custom-boss-volgrass /path/to/azerothcore/modules/

# Windows (PowerShell)
Copy-Item -Recurse mod-custom-boss-volgrass E:\server\azerothcore-wotlk\modules\
```

### 2. CMake 재생성

새 모듈을 인식하도록 CMake를 재실행합니다:

```bash
cd /path/to/azerothcore/build

# Linux/macOS
cmake ../ -DCMAKE_INSTALL_PREFIX=/path/to/server

# Windows
cmake ../ -DCMAKE_INSTALL_PREFIX=E:\server\azerothcore-wotlk\env\dist
```

### 3. 컴파일 및 설치

```bash
# Linux/macOS (멀티코어 빌드)
cmake --build . --config RelWithDebInfo --target install -j $(nproc)

# Windows
cmake --build . --config RelWithDebInfo --target install -j 4
```

### 4. 설정 파일 복사

빌드 후 자동으로 `etc/modules/` 디렉토리에 복사되지만, 수동으로 복사할 수도 있습니다:

```bash
# Linux/macOS
cp modules/mod-custom-boss-volgrass/conf/mod_custom_boss_volgrass.conf.dist \
   /path/to/server/etc/mod_custom_boss_volgrass.conf

# Windows
copy modules\mod-custom-boss-volgrass\conf\mod_custom_boss_volgrass.conf.dist ^
     E:\server\azerothcore-wotlk\env\dist\etc\mod_custom_boss_volgrass.conf
```

## 데이터베이스 설정

### SQL 파일 실행

`world` 데이터베이스에 SQL 파일을 실행합니다:

```bash
# MySQL 클라이언트 사용
mysql -u root -p world < modules/mod-custom-boss-volgrass/sql/world/base/boss_volgrass.sql

# 또는 HeidiSQL, MySQL Workbench 등에서 직접 실행
```

### 포함된 내용

SQL 파일은 다음을 생성합니다:

1. **creature_template** (Entry: 900000)
   - 보스 기본 스탯
   - ScriptName: `boss_volgrass`

2. **creature_text** (68개 멘트)
   - GroupID 0~10
   - 각 이벤트별 3~6개의 랜덤 멘트

3. **creature** (예시 스폰)
   - 기본 좌표: 엘윈 숲 (테스트용)
   - **실제 사용 시 좌표 수정 필요**

## 설정 파일

`mod_custom_boss_volgrass.conf` 파일 편집:

```conf
###################################################################################################
# BOSS SETTINGS
###################################################################################################

# 하수인 소환 Entry ID (0 = 비활성화)
Volgrass.Add.EntryID = 0

# Phase 2 소환 개수
Volgrass.Phase2.AddCount = 2

# Phase 3 소환 개수 (폭주 시 더 많이)
Volgrass.Phase3.AddCount = 4

# 소환 주기 (밀리초)
Volgrass.Summon.Interval = 30000
```

### 하수인 설정 방법

1. **하수인 몬스터 선택**
   - 적절한 난이도의 몬스터 Entry ID 찾기
   - 예: `creature_template`에서 레벨 80 전후 몬스터

2. **설정 파일 수정**
   ```conf
   Volgrass.Add.EntryID = 12345  # 실제 Entry ID로 변경
   ```

3. **서버 재시작**

## 보스 스폰

### 기본 스폰 수정

SQL 파일의 기본 스폰 좌표를 원하는 위치로 수정:

```sql
DELETE FROM `creature` WHERE `id1` = 900000;
INSERT INTO `creature` 
(`guid`, `id1`, `map`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`)
VALUES
(9900000, 900000, 0, -8913.23, 510.887, 96.6128, 0.0, 300);
--              ↑ 맵   ↑ X좌표  ↑ Y좌표  ↑ Z좌표  ↑ 방향  ↑ 리스폰(초)
```

### 권장 스폰 위치

- **넓은 공간**: 끌어당김→폭발 기믹을 위해 반경 30m 이상
- **장애물 최소화**: 도약 스킬이 원활히 작동하도록
- **접근성**: 플레이어가 쉽게 도달 가능한 위치

### GM 명령어로 스폰

```
.npc add 900000
.npc set phase 0
```

## 테스트

### 1. 서버 시작

```bash
./worldserver
```

### 2. 로그 확인

서버 시작 시 다음 로그가 표시되어야 합니다:

```
Volgrass: Module loaded successfully
Volgrass: Loaded config - AddEntry: 0, P2Count: 2, P3Count: 4, Interval: 30000ms
Volgrass: Boss script registered
```

### 3. 인게임 테스트

1. **보스 확인**
   ```
   .npc info
   # Entry: 900000, Name: Volgrass the Invader
   ```

2. **전투 시작**
   - 보스 공격
   - 서버 공지 확인: "[침공 경보] 섬의 중심에서..."
   - 보스 멘트 확인 (랜덤으로 출력됨)

3. **Phase 전환 확인**
   - 70% HP: Phase 2 서버 공지 + 보스 멘트
   - 40% HP: Phase 3 서버 공지 + 보스 멘트

4. **기믹 확인**
   - 죽음의 샘 → 대폭발 콤보
   - 멘트가 매번 다르게 출력되는지 확인

### 4. 디버그 로그

문제 발생 시 로그 레벨 조정:

```bash
# worldserver.conf 또는 authserver.conf
Logger.scripts.creature=3,Console Server
```

## 문제 해결

### 보스가 스폰되지 않음

1. **SQL 실행 확인**
   ```sql
   SELECT * FROM creature_template WHERE entry = 900000;
   SELECT * FROM creature WHERE id1 = 900000;
   ```

2. **GUID 중복 확인**
   ```sql
   SELECT * FROM creature WHERE guid = 9900000;
   # 다른 몬스터가 같은 GUID 사용 시 변경
   ```

### 보스 멘트가 출력되지 않음

1. **creature_text 확인**
   ```sql
   SELECT * FROM creature_text WHERE CreatureID = 900000;
   # 68개 행이 있어야 함
   ```

2. **로그 확인**
   ```
   # 멘트 시스템 오류 메시지 확인
   ```

### 컴파일 에러

1. **CMake 재생성**
   ```bash
   cd build
   rm -rf CMakeCache.txt CMakeFiles/
   cmake ../ -DCMAKE_INSTALL_PREFIX=/path/to/server
   ```

2. **헤더 파일 확인**
   - `boss_volgrass.h` 존재 확인
   - `loader.cpp` 존재 확인

### 서버 크래시

1. **로그 파일 확인**
   ```bash
   tail -f Server.log
   ```

2. **스펠 ID 확인**
   - 64746, 64779, 47756, 64443이 spell.dbc에 존재하는지 확인

3. **Cell::VisitObjects 오류**
   - AzerothCore 버전 확인
   - 최신 버전으로 업데이트

## 업그레이드

### 모듈 업데이트 시

1. **백업**
   ```bash
   cp -r modules/mod-custom-boss-volgrass modules/mod-custom-boss-volgrass.backup
   ```

2. **새 버전 복사**
   ```bash
   cp -r new_mod-custom-boss-volgrass modules/
   ```

3. **CMake 재생성 및 빌드**
   ```bash
   cd build
   cmake ../
   cmake --build . --config RelWithDebInfo --target install
   ```

4. **SQL 업데이트 확인**
   - 새 SQL 파일이 있다면 실행

## 지원

- **GitHub Issues**: [프로젝트 저장소]
- **AzerothCore Discord**: [#scripting 채널]
- **문서**: README.md, 주석 참조

---

설치 완료 후 `README.md`를 참조하여 전투 기믹과 공략법을 확인하세요!
