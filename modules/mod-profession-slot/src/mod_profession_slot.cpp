/*
 * mod-profession-slot
 *
 * Grants an extra PRIMARY profession slot when a configured item is used
 * ("장인의 인장", item entry 600035 by default).
 *
 * Design notes:
 *  - The "free primary profession points" field (PLAYER_CHARACTER_POINTS2) is NOT
 *    persistent: the core recomputes it on every login as
 *    (MaxPrimaryTradeSkill - learnedPrimaryProfessions), clamped at 0.
 *    So the extra slot must be stored separately (table `mod_profession_slot`)
 *    and re-applied on login by recomputing:
 *        free = max(0, (MaxPrimaryTradeSkill + bonus) - learnedPrimaryProfessions)
 *  - The item is bound through item_template.ScriptName = 'item_artisan_seal'
 *    (see data/sql/db-world). Spell 51150 on the item is a shared dummy used by
 *    many custom items, so we must NOT hook the spell; we hook the item instead.
 *  - Fully self-contained: no core or mod-ale edits, so upstream git updates
 *    never conflict.
 */

#include "Chat.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Item.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "World.h"

namespace
{
    bool IsFeatureEnabled()
    {
        return sConfigMgr->GetOption<bool>("ProfessionSlot.Enable", true);
    }

    uint8 GetMaxBonus()
    {
        return static_cast<uint8>(sConfigMgr->GetOption<uint32>("ProfessionSlot.MaxBonus", 1));
    }

    uint8 LoadBonus(ObjectGuid guid)
    {
        if (QueryResult result = CharacterDatabase.Query("SELECT `bonus` FROM `mod_profession_slot` WHERE `guid` = {}", guid.GetCounter()))
            return result->Fetch()[0].Get<uint8>();

        return 0;
    }

    void SaveBonus(ObjectGuid guid, uint8 bonus)
    {
        CharacterDatabase.Execute("INSERT INTO `mod_profession_slot` (`guid`, `bonus`) VALUES ({}, {}) ON DUPLICATE KEY UPDATE `bonus` = {}",
            guid.GetCounter(), uint32(bonus), uint32(bonus));
    }

    uint32 CountLearnedPrimaryProfessions(Player* player)
    {
        uint32 count = 0;
        for (auto const& [spellId, playerSpell] : player->GetSpellMap())
        {
            if (playerSpell->State == PLAYERSPELL_REMOVED)
                continue;

            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
                if (spellInfo->IsPrimaryProfessionFirstRank())
                    ++count;
        }

        return count;
    }

    // Recompute free primary profession points as (baseMax + bonus - learned), clamped at 0.
    // This is robust against the login recalc and the core unlearn cap.
    void ApplyFreeProfessions(Player* player, uint8 bonus)
    {
        uint32 const baseMax = sWorld->getIntConfig(CONFIG_MAX_PRIMARY_TRADE_SKILL);
        uint32 const learned = CountLearnedPrimaryProfessions(player);
        uint32 const total = baseMax + bonus;
        player->SetFreePrimaryProfessions(total > learned ? total - learned : 0);
    }
}

class ProfessionSlotPlayerScript : public PlayerScript
{
public:
    ProfessionSlotPlayerScript() : PlayerScript("ProfessionSlotPlayerScript", {
        PLAYERHOOK_ON_LOGIN
    }) { }

    void OnPlayerLogin(Player* player) override
    {
        if (!IsFeatureEnabled())
            return;

        uint8 bonus = LoadBonus(player->GetGUID());
        if (!bonus)
            return;

        // Defensive clamp in case the admin lowered ProfessionSlot.MaxBonus.
        uint8 const maxBonus = GetMaxBonus();
        if (bonus > maxBonus)
            bonus = maxBonus;

        ApplyFreeProfessions(player, bonus);
    }
};

class ProfessionSlotItemScript : public ItemScript
{
public:
    ProfessionSlotItemScript() : ItemScript("item_artisan_seal") { }

    bool OnUse(Player* player, Item* item, SpellCastTargets const& /*targets*/) override
    {
        if (!IsFeatureEnabled())
            return false;

        uint8 const maxBonus = GetMaxBonus();
        uint8 bonus = LoadBonus(player->GetGUID());

        if (bonus >= maxBonus)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("이미 최대 전문기술 슬롯에 도달했습니다.");
            // Abort: return true so the core does NOT cast the item's use-spell, so the item is
            // not consumed, and clear the client's pending "use" (gray) state.
            player->SendEquipError(EQUIP_ERR_CANT_DO_RIGHT_NOW, item, nullptr);
            return true;
        }

        ++bonus;
        SaveBonus(player->GetGUID(), bonus);

        // Immediate reflection: bump the free slots now so the player can learn a new profession
        // without relogging.
        player->SetFreePrimaryProfessions(player->GetFreePrimaryProfessionPoints() + 1);

        ChatHandler(player->GetSession()).PSendSysMessage("장인의 인장 효과로 전문기술을 하나 더 배울 수 있습니다. (추가 슬롯 {}/{})", uint32(bonus), uint32(maxBonus));

        // Return false so the core casts the item's use-spell and consumes the item.
        // Consumption requires item_template.spellcharges_1 = -1 (see data/sql/db-world).
        return false;
    }
};

void AddSC_mod_profession_slot()
{
    new ProfessionSlotPlayerScript();
    new ProfessionSlotItemScript();
}
