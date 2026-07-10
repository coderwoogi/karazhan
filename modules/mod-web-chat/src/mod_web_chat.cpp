/*
 * mod-web-chat  —  웹 ↔ 인게임 채팅 브리지 (AzerothCore 모듈)
 *
 *  수신(인게임 → 웹): PlayerScript::OnPlayerChat 훅으로 say/yell/whisper/guild/officer/party/raid/channel
 *                     발화를 acore_characters.web_ingame_chat 에 적재한다.
 *  송신(웹 → 인게임): WorldScript::OnUpdate 타이머로 web_outgoing_chat 의 pending 행을 폴링,
 *                     계정 대표 캐릭터 이름 + <GM> 마크로 해당 채널/타입에 주입 후 sent 로 표시한다.
 *
 *  대상 DB: CharacterDatabase (= acore_characters). 브리지 테이블은 웹(create_web_chat_bridge.sql)
 *           또는 웹 백엔드 ensureWebChatSchema() 가 생성한다.
 *
 *  대상 코어: 최신 AzerothCore (스크립트 훅 리팩터 이후 — PlayerScript 훅이 OnPlayerXxx 로 개명,
 *             세션 브로드캐스트가 WorldSessionMgr 로 이동된 버전).
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "World.h"
#include "WorldSession.h"
#include "WorldSessionMgr.h"
#include "ObjectAccessor.h"
#include "CharacterCache.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include <sstream>

namespace
{
    bool        g_enable        = true;
    uint32      g_pollIntervalMs = 3000;   // 송신 큐 폴링 주기(ms)
    uint32      g_maxPerTick     = 20;     // 1회 폴링 시 처리할 최대 건수
    std::string g_worldChannel   = "World"; // 'world' 타입을 주입할 글로벌 채널 이름

    std::string ChatTypeToString(uint32 type)
    {
        switch (type)
        {
            case CHAT_MSG_SAY:            return "say";
            case CHAT_MSG_YELL:           return "yell";
            case CHAT_MSG_WHISPER:        return "whisper";
            case CHAT_MSG_GUILD:          return "guild";
            case CHAT_MSG_OFFICER:        return "officer";
            case CHAT_MSG_PARTY:
            case CHAT_MSG_PARTY_LEADER:   return "party";
            case CHAT_MSG_RAID:
            case CHAT_MSG_RAID_LEADER:
            case CHAT_MSG_RAID_WARNING:   return "raid";
            case CHAT_MSG_CHANNEL:        return "channel";
            default:                      return "say";
        }
    }

    // web_ingame_chat 적재(이스케이프 후 INSERT)
    void StoreIncoming(std::string const& chatType, std::string channelName, Player* player,
                       std::string targetName, uint32 lang, std::string message)
    {
        if (!player)
            return;

        // 애드온 메시지(LANG_ADDON=0xFFFFFFFF)는 기계 페이로드 - 웹 채팅에 기록하지 않는다.
        // (language 컬럼 범위 초과로 MySQL 1264 오류를 유발하던 원인)
        if (lang == LANG_ADDON)
            return;

        std::string sender = player->GetName();
        uint32 guid = player->GetGUID().GetCounter();
        uint32 acc  = player->GetSession() ? player->GetSession()->GetAccountId() : 0;
        uint32 gm   = (player->GetSession() && player->GetSession()->GetSecurity() > SEC_PLAYER) ? 1 : 0;

        CharacterDatabase.EscapeString(sender);
        CharacterDatabase.EscapeString(channelName);
        CharacterDatabase.EscapeString(targetName);
        CharacterDatabase.EscapeString(message);

        std::ostringstream q;
        q << "INSERT INTO web_ingame_chat "
          << "(chat_type, channel_name, sender_guid, sender_name, sender_acc, sender_gm, target_name, language, message) VALUES ('"
          << chatType << "','" << channelName << "'," << guid << ",'" << sender << "'," << acc << ","
          << gm << ",'" << targetName << "'," << lang << ",'" << message << "')";
        CharacterDatabase.Execute(q.str().c_str());
    }
}

// ───────────────────────── 수신: 인게임 → 웹 ─────────────────────────
// 이 코어 버전엔 void OnPlayerChat(...) 훅이 없다. 채팅 타깃/채널 맥락이 있는
// OnPlayerCanUseChat(bool, 5종)을 읽기 훅으로 사용한다(기록 후 항상 true 반환 → 채팅 정상 진행).
class WebChat_PlayerScript : public PlayerScript
{
public:
    WebChat_PlayerScript() : PlayerScript("WebChat_PlayerScript", {
        PLAYERHOOK_CAN_PLAYER_USE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_PRIVATE_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GROUP_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_GUILD_CHAT,
        PLAYERHOOK_CAN_PLAYER_USE_CHANNEL_CHAT
    }) {}

    // say / yell / 등 (대상 없음)
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg) override
    {
        if (g_enable && !msg.empty())
            StoreIncoming(ChatTypeToString(type), "", player, "", lang, msg);
        return true;
    }

    // whisper (수신자 지정)
    bool OnPlayerCanUseChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Player* receiver) override
    {
        if (g_enable && !msg.empty())
        {
            std::string target = receiver ? receiver->GetName() : "";
            StoreIncoming("whisper", "", player, target, lang, msg);
        }
        return true;
    }

    // party / raid
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Group* /*group*/) override
    {
        if (g_enable && !msg.empty())
            StoreIncoming(ChatTypeToString(type), "", player, "", lang, msg);
        return true;
    }

    // guild / officer
    bool OnPlayerCanUseChat(Player* player, uint32 type, uint32 lang, std::string& msg, Guild* /*guild*/) override
    {
        if (g_enable && !msg.empty())
            StoreIncoming(ChatTypeToString(type), "", player, "", lang, msg);
        return true;
    }

    // channel (월드/일반 등 사용자 채널)
    bool OnPlayerCanUseChat(Player* player, uint32 /*type*/, uint32 lang, std::string& msg, Channel* channel) override
    {
        if (g_enable && !msg.empty())
        {
            std::string chName = channel ? channel->GetName() : "";
            StoreIncoming("channel", chName, player, "", lang, msg);
        }
        return true;
    }
};

// ───────────────────────── 송신: 웹 → 인게임 ─────────────────────────
class WebChat_WorldScript : public WorldScript
{
public:
    WebChat_WorldScript() : WorldScript("WebChat_WorldScript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        WORLDHOOK_ON_UPDATE
    }) {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        g_enable         = sConfigMgr->GetOption<bool>("WebChat.Enable", true);
        g_pollIntervalMs = sConfigMgr->GetOption<uint32>("WebChat.PollIntervalMs", 3000);
        g_maxPerTick     = sConfigMgr->GetOption<uint32>("WebChat.MaxPerTick", 20);
        g_worldChannel   = sConfigMgr->GetOption<std::string>("WebChat.WorldChannel", "World");
    }

    void OnUpdate(uint32 diff) override
    {
        if (!g_enable)
            return;

        _timer += diff;
        if (_timer < g_pollIntervalMs)
            return;
        _timer = 0;

        ProcessOutgoing();
        ProcessGoldOps();
    }

private:
    uint32 _timer = 0;

    void ProcessOutgoing()
    {
        std::ostringstream sel;
        sel << "SELECT id, chat_type, channel_name, target_name, sender_acc, sender_name, gm_mark, message "
            << "FROM web_outgoing_chat WHERE status='pending' ORDER BY id ASC LIMIT " << g_maxPerTick;

        QueryResult res = CharacterDatabase.Query(sel.str().c_str());
        if (!res)
            return;

        do
        {
            Field* f       = res->Fetch();
            uint64 id      = f[0].Get<uint64>();
            std::string ct = f[1].Get<std::string>();
            std::string ch = f[2].Get<std::string>();
            std::string tg = f[3].Get<std::string>();
            std::string sn = f[5].Get<std::string>();
            bool gmMark    = f[6].Get<uint8>() != 0;
            std::string msg = f[7].Get<std::string>();

            std::string err;
            bool ok = Deliver(ct, ch, tg, sn, gmMark, msg, err);

            std::string e = err;
            CharacterDatabase.EscapeString(e);
            std::ostringstream upd;
            upd << "UPDATE web_outgoing_chat SET status='" << (ok ? "sent" : "failed")
                << "', error='" << e << "', sent_at=NOW() WHERE id=" << id;
            CharacterDatabase.Execute(upd.str().c_str());
        } while (res->NextRow());
    }

    // ── 골드 변경 큐(web_gold_ops) 처리 ──────────────────────────────
    // 접속 중 캐릭터는 살아있는 Player 객체에 즉시 반영(서버 저장 덮어쓰기 문제 회피),
    // 오프라인은 characters.money 를 직접 갱신. mode: set|add|sub, 단위: copper.
    void ProcessGoldOps()
    {
        QueryResult res = CharacterDatabase.Query(
            "SELECT id, char_guid, mode, amount_copper FROM web_gold_ops WHERE status='pending' ORDER BY id ASC LIMIT 20");
        if (!res)
            return;

        do
        {
            Field* f       = res->Fetch();
            uint64 id      = f[0].Get<uint64>();
            uint32 cguid   = f[1].Get<uint32>();
            std::string md = f[2].Get<std::string>();
            uint64 amt     = f[3].Get<uint64>();

            uint64 result = 0;
            std::string err;
            bool ok = ApplyGoldOp(cguid, md, amt, result, err);

            CharacterDatabase.EscapeString(err);
            std::ostringstream upd;
            upd << "UPDATE web_gold_ops SET status='" << (ok ? "done" : "failed")
                << "', error='" << err << "', result_money=" << result
                << ", processed_at=NOW() WHERE id=" << id;
            CharacterDatabase.Execute(upd.str().c_str());
        } while (res->NextRow());
    }

    bool ApplyGoldOp(uint32 lowGuid, std::string const& mode, uint64 amtCopper, uint64& resultMoney, std::string& err)
    {
        auto computeNew = [&](uint64 cur) -> uint64
        {
            uint64 nv;
            if (mode == "set")      nv = amtCopper;
            else if (mode == "sub") nv = (cur > amtCopper) ? (cur - amtCopper) : 0;
            else                    nv = cur + amtCopper; // add
            if (nv > uint64(MAX_MONEY_AMOUNT))
                nv = uint64(MAX_MONEY_AMOUNT);
            return nv;
        };

        if (Player* p = ObjectAccessor::FindPlayerByLowGUID(lowGuid))
        {
            uint64 nv = computeNew(p->GetMoney());
            p->SetMoney(uint32(nv));
            resultMoney = nv;
            return true;
        }

        // 오프라인 → DB 직접 갱신
        std::ostringstream sel;
        sel << "SELECT money FROM characters WHERE guid=" << lowGuid;
        QueryResult r = CharacterDatabase.Query(sel.str().c_str());
        if (!r) { err = "character not found"; return false; }
        uint64 cur = r->Fetch()[0].Get<uint32>();
        uint64 nv = computeNew(cur);
        std::ostringstream upd;
        upd << "UPDATE characters SET money=" << uint32(nv) << " WHERE guid=" << lowGuid;
        CharacterDatabase.Execute(upd.str().c_str());
        resultMoney = nv;
        return true;
    }

    // 발신자(대표 캐릭터) GUID 조회 — 패킷의 발신자 식별용(오프라인이어도 이름은 패킷에 포함)
    ObjectGuid ResolveSenderGuid(std::string const& name)
    {
        if (name.empty())
            return ObjectGuid::Empty;
        return sCharacterCache->GetCharacterGuidByName(name);
    }

    // 실제 주입. 성공 시 true.
    bool Deliver(std::string const& ct, std::string const& channelName, std::string const& target,
                 std::string const& sender, bool gmMark, std::string const& message, std::string& err)
    {
        ObjectGuid senderGuid = ResolveSenderGuid(sender);
        uint8 tag = gmMark ? uint8(CHAT_TAG_GM) : uint8(CHAT_TAG_NONE);

        if (ct == "whisper")
        {
            Player* to = ObjectAccessor::FindPlayerByName(target);
            if (!to) { err = "수신자 오프라인"; return false; }
            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_UNIVERSAL, senderGuid,
                                         to->GetGUID(), message, tag, sender, to->GetName());
            to->SendDirectMessage(&data);
            return true;
        }

        if (ct == "guild" || ct == "officer")
        {
            Guild* guild = nullptr;
            if (senderGuid)
                if (uint32 gid = sCharacterCache->GetCharacterGuildIdByGuid(senderGuid))
                    guild = sGuildMgr->GetGuildById(gid);
            if (!guild) { err = "대표 캐릭터 길드 없음"; return false; }

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, ct == "officer" ? CHAT_MSG_OFFICER : CHAT_MSG_GUILD,
                                         LANG_UNIVERSAL, senderGuid, ObjectGuid::Empty, message, tag, sender);
            guild->BroadcastPacket(&data);
            return true;
        }

        if (ct == "channel" || ct == "world")
        {
            std::string chName = (ct == "world") ? g_worldChannel : channelName;
            if (chName.empty()) { err = "채널 미지정"; return false; }

            WorldPacket data;
            ChatHandler::BuildChatPacket(data, CHAT_MSG_CHANNEL, LANG_UNIVERSAL, senderGuid,
                                         ObjectGuid::Empty, message, tag, sender, "", 0, false, chName);
            // 전 세션 송출. 채널 멤버에게만 보내려면 ChannelMgr 로 채널을 찾아 분기할 것.
            sWorldSessionMgr->SendGlobalMessage(&data);
            return true;
        }

        // say / yell / party / raid: 발신 캐릭터가 접속 중이어야 위치/그룹 맥락이 성립
        Player* online = ObjectAccessor::FindPlayerByName(sender);
        if (!online) { err = "say/yell/파티/공대는 대표 캐릭터 접속 필요"; return false; }

        ChatMsg msgType = CHAT_MSG_SAY;
        if (ct == "yell") msgType = CHAT_MSG_YELL;
        else if (ct == "party") msgType = CHAT_MSG_PARTY;
        else if (ct == "raid") msgType = CHAT_MSG_RAID;

        WorldPacket data;
        ChatHandler::BuildChatPacket(data, msgType, LANG_UNIVERSAL, online->GetGUID(),
                                     ObjectGuid::Empty, message, tag, sender);
        if (ct == "say" || ct == "yell")
            online->SendMessageToSetInRange(&data, msgType == CHAT_MSG_YELL ? 300.0f : 25.0f, true);
        else if (Group* grp = online->GetGroup())
            grp->BroadcastPacket(&data, false);
        else { err = "파티/공대 없음"; return false; }
        return true;
    }
};

void Addmod_web_chatScripts()
{
    new WebChat_PlayerScript();
    new WebChat_WorldScript();
}
