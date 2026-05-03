# Karazhan Module Commands

이 문서는 현재 `modules/` 아래 사용 중인 모듈의 점 명령어를 정리한다.
애드온 내부 통신용 addon chat 명령은 사용자가 직접 입력하는 명령어가
아니므로 제외했다.

권한 표기:
- `Player`: 일반 플레이어
- `Moderator`: 운영자
- `GM`: 게임마스터
- `Admin`: 관리자
- `Console`: 월드서버 콘솔 입력 가능 여부

## 명령어가 있는 모듈

### mod-acore-subscriptions

소스: `modules/mod-acore-subscriptions/src/cs_acore_subscriptions.cpp`

| 명령어 | 권한 | Console | 설명 |
| --- | --- | --- | --- |
| `.subscriptions info [player]` | Player | Yes | 대상 또는 자기 자신의 구독 정보를 확인한다. 일반 플레이어는 자기 자신만 확인한다. |
| `.subscriptions update [player] <membershipLevel>` | Moderator | Yes | 온라인 대상의 구독 레벨을 임시 변경한다. 재접속 시 DB 값 기준으로 돌아간다. |

### mod-better-item-reloading

소스: `modules/mod-better-item-reloading/src/BetterItemReloading.cpp`

| 명령어 | 권한 | Console | 설명 |
| --- | --- | --- | --- |
| `.breload item <itemEntry...>` | Admin | Yes | `item_template`의 지정 아이템을 서버 메모리에 재로드한다. 여러 아이템 ID를 공백으로 입력할 수 있다. 게임 내 사용 시 장착 중인 해당 아이템은 제거 후 재장착을 시도한다. |

### mod-blackmarket

소스: `modules/mod-blackmarket/src/BlackMarketCommands.cpp`

| 명령어 | 권한 | Console | 설명 |
| --- | --- | --- | --- |
| `.blackmarket tele add <comment>` | GM | No | 현재 캐릭터 위치를 암상인 스폰 위치로 등록한다. `comment`는 위치 설명으로 저장된다. |
| `.blackmarket enable` | Admin | No | 암상인 시스템을 활성화한다. |
| `.blackmarket disable` | Admin | No | 암상인 시스템을 비활성화한다. 활성 암상인이 있으면 모듈 로직에 따라 정리된다. |
| `.blackmarket toggle` | Admin | No | 암상인 시스템 활성/비활성을 전환한다. |
| `.blackmarket status` | GM | No | 암상인 시스템 활성 여부, 현재 스폰 여부, 세션 ID를 출력한다. |
| `.blackmarket go` | GM | No | 현재 월드에 로드된 암상인 NPC 위치로 이동한다. NPC가 로드되어 있지 않으면 안내 메시지만 출력한다. |

### mod-solo-arena

소스: `modules/mod-solo-arena/src/SoloArena.cpp`

| 명령어 | 권한 | Console | 설명 |
| --- | --- | --- | --- |
| `.trial reload` | GM | Yes | 시련 단계/보상/기믹 데이터를 다시 불러오고, 활성 세션의 그림자 능력치를 재적용한다. |
| `.trial score show` | GM | No | 선택 대상 또는 자기 자신의 4/5/6단계 아라시 시련 점수를 표시한다. |
| `.trial score player <score>` | GM | No | 선택 대상 또는 자기 자신의 플레이어 측 아라시 시련 점수를 지정한다. |
| `.trial score shadow <score>` | GM | No | 선택 대상 또는 자기 자신의 그림자 측 아라시 시련 점수를 지정한다. |
| `.trial score set player <score>` | GM | No | `.trial score player <score>`와 동일한 점수 설정 명령이다. |
| `.trial score set shadow <score>` | GM | No | `.trial score shadow <score>`와 동일한 점수 설정 명령이다. |

### mod-transmog

소스: `modules/mod-transmog/src/cs_transmog.cpp`

| 명령어 | 권한 | Console | 설명 |
| --- | --- | --- | --- |
| `.transmog <hide>` | Player | No | 형상변환 외형 표시/숨김 설정을 변경한다. `hide`는 bool 인자로 처리된다. |
| `.transmog sync` | Player | No | 계정의 형상변환 수집 목록을 클라이언트로 다시 동기화한다. |
| `.transmog portable` | Player | No | 조건을 만족하면 휴대용 형상변환 NPC 주문을 시전한다. 구독/Plus 기능 설정에 영향을 받는다. |
| `.transmog interface <enable>` | Player | No | 형상변환 상인 인터페이스 설정을 변경한다. `enable`은 bool 인자로 처리된다. |
| `.transmog reload` | Admin | Yes | 형상변환 설정과 수집 데이터를 다시 불러온다. |
| `.transmog add [player] <item>` | Moderator | Yes | 대상 계정의 형상변환 수집 목록에 아이템 외형을 추가한다. 대상이 없으면 선택 대상 또는 자기 자신을 사용한다. |
| `.transmog add set [player] <itemSetId>` | Moderator | Yes | 대상 계정의 형상변환 수집 목록에 아이템 세트 외형을 추가한다. |

## 명령어가 없는 모듈

아래 모듈에서는 `CommandScript` 기반 점 명령어가 확인되지 않았다.

| 모듈 | 비고 |
| --- | --- |
| `mod-ale` | Lua 엔진/훅 제공 모듈. 별도 점 명령어 없음. |
| `mod-auto-levelup` | 별도 점 명령어 없음. |
| `mod-azeroth-flying` | 별도 점 명령어 없음. |
| `mod-custom-boss-volgrass` | 별도 점 명령어 없음. |
| `mod-custom-changes` | 별도 점 명령어 없음. |
| `mod-instance-bonus-mission` | 별도 점 명령어 없음. |
| `mod-item-karazhan` | 애드온 내부 명령은 있으나 직접 입력하는 점 명령어 없음. |
| `mod-learn-spells` | 별도 점 명령어 없음. |
| `mod-login-info-filter` | 별도 점 명령어 없음. |
| `mod-raid-reset-ticket` | 아이템 사용 스크립트 모듈. 별도 점 명령어 없음. |
| `mod-random-quest` | 별도 점 명령어 없음. |
| `mod-reward-played-time-improved` | 별도 점 명령어 없음. |

## 확인 기준

- `modules/` 하위 C++ 파일에서 `CommandScript` 구현을 기준으로 확인했다.
- `PlayerScript`에서 애드온 메시지를 처리하는 내부 명령은 일반 사용자가 직접 입력하는
  서버 점 명령어가 아니므로 표에서 제외했다.
