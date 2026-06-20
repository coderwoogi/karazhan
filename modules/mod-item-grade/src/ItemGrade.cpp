/*
 * mod-item-grade
 *
 * 아이템 등급(Grade) 스탯 스케일링 모듈.
 *
 * - 등급(S/A/B/C/D)은 "몬스터를 잡아 루팅한 아이템"에만 부여된다.
 *   (상점/제작/퀘스트/우편 등으로 얻은 아이템은 등급이 없다 = 스케일 안 함)
 * - 부여된 등급은 별도 테이블 `mod_item_grade(item_guid, grade)` 에 저장되고,
 *   메모리 맵으로 캐싱된다. 저장 방식이므로 GUID 가 바뀌어도(=karazhan 강화 등)
 *   등급을 명시적으로 옮길 수 있다.
 * - 장착 시 기본 옵션값(ItemStat[])에 등급 배수를 곱한다(장착/해제 대칭 보장).
 * - 인챈트(마부)는 별도 경로라 영향 없음. GUID 유지형 강화도 등급 보존.
 *
 * 기획서: doc/item_grade_scaling_plan_ko.md
 */

#include "Bag.h"
#include "Chat.h"
#include "CommandScript.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "StringFormat.h"
#include "TradeData.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <cctype>
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
    constexpr char const* ADDON_PREFIX = "IGRADE";

    enum ItemGradeId : uint8
    {
        GRADE_D = 0, // 기본(배수 1.0, ±0%)
        GRADE_C = 1, // +2%
        GRADE_B = 2, // +4%
        GRADE_A = 3, // +6%
        GRADE_S = 4, // +8%
        GRADE_MAX
    };

    struct ItemGradeConfig
    {
        bool   enabled      = true;
        bool   announceLoot = true;
        bool   addonPush     = true;
        uint32 syncIntervalMs = 1000;
        // 주기적 강제 재전송 간격(ms). 유실된 등급 푸시를 자동 복구. 0이면 비활성.
        uint32 forceResyncMs = 10000;
        // 인덱스 순서: D, C, B, A, S
        // [테스트용] 모든 등급 균등(각 20%). 운영 시 원하는 확률로 조정.
        uint32 chance[GRADE_MAX] = { 2000, 2000, 2000, 2000, 2000 };
        float  mult[GRADE_MAX]   = { 1.00f, 1.02f, 1.04f, 1.06f, 1.08f };

        void Load()
        {
            enabled      = sConfigMgr->GetOption<bool>("ItemGrade.Enable", true);
            announceLoot = sConfigMgr->GetOption<bool>("ItemGrade.AnnounceLoot", true);
            addonPush    = sConfigMgr->GetOption<bool>("ItemGrade.AddonPush", true);
            syncIntervalMs = sConfigMgr->GetOption<uint32>("ItemGrade.SyncIntervalMs", 1000);
            forceResyncMs = sConfigMgr->GetOption<uint32>("ItemGrade.ForceResyncIntervalMs", 10000);

            // [테스트용] 기본값 균등(각 20%). 운영 시 conf 또는 여기서 조정.
            chance[GRADE_D] = sConfigMgr->GetOption<uint32>("ItemGrade.Chance.D", 2000);
            chance[GRADE_C] = sConfigMgr->GetOption<uint32>("ItemGrade.Chance.C", 2000);
            chance[GRADE_B] = sConfigMgr->GetOption<uint32>("ItemGrade.Chance.B", 2000);
            chance[GRADE_A] = sConfigMgr->GetOption<uint32>("ItemGrade.Chance.A", 2000);
            chance[GRADE_S] = sConfigMgr->GetOption<uint32>("ItemGrade.Chance.S", 2000);

            mult[GRADE_D] = sConfigMgr->GetOption<float>("ItemGrade.Mult.D", 1.00f);
            mult[GRADE_C] = sConfigMgr->GetOption<float>("ItemGrade.Mult.C", 1.02f);
            mult[GRADE_B] = sConfigMgr->GetOption<float>("ItemGrade.Mult.B", 1.04f);
            mult[GRADE_A] = sConfigMgr->GetOption<float>("ItemGrade.Mult.A", 1.06f);
            mult[GRADE_S] = sConfigMgr->GetOption<float>("ItemGrade.Mult.S", 1.08f);

            _chanceSum = 0;
            for (uint8 i = 0; i < GRADE_MAX; ++i)
                _chanceSum += chance[i];

            if (_chanceSum == 0)
            {
                LOG_ERROR("module.itemgrade", "ItemGrade: 확률 합계가 0. 기본값 복구.");
                chance[GRADE_D] = 5000; chance[GRADE_C] = 3000; chance[GRADE_B] = 1300;
                chance[GRADE_A] = 600;  chance[GRADE_S] = 100;
                _chanceSum = 10000;
            }
        }

        uint32 ChanceSum() const { return _chanceSum; }

    private:
        uint32 _chanceSum = 10000;
    };

    ItemGradeConfig sCfg;

    // 부여된 등급 캐시: itemGuid(LowType) -> 등급. (B 포함, B 는 DB 저장만 생략)
    std::unordered_map<ObjectGuid::LowType, uint8> g_grades;

    // 플레이어별 마지막 전송 시그니처(레이아웃 변화 감지) + 동기화 타이머
    std::unordered_map<ObjectGuid::LowType, uint32> g_lastSig;
    std::unordered_map<ObjectGuid::LowType, uint32> g_updAccum;
    // 로그인 후 1회 강제 재전송용 카운트다운(ms). 클라 준비 전 푸시 누락 보정.
    std::unordered_map<ObjectGuid::LowType, uint32> g_loginRepush;
    // 주기적 강제 재전송 누적 타이머(자가 치유: 유실된 푸시 복구).
    std::unordered_map<ObjectGuid::LowType, uint32> g_forceAccum;

    uint8 RollGrade()
    {
        uint32 roll = urand(0, sCfg.ChanceSum() - 1);
        uint32 cumulative = 0;
        for (uint8 i = 0; i < GRADE_MAX; ++i)
        {
            cumulative += sCfg.chance[i];
            if (roll < cumulative)
                return i;
        }
        return GRADE_D; // 도달 불가(안전 폴백): 기본 등급
    }

    // -1: 등급 없음(스케일 안 함). 그 외: 0..4
    int GetStoredGrade(ObjectGuid::LowType guidLow)
    {
        auto it = g_grades.find(guidLow);
        if (it == g_grades.end())
            return -1;
        return int(it->second);
    }

    bool IsEligible(ItemTemplate const* proto)
    {
        if (!proto)
            return false;
        // 모든 장비(무기/방어구·장신구) — 품질/스탯 유무/세트 무관.
        // 잡템·소비·재료·퀘스트템은 대상 아님.
        return proto->Class == ITEM_CLASS_WEAPON || proto->Class == ITEM_CLASS_ARMOR;
    }

    char const* GradeLetter(uint8 grade)
    {
        switch (grade)
        {
            case GRADE_S: return "S";
            case GRADE_A: return "A";
            case GRADE_B: return "B";
            case GRADE_C: return "C";
            case GRADE_D: return "D";
            default:      return "B";
        }
    }

    char const* GradeColor(uint8 grade)
    {
        switch (grade)
        {
            case GRADE_S: return "ffff8000"; // 주황(전설)
            case GRADE_A: return "ffa335ee"; // 보라(영웅)
            case GRADE_B: return "ff0070dd"; // 파랑(희귀)
            case GRADE_C: return "ff1eff00"; // 초록(고급)
            case GRADE_D: return "ffffffff"; // 흰색(기본)
            default:      return "ffffffff";
        }
    }

    char const* GradeMark(uint8 grade)
    {
        return grade == GRADE_S ? "★ " : "";
    }

    // "S/A/B/C/D" 또는 "0~4" → 등급 인덱스. 실패 시 -1.
    int ParseGradeArg(std::string s)
    {
        size_t a = s.find_first_not_of(" \t");
        size_t b = s.find_last_not_of(" \t");
        if (a == std::string::npos)
            return -1;
        s = s.substr(a, b - a + 1);
        if (s.size() != 1)
            return -1;

        switch (char(std::toupper(static_cast<unsigned char>(s[0]))))
        {
            case 'D': case '0': return GRADE_D;
            case 'C': case '1': return GRADE_C;
            case 'B': case '2': return GRADE_B;
            case 'A': case '3': return GRADE_A;
            case 'S': case '4': return GRADE_S;
            default:            return -1;
        }
    }

    void StoreGrade(ObjectGuid::LowType guidLow, uint8 grade)
    {
        g_grades[guidLow] = grade;
        // 모든 등급을 저장(B 포함) → 재접속 후에도 모든 아이템에 등급 표시 유지
        CharacterDatabase.Execute(
            "REPLACE INTO mod_item_grade (item_guid, grade) VALUES ({}, {})",
            guidLow, uint32(grade));
    }

    // ----- 애드온 통신 (클라 좌표별 등급 푸시) -----

    void SendAddonRaw(Player* player, std::string const& payload)
    {
        if (!player || !player->GetSession())
            return;

        std::string full = std::string(ADDON_PREFIX) + "\t" + payload;

        WorldPacket data(SMSG_MESSAGECHAT, 100);
        data << uint8(CHAT_MSG_WHISPER);
        data << int32(LANG_ADDON);
        data << player->GetGUID();
        data << uint32(0);
        data << player->GetGUID();
        data << uint32(full.length() + 1);
        data << full;
        data << uint8(0);
        player->GetSession()->SendPacket(&data);
    }

    // 아이템 1개의 등급 엔트리("key=letter,pct"). 등급 없음/기본(B)이면 빈 문자열.
    std::string MakeEntry(std::string const& key, Item* item)
    {
        if (!item || !IsEligible(item->GetTemplate()))
            return "";
        int grade = GetStoredGrade(item->GetGUID().GetCounter());
        if (grade < 0) // 등급 없음(비-루팅/기존 아이템)만 제외. B 등급은 표시.
            return "";
        int32 pct = int32(std::lround(sCfg.mult[grade] * 100.0f));
        return Acore::StringFormat("{}={},{}", key, GradeLetter(uint8(grade)), pct);
    }

    // 플레이어의 모든 등급 엔트리를 클라 좌표 기준으로 수집
    void CollectEntries(Player* player, std::vector<std::string>& out)
    {
        for (uint8 s = EQUIPMENT_SLOT_START; s < EQUIPMENT_SLOT_END; ++s)
            if (std::string e = MakeEntry(Acore::StringFormat("e{}", s + 1),
                    player->GetItemByPos(INVENTORY_SLOT_BAG_0, s)); !e.empty())
                out.push_back(e);

        for (uint8 s = INVENTORY_SLOT_ITEM_START; s < INVENTORY_SLOT_ITEM_END; ++s)
            if (std::string e = MakeEntry(
                    Acore::StringFormat("b0:{}", s - INVENTORY_SLOT_ITEM_START + 1),
                    player->GetItemByPos(INVENTORY_SLOT_BAG_0, s)); !e.empty())
                out.push_back(e);

        for (uint8 bagSlot = INVENTORY_SLOT_BAG_START; bagSlot < INVENTORY_SLOT_BAG_END; ++bagSlot)
        {
            Bag* bag = player->GetBagByPos(bagSlot);
            if (!bag)
                continue;
            uint8 clientBag = bagSlot - INVENTORY_SLOT_BAG_START + 1;
            for (uint8 i = 0; i < bag->GetBagSize(); ++i)
                if (std::string e = MakeEntry(Acore::StringFormat("b{}:{}", clientBag, i + 1),
                        player->GetItemByPos(bagSlot, i)); !e.empty())
                    out.push_back(e);
        }
    }

    uint32 HashEntries(std::vector<std::string> const& entries)
    {
        uint32 h = 2166136261u; // FNV-1a
        for (std::string const& e : entries)
            for (char c : e)
            {
                h ^= uint8(c);
                h *= 16777619u;
            }
        return h;
    }

    void SendEntries(Player* player, std::vector<std::string> const& entries,
        char const* clrCmd, char const* addCmd, char const* endCmd)
    {
        SendAddonRaw(player, clrCmd);
        std::string addPrefix = std::string(addCmd) + "\n";
        std::string buffer;
        for (std::string const& e : entries)
        {
            if (!buffer.empty() && buffer.size() + e.size() + 1 > 230)
            {
                SendAddonRaw(player, addPrefix + buffer);
                buffer.clear();
            }
            if (!buffer.empty())
                buffer += ";";
            buffer += e;
        }
        if (!buffer.empty())
            SendAddonRaw(player, addPrefix + buffer);
        SendAddonRaw(player, endCmd);
    }

    // 살펴보기(Inspect): 인스펙터(viewer)에게 대상의 장착 장비 등급을 전송.
    void SendInspectGrades(Player* viewer, Player* target)
    {
        if (!sCfg.enabled || !sCfg.addonPush || !viewer || !target)
            return;

        std::vector<std::string> entries;
        for (uint8 s = EQUIPMENT_SLOT_START; s < EQUIPMENT_SLOT_END; ++s)
        {
            std::string e = MakeEntry(Acore::StringFormat("e{}", s + 1),
                target->GetItemByPos(INVENTORY_SLOT_BAG_0, s));
            if (!e.empty())
                entries.push_back(e);
        }
        SendEntries(viewer, entries, "ICLR", "IADD", "IEND");
    }

    // 레이아웃 시그니처가 바뀌었을 때(또는 force)만 클라에 재전송.
    // 아이템 이동/재배치로 슬롯 매핑이 어긋나는 문제를 주기적으로 보정한다.
    void RefreshIfChanged(Player* player, bool force)
    {
        if (!sCfg.enabled || !sCfg.addonPush || !player)
            return;

        std::vector<std::string> entries;
        CollectEntries(player, entries);
        uint32 sig = HashEntries(entries);

        ObjectGuid::LowType guidLow = player->GetGUID().GetCounter();
        auto it = g_lastSig.find(guidLow);
        if (!force && it != g_lastSig.end() && it->second == sig)
            return;

        g_lastSig[guidLow] = sig;
        SendEntries(player, entries, "CLR", "ADD", "END");
    }

    void AnnounceLoot(Player* player, Item* item, uint8 grade)
    {
        // 기본 등급(배수 1.0 = 현재 D)은 알림 생략(도배 방지). 보너스 등급만 알림.
        if (!sCfg.announceLoot || sCfg.mult[grade] == 1.0f)
            return;

        ItemTemplate const* proto = item->GetTemplate();
        std::string link = Acore::StringFormat(
            "|cffffffff|Hitem:{}:0:0:0:0:0:0:0:0:0:0|h[{}]|h|r",
            proto->ItemId, proto->Name1);

        std::string text = Acore::StringFormat(
            "{} |c{}{}[{}등급]|r 획득!",
            link, GradeColor(grade), GradeMark(grade), GradeLetter(grade));

        // 본인에게만 보이는 귓속말로 전송
        WorldPacket data;
        ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, LANG_UNIVERSAL, player, player, text);
        player->GetSession()->SendPacket(&data);
    }
}

// ---------------------------------------------------------------------------
// 외부 모듈 연동용 브리지 (mod-item-karazhan 등 아이템 재생성 시 등급 전달)
//   karazhan 에서 아래 선언을 그대로 두고 호출하면 링크 시점에 연결된다.
// ---------------------------------------------------------------------------
namespace ItemGradeBridge
{
    // 강화 등으로 oldGuid 아이템이 newGuid 새 아이템으로 교체될 때 등급을 옮긴다.
    void OnItemRecreated(ObjectGuid::LowType oldGuidLow, ObjectGuid::LowType newGuidLow)
    {
        int grade = GetStoredGrade(oldGuidLow);
        if (grade < 0)
            return; // 원본에 등급이 없으면 새 아이템도 등급 없음
        StoreGrade(newGuidLow, uint8(grade));
        // oldGuid 행은 원본 삭제 시 OnItemDelFromDB 에서 정리됨
    }
}

class ItemGradeWorldScript : public WorldScript
{
public:
    ItemGradeWorldScript() : WorldScript("ItemGradeWorldScript") { }

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        sCfg.Load();
    }

    void OnStartup() override
    {
        CharacterDatabase.DirectExecute(
            "CREATE TABLE IF NOT EXISTS `mod_item_grade` ("
            "`item_guid` INT UNSIGNED NOT NULL,"
            "`grade` TINYINT UNSIGNED NOT NULL DEFAULT 2,"
            "PRIMARY KEY (`item_guid`)"
            ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

        // 고아행 정리: 더 이상 존재하지 않는 아이템의 등급행 제거.
        // (캐릭터 삭제 등으로 item_instance 가 raw SQL 일괄 삭제되어
        //  OnItemDelFromDB 훅을 거치지 않은 경우까지 안전하게 회수)
        CharacterDatabase.DirectExecute(
            "DELETE g FROM mod_item_grade g "
            "LEFT JOIN item_instance i ON g.item_guid = i.guid "
            "WHERE i.guid IS NULL");

        g_grades.clear();
        if (QueryResult result = CharacterDatabase.Query("SELECT item_guid, grade FROM mod_item_grade"))
        {
            do
            {
                Field* f = result->Fetch();
                g_grades[f[0].Get<uint32>()] = f[1].Get<uint8>();
            } while (result->NextRow());
        }

        LOG_INFO("module.itemgrade", "ItemGrade: {} 개 등급 로드됨.", g_grades.size());
    }
};

class ItemGradePlayerScript : public PlayerScript
{
public:
    ItemGradePlayerScript() : PlayerScript("ItemGradePlayerScript", {
        PLAYERHOOK_ON_APPLY_ITEM_MODS_BEFORE,
        PLAYERHOOK_ON_APPLY_ENCHANTMENT_ITEM_MODS_BEFORE,
        PLAYERHOOK_ON_LOGIN,
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_EQUIP,
        PLAYERHOOK_ON_LOOT_ITEM,
        PLAYERHOOK_ON_UPDATE,
        PLAYERHOOK_ON_AFTER_MOVE_ITEM_FROM_INVENTORY,
        PLAYERHOOK_CAN_SET_TRADE_ITEM
    }) { }

    void OnPlayerApplyItemModsBefore(Player* player, uint8 slot, bool /*apply*/,
        uint8 /*statIndex*/, uint32 /*statType*/, int32& val) override
    {
        if (!sCfg.enabled || val == 0 || !player)
            return;

        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            return;

        int grade = GetStoredGrade(item->GetGUID().GetCounter());
        if (grade < 0) // 등급 없는 아이템(비-몬스터드랍 등)은 스케일 안 함
            return;

        float mult = sCfg.mult[grade];
        if (mult == 1.0f)
            return;

        int32 newVal = int32(std::lround(double(val) * double(mult)));
        if (val > 0 && newVal < 1)
            newVal = 1;
        val = newVal;
    }

    // 랜덤 속성/접미사("곰의~" 등) 옵션에 등급 배수 적용.
    // PROP 슬롯(7~)만 대상 — 일반 인챈트(마부)·보석은 건드리지 않는다.
    void OnPlayerApplyEnchantmentItemModsBefore(Player* /*player*/, Item* item, EnchantmentSlot slot,
        bool /*apply*/, uint32 /*enchant_spell_id*/, uint32& enchant_amount) override
    {
        if (!sCfg.enabled || enchant_amount == 0 || !item)
            return;
        if (slot < PROP_ENCHANTMENT_SLOT_0) // 마부/보석 등은 제외
            return;

        int grade = GetStoredGrade(item->GetGUID().GetCounter());
        if (grade < 0)
            return;

        float mult = sCfg.mult[grade];
        if (mult == 1.0f)
            return;

        uint32 newAmount = uint32(std::lround(double(enchant_amount) * double(mult)));
        if (newAmount < 1)
            newAmount = 1;
        enchant_amount = newAmount;
    }

    // 몬스터 루팅 시에만 등급 부여
    void OnPlayerLootItem(Player* player, Item* item, uint32 /*count*/, ObjectGuid lootguid) override
    {
        if (!sCfg.enabled || !player || !item)
            return;
        // 몬스터(크리처) + 상자/오브젝트(GameObject) + 컨테이너 아이템(보상가방/
        // 자물쇠 상자 등, IsItem) 루팅
        if (!lootguid.IsCreature() && !lootguid.IsGameObject() && !lootguid.IsItem())
            return;
        if (!IsEligible(item->GetTemplate()))
            return;

        ObjectGuid::LowType guidLow = item->GetGUID().GetCounter();
        if (g_grades.find(guidLow) != g_grades.end())
            return; // 이미 부여됨

        uint8 grade = RollGrade();
        StoreGrade(guidLow, grade);
        AnnounceLoot(player, item, grade);
        RefreshIfChanged(player, false);
    }

    void OnPlayerLogin(Player* player) override
    {
        RefreshIfChanged(player, true);
        // 클라 애드온이 완전히 준비된 뒤 1회 더 강제 전송(로그인 시점 누락 방지)
        g_loginRepush[player->GetGUID().GetCounter()] = 3000;
    }

    void OnPlayerLogout(Player* player) override
    {
        ObjectGuid::LowType guidLow = player->GetGUID().GetCounter();
        g_lastSig.erase(guidLow);
        g_updAccum.erase(guidLow);
        g_loginRepush.erase(guidLow);
        g_forceAccum.erase(guidLow);
    }

    void OnPlayerEquip(Player* player, Item* /*it*/, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        RefreshIfChanged(player, false);
    }

    void OnPlayerAfterMoveItemFromInventory(Player* player, Item* /*it*/, uint8 /*bag*/, uint8 /*slot*/, bool /*update*/) override
    {
        RefreshIfChanged(player, false);
    }

    // 거래: 내가 거래창에 올린 아이템의 등급을 거래 상대에게 전송.
    bool OnPlayerCanSetTradeItem(Player* player, Item* tradedItem, uint8 tradeSlot) override
    {
        if (sCfg.enabled && sCfg.addonPush && player)
        {
            TradeData* td = player->GetTradeData();
            Player* partner = td ? td->GetTrader() : nullptr;
            if (partner)
            {
                uint8 clientIdx = tradeSlot + 1; // 클라 거래슬롯(1-base)
                std::string entry = MakeEntry(Acore::StringFormat("{}", clientIdx), tradedItem);
                if (!entry.empty())
                    SendAddonRaw(partner, "TSET\n" + entry);
                else
                    SendAddonRaw(partner, Acore::StringFormat("TDEL\n{}", clientIdx));
            }
        }
        return true; // 항상 거래 허용
    }

    // 주기적 보정: 가방 내 드래그 등 별도 훅이 없는 변경까지 잡아낸다.
    void OnPlayerUpdate(Player* player, uint32 p_time) override
    {
        if (!sCfg.enabled || !sCfg.addonPush || !player)
            return;

        ObjectGuid::LowType guidLow = player->GetGUID().GetCounter();

        // 로그인 후 1회 강제 재전송(클라 준비 완료 대비)
        auto rp = g_loginRepush.find(guidLow);
        if (rp != g_loginRepush.end())
        {
            if (rp->second <= p_time)
            {
                g_loginRepush.erase(rp);
                RefreshIfChanged(player, true);
            }
            else
                rp->second -= p_time;
        }

        // 주기적 강제 재전송(자가 치유): 유실/누락된 푸시를 일정 간격으로 복구.
        // (시그니처가 같아도 강제로 다시 보내므로 "한 번 놓치면 영영 안 옴" 문제 해소)
        if (sCfg.forceResyncMs > 0)
        {
            uint32& facc = g_forceAccum[guidLow];
            facc += p_time;
            if (facc >= sCfg.forceResyncMs)
            {
                facc = 0;
                RefreshIfChanged(player, true);
                return;
            }
        }

        uint32& acc = g_updAccum[guidLow];
        acc += p_time;
        if (acc < sCfg.syncIntervalMs)
            return;
        acc = 0;
        RefreshIfChanged(player, false);
    }
};

class ItemGradeGlobalScript : public GlobalScript
{
public:
    ItemGradeGlobalScript() : GlobalScript("ItemGradeGlobalScript") { }

    void OnItemDelFromDB(CharacterDatabaseTransaction /*trans*/, ObjectGuid::LowType itemGuid) override
    {
        g_grades.erase(itemGuid);
        CharacterDatabase.Execute("DELETE FROM mod_item_grade WHERE item_guid = {}", itemGuid);
    }
};

// 살펴보기(Inspect): CMSG_INSPECT 를 가로채 인스펙터에게 대상 장비 등급 전송.
class ItemGradeServerScript : public ServerScript
{
public:
    ItemGradeServerScript() : ServerScript("ItemGradeServerScript", { SERVERHOOK_CAN_PACKET_RECEIVE }) { }

    bool CanPacketReceive(WorldSession* session, WorldPacket const& packet) override
    {
        if (sCfg.enabled && sCfg.addonPush && session && packet.GetOpcode() == CMSG_INSPECT)
        {
            if (Player* viewer = session->GetPlayer())
            {
                WorldPacket copy(packet); // const& 라 복사 후 읽기
                ObjectGuid guid;
                copy >> guid;
                if (Player* target = ObjectAccessor::GetPlayer(*viewer, guid))
                    SendInspectGrades(viewer, target);
            }
        }
        return true; // 정상 처리 계속
    }
};

using namespace Acore::ChatCommands;

class ItemGradeCommandScript : public CommandScript
{
public:
    ItemGradeCommandScript() : CommandScript("ItemGradeCommandScript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable itemGradeTable =
        {
            { "add",  HandleAddCommand,  SEC_GAMEMASTER, Console::No },
            { "info", HandleInfoCommand, SEC_GAMEMASTER, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "itemgrade", itemGradeTable },
        };
        return commandTable;
    }

    // .itemgrade add <itemId|아이템링크> <grade> : 지정 등급으로 아이템 생성(본인 지급)
    //   아이템은 숫자 ID 또는 쉬프트-클릭 링크 모두 허용.
    static bool HandleAddCommand(ChatHandler* handler, ItemTemplate const* itemTemplate, Optional<std::string> gradeArg)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return false;

        if (!itemTemplate)
        {
            handler->PSendSysMessage("[ItemGrade] 사용법: .itemgrade add <아이템ID 또는 링크> <S/A/B/C/D>");
            handler->SetSentErrorMessage(true);
            return false;
        }

        int grade = gradeArg ? ParseGradeArg(*gradeArg) : -1;
        if (grade < 0)
        {
            handler->PSendSysMessage("[ItemGrade] 등급은 S/A/B/C/D(또는 0~4). 예: .itemgrade add 49623 S");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 itemId = itemTemplate->ItemId;
        ItemTemplate const* proto = itemTemplate;

        ItemPosCountVec dest;
        InventoryResult res = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, 1);
        if (res != EQUIP_ERR_OK)
        {
            player->SendEquipError(res, nullptr, nullptr, itemId);
            return true;
        }

        int32 randProp = Item::GenerateItemRandomPropertyId(itemId);
        Item* item = player->StoreNewItem(dest, itemId, true, randProp);
        if (!item)
        {
            handler->PSendSysMessage("[ItemGrade] 아이템 생성 실패.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        StoreGrade(item->GetGUID().GetCounter(), uint8(grade));
        player->SendNewItem(item, 1, false, true);

        handler->PSendSysMessage("[ItemGrade] [{}] 을(를) |c{}{}{}등급|r (x{:.2f}) 으로 생성했습니다.",
            proto->Name1, GradeColor(uint8(grade)), GradeMark(uint8(grade)),
            GradeLetter(uint8(grade)), sCfg.mult[grade]);
        return true;
    }

    // .itemgrade info : 장착 장비의 등급/배수와 스탯 적용값을 출력(검증용)
    static bool HandleInfoCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession() ? handler->GetSession()->GetPlayer() : nullptr;
        if (!player)
            return false;

        handler->PSendSysMessage("[ItemGrade] 장착 장비 등급/스탯(기본 -> 적용):");
        bool any = false;
        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (!item || !IsEligible(item->GetTemplate()))
                continue;

            any = true;
            ItemTemplate const* proto = item->GetTemplate();
            int grade = GetStoredGrade(item->GetGUID().GetCounter());
            if (grade < 0)
            {
                handler->PSendSysMessage("  슬롯 {}: [{}] - 등급 없음", slot, proto->Name1);
                continue;
            }

            float mult = sCfg.mult[grade];
            handler->PSendSysMessage("  슬롯 {}: [{}] = {}등급 (x{:.2f})",
                slot, proto->Name1, GradeLetter(uint8(grade)), mult);

            for (uint32 i = 0; i < proto->StatsCount && i < MAX_ITEM_PROTO_STATS; ++i)
            {
                int32 base = proto->ItemStat[i].ItemStatValue;
                if (base == 0)
                    continue;
                int32 scaled = int32(std::lround(double(base) * double(mult)));
                if (base > 0 && scaled < 1)
                    scaled = 1;
                handler->PSendSysMessage("      스탯타입 {}: {} -> {}",
                    proto->ItemStat[i].ItemStatType, base, scaled);
            }
        }
        if (!any)
            handler->PSendSysMessage("  (대상 장비가 없습니다)");
        return true;
    }
};

void AddItemGradeScripts()
{
    new ItemGradeWorldScript();
    new ItemGradePlayerScript();
    new ItemGradeGlobalScript();
    new ItemGradeServerScript();
    new ItemGradeCommandScript();
}
