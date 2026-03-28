# HeroStoneUI

`현자석.lua`가 사용하는 클라이언트 애드온이다.

## 클라이언트 설치

아래 폴더를 그대로 클라이언트 `Interface/AddOns` 아래에 복사한다.

- `HeroStoneUI`

즉 최종 경로는 다음과 같다.

- `WoW/Interface/AddOns/HeroStoneUI/HeroStoneUI.toc`
- `WoW/Interface/AddOns/HeroStoneUI/HeroStoneUI.lua`
- `WoW/Interface/AddOns/HeroStoneUI/Art/...`

## 서버 반영

운영 서버 Lua 파일:

- `E:/server/operate/lua_scripts/현자석.lua`

저장소 기준본:

- `E:/server/azerothcore-wotlk/tools/server_lua/현자석.lua`

## 구조

- 서버 Lua가 `HERO_STONE_UI` 애드온 메시지를 전송한다.
- 클라이언트 애드온이 UI를 렌더한다.
- 버튼 클릭 시 애드온이 다시 `HERO_STONE_UI`로 응답한다.
