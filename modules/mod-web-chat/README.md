# mod-web-chat — 웹 ↔ 인게임 채팅 브리지 (AzerothCore 모듈)

웹 관리자 콘솔의 "인게임 채팅" 화면과 게임 월드 채팅을 양방향으로 연결한다.

- **수신(인게임 → 웹)**: 플레이어 채팅(say/yell/whisper/guild/officer/party/raid/channel)을
  `acore_characters.web_ingame_chat` 에 적재 → 웹이 폴링해 표시.
- **송신(웹 → 인게임)**: 웹이 `web_outgoing_chat` 에 큐잉 → 모듈이 타이머로 폴링,
  **계정 대표 캐릭터 이름 + `<GM>` 마크**로 해당 채널/타입에 주입.

## 1. 사전 준비 — 브리지 테이블

웹 백엔드가 기동 시 자동 생성(`ensureWebChatSchema`)하지만, 수동 적용도 가능하다:

```
mysql -u root -p acore_characters < create_web_chat_bridge.sql
```

(두 테이블: `web_ingame_chat`, `web_outgoing_chat`)

## 2. 모듈 설치 / 빌드

```
cp -r mod-web-chat  /path/to/azerothcore/modules/
cd /path/to/azerothcore/build
cmake ..            # 새 모듈 인식
make -j$(nproc)
make install
```

- `conf/mod_web_chat.conf.dist` → 서버 `configs/modules/` 에 `mod_web_chat.conf` 로 복사 후 값 조정.
- worldserver 재시작.

## 3. 설정 (mod_web_chat.conf)

| 옵션 | 기본 | 설명 |
|---|---|---|
| `WebChat.Enable` | 1 | 브리지 사용 |
| `WebChat.PollIntervalMs` | 3000 | 송신 큐 폴링 주기(ms) |
| `WebChat.MaxPerTick` | 20 | 1회 처리 최대 건수 |
| `WebChat.WorldChannel` | "World" | 'world' 타입 주입 글로벌 채널명 |

## 4. 채널별 동작 / 제약

| 타입 | 수신 | 송신 | 비고 |
|---|---|---|---|
| guild/officer | ✅ | ✅ | 대표 캐릭터의 길드로 브로드캐스트(오프라인 OK) |
| whisper | ✅ | ✅ | 송신은 수신자 **접속 중**일 때 |
| channel/world | ✅ | ✅ | 전 세션 송출. 채널 멤버 한정하려면 `ChannelMgr` 분기 추가 |
| say/yell/party/raid | ✅ | △ | 송신은 대표 캐릭터가 **접속 중**이어야 위치/그룹 성립 |

## 5. 코어 버전 호환

최신 AzerothCore(스크립트 훅 리팩터 이후) 기준:
- PlayerScript 훅은 `OnPlayerChat` (구버전 `OnChat` 아님).
- 세션 브로드캐스트는 `sWorldSessionMgr->SendGlobalMessage` (구버전 `sWorld->SendGlobalMessage` 아님).

`ChatHandler::BuildChatPacket`, `sCharacterCache->GetCharacterGuidByName / GetCharacterGuildIdByGuid`,
`Guild::BroadcastPacket` 등은 코어 버전에 따라 시그니처가 다를 수 있으니 빌드 오류 시 해당 선언에 맞춰 조정.
