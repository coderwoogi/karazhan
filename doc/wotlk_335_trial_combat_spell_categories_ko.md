# 3.3.5 시련 전투용 직업 스펠 카테고리

이 문서는 Wowhead WotLK 한국어 직업 능력 페이지를 기준으로 시련 전투 AI/전투 로직에서 쓰기 쉽도록 전투 목적별로 재분류한 목록입니다.

## 기준

| 항목 | 내용 |
|---|---|
| 기준 사이트 | `https://www.wowhead.com/wotlk/ko/spells/abilities/{class}` |
| 직업 URL | `warrior`, `paladin`, `hunter`, `rogue`, `priest`, `death-knight`, `shaman`, `mage`, `warlock`, `druid` |
| 포함 범위 | Wowhead WotLK `spells/abilities/{직업}` 페이지에 노출되는 직업 능력 |
| 제외 범위 | 해당 Wowhead 직업 능력 페이지에 없는 특성 전용 스펠은 포함하지 않습니다. 예: 사제 `침묵(15487)` |
| 랭크 처리 | 같은 직업/같은 스펠명은 최고 레벨/최고 랭크만 표시 |
| 목적 | 시련 그림자/AI가 전투 상황에 따라 어떤 스펠을 사용할지 분류하기 위한 기획 자료 |

## 카테고리 요약

| 코드 | 카테고리 | 스펠 수 | 사용 시점 |
|---|---|---:|---|
| `PRECOMBAT_BUFF` | 전투 전 버프/준비 | 38 | 전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다. |
| `STANCE_FORM_AURA` | 태세/형상/오라/폼 전환 | 25 | 직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다. |
| `ATTACK_SINGLE` | 전투 중 공격기 - 단일 대상 | 79 | 주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다. |
| `ATTACK_AOE` | 전투 중 공격기 - 광역/다중 대상 | 24 | 여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다. |
| `CC` | 전투 중 CC/메즈/이동 제한 | 22 | 기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다. |
| `INTERRUPT` | 차단/침묵/시전 방해 | 6 | 주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다. |
| `DISPEL_CLEANSE` | 해제/정화 | 10 | 아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다. |
| `DEFENSIVE` | 생존/방어/피해 감소 | 25 | 자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다. |
| `HEAL` | 치유/회복 | 25 | 체력 회복, 보호막성 회복, 생명력 회복 계열입니다. |
| `RESURRECTION` | 부활 | 4 | 죽은 아군을 살리는 기술입니다. 전투 중 사용 가능 여부는 스펠별로 다릅니다. |
| `RESOURCE` | 마나/자원 회복 | 7 | 마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다. |
| `MOBILITY` | 이동/돌진/도주 | 9 | 돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다. |
| `TAUNT_THREAT` | 어그로/도발/위협 제어 | 12 | 대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다. |
| `PET_SUMMON` | 소환/펫/하수인 | 33 | 전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다. |
| `UTILITY_COMBAT` | 전투 유틸/특수 상황 | 156 | 위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다. |
| `EXCLUDE_NONCOMBAT` | 전투 사용 제외 후보 | 48 | 순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다. |

## 시련 전투 적용 가이드

| 우선순위 | 상황 | 권장 카테고리 |
|---:|---|---|
| 1 | 전투 시작 전 또는 재시작 직후 | `전투 전 버프/준비`, `태세/형상/오라/폼 전환`, `소환/펫/하수인` |
| 2 | 적 캐스터가 주문 시전 중 | `차단/침묵/시전 방해` |
| 3 | 체력이 위험함 | `생존/방어/피해 감소`, `치유/회복` |
| 4 | 마나 또는 자원이 부족함 | `마나/자원 회복` |
| 5 | 적을 묶거나 시간을 벌어야 함 | `전투 중 CC/메즈/이동 제한` |
| 6 | 거리가 벌어짐 | `이동/돌진/도주` |
| 7 | 탱커형 그림자가 대상 고정을 해야 함 | `어그로/도발/위협 제어` |
| 8 | 일반 딜 사이클 | `전투 중 공격기 - 단일 대상`, `전투 중 공격기 - 광역/다중 대상` |
| 9 | 사망 후 복구 로직 | `부활` |

## 직업별 전투 스펠 분류

## 전사 (Warrior)

Wowhead 기준 URL: [warrior](https://www.wowhead.com/wotlk/ko/spells/abilities/warrior)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 3 |
| 태세/형상/오라/폼 전환 | 3 |
| 전투 중 공격기 - 단일 대상 | 4 |
| 전투 중 공격기 - 광역/다중 대상 | 3 |
| 전투 중 CC/메즈/이동 제한 | 2 |
| 차단/침묵/시전 방해 | 2 |
| 생존/방어/피해 감소 | 3 |
| 치유/회복 | 1 |
| 마나/자원 회복 | 2 |
| 이동/돌진/도주 | 3 |
| 어그로/도발/위협 제어 | 3 |
| 전투 유틸/특수 상황 | 11 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [47436](https://www.wowhead.com/wotlk/ko/spell=47436) | 전투의 외침 9 레벨 | 78 | 전투 전 유지 확인 |
| Wowhead | [47437](https://www.wowhead.com/wotlk/ko/spell=47437) | 사기의 외침 8 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [47440](https://www.wowhead.com/wotlk/ko/spell=47440) | 지휘의 외침 3 레벨 | 80 | 전투 전 유지 확인 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2457](https://www.wowhead.com/wotlk/ko/spell=2457) | 전투 태세 | 1 | 필요 태세/형상일 때 사용 |
| Wowhead | [71](https://www.wowhead.com/wotlk/ko/spell=71) | 방어 태세 | 10 | 필요 태세/형상일 때 사용 |
| Wowhead | [2458](https://www.wowhead.com/wotlk/ko/spell=2458) | 광폭 태세 | 30 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [64382](https://www.wowhead.com/wotlk/ko/spell=64382) | 분쇄의 투척 | 71 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47465](https://www.wowhead.com/wotlk/ko/spell=47465) | 분쇄 10 레벨 | 76 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47450](https://www.wowhead.com/wotlk/ko/spell=47450) | 영웅의 일격 13 레벨 | 76 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47471](https://www.wowhead.com/wotlk/ko/spell=47471) | 마무리 일격 9 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1680](https://www.wowhead.com/wotlk/ko/spell=1680) | 소용돌이 | 36 | 다수 대상일 때 사용 |
| Wowhead | [47520](https://www.wowhead.com/wotlk/ko/spell=47520) | 회전베기 8 레벨 | 77 | 다수 대상일 때 사용 |
| Wowhead | [47502](https://www.wowhead.com/wotlk/ko/spell=47502) | 천둥벼락 9 레벨 | 78 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [676](https://www.wowhead.com/wotlk/ko/spell=676) | 무장 해제 | 18 | 대상 제어/시간 벌기 |
| Wowhead | [5246](https://www.wowhead.com/wotlk/ko/spell=5246) | 위협의 외침 | 22 | 대상 제어/시간 벌기 |

### 차단/침묵/시전 방해

주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [72](https://www.wowhead.com/wotlk/ko/spell=72) | 방패 가격 | 12 | 적 시전 중 우선 사용 |
| Wowhead | [6552](https://www.wowhead.com/wotlk/ko/spell=6552) | 자루 공격 | 38 | 적 시전 중 우선 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2565](https://www.wowhead.com/wotlk/ko/spell=2565) | 방패 막기 | 16 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [871](https://www.wowhead.com/wotlk/ko/spell=871) | 방패의 벽 | 28 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [23920](https://www.wowhead.com/wotlk/ko/spell=23920) | 주문 반사 | 64 | 체력 위험 또는 큰 피해 예측 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [55694](https://www.wowhead.com/wotlk/ko/spell=55694) | 분노의 재생력 | 75 | 아군/자신 체력 회복 |

### 마나/자원 회복

마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2687](https://www.wowhead.com/wotlk/ko/spell=2687) | 피의 분노 | 10 | 자원 부족 시 사용 |
| Wowhead | [18499](https://www.wowhead.com/wotlk/ko/spell=18499) | 광전사의 격노 | 32 | 자원 부족 시 사용 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [34428](https://www.wowhead.com/wotlk/ko/spell=34428) | 승리의 돌진 | 6 | 거리 조절/접근/이탈 |
| Wowhead | [20252](https://www.wowhead.com/wotlk/ko/spell=20252) | 봉쇄 | 30 | 거리 조절/접근/이탈 |
| Wowhead | [11578](https://www.wowhead.com/wotlk/ko/spell=11578) | 돌진 3 레벨 | 46 | 거리 조절/접근/이탈 |

### 어그로/도발/위협 제어

대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [355](https://www.wowhead.com/wotlk/ko/spell=355) | 도발 | 10 | 대상 고정/위협 제어 |
| Wowhead | [694](https://www.wowhead.com/wotlk/ko/spell=694) | 도발의 일격 | 16 | 대상 고정/위협 제어 |
| Wowhead | [1161](https://www.wowhead.com/wotlk/ko/spell=1161) | 도전의 외침 | 26 | 대상 고정/위협 제어 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1715](https://www.wowhead.com/wotlk/ko/spell=1715) | 무력화 | 8 | 조건부 전투 유틸 |
| Wowhead | [7386](https://www.wowhead.com/wotlk/ko/spell=7386) | 방어구 가르기 | 10 | 조건부 전투 유틸 |
| Wowhead | [7384](https://www.wowhead.com/wotlk/ko/spell=7384) | 제압 | 12 | 조건부 전투 유틸 |
| Wowhead | [20230](https://www.wowhead.com/wotlk/ko/spell=20230) | 보복 | 20 | 조건부 전투 유틸 |
| Wowhead | [12678](https://www.wowhead.com/wotlk/ko/spell=12678) | 태세 숙련 지속효과 | 20 | 조건부 전투 유틸 |
| Wowhead | [1719](https://www.wowhead.com/wotlk/ko/spell=1719) | 무모한 희생 | 50 | 조건부 전투 유틸 |
| Wowhead | [3411](https://www.wowhead.com/wotlk/ko/spell=3411) | 가로막기 | 70 | 조건부 전투 유틸 |
| Wowhead | [47475](https://www.wowhead.com/wotlk/ko/spell=47475) | 격돌 8 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [47488](https://www.wowhead.com/wotlk/ko/spell=47488) | 방패 밀쳐내기 8 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [57823](https://www.wowhead.com/wotlk/ko/spell=57823) | 복수 9 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [57755](https://www.wowhead.com/wotlk/ko/spell=57755) | 영웅의 투척 | 80 | 조건부 전투 유틸 |


## 성기사 (Paladin)

Wowhead 기준 URL: [paladin](https://www.wowhead.com/wotlk/ko/spells/abilities/paladin)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 7 |
| 태세/형상/오라/폼 전환 | 13 |
| 전투 중 공격기 - 단일 대상 | 6 |
| 전투 중 공격기 - 광역/다중 대상 | 1 |
| 전투 중 CC/메즈/이동 제한 | 1 |
| 해제/정화 | 1 |
| 생존/방어/피해 감소 | 6 |
| 치유/회복 | 4 |
| 어그로/도발/위협 제어 | 3 |
| 소환/펫/하수인 | 2 |
| 전투 유틸/특수 상황 | 13 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [20217](https://www.wowhead.com/wotlk/ko/spell=20217) | 왕의 축복 | 20 | 전투 전 유지 확인 |
| Wowhead | [25899](https://www.wowhead.com/wotlk/ko/spell=25899) | 상급 성역의 축복 | 60 | 전투 전 유지 확인 |
| Wowhead | [25898](https://www.wowhead.com/wotlk/ko/spell=25898) | 상급 왕의 축복 | 60 | 전투 전 유지 확인 |
| Wowhead | [48938](https://www.wowhead.com/wotlk/ko/spell=48938) | 상급 지혜의 축복 5 레벨 | 77 | 전투 전 유지 확인 |
| Wowhead | [48936](https://www.wowhead.com/wotlk/ko/spell=48936) | 지혜의 축복 9 레벨 | 77 | 전투 전 유지 확인 |
| Wowhead | [48934](https://www.wowhead.com/wotlk/ko/spell=48934) | 상급 힘의 축복 5 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [48932](https://www.wowhead.com/wotlk/ko/spell=48932) | 힘의 축복 10 레벨 | 79 | 전투 전 유지 확인 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [21084](https://www.wowhead.com/wotlk/ko/spell=21084) | 정의의 문장 | 1 | 필요 태세/형상일 때 사용 |
| Wowhead | [20164](https://www.wowhead.com/wotlk/ko/spell=20164) | 응징의 문장 | 22 | 필요 태세/형상일 때 사용 |
| Wowhead | [19746](https://www.wowhead.com/wotlk/ko/spell=19746) | 집중의 오라 | 22 | 필요 태세/형상일 때 사용 |
| Wowhead | [20165](https://www.wowhead.com/wotlk/ko/spell=20165) | 빛의 문장 | 30 | 필요 태세/형상일 때 사용 |
| Wowhead | [20166](https://www.wowhead.com/wotlk/ko/spell=20166) | 지혜의 문장 | 38 | 필요 태세/형상일 때 사용 |
| Wowhead | [32223](https://www.wowhead.com/wotlk/ko/spell=32223) | 성전사의 오라 | 62 | 필요 태세/형상일 때 사용 |
| Wowhead | [31801](https://www.wowhead.com/wotlk/ko/spell=31801) | 복수의 문장 | 64 | 필요 태세/형상일 때 사용 |
| Wowhead | [348704](https://www.wowhead.com/wotlk/ko/spell=348704) | 타락의 문장 | 64 | 필요 태세/형상일 때 사용 |
| Wowhead | [48943](https://www.wowhead.com/wotlk/ko/spell=48943) | 암흑 저항의 오라 5 레벨 | 76 | 필요 태세/형상일 때 사용 |
| Wowhead | [54043](https://www.wowhead.com/wotlk/ko/spell=54043) | 응보의 오라 7 레벨 | 76 | 필요 태세/형상일 때 사용 |
| Wowhead | [48945](https://www.wowhead.com/wotlk/ko/spell=48945) | 냉기 저항의 오라 5 레벨 | 77 | 필요 태세/형상일 때 사용 |
| Wowhead | [48947](https://www.wowhead.com/wotlk/ko/spell=48947) | 화염 저항의 오라 5 레벨 | 78 | 필요 태세/형상일 때 사용 |
| Wowhead | [48942](https://www.wowhead.com/wotlk/ko/spell=48942) | 기원의 오라 10 레벨 | 79 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [20271](https://www.wowhead.com/wotlk/ko/spell=20271) | 빛의 심판 | 4 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53408](https://www.wowhead.com/wotlk/ko/spell=53408) | 지혜의 심판 | 12 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53407](https://www.wowhead.com/wotlk/ko/spell=53407) | 응징의 심판 | 28 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [356112](https://www.wowhead.com/wotlk/ko/spell=356112) | 타락의 심판 1 레벨 | 64 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48801](https://www.wowhead.com/wotlk/ko/spell=48801) | 퇴마술 9 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48806](https://www.wowhead.com/wotlk/ko/spell=48806) | 천벌의 망치 6 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48819](https://www.wowhead.com/wotlk/ko/spell=48819) | 신성화 8 레벨 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [10308](https://www.wowhead.com/wotlk/ko/spell=10308) | 심판의 망치 4 레벨 | 54 | 대상 제어/시간 벌기 |

### 해제/정화

아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [4987](https://www.wowhead.com/wotlk/ko/spell=4987) | 정화 | 42 | 해제 가능한 효과가 있을 때 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [498](https://www.wowhead.com/wotlk/ko/spell=498) | 신의 가호 | 6 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [1044](https://www.wowhead.com/wotlk/ko/spell=1044) | 자유의 손길 | 18 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [642](https://www.wowhead.com/wotlk/ko/spell=642) | 천상의 보호막 | 34 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [10278](https://www.wowhead.com/wotlk/ko/spell=10278) | 보호의 손길 3 레벨 | 38 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [6940](https://www.wowhead.com/wotlk/ko/spell=6940) | 희생의 손길 | 46 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [53601](https://www.wowhead.com/wotlk/ko/spell=53601) | 성스러운 보호막 1 레벨 | 80 | 체력 위험 또는 큰 피해 예측 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [54968](https://www.wowhead.com/wotlk/ko/spell=54968) | 성스러운 빛의 문양 | 1 | 아군/자신 체력 회복 |
| Wowhead | [48788](https://www.wowhead.com/wotlk/ko/spell=48788) | 신의 축복 5 레벨 | 78 | 아군/자신 체력 회복 |
| Wowhead | [48785](https://www.wowhead.com/wotlk/ko/spell=48785) | 빛의 섬광 9 레벨 | 79 | 아군/자신 체력 회복 |
| Wowhead | [48782](https://www.wowhead.com/wotlk/ko/spell=48782) | 성스러운 빛 13 레벨 | 80 | 아군/자신 체력 회복 |

### 어그로/도발/위협 제어

대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [31789](https://www.wowhead.com/wotlk/ko/spell=31789) | 정의의 방어 | 14 | 대상 고정/위협 제어 |
| Wowhead | [62124](https://www.wowhead.com/wotlk/ko/spell=62124) | 심판의 손길 | 16 | 대상 고정/위협 제어 |
| Wowhead | [25780](https://www.wowhead.com/wotlk/ko/spell=25780) | 정의의 격노 | 16 | 대상 고정/위협 제어 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [34769](https://www.wowhead.com/wotlk/ko/spell=34769) | 전투마 소환 소환 | 20 | 전투 전 또는 펫 부재 시 |
| Wowhead | [34767](https://www.wowhead.com/wotlk/ko/spell=34767) | 군마 소환 소환 | 40 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1152](https://www.wowhead.com/wotlk/ko/spell=1152) | 순화 | 8 | 조건부 전투 유틸 |
| Wowhead | [5502](https://www.wowhead.com/wotlk/ko/spell=5502) | 언데드 감지 | 20 | 조건부 전투 유틸 |
| Wowhead | [13819](https://www.wowhead.com/wotlk/ko/spell=13819) | 전투마 소환 | 20 | 조건부 전투 유틸 |
| Wowhead | [10326](https://www.wowhead.com/wotlk/ko/spell=10326) | 악령 퇴치 | 24 | 조건부 전투 유틸 |
| Wowhead | [1038](https://www.wowhead.com/wotlk/ko/spell=1038) | 구원의 손길 | 26 | 조건부 전투 유틸 |
| Wowhead | [19752](https://www.wowhead.com/wotlk/ko/spell=19752) | 성스러운 중재 | 30 | 조건부 전투 유틸 |
| Wowhead | [23214](https://www.wowhead.com/wotlk/ko/spell=23214) | 군마 소환 | 40 | 조건부 전투 유틸 |
| Wowhead | [356110](https://www.wowhead.com/wotlk/ko/spell=356110) | 피의 타락 | 64 | 조건부 전투 유틸 |
| Wowhead | [31884](https://www.wowhead.com/wotlk/ko/spell=31884) | 응징의 격노 | 70 | 조건부 전투 유틸 |
| Wowhead | [54428](https://www.wowhead.com/wotlk/ko/spell=54428) | 신성한 기도 | 71 | 조건부 전투 유틸 |
| Wowhead | [48817](https://www.wowhead.com/wotlk/ko/spell=48817) | 신의 격노 5 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [48950](https://www.wowhead.com/wotlk/ko/spell=48950) | 구원 7 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [61411](https://www.wowhead.com/wotlk/ko/spell=61411) | 정의의 방패 2 레벨 | 80 | 조건부 전투 유틸 |


## 사냥꾼 (Hunter)

Wowhead 기준 URL: [hunter](https://www.wowhead.com/wotlk/ko/spells/abilities/hunter)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 1 |
| 전투 중 공격기 - 단일 대상 | 15 |
| 전투 중 공격기 - 광역/다중 대상 | 3 |
| 이동/돌진/도주 | 1 |
| 어그로/도발/위협 제어 | 1 |
| 소환/펫/하수인 | 3 |
| 전투 유틸/특수 상황 | 21 |
| 전투 사용 제외 후보 | 13 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [53338](https://www.wowhead.com/wotlk/ko/spell=53338) | 사냥꾼의 징표 5 레벨 | 76 | 전투 전 유지 확인 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [75](https://www.wowhead.com/wotlk/ko/spell=75) | 자동 사격 | 1 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [5116](https://www.wowhead.com/wotlk/ko/spell=5116) | 충격포 | 8 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [20736](https://www.wowhead.com/wotlk/ko/spell=20736) | 견제 사격 1 레벨 | 12 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [2974](https://www.wowhead.com/wotlk/ko/spell=2974) | 날개 절단 | 12 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [3043](https://www.wowhead.com/wotlk/ko/spell=3043) | 전갈 쐐기 | 22 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [3034](https://www.wowhead.com/wotlk/ko/spell=3034) | 살무사 쐐기 | 36 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [19801](https://www.wowhead.com/wotlk/ko/spell=19801) | 평정의 사격 | 60 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49052](https://www.wowhead.com/wotlk/ko/spell=49052) | 고정 사격 4 레벨 | 77 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48996](https://www.wowhead.com/wotlk/ko/spell=48996) | 랩터의 일격 11 레벨 | 77 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49056](https://www.wowhead.com/wotlk/ko/spell=49056) | 제물의 덫 8 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49001](https://www.wowhead.com/wotlk/ko/spell=49001) | 독사 쐐기 12 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49045](https://www.wowhead.com/wotlk/ko/spell=49045) | 신비한 사격 11 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [61006](https://www.wowhead.com/wotlk/ko/spell=61006) | 마무리 사격 3 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53339](https://www.wowhead.com/wotlk/ko/spell=53339) | 살쾡이의 이빨 6 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [60192](https://www.wowhead.com/wotlk/ko/spell=60192) | 얼음의 화살 1 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49067](https://www.wowhead.com/wotlk/ko/spell=49067) | 폭발의 덫 6 레벨 | 77 | 다수 대상일 때 사용 |
| Wowhead | [58434](https://www.wowhead.com/wotlk/ko/spell=58434) | 연발 사격 6 레벨 | 80 | 다수 대상일 때 사용 |
| Wowhead | [49048](https://www.wowhead.com/wotlk/ko/spell=49048) | 일제 사격 8 레벨 | 80 | 다수 대상일 때 사용 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [781](https://www.wowhead.com/wotlk/ko/spell=781) | 철수 | 20 | 거리 조절/접근/이탈 |

### 어그로/도발/위협 제어

대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [34477](https://www.wowhead.com/wotlk/ko/spell=34477) | 눈속임 | 70 | 대상 고정/위협 제어 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [982](https://www.wowhead.com/wotlk/ko/spell=982) | 야수 되살리기 | 10 | 전투 전 또는 펫 부재 시 |
| Wowhead | [883](https://www.wowhead.com/wotlk/ko/spell=883) | 야수 부르기 | 10 | 전투 전 또는 펫 부재 시 |
| Wowhead | [62757](https://www.wowhead.com/wotlk/ko/spell=62757) | 맡긴 야수 부르기 | 80 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [13163](https://www.wowhead.com/wotlk/ko/spell=13163) | 원숭이의 상 | 4 | 조건부 전투 유틸 |
| Wowhead | [6197](https://www.wowhead.com/wotlk/ko/spell=6197) | 독수리의 눈 | 14 | 조건부 전투 유틸 |
| Wowhead | [5118](https://www.wowhead.com/wotlk/ko/spell=5118) | 치타의 상 | 16 | 조건부 전투 유틸 |
| Wowhead | [34074](https://www.wowhead.com/wotlk/ko/spell=34074) | 독사의 상 | 20 | 조건부 전투 유틸 |
| Wowhead | [3045](https://www.wowhead.com/wotlk/ko/spell=3045) | 속사 | 26 | 조건부 전투 유틸 |
| Wowhead | [13809](https://www.wowhead.com/wotlk/ko/spell=13809) | 냉기의 덫 | 28 | 조건부 전투 유틸 |
| Wowhead | [13161](https://www.wowhead.com/wotlk/ko/spell=13161) | 야수의 상 | 30 | 조건부 전투 유틸 |
| Wowhead | [5384](https://www.wowhead.com/wotlk/ko/spell=5384) | 죽은척하기 | 30 | 조건부 전투 유틸 |
| Wowhead | [1543](https://www.wowhead.com/wotlk/ko/spell=1543) | 섬광 | 32 | 조건부 전투 유틸 |
| Wowhead | [13159](https://www.wowhead.com/wotlk/ko/spell=13159) | 치타 무리의 상 | 40 | 조건부 전투 유틸 |
| Wowhead | [14327](https://www.wowhead.com/wotlk/ko/spell=14327) | 야수 겁주기 3 레벨 | 46 | 조건부 전투 유틸 |
| Wowhead | [19263](https://www.wowhead.com/wotlk/ko/spell=19263) | 공격 저지 | 60 | 조건부 전투 유틸 |
| Wowhead | [14311](https://www.wowhead.com/wotlk/ko/spell=14311) | 얼음의 덫 3 레벨 | 60 | 조건부 전투 유틸 |
| Wowhead | [34026](https://www.wowhead.com/wotlk/ko/spell=34026) | 살상 명령 | 66 | 조건부 전투 유틸 |
| Wowhead | [27044](https://www.wowhead.com/wotlk/ko/spell=27044) | 매의 상 8 레벨 | 68 | 조건부 전투 유틸 |
| Wowhead | [34600](https://www.wowhead.com/wotlk/ko/spell=34600) | 뱀 덫 | 68 | 조건부 전투 유틸 |
| Wowhead | [53271](https://www.wowhead.com/wotlk/ko/spell=53271) | 주인의 부름 | 75 | 조건부 전투 유틸 |
| Wowhead | [49071](https://www.wowhead.com/wotlk/ko/spell=49071) | 야생의 상 4 레벨 | 76 | 조건부 전투 유틸 |
| Wowhead | [425777](https://www.wowhead.com/wotlk/ko/spell=425777) | 덫 발사: 폭발 덫 1 레벨 | 77 | 조건부 전투 유틸 |
| Wowhead | [48990](https://www.wowhead.com/wotlk/ko/spell=48990) | 동물 치료 10 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [61847](https://www.wowhead.com/wotlk/ko/spell=61847) | 용매의 상 2 레벨 | 80 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1494](https://www.wowhead.com/wotlk/ko/spell=1494) | 야수 추적 | 1 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [6991](https://www.wowhead.com/wotlk/ko/spell=6991) | 먹이주기 | 10 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [1515](https://www.wowhead.com/wotlk/ko/spell=1515) | 야수 길들이기 | 10 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [2641](https://www.wowhead.com/wotlk/ko/spell=2641) | 야수 소환 해제 | 10 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19883](https://www.wowhead.com/wotlk/ko/spell=19883) | 인간형 추적 | 10 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [1002](https://www.wowhead.com/wotlk/ko/spell=1002) | 야수의 눈 | 14 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19884](https://www.wowhead.com/wotlk/ko/spell=19884) | 언데드 추적 | 18 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [1462](https://www.wowhead.com/wotlk/ko/spell=1462) | 야수 연구 | 24 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19885](https://www.wowhead.com/wotlk/ko/spell=19885) | 은신 추적 | 24 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19880](https://www.wowhead.com/wotlk/ko/spell=19880) | 정령 추적 | 26 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19878](https://www.wowhead.com/wotlk/ko/spell=19878) | 악마 추적 | 32 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19882](https://www.wowhead.com/wotlk/ko/spell=19882) | 거인 추적 | 40 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [19879](https://www.wowhead.com/wotlk/ko/spell=19879) | 용족 추적 | 50 | 시련 전투 AI 사용 제외 권장 |


## 도적 (Rogue)

Wowhead 기준 URL: [rogue](https://www.wowhead.com/wotlk/ko/spells/abilities/rogue)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 태세/형상/오라/폼 전환 | 1 |
| 전투 중 공격기 - 단일 대상 | 4 |
| 전투 중 공격기 - 광역/다중 대상 | 1 |
| 전투 중 CC/메즈/이동 제한 | 5 |
| 차단/침묵/시전 방해 | 1 |
| 생존/방어/피해 감소 | 3 |
| 이동/돌진/도주 | 1 |
| 전투 유틸/특수 상황 | 13 |
| 전투 사용 제외 후보 | 3 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1784](https://www.wowhead.com/wotlk/ko/spell=1784) | 은신 | 1 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48672](https://www.wowhead.com/wotlk/ko/spell=48672) | 파열 9 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48657](https://www.wowhead.com/wotlk/ko/spell=48657) | 기습 12 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [57993](https://www.wowhead.com/wotlk/ko/spell=57993) | 독살 4 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48638](https://www.wowhead.com/wotlk/ko/spell=48638) | 사악한 일격 12 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [51723](https://www.wowhead.com/wotlk/ko/spell=51723) | 칼날 부채 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1776](https://www.wowhead.com/wotlk/ko/spell=1776) | 후려치기 | 6 | 대상 제어/시간 벌기 |
| Wowhead | [1833](https://www.wowhead.com/wotlk/ko/spell=1833) | 비열한 습격 | 26 | 대상 제어/시간 벌기 |
| Wowhead | [2094](https://www.wowhead.com/wotlk/ko/spell=2094) | 실명 | 34 | 대상 제어/시간 벌기 |
| Wowhead | [8643](https://www.wowhead.com/wotlk/ko/spell=8643) | 급소 가격 2 레벨 | 50 | 대상 제어/시간 벌기 |
| Wowhead | [48676](https://www.wowhead.com/wotlk/ko/spell=48676) | 목조르기 10 레벨 | 80 | 대상 제어/시간 벌기 |

### 차단/침묵/시전 방해

주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1766](https://www.wowhead.com/wotlk/ko/spell=1766) | 발차기 | 12 | 적 시전 중 우선 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [26669](https://www.wowhead.com/wotlk/ko/spell=26669) | 회피 2 레벨 | 50 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [26889](https://www.wowhead.com/wotlk/ko/spell=26889) | 소멸 3 레벨 | 62 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [31224](https://www.wowhead.com/wotlk/ko/spell=31224) | 그림자 망토 | 66 | 체력 위험 또는 큰 피해 예측 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [11305](https://www.wowhead.com/wotlk/ko/spell=11305) | 전력 질주 3 레벨 | 58 | 거리 조절/접근/이탈 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [8647](https://www.wowhead.com/wotlk/ko/spell=8647) | 약점 노출 | 14 | 조건부 전투 유틸 |
| Wowhead | [51722](https://www.wowhead.com/wotlk/ko/spell=51722) | 장비 분해 | 20 | 조건부 전투 유틸 |
| Wowhead | [1725](https://www.wowhead.com/wotlk/ko/spell=1725) | 혼란 | 22 | 조건부 전투 유틸 |
| Wowhead | [2836](https://www.wowhead.com/wotlk/ko/spell=2836) | 함정 감지 지속효과 | 24 | 조건부 전투 유틸 |
| Wowhead | [1860](https://www.wowhead.com/wotlk/ko/spell=1860) | 낙법 지속효과 | 40 | 조건부 전투 유틸 |
| Wowhead | [6774](https://www.wowhead.com/wotlk/ko/spell=6774) | 난도질 2 레벨 | 42 | 조건부 전투 유틸 |
| Wowhead | [5938](https://www.wowhead.com/wotlk/ko/spell=5938) | 독칼 | 70 | 조건부 전투 유틸 |
| Wowhead | [51724](https://www.wowhead.com/wotlk/ko/spell=51724) | 기절시키기 4 레벨 | 71 | 조건부 전투 유틸 |
| Wowhead | [57934](https://www.wowhead.com/wotlk/ko/spell=57934) | 속임수 거래 | 75 | 조건부 전투 유틸 |
| Wowhead | [48674](https://www.wowhead.com/wotlk/ko/spell=48674) | 죽음의 투척 3 레벨 | 76 | 조건부 전투 유틸 |
| Wowhead | [48659](https://www.wowhead.com/wotlk/ko/spell=48659) | 교란 8 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [48668](https://www.wowhead.com/wotlk/ko/spell=48668) | 절개 12 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [48691](https://www.wowhead.com/wotlk/ko/spell=48691) | 매복 10 레벨 | 80 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1804](https://www.wowhead.com/wotlk/ko/spell=1804) | 자물쇠 따기 | 1 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [921](https://www.wowhead.com/wotlk/ko/spell=921) | 훔치기 | 4 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [1842](https://www.wowhead.com/wotlk/ko/spell=1842) | 함정 해제 | 30 | 시련 전투 AI 사용 제외 권장 |


## 사제 (Priest)

Wowhead 기준 URL: [priest](https://www.wowhead.com/wotlk/ko/spells/abilities/priest)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 8 |
| 전투 중 공격기 - 단일 대상 | 7 |
| 전투 중 공격기 - 광역/다중 대상 | 1 |
| 해제/정화 | 3 |
| 생존/방어/피해 감소 | 2 |
| 치유/회복 | 10 |
| 부활 | 1 |
| 전투 유틸/특수 상황 | 13 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [6346](https://www.wowhead.com/wotlk/ko/spell=6346) | 공포의 수호물 | 20 | 전투 전 유지 확인 |
| Wowhead | [605](https://www.wowhead.com/wotlk/ko/spell=605) | 정신 지배 | 30 | 전투 전 유지 확인 |
| Wowhead | [48169](https://www.wowhead.com/wotlk/ko/spell=48169) | 어둠의 보호 5 레벨 | 76 | 전투 전 유지 확인 |
| Wowhead | [48168](https://www.wowhead.com/wotlk/ko/spell=48168) | 내면의 열정 9 레벨 | 77 | 전투 전 유지 확인 |
| Wowhead | [48161](https://www.wowhead.com/wotlk/ko/spell=48161) | 신의 권능: 인내 8 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [48162](https://www.wowhead.com/wotlk/ko/spell=48162) | 인내의 기원 4 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [48074](https://www.wowhead.com/wotlk/ko/spell=48074) | 정신력의 기원 3 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [48073](https://www.wowhead.com/wotlk/ko/spell=48073) | 천상의 정신 6 레벨 | 80 | 전투 전 유지 확인 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [58381](https://www.wowhead.com/wotlk/ko/spell=58381) | 정신의 채찍 9 레벨 | 0 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48135](https://www.wowhead.com/wotlk/ko/spell=48135) | 신성한 불꽃 11 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48123](https://www.wowhead.com/wotlk/ko/spell=48123) | 성스러운 일격 12 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48127](https://www.wowhead.com/wotlk/ko/spell=48127) | 정신 분열 13 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48300](https://www.wowhead.com/wotlk/ko/spell=48300) | 파멸의 역병 9 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48125](https://www.wowhead.com/wotlk/ko/spell=48125) | 어둠의 권능: 고통 12 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53023](https://www.wowhead.com/wotlk/ko/spell=53023) | 정신 불태우기 2 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48078](https://www.wowhead.com/wotlk/ko/spell=48078) | 신성한 폭발 9 레벨 | 80 | 다수 대상일 때 사용 |

### 해제/정화

아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [528](https://www.wowhead.com/wotlk/ko/spell=528) | 질병 치료 | 14 | 해제 가능한 효과가 있을 때 사용 |
| Wowhead | [552](https://www.wowhead.com/wotlk/ko/spell=552) | 질병 해제 | 32 | 해제 가능한 효과가 있을 때 사용 |
| Wowhead | [988](https://www.wowhead.com/wotlk/ko/spell=988) | 마법 무효화 2 레벨 | 36 | 해제 가능한 효과가 있을 때 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48170](https://www.wowhead.com/wotlk/ko/spell=48170) | 암흑 보호의 기원 3 레벨 | 77 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [48066](https://www.wowhead.com/wotlk/ko/spell=48066) | 신의 권능: 보호막 14 레벨 | 80 | 체력 위험 또는 큰 피해 예측 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [63544](https://www.wowhead.com/wotlk/ko/spell=63544) | 소생 강화 | 1 | 아군/자신 체력 회복 |
| Wowhead | [2053](https://www.wowhead.com/wotlk/ko/spell=2053) | 하급 치유 3 레벨 | 10 | 아군/자신 체력 회복 |
| Wowhead | [70772](https://www.wowhead.com/wotlk/ko/spell=70772) | 축복받은 치유 | 20 | 아군/자신 체력 회복 |
| Wowhead | [6064](https://www.wowhead.com/wotlk/ko/spell=6064) | 치유 4 레벨 | 34 | 아군/자신 체력 회복 |
| Wowhead | [48072](https://www.wowhead.com/wotlk/ko/spell=48072) | 치유의 기원 7 레벨 | 76 | 아군/자신 체력 회복 |
| Wowhead | [48120](https://www.wowhead.com/wotlk/ko/spell=48120) | 결속의 치유 3 레벨 | 78 | 아군/자신 체력 회복 |
| Wowhead | [48063](https://www.wowhead.com/wotlk/ko/spell=48063) | 상급 치유 9 레벨 | 78 | 아군/자신 체력 회복 |
| Wowhead | [48071](https://www.wowhead.com/wotlk/ko/spell=48071) | 순간 치유 11 레벨 | 79 | 아군/자신 체력 회복 |
| Wowhead | [48113](https://www.wowhead.com/wotlk/ko/spell=48113) | 회복의 기원 3 레벨 | 79 | 아군/자신 체력 회복 |
| Wowhead | [48068](https://www.wowhead.com/wotlk/ko/spell=48068) | 소생 14 레벨 | 80 | 아군/자신 체력 회복 |

### 부활

죽은 아군을 살리는 기술입니다. 전투 중 사용 가능 여부는 스펠별로 다릅니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48171](https://www.wowhead.com/wotlk/ko/spell=48171) | 부활 7 레벨 | 78 | 사망 아군 복구 상황 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49868](https://www.wowhead.com/wotlk/ko/spell=49868) | 어둠의 형상 | 0 | 조건부 전투 유틸 |
| Wowhead | [586](https://www.wowhead.com/wotlk/ko/spell=586) | 소실 | 8 | 조건부 전투 유틸 |
| Wowhead | [453](https://www.wowhead.com/wotlk/ko/spell=453) | 평정 | 20 | 조건부 전투 유틸 |
| Wowhead | [8129](https://www.wowhead.com/wotlk/ko/spell=8129) | 마나 연소 | 24 | 조건부 전투 유틸 |
| Wowhead | [1706](https://www.wowhead.com/wotlk/ko/spell=1706) | 공중 부양 | 34 | 조건부 전투 유틸 |
| Wowhead | [10909](https://www.wowhead.com/wotlk/ko/spell=10909) | 마음의 눈 2 레벨 | 44 | 조건부 전투 유틸 |
| Wowhead | [10890](https://www.wowhead.com/wotlk/ko/spell=10890) | 영혼의 절규 4 레벨 | 56 | 조건부 전투 유틸 |
| Wowhead | [10955](https://www.wowhead.com/wotlk/ko/spell=10955) | 언데드 속박 3 레벨 | 60 | 조건부 전투 유틸 |
| Wowhead | [34433](https://www.wowhead.com/wotlk/ko/spell=34433) | 어둠의 마귀 | 66 | 조건부 전투 유틸 |
| Wowhead | [32375](https://www.wowhead.com/wotlk/ko/spell=32375) | 대규모 무효화 | 70 | 조건부 전투 유틸 |
| Wowhead | [48158](https://www.wowhead.com/wotlk/ko/spell=48158) | 어둠의 권능: 죽음 4 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [64843](https://www.wowhead.com/wotlk/ko/spell=64843) | 천상의 찬가 1 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [64904](https://www.wowhead.com/wotlk/ko/spell=64904) | 희망의 찬가 | 80 | 조건부 전투 유틸 |


## 죽음의 기사 (Death Knight)

Wowhead 기준 URL: [death-knight](https://www.wowhead.com/wotlk/ko/spells/abilities/death-knight)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 1 |
| 태세/형상/오라/폼 전환 | 2 |
| 전투 중 공격기 - 단일 대상 | 8 |
| 전투 중 공격기 - 광역/다중 대상 | 2 |
| 전투 중 CC/메즈/이동 제한 | 3 |
| 차단/침묵/시전 방해 | 1 |
| 생존/방어/피해 감소 | 3 |
| 부활 | 1 |
| 마나/자원 회복 | 2 |
| 어그로/도발/위협 제어 | 1 |
| 소환/펫/하수인 | 2 |
| 전투 유틸/특수 상황 | 21 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [53344](https://www.wowhead.com/wotlk/ko/spell=53344) | 타락한 성전사의 룬 | 70 | 전투 전 유지 확인 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48263](https://www.wowhead.com/wotlk/ko/spell=48263) | 냉기의 형상 | 57 | 필요 태세/형상일 때 사용 |
| Wowhead | [48265](https://www.wowhead.com/wotlk/ko/spell=48265) | 부정의 형상 | 70 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [59879](https://www.wowhead.com/wotlk/ko/spell=59879) | 피의 역병 지속효과 | 1 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53342](https://www.wowhead.com/wotlk/ko/spell=53342) | 주문분쇄자의 룬 | 57 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [53323](https://www.wowhead.com/wotlk/ko/spell=53323) | 검분쇄자의 룬 | 63 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [56815](https://www.wowhead.com/wotlk/ko/spell=56815) | 룬의 일격 | 67 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49921](https://www.wowhead.com/wotlk/ko/spell=49921) | 역병의 일격 6 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [62904](https://www.wowhead.com/wotlk/ko/spell=62904) | 죽음의 고리 5 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49924](https://www.wowhead.com/wotlk/ko/spell=49924) | 죽음의 일격 5 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49930](https://www.wowhead.com/wotlk/ko/spell=49930) | 피의 일격 6 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49941](https://www.wowhead.com/wotlk/ko/spell=49941) | 피의 소용돌이 4 레벨 | 78 | 다수 대상일 때 사용 |
| Wowhead | [49938](https://www.wowhead.com/wotlk/ko/spell=49938) | 죽음과 부패 4 레벨 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49576](https://www.wowhead.com/wotlk/ko/spell=49576) | 죽음의 손아귀 | 55 | 대상 제어/시간 벌기 |
| Wowhead | [45524](https://www.wowhead.com/wotlk/ko/spell=45524) | 얼음 결계 | 58 | 대상 제어/시간 벌기 |
| Wowhead | [47476](https://www.wowhead.com/wotlk/ko/spell=47476) | 질식시키기 | 59 | 대상 제어/시간 벌기 |

### 차단/침묵/시전 방해

주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [47528](https://www.wowhead.com/wotlk/ko/spell=47528) | 정신 얼리기 | 57 | 적 시전 중 우선 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [70164](https://www.wowhead.com/wotlk/ko/spell=70164) | 네루비안 등껍질의 룬 | 40 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [48792](https://www.wowhead.com/wotlk/ko/spell=48792) | 얼음같은 인내력 | 62 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [48707](https://www.wowhead.com/wotlk/ko/spell=48707) | 대마법 보호막 | 68 | 체력 위험 또는 큰 피해 예측 |

### 부활

죽은 아군을 살리는 기술입니다. 전투 중 사용 가능 여부는 스펠별로 다릅니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [61999](https://www.wowhead.com/wotlk/ko/spell=61999) | 아군 되살리기 | 72 | 사망 아군 복구 상황 |

### 마나/자원 회복

마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [45529](https://www.wowhead.com/wotlk/ko/spell=45529) | 혈기 전환 | 64 | 자원 부족 시 사용 |
| Wowhead | [47568](https://www.wowhead.com/wotlk/ko/spell=47568) | 룬 무기 강화 | 75 | 자원 부족 시 사용 |

### 어그로/도발/위협 제어

대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [56222](https://www.wowhead.com/wotlk/ko/spell=56222) | 어둠의 명령 | 65 | 대상 고정/위협 제어 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [46584](https://www.wowhead.com/wotlk/ko/spell=46584) | 시체 되살리기 | 56 | 전투 전 또는 펫 부재 시 |
| Wowhead | [42650](https://www.wowhead.com/wotlk/ko/spell=42650) | 사자의 군대 | 80 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [59921](https://www.wowhead.com/wotlk/ko/spell=59921) | 서리 열병 지속효과 | 0 | 조건부 전투 유틸 |
| Wowhead | [52284](https://www.wowhead.com/wotlk/ko/spell=52284) | 죽음의 요새의 결의 1 레벨 | 0 | 조건부 전투 유틸 |
| Wowhead | [51986](https://www.wowhead.com/wotlk/ko/spell=51986) | 창백한 말 2 레벨 | 0 | 조건부 전투 유틸 |
| Wowhead | [53428](https://www.wowhead.com/wotlk/ko/spell=53428) | 룬벼리기 | 55 | 조건부 전투 유틸 |
| Wowhead | [49142](https://www.wowhead.com/wotlk/ko/spell=49142) | 얼어붙은 룬 무기 | 55 | 조건부 전투 유틸 |
| Wowhead | [49410](https://www.wowhead.com/wotlk/ko/spell=49410) | 재빠른 손놀림 지속효과 | 55 | 조건부 전투 유틸 |
| Wowhead | [53341](https://www.wowhead.com/wotlk/ko/spell=53341) | 잿더미 빙하의 룬 | 55 | 조건부 전투 유틸 |
| Wowhead | [48778](https://www.wowhead.com/wotlk/ko/spell=48778) | 죽음의 군마 소환 | 55 | 조건부 전투 유틸 |
| Wowhead | [50977](https://www.wowhead.com/wotlk/ko/spell=50977) | 죽음의 문 | 55 | 조건부 전투 유틸 |
| Wowhead | [53343](https://www.wowhead.com/wotlk/ko/spell=53343) | 칼날얼음의 룬 | 55 | 조건부 전투 유틸 |
| Wowhead | [48266](https://www.wowhead.com/wotlk/ko/spell=48266) | 혈기의 형상 | 55 | 조건부 전투 유틸 |
| Wowhead | [50842](https://www.wowhead.com/wotlk/ko/spell=50842) | 전염병 | 56 | 조건부 전투 유틸 |
| Wowhead | [54447](https://www.wowhead.com/wotlk/ko/spell=54447) | 주문파괴자의 룬 | 57 | 조건부 전투 유틸 |
| Wowhead | [53331](https://www.wowhead.com/wotlk/ko/spell=53331) | 리치 파멸의 룬 | 60 | 조건부 전투 유틸 |
| Wowhead | [3714](https://www.wowhead.com/wotlk/ko/spell=3714) | 얼음길 | 61 | 조건부 전투 유틸 |
| Wowhead | [54446](https://www.wowhead.com/wotlk/ko/spell=54446) | 검파괴자의 룬 | 63 | 조건부 전투 유틸 |
| Wowhead | [48743](https://www.wowhead.com/wotlk/ko/spell=48743) | 죽음의 서약 | 66 | 조건부 전투 유틸 |
| Wowhead | [62158](https://www.wowhead.com/wotlk/ko/spell=62158) | 돌가죽 가고일의 룬 | 70 | 조건부 전투 유틸 |
| Wowhead | [57623](https://www.wowhead.com/wotlk/ko/spell=57623) | 겨울의 뿔피리 2 레벨 | 75 | 조건부 전투 유틸 |
| Wowhead | [49909](https://www.wowhead.com/wotlk/ko/spell=49909) | 얼음 손길 5 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [51425](https://www.wowhead.com/wotlk/ko/spell=51425) | 절멸 4 레벨 | 79 | 조건부 전투 유틸 |


## 주술사 (Shaman)

Wowhead 기준 URL: [shaman](https://www.wowhead.com/wotlk/ko/spells/abilities/shaman)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 7 |
| 전투 중 공격기 - 단일 대상 | 3 |
| 전투 중 공격기 - 광역/다중 대상 | 2 |
| 전투 중 CC/메즈/이동 제한 | 1 |
| 차단/침묵/시전 방해 | 1 |
| 해제/정화 | 3 |
| 치유/회복 | 4 |
| 소환/펫/하수인 | 19 |
| 전투 유틸/특수 상황 | 10 |
| 전투 사용 제외 후보 | 4 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [10399](https://www.wowhead.com/wotlk/ko/spell=10399) | 대지의 무기 4 레벨 | 24 | 전투 전 유지 확인 |
| Wowhead | [57960](https://www.wowhead.com/wotlk/ko/spell=57960) | 물의 보호막 9 레벨 | 76 | 전투 전 유지 확인 |
| Wowhead | [58796](https://www.wowhead.com/wotlk/ko/spell=58796) | 냉기의 무기 9 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [51994](https://www.wowhead.com/wotlk/ko/spell=51994) | 대지생명의 무기 6 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [49281](https://www.wowhead.com/wotlk/ko/spell=49281) | 번개 보호막 11 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [58790](https://www.wowhead.com/wotlk/ko/spell=58790) | 불꽃의 무기 10 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [58804](https://www.wowhead.com/wotlk/ko/spell=58804) | 질풍의 무기 8 레벨 | 80 | 전투 전 유지 확인 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49236](https://www.wowhead.com/wotlk/ko/spell=49236) | 냉기 충격 7 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49238](https://www.wowhead.com/wotlk/ko/spell=49238) | 번개 화살 14 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49233](https://www.wowhead.com/wotlk/ko/spell=49233) | 화염 충격 9 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [61657](https://www.wowhead.com/wotlk/ko/spell=61657) | 불꽃 회오리 9 레벨 | 80 | 다수 대상일 때 사용 |
| Wowhead | [49271](https://www.wowhead.com/wotlk/ko/spell=49271) | 연쇄 번개 8 레벨 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [51514](https://www.wowhead.com/wotlk/ko/spell=51514) | 주술 | 80 | 대상 제어/시간 벌기 |

### 차단/침묵/시전 방해

주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49231](https://www.wowhead.com/wotlk/ko/spell=49231) | 대지 충격 10 레벨 | 79 | 적 시전 중 우선 사용 |

### 해제/정화

아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [526](https://www.wowhead.com/wotlk/ko/spell=526) | 독소 해제 | 16 | 해제 가능한 효과가 있을 때 사용 |
| Wowhead | [8012](https://www.wowhead.com/wotlk/ko/spell=8012) | 정화 2 레벨 | 32 | 해제 가능한 효과가 있을 때 사용 |
| Wowhead | [8170](https://www.wowhead.com/wotlk/ko/spell=8170) | 정화 토템 | 38 | 해제 가능한 효과가 있을 때 사용 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49276](https://www.wowhead.com/wotlk/ko/spell=49276) | 하급 치유의 물결 9 레벨 | 77 | 아군/자신 체력 회복 |
| Wowhead | [55459](https://www.wowhead.com/wotlk/ko/spell=55459) | 연쇄 치유 7 레벨 | 80 | 아군/자신 체력 회복 |
| Wowhead | [49273](https://www.wowhead.com/wotlk/ko/spell=49273) | 치유의 물결 14 레벨 | 80 | 아군/자신 체력 회복 |
| Wowhead | [58757](https://www.wowhead.com/wotlk/ko/spell=58757) | 치유의 토템 9 레벨 | 80 | 아군/자신 체력 회복 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2484](https://www.wowhead.com/wotlk/ko/spell=2484) | 속박의 토템 | 6 | 전투 전 또는 펫 부재 시 |
| Wowhead | [2645](https://www.wowhead.com/wotlk/ko/spell=2645) | 늑대 정령 | 16 | 전투 전 또는 펫 부재 시 |
| Wowhead | [8143](https://www.wowhead.com/wotlk/ko/spell=8143) | 진동의 토템 | 18 | 전투 전 또는 펫 부재 시 |
| Wowhead | [8177](https://www.wowhead.com/wotlk/ko/spell=8177) | 마법흡수 토템 | 30 | 전투 전 또는 펫 부재 시 |
| Wowhead | [8512](https://www.wowhead.com/wotlk/ko/spell=8512) | 질풍의 토템 | 32 | 전투 전 또는 펫 부재 시 |
| Wowhead | [6495](https://www.wowhead.com/wotlk/ko/spell=6495) | 감시의 토템 | 34 | 전투 전 또는 펫 부재 시 |
| Wowhead | [3738](https://www.wowhead.com/wotlk/ko/spell=3738) | 천벌의 토템 | 64 | 전투 전 또는 펫 부재 시 |
| Wowhead | [2062](https://www.wowhead.com/wotlk/ko/spell=2062) | 대지의 정령 토템 | 66 | 전투 전 또는 펫 부재 시 |
| Wowhead | [2894](https://www.wowhead.com/wotlk/ko/spell=2894) | 불의 정령 토템 | 68 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58753](https://www.wowhead.com/wotlk/ko/spell=58753) | 돌가죽 토템 10 레벨 | 78 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58582](https://www.wowhead.com/wotlk/ko/spell=58582) | 돌발톱 토템 10 레벨 | 78 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58734](https://www.wowhead.com/wotlk/ko/spell=58734) | 용암 토템 7 레벨 | 78 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58745](https://www.wowhead.com/wotlk/ko/spell=58745) | 냉기 저항 토템 6 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58643](https://www.wowhead.com/wotlk/ko/spell=58643) | 대지력 토템 8 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58774](https://www.wowhead.com/wotlk/ko/spell=58774) | 마나샘 토템 8 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58656](https://www.wowhead.com/wotlk/ko/spell=58656) | 불꽃의 토템 8 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58704](https://www.wowhead.com/wotlk/ko/spell=58704) | 불타는 토템 10 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58749](https://www.wowhead.com/wotlk/ko/spell=58749) | 자연 저항 토템 6 레벨 | 80 | 전투 전 또는 펫 부재 시 |
| Wowhead | [58739](https://www.wowhead.com/wotlk/ko/spell=58739) | 화염 저항 토템 6 레벨 | 80 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [57994](https://www.wowhead.com/wotlk/ko/spell=57994) | 칼 바람 | 16 | 조건부 전투 유틸 |
| Wowhead | [546](https://www.wowhead.com/wotlk/ko/spell=546) | 수면 걷기 | 28 | 조건부 전투 유틸 |
| Wowhead | [66842](https://www.wowhead.com/wotlk/ko/spell=66842) | 원소의 부름 | 30 | 조건부 전투 유틸 |
| Wowhead | [20608](https://www.wowhead.com/wotlk/ko/spell=20608) | 윤회 지속효과 | 30 | 조건부 전투 유틸 |
| Wowhead | [66843](https://www.wowhead.com/wotlk/ko/spell=66843) | 선조의 부름 | 40 | 조건부 전투 유틸 |
| Wowhead | [66844](https://www.wowhead.com/wotlk/ko/spell=66844) | 영혼의 부름 | 50 | 조건부 전투 유틸 |
| Wowhead | [32182](https://www.wowhead.com/wotlk/ko/spell=32182) | 영웅심 | 70 | 조건부 전투 유틸 |
| Wowhead | [2825](https://www.wowhead.com/wotlk/ko/spell=2825) | 피의 욕망 | 70 | 조건부 전투 유틸 |
| Wowhead | [49277](https://www.wowhead.com/wotlk/ko/spell=49277) | 고대의 영혼 7 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [60043](https://www.wowhead.com/wotlk/ko/spell=60043) | 용암 폭발 2 레벨 | 80 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [131](https://www.wowhead.com/wotlk/ko/spell=131) | 수중 호흡 | 22 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [6196](https://www.wowhead.com/wotlk/ko/spell=6196) | 천리안 | 26 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [556](https://www.wowhead.com/wotlk/ko/spell=556) | 영혼의 귀환 | 30 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [36936](https://www.wowhead.com/wotlk/ko/spell=36936) | 토템의 귀환 | 30 | 시련 전투 AI 사용 제외 권장 |


## 마법사 (Mage)

Wowhead 기준 URL: [mage](https://www.wowhead.com/wotlk/ko/spells/abilities/mage)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 5 |
| 전투 중 공격기 - 단일 대상 | 8 |
| 전투 중 공격기 - 광역/다중 대상 | 3 |
| 전투 중 CC/메즈/이동 제한 | 2 |
| 차단/침묵/시전 방해 | 1 |
| 해제/정화 | 1 |
| 생존/방어/피해 감소 | 5 |
| 치유/회복 | 1 |
| 마나/자원 회복 | 1 |
| 이동/돌진/도주 | 1 |
| 소환/펫/하수인 | 1 |
| 전투 유틸/특수 상황 | 22 |
| 전투 사용 제외 후보 | 16 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [43024](https://www.wowhead.com/wotlk/ko/spell=43024) | 마법사 갑옷 6 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [43008](https://www.wowhead.com/wotlk/ko/spell=43008) | 얼음 갑옷 6 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [43046](https://www.wowhead.com/wotlk/ko/spell=43046) | 타오르는 갑옷 3 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [61024](https://www.wowhead.com/wotlk/ko/spell=61024) | 달라란의 지능 7 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [42995](https://www.wowhead.com/wotlk/ko/spell=42995) | 신비한 지능 7 레벨 | 80 | 전투 전 유지 확인 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [42859](https://www.wowhead.com/wotlk/ko/spell=42859) | 불태우기 11 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42914](https://www.wowhead.com/wotlk/ko/spell=42914) | 얼음창 3 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42833](https://www.wowhead.com/wotlk/ko/spell=42833) | 화염구 16 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42846](https://www.wowhead.com/wotlk/ko/spell=42846) | 신비한 화살 13 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42842](https://www.wowhead.com/wotlk/ko/spell=42842) | 얼음 화살 16 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42897](https://www.wowhead.com/wotlk/ko/spell=42897) | 비전 작렬 4 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47610](https://www.wowhead.com/wotlk/ko/spell=47610) | 얼음불꽃 화살 2 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [42873](https://www.wowhead.com/wotlk/ko/spell=42873) | 화염 작렬 11 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [42926](https://www.wowhead.com/wotlk/ko/spell=42926) | 불기둥 9 레벨 | 79 | 다수 대상일 때 사용 |
| Wowhead | [42940](https://www.wowhead.com/wotlk/ko/spell=42940) | 눈보라 9 레벨 | 80 | 다수 대상일 때 사용 |
| Wowhead | [42921](https://www.wowhead.com/wotlk/ko/spell=42921) | 신비한 폭발 10 레벨 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [12826](https://www.wowhead.com/wotlk/ko/spell=12826) | 변이 4 레벨 | 60 | 대상 제어/시간 벌기 |
| Wowhead | [42917](https://www.wowhead.com/wotlk/ko/spell=42917) | 얼음 회오리 6 레벨 | 75 | 대상 제어/시간 벌기 |

### 차단/침묵/시전 방해

주문 시전 중인 적을 끊거나 일정 시간 같은 계열 주문을 막는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2139](https://www.wowhead.com/wotlk/ko/spell=2139) | 마법 차단 | 24 | 적 시전 중 우선 사용 |

### 해제/정화

아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [475](https://www.wowhead.com/wotlk/ko/spell=475) | 저주 해제 | 18 | 해제 가능한 효과가 있을 때 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [7301](https://www.wowhead.com/wotlk/ko/spell=7301) | 냉기 갑옷 3 레벨 | 20 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [45438](https://www.wowhead.com/wotlk/ko/spell=45438) | 얼음 방패 | 30 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [43010](https://www.wowhead.com/wotlk/ko/spell=43010) | 화염계 수호 7 레벨 | 78 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [43012](https://www.wowhead.com/wotlk/ko/spell=43012) | 냉기계 수호 7 레벨 | 79 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [43020](https://www.wowhead.com/wotlk/ko/spell=43020) | 마나 보호막 9 레벨 | 79 | 체력 위험 또는 큰 피해 예측 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [42956](https://www.wowhead.com/wotlk/ko/spell=42956) | 원기 회복의 음식 창조 2 레벨 | 80 | 아군/자신 체력 회복 |

### 마나/자원 회복

마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [12051](https://www.wowhead.com/wotlk/ko/spell=12051) | 환기 | 20 | 자원 부족 시 사용 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [1953](https://www.wowhead.com/wotlk/ko/spell=1953) | 점멸 | 20 | 거리 조절/접근/이탈 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [70909](https://www.wowhead.com/wotlk/ko/spell=70909) | 물의 정령 소환 (견본) | 50 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [49361](https://www.wowhead.com/wotlk/ko/spell=49361) | 차원의 문: 스토나드 | 35 | 조건부 전투 유틸 |
| Wowhead | [49360](https://www.wowhead.com/wotlk/ko/spell=49360) | 차원의 문: 테라모어 | 35 | 조건부 전투 유틸 |
| Wowhead | [10059](https://www.wowhead.com/wotlk/ko/spell=10059) | 차원의 문: 스톰윈드 | 40 | 조건부 전투 유틸 |
| Wowhead | [32267](https://www.wowhead.com/wotlk/ko/spell=32267) | 차원의 문: 실버문 | 40 | 조건부 전투 유틸 |
| Wowhead | [11416](https://www.wowhead.com/wotlk/ko/spell=11416) | 차원의 문: 아이언포지 | 40 | 조건부 전투 유틸 |
| Wowhead | [11418](https://www.wowhead.com/wotlk/ko/spell=11418) | 차원의 문: 언더시티 | 40 | 조건부 전투 유틸 |
| Wowhead | [32266](https://www.wowhead.com/wotlk/ko/spell=32266) | 차원의 문: 엑소다르 | 40 | 조건부 전투 유틸 |
| Wowhead | [11417](https://www.wowhead.com/wotlk/ko/spell=11417) | 차원의 문: 오그리마 | 40 | 조건부 전투 유틸 |
| Wowhead | [11419](https://www.wowhead.com/wotlk/ko/spell=11419) | 차원의 문: 다르나서스 | 50 | 조건부 전투 유틸 |
| Wowhead | [11420](https://www.wowhead.com/wotlk/ko/spell=11420) | 차원의 문: 썬더 블러프 | 50 | 조건부 전투 유틸 |
| Wowhead | [35717](https://www.wowhead.com/wotlk/ko/spell=35717) | 차원의 문: 샤트라스 | 65 | 조건부 전투 유틸 |
| Wowhead | [66](https://www.wowhead.com/wotlk/ko/spell=66) | 투명화 | 68 | 조건부 전투 유틸 |
| Wowhead | [27090](https://www.wowhead.com/wotlk/ko/spell=27090) | 음료 창조 9 레벨 | 70 | 조건부 전투 유틸 |
| Wowhead | [33717](https://www.wowhead.com/wotlk/ko/spell=33717) | 음식 창조 8 레벨 | 70 | 조건부 전투 유틸 |
| Wowhead | [53142](https://www.wowhead.com/wotlk/ko/spell=53142) | 차원의 문: 달라란 | 74 | 조건부 전투 유틸 |
| Wowhead | [43015](https://www.wowhead.com/wotlk/ko/spell=43015) | 마법 감쇠 7 레벨 | 76 | 조건부 전투 유틸 |
| Wowhead | [43017](https://www.wowhead.com/wotlk/ko/spell=43017) | 마법 증폭 7 레벨 | 77 | 조건부 전투 유틸 |
| Wowhead | [42931](https://www.wowhead.com/wotlk/ko/spell=42931) | 냉기 돌풍 8 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [61316](https://www.wowhead.com/wotlk/ko/spell=61316) | 달라란의 총명함 3 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [43002](https://www.wowhead.com/wotlk/ko/spell=43002) | 신비한 총명함 3 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [55342](https://www.wowhead.com/wotlk/ko/spell=55342) | 환영 복제 | 80 | 조건부 전투 유틸 |
| Wowhead | [413841](https://www.wowhead.com/wotlk/ko/spell=413841) | 점화 | 99 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [130](https://www.wowhead.com/wotlk/ko/spell=130) | 저속 낙하 | 12 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3561](https://www.wowhead.com/wotlk/ko/spell=3561) | 순간이동: 스톰윈드 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [32272](https://www.wowhead.com/wotlk/ko/spell=32272) | 순간이동: 실버문 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3562](https://www.wowhead.com/wotlk/ko/spell=3562) | 순간이동: 아이언포지 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3563](https://www.wowhead.com/wotlk/ko/spell=3563) | 순간이동: 언더시티 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [32271](https://www.wowhead.com/wotlk/ko/spell=32271) | 순간이동: 엑소다르 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3567](https://www.wowhead.com/wotlk/ko/spell=3567) | 순간이동: 오그리마 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3565](https://www.wowhead.com/wotlk/ko/spell=3565) | 순간이동: 다르나서스 | 30 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [3566](https://www.wowhead.com/wotlk/ko/spell=3566) | 순간이동: 썬더 블러프 | 30 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [49358](https://www.wowhead.com/wotlk/ko/spell=49358) | 순간이동: 스토나드 | 35 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [49359](https://www.wowhead.com/wotlk/ko/spell=49359) | 순간이동: 테라모어 | 35 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [35715](https://www.wowhead.com/wotlk/ko/spell=35715) | 순간이동: 샤트라스 | 60 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [30449](https://www.wowhead.com/wotlk/ko/spell=30449) | 마법 훔치기 | 70 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [53140](https://www.wowhead.com/wotlk/ko/spell=53140) | 순간이동: 달라란 | 71 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [42985](https://www.wowhead.com/wotlk/ko/spell=42985) | 마나석 창조 6 레벨 | 77 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [58659](https://www.wowhead.com/wotlk/ko/spell=58659) | 원기 회복의 의식 2 레벨 | 80 | 시련 전투 AI 사용 제외 권장 |


## 흑마법사 (Warlock)

Wowhead 기준 URL: [warlock](https://www.wowhead.com/wotlk/ko/spells/abilities/warlock)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 3 |
| 태세/형상/오라/폼 전환 | 1 |
| 전투 중 공격기 - 단일 대상 | 14 |
| 전투 중 공격기 - 광역/다중 대상 | 4 |
| 전투 중 CC/메즈/이동 제한 | 4 |
| 생존/방어/피해 감소 | 1 |
| 마나/자원 회복 | 1 |
| 이동/돌진/도주 | 1 |
| 소환/펫/하수인 | 6 |
| 전투 유틸/특수 상황 | 11 |
| 전투 사용 제외 후보 | 10 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [696](https://www.wowhead.com/wotlk/ko/spell=696) | 악마의 피부 2 레벨 | 10 | 전투 전 유지 확인 |
| Wowhead | [47893](https://www.wowhead.com/wotlk/ko/spell=47893) | 마의 갑옷 4 레벨 | 79 | 전투 전 유지 확인 |
| Wowhead | [47889](https://www.wowhead.com/wotlk/ko/spell=47889) | 악마의 갑옷 8 레벨 | 80 | 전투 전 유지 확인 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [50589](https://www.wowhead.com/wotlk/ko/spell=50589) | 제물의 오라 악마 | 60 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [75445](https://www.wowhead.com/wotlk/ko/spell=75445) | 악마의 제물 3 레벨 | 0 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [5138](https://www.wowhead.com/wotlk/ko/spell=5138) | 마나 흡수 | 24 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [11719](https://www.wowhead.com/wotlk/ko/spell=11719) | 언어의 저주 2 레벨 | 50 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [50511](https://www.wowhead.com/wotlk/ko/spell=50511) | 무력화 저주 9 레벨 | 71 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47813](https://www.wowhead.com/wotlk/ko/spell=47813) | 부패 10 레벨 | 77 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47855](https://www.wowhead.com/wotlk/ko/spell=47855) | 영혼 흡수 6 레벨 | 77 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47857](https://www.wowhead.com/wotlk/ko/spell=47857) | 생명력 흡수 9 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47865](https://www.wowhead.com/wotlk/ko/spell=47865) | 원소의 저주 5 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47860](https://www.wowhead.com/wotlk/ko/spell=47860) | 죽음의 고리 6 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47864](https://www.wowhead.com/wotlk/ko/spell=47864) | 고통의 저주 9 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47815](https://www.wowhead.com/wotlk/ko/spell=47815) | 불타는 고통 10 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47809](https://www.wowhead.com/wotlk/ko/spell=47809) | 어둠의 화살 13 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47811](https://www.wowhead.com/wotlk/ko/spell=47811) | 제물 11 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [47867](https://www.wowhead.com/wotlk/ko/spell=47867) | 파멸의 저주 3 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [50581](https://www.wowhead.com/wotlk/ko/spell=50581) | 어둠의 회전베기 악마 | 60 | 다수 대상일 때 사용 |
| Wowhead | [47823](https://www.wowhead.com/wotlk/ko/spell=47823) | 지옥의 불길 5 레벨 | 78 | 다수 대상일 때 사용 |
| Wowhead | [47820](https://www.wowhead.com/wotlk/ko/spell=47820) | 불의 비 7 레벨 | 79 | 다수 대상일 때 사용 |
| Wowhead | [47836](https://www.wowhead.com/wotlk/ko/spell=47836) | 부패의 씨앗 3 레벨 | 80 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [23161](https://www.wowhead.com/wotlk/ko/spell=23161) | 공포마 소환 | 40 | 대상 제어/시간 벌기 |
| Wowhead | [18647](https://www.wowhead.com/wotlk/ko/spell=18647) | 추방 2 레벨 | 48 | 대상 제어/시간 벌기 |
| Wowhead | [17928](https://www.wowhead.com/wotlk/ko/spell=17928) | 공포의 울부짖음 2 레벨 | 54 | 대상 제어/시간 벌기 |
| Wowhead | [6215](https://www.wowhead.com/wotlk/ko/spell=6215) | 공포 3 레벨 | 56 | 대상 제어/시간 벌기 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [47891](https://www.wowhead.com/wotlk/ko/spell=47891) | 암흑계 수호 6 레벨 | 78 | 체력 위험 또는 큰 피해 예측 |

### 마나/자원 회복

마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [57946](https://www.wowhead.com/wotlk/ko/spell=57946) | 생명력 전환 8 레벨 | 80 | 자원 부족 시 사용 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [54785](https://www.wowhead.com/wotlk/ko/spell=54785) | 악마의 돌진 악마 | 60 | 거리 조절/접근/이탈 |

### 소환/펫/하수인

전투에 참여하는 펫, 악마, 정령, 토템, 구울 등을 소환하거나 제어합니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [688](https://www.wowhead.com/wotlk/ko/spell=688) | 임프 소환 소환 | 1 | 전투 전 또는 펫 부재 시 |
| Wowhead | [697](https://www.wowhead.com/wotlk/ko/spell=697) | 보이드워커 소환 소환 | 10 | 전투 전 또는 펫 부재 시 |
| Wowhead | [712](https://www.wowhead.com/wotlk/ko/spell=712) | 서큐버스 소환 소환 | 20 | 전투 전 또는 펫 부재 시 |
| Wowhead | [713](https://www.wowhead.com/wotlk/ko/spell=713) | 인큐버스 소환 소환 | 20 | 전투 전 또는 펫 부재 시 |
| Wowhead | [691](https://www.wowhead.com/wotlk/ko/spell=691) | 지옥사냥개 소환 소환 | 30 | 전투 전 또는 펫 부재 시 |
| Wowhead | [48018](https://www.wowhead.com/wotlk/ko/spell=48018) | 악마의 마법진: 소환 | 80 | 전투 전 또는 펫 부재 시 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [59671](https://www.wowhead.com/wotlk/ko/spell=59671) | 도전의 울부짖음 악마 | 1 | 조건부 전투 유틸 |
| Wowhead | [5697](https://www.wowhead.com/wotlk/ko/spell=5697) | 영원의 숨결 | 16 | 조건부 전투 유틸 |
| Wowhead | [5500](https://www.wowhead.com/wotlk/ko/spell=5500) | 악마 감지 | 24 | 조건부 전투 유틸 |
| Wowhead | [1122](https://www.wowhead.com/wotlk/ko/spell=1122) | 불지옥 소환 | 50 | 조건부 전투 유틸 |
| Wowhead | [29858](https://www.wowhead.com/wotlk/ko/spell=29858) | 영혼 붕괴 | 66 | 조건부 전투 유틸 |
| Wowhead | [61191](https://www.wowhead.com/wotlk/ko/spell=61191) | 악마 예속 4 레벨 | 72 | 조건부 전투 유틸 |
| Wowhead | [47856](https://www.wowhead.com/wotlk/ko/spell=47856) | 생명력 집중 9 레벨 | 76 | 조건부 전투 유틸 |
| Wowhead | [47878](https://www.wowhead.com/wotlk/ko/spell=47878) | 생명석 창조 8 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [47838](https://www.wowhead.com/wotlk/ko/spell=47838) | 소각 4 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [61290](https://www.wowhead.com/wotlk/ko/spell=61290) | 암흑불길 2 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [47825](https://www.wowhead.com/wotlk/ko/spell=47825) | 영혼의 불꽃 6 레벨 | 80 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [698](https://www.wowhead.com/wotlk/ko/spell=698) | 소환 의식 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [5784](https://www.wowhead.com/wotlk/ko/spell=5784) | 지옥마 소환 소환 | 20 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [126](https://www.wowhead.com/wotlk/ko/spell=126) | 킬로그의 눈 소환 | 22 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [132](https://www.wowhead.com/wotlk/ko/spell=132) | 투명체 감지 | 26 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [18540](https://www.wowhead.com/wotlk/ko/spell=18540) | 파멸의 의식 | 60 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [47884](https://www.wowhead.com/wotlk/ko/spell=47884) | 영혼석 창조 7 레벨 | 76 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [47888](https://www.wowhead.com/wotlk/ko/spell=47888) | 주문석 창조 6 레벨 | 78 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [48020](https://www.wowhead.com/wotlk/ko/spell=48020) | 악마의 마법진: 순간이동 | 80 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [58887](https://www.wowhead.com/wotlk/ko/spell=58887) | 영혼의 의식 2 레벨 | 80 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [60220](https://www.wowhead.com/wotlk/ko/spell=60220) | 화염석 창조 7 레벨 | 80 | 시련 전투 AI 사용 제외 권장 |


## 드루이드 (Druid)

Wowhead 기준 URL: [druid](https://www.wowhead.com/wotlk/ko/spells/abilities/druid)

### 카테고리 요약

| 카테고리 | 스펠 수 |
|---|---:|
| 전투 전 버프/준비 | 3 |
| 태세/형상/오라/폼 전환 | 5 |
| 전투 중 공격기 - 단일 대상 | 10 |
| 전투 중 공격기 - 광역/다중 대상 | 4 |
| 전투 중 CC/메즈/이동 제한 | 4 |
| 해제/정화 | 2 |
| 생존/방어/피해 감소 | 2 |
| 치유/회복 | 5 |
| 부활 | 2 |
| 마나/자원 회복 | 1 |
| 이동/돌진/도주 | 2 |
| 어그로/도발/위협 제어 | 4 |
| 전투 유틸/특수 상황 | 21 |
| 전투 사용 제외 후보 | 2 |

### 전투 전 버프/준비

전투 시작 전에 미리 유지하면 좋은 강화 효과, 오라, 문장, 무기 강화, 보호막입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [53307](https://www.wowhead.com/wotlk/ko/spell=53307) | 가시 8 레벨 | 74 | 전투 전 유지 확인 |
| Wowhead | [48470](https://www.wowhead.com/wotlk/ko/spell=48470) | 야생의 선물 4 레벨 | 80 | 전투 전 유지 확인 |
| Wowhead | [48469](https://www.wowhead.com/wotlk/ko/spell=48469) | 야생의 징표 9 레벨 | 80 | 전투 전 유지 확인 |

### 태세/형상/오라/폼 전환

직업 운용 상태를 바꾸는 태세, 형상, 오라, 존재감 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [5487](https://www.wowhead.com/wotlk/ko/spell=5487) | 곰 변신 변신 | 10 | 필요 태세/형상일 때 사용 |
| Wowhead | [1066](https://www.wowhead.com/wotlk/ko/spell=1066) | 바다표범 변신 변신 | 16 | 필요 태세/형상일 때 사용 |
| Wowhead | [768](https://www.wowhead.com/wotlk/ko/spell=768) | 표범 변신 변신 | 20 | 필요 태세/형상일 때 사용 |
| Wowhead | [9634](https://www.wowhead.com/wotlk/ko/spell=9634) | 광포한 곰 변신 변신 | 40 | 필요 태세/형상일 때 사용 |
| Wowhead | [33891](https://www.wowhead.com/wotlk/ko/spell=33891) | 생명의 나무 변신 | 50 | 필요 태세/형상일 때 사용 |

### 전투 중 공격기 - 단일 대상

주 대상에게 사용하는 직접 피해, 도트, 마무리 일격, 주력 딜링 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [770](https://www.wowhead.com/wotlk/ko/spell=770) | 요정의 불꽃 | 18 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [16857](https://www.wowhead.com/wotlk/ko/spell=16857) | 요정의 불꽃 (야성) | 18 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [8983](https://www.wowhead.com/wotlk/ko/spell=8983) | 강타 3 레벨 | 46 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48574](https://www.wowhead.com/wotlk/ko/spell=48574) | 갈퀴 발톱 7 레벨 | 78 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48579](https://www.wowhead.com/wotlk/ko/spell=48579) | 찢어발기기 7 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48461](https://www.wowhead.com/wotlk/ko/spell=48461) | 천벌 12 레벨 | 79 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48463](https://www.wowhead.com/wotlk/ko/spell=48463) | 달빛 섬광 14 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [49800](https://www.wowhead.com/wotlk/ko/spell=49800) | 도려내기 9 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48564](https://www.wowhead.com/wotlk/ko/spell=48564) | 짓이기기 (곰) 5 레벨 | 80 | 단일 대상 기본 전투 로테이션 |
| Wowhead | [48566](https://www.wowhead.com/wotlk/ko/spell=48566) | 짓이기기 (표범) 5 레벨 | 80 | 단일 대상 기본 전투 로테이션 |

### 전투 중 공격기 - 광역/다중 대상

여러 적을 동시에 공격하거나 지역 피해를 주는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [33943](https://www.wowhead.com/wotlk/ko/spell=33943) | 폭풍까마귀 변신 변신 | 60 | 다수 대상일 때 사용 |
| Wowhead | [40120](https://www.wowhead.com/wotlk/ko/spell=40120) | 빠른 폭풍까마귀 변신 변신 | 70 | 다수 대상일 때 사용 |
| Wowhead | [62078](https://www.wowhead.com/wotlk/ko/spell=62078) | 휘둘러치기 (표범) 1 레벨 | 71 | 다수 대상일 때 사용 |
| Wowhead | [48562](https://www.wowhead.com/wotlk/ko/spell=48562) | 휘둘러치기 (곰) 8 레벨 | 77 | 다수 대상일 때 사용 |

### 전투 중 CC/메즈/이동 제한

기절, 공포, 변이, 속박, 침묵성 군중제어, 이동 방해입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [18658](https://www.wowhead.com/wotlk/ko/spell=18658) | 겨울잠 3 레벨 | 58 | 대상 제어/시간 벌기 |
| Wowhead | [33786](https://www.wowhead.com/wotlk/ko/spell=33786) | 회오리바람 | 70 | 대상 제어/시간 벌기 |
| Wowhead | [53308](https://www.wowhead.com/wotlk/ko/spell=53308) | 휘감는 뿌리 8 레벨 | 78 | 대상 제어/시간 벌기 |
| Wowhead | [48480](https://www.wowhead.com/wotlk/ko/spell=48480) | 후려치기 10 레벨 | 79 | 대상 제어/시간 벌기 |

### 해제/정화

아군의 해로운 효과를 지우거나 적의 이로운 효과를 제거하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [2782](https://www.wowhead.com/wotlk/ko/spell=2782) | 저주 해제 | 24 | 해제 가능한 효과가 있을 때 사용 |
| Wowhead | [2893](https://www.wowhead.com/wotlk/ko/spell=2893) | 독 해제 | 26 | 해제 가능한 효과가 있을 때 사용 |

### 생존/방어/피해 감소

자신 또는 아군이 죽지 않도록 쓰는 방어기, 면역기, 피해 감소기입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [22842](https://www.wowhead.com/wotlk/ko/spell=22842) | 광포한 재생력 | 36 | 체력 위험 또는 큰 피해 예측 |
| Wowhead | [22812](https://www.wowhead.com/wotlk/ko/spell=22812) | 나무 껍질 | 44 | 체력 위험 또는 큰 피해 예측 |

### 치유/회복

체력 회복, 보호막성 회복, 생명력 회복 계열입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48443](https://www.wowhead.com/wotlk/ko/spell=48443) | 재생 12 레벨 | 77 | 아군/자신 체력 회복 |
| Wowhead | [48378](https://www.wowhead.com/wotlk/ko/spell=48378) | 치유의 손길 15 레벨 | 79 | 아군/자신 체력 회복 |
| Wowhead | [50464](https://www.wowhead.com/wotlk/ko/spell=50464) | 육성 1 레벨 | 80 | 아군/자신 체력 회복 |
| Wowhead | [48451](https://www.wowhead.com/wotlk/ko/spell=48451) | 피어나는 생명 3 레벨 | 80 | 아군/자신 체력 회복 |
| Wowhead | [48441](https://www.wowhead.com/wotlk/ko/spell=48441) | 회복 15 레벨 | 80 | 아군/자신 체력 회복 |

### 부활

죽은 아군을 살리는 기술입니다. 전투 중 사용 가능 여부는 스펠별로 다릅니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [48477](https://www.wowhead.com/wotlk/ko/spell=48477) | 환생 7 레벨 | 79 | 사망 아군 복구 상황 |
| Wowhead | [50763](https://www.wowhead.com/wotlk/ko/spell=50763) | 되살리기 7 레벨 | 80 | 사망 아군 복구 상황 |

### 마나/자원 회복

마나, 분노, 기력, 룬 마력, 생명력 전환 등 자원을 확보하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [29166](https://www.wowhead.com/wotlk/ko/spell=29166) | 정신 자극 | 40 | 자원 부족 시 사용 |

### 이동/돌진/도주

돌진, 점멸, 도약, 전력 질주 같은 위치 제어 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [16979](https://www.wowhead.com/wotlk/ko/spell=16979) | 야성의 돌진 (곰) | 20 | 거리 조절/접근/이탈 |
| Wowhead | [49376](https://www.wowhead.com/wotlk/ko/spell=49376) | 야성의 돌진 (표범) | 20 | 거리 조절/접근/이탈 |

### 어그로/도발/위협 제어

대상에게 자신을 공격하게 하거나 위협 수준을 제어하는 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [6795](https://www.wowhead.com/wotlk/ko/spell=6795) | 포효 | 10 | 대상 고정/위협 제어 |
| Wowhead | [5209](https://www.wowhead.com/wotlk/ko/spell=5209) | 도전의 포효 | 28 | 대상 고정/위협 제어 |
| Wowhead | [52610](https://www.wowhead.com/wotlk/ko/spell=52610) | 야생의 포효 1 레벨 | 75 | 대상 고정/위협 제어 |
| Wowhead | [48560](https://www.wowhead.com/wotlk/ko/spell=48560) | 위협의 포효 8 레벨 | 77 | 대상 고정/위협 제어 |

### 전투 유틸/특수 상황

위 카테고리로 명확히 나누기 어렵지만 전투 판단에 쓸 수 있는 보조 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [16961](https://www.wowhead.com/wotlk/ko/spell=16961) | 야수의 분노 2 레벨 | 0 | 조건부 전투 유틸 |
| Wowhead | [5229](https://www.wowhead.com/wotlk/ko/spell=5229) | 분노 | 12 | 조건부 전투 유틸 |
| Wowhead | [8946](https://www.wowhead.com/wotlk/ko/spell=8946) | 해독 | 14 | 조건부 전투 유틸 |
| Wowhead | [783](https://www.wowhead.com/wotlk/ko/spell=783) | 치타 변신 변신 | 16 | 조건부 전투 유틸 |
| Wowhead | [5215](https://www.wowhead.com/wotlk/ko/spell=5215) | 숨기 | 20 | 조건부 전투 유틸 |
| Wowhead | [20719](https://www.wowhead.com/wotlk/ko/spell=20719) | 살쾡이의 우아함 지속효과 | 40 | 조건부 전투 유틸 |
| Wowhead | [62600](https://www.wowhead.com/wotlk/ko/spell=62600) | 야생의 방어 지속효과 | 40 | 조건부 전투 유틸 |
| Wowhead | [33357](https://www.wowhead.com/wotlk/ko/spell=33357) | 질주 3 레벨 | 65 | 조건부 전투 유틸 |
| Wowhead | [26995](https://www.wowhead.com/wotlk/ko/spell=26995) | 동물 달래기 4 레벨 | 70 | 조건부 전투 유틸 |
| Wowhead | [49802](https://www.wowhead.com/wotlk/ko/spell=49802) | 무력화 2 레벨 | 74 | 조건부 전투 유틸 |
| Wowhead | [48575](https://www.wowhead.com/wotlk/ko/spell=48575) | 웅크리기 6 레벨 | 76 | 조건부 전투 유틸 |
| Wowhead | [49803](https://www.wowhead.com/wotlk/ko/spell=49803) | 암습 5 레벨 | 77 | 조건부 전투 유틸 |
| Wowhead | [48465](https://www.wowhead.com/wotlk/ko/spell=48465) | 별빛 섬광 10 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [53312](https://www.wowhead.com/wotlk/ko/spell=53312) | 자연의 손아귀 8 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [48577](https://www.wowhead.com/wotlk/ko/spell=48577) | 흉포한 이빨 8 레벨 | 78 | 조건부 전투 유틸 |
| Wowhead | [50213](https://www.wowhead.com/wotlk/ko/spell=50213) | 맹공격 6 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [48570](https://www.wowhead.com/wotlk/ko/spell=48570) | 할퀴기 8 레벨 | 79 | 조건부 전투 유틸 |
| Wowhead | [48568](https://www.wowhead.com/wotlk/ko/spell=48568) | 가르기 3 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [48572](https://www.wowhead.com/wotlk/ko/spell=48572) | 칼날 발톱 9 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [48447](https://www.wowhead.com/wotlk/ko/spell=48447) | 평온 7 레벨 | 80 | 조건부 전투 유틸 |
| Wowhead | [48467](https://www.wowhead.com/wotlk/ko/spell=48467) | 허리케인 5 레벨 | 80 | 조건부 전투 유틸 |

### 전투 사용 제외 후보

순간이동, 포탈, 전문기술성 소환, 추적 등 시련 전투 로직에 넣지 않는 편이 안전한 기술입니다.

| 출처 | 주문 ID | 스펠 | 요구 레벨 | 사용 메모 |
|---|---:|---|---:|---|
| Wowhead | [18960](https://www.wowhead.com/wotlk/ko/spell=18960) | 순간이동: 달의 숲 | 10 | 시련 전투 AI 사용 제외 권장 |
| Wowhead | [5225](https://www.wowhead.com/wotlk/ko/spell=5225) | 인간형 추적 | 32 | 시련 전투 AI 사용 제외 권장 |
