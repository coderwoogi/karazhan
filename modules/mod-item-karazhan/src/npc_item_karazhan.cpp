/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "ScriptedGossip.h"
#include "Item.h"
#include "ItemKarazhan.h"
#include "Chat.h"
#include "DatabaseEnv.h"
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include "StringFormat.h"

namespace
{
    constexpr uint8 ENHANCE_TYPE_NONE = 0;
    constexpr uint8 ENHANCE_TYPE_MELEE = 1;
    constexpr uint8 ENHANCE_TYPE_CASTER = 2;
    constexpr uint8 ENHANCE_TYPE_HEALER = 3;
    constexpr uint8 ENHANCE_TYPE_TANK = 4;
    
}

enum GossipActions
{
    ACTION_MAIN_MENU = 1,
    ACTION_INFO = 2,
    ACTION_SHOW_ITEMS = 3,
    ACTION_GOODBYE = 4,

    ACTION_ITEM_SELECT = 100,
    ACTION_ENHANCE_CONFIRM = 1000,
    ACTION_ENHANCE_DO = 2000
};

static uint32 EncodeTypedAction(uint32 baseAction, uint8 slot,
    uint8 enhanceType)
{
    return baseAction + (slot * 10) +
        static_cast<uint32>(enhanceType);
}

static bool DecodeTypedAction(uint32 action, uint32 baseAction,
    uint8& slot,
    uint8& enhanceType)
{
    if (action < baseAction || action >= (baseAction + 1000))
        return false;

    uint32 value = action - baseAction;
    slot = value / 10;

    uint32 typeValue = value % 10;
    if (typeValue != ENHANCE_TYPE_MELEE &&
        typeValue != ENHANCE_TYPE_CASTER &&
        typeValue != ENHANCE_TYPE_HEALER &&
        typeValue != ENHANCE_TYPE_TANK)
        return false;

    enhanceType = static_cast<uint8>(typeValue);
    return true;
}

static void AddEnhanceConfigPreview(Player* player, uint32 action,
    KarazhanEnchantConfig const* config)
{
    if (!config)
        return;

    auto addMaterialPreview = [&](uint32 itemId, uint32 count)
    {
        if (itemId == 0 || count == 0)
            return;

        std::string materialName = sItemKarazhanMgr->GetItemNameLocale(
            itemId, player);
        std::ostringstream materialMsg;
        materialMsg << "  재료: |Hitem:" << itemId
                    << ":0:0:0:0:0:0:0|h[" << materialName
                    << "]|h x" << count;
        AddGossipItemFor(player, GOSSIP_ICON_DOT, materialMsg.str(),
            GOSSIP_SENDER_MAIN, action);
    };

    std::ostringstream costMsg;
    costMsg << "  비용: " << config->goldCost << " 골드";
    AddGossipItemFor(player, GOSSIP_ICON_DOT, costMsg.str(),
        GOSSIP_SENDER_MAIN, action);

    addMaterialPreview(config->material1, config->material1Count);
    addMaterialPreview(config->material2, config->material2Count);
    addMaterialPreview(config->material3, config->material3Count);

    std::ostringstream successMsg;
    successMsg << "  성공: " << std::fixed << std::setprecision(1)
               << config->successRate << "%";
    AddGossipItemFor(player, GOSSIP_ICON_DOT, successMsg.str(),
        GOSSIP_SENDER_MAIN, action);
}

class npc_item_karazhan : public CreatureScript
{
public:
    npc_item_karazhan() : CreatureScript("npc_item_karazhan") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "아이템 강화 시스템에 대해 알려주세요", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "장착 중인 장비를 보여주세요", GOSSIP_SENDER_MAIN, ACTION_SHOW_ITEMS);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "안녕히 계세요", GOSSIP_SENDER_MAIN, ACTION_GOODBYE);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);

        if (action == ACTION_MAIN_MENU)
        {
            return OnGossipHello(player, creature);
        }

        if (action == ACTION_INFO)
        {
            ShowEnhancementInfo(player, creature);
            return true;
        }

        if (action == ACTION_SHOW_ITEMS)
        {
            ShowEquipmentList(player, creature);
            return true;
        }

        if (action == ACTION_GOODBYE)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (action >= ACTION_ITEM_SELECT && action < ACTION_ENHANCE_CONFIRM)
        {
            uint8 slot = action - ACTION_ITEM_SELECT;
            ShowTypeSelection(player, creature, slot);
            return true;
        }

        uint8 slot = 0;
        uint8 enhanceType = ENHANCE_TYPE_NONE;
        if (DecodeTypedAction(action, ACTION_ENHANCE_CONFIRM, slot,
            enhanceType))
        {
            ShowEnhanceConfirm(player, creature, slot, enhanceType);
            return true;
        }

        if (DecodeTypedAction(action, ACTION_ENHANCE_DO, slot, enhanceType))
        {
            DoEnhancement(player, creature, slot, enhanceType);
            return true;
        }

        return false;
    }

private:
    void ShowEnhancementInfo(Player* player, Creature* creature)
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "=== 카라잔 강화 시스템 ===", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "장착 중인 장비를 강화할 수 있습니다", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "무기는 최대 +100 강화", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "방어구는 최대 +50 강화", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "장신구는 최대 +30 강화", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "강화 단계가 올라갈수록 성공 확률이 낮아집니다", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "일부 단계에서는 실패 시 아이템이 파괴될 수 있습니다", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, "강화 비용은 단계가 올라갈수록 증가합니다", GOSSIP_SENDER_MAIN, ACTION_INFO);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 뒤로 가기", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowEquipmentList(Player* player, Creature* creature)
    {
        const char* slotNames[EQUIPMENT_SLOT_END] = {
            "머리", "목", "어깨", "셔츠", "가슴",
            "허리", "다리", "발", "손목", "손",
            "반지1", "반지2", "장신구1", "장신구2",
            "등", "주무기", "보조무기", "원거리", "휘장"
        };

        const char* qualityColors[8] = {
            "|cff9d9d9d", "|cffffffff", "|cff1eff00", "|cff0070dd",
            "|cFFA335EE", "|cffff8000", "|cffe6cc80", "|cffe6cc80"
        };

        bool hasItems = false;

        // Preload enhancement data for all equipped items
        std::unordered_map<uint32, uint8> enhanceLevels;

        QueryResult result = CharacterDatabase.Query(
            "SELECT item_guid, enhance_level FROM karazhan_item_enhance WHERE owner_guid = {}",
            player->GetGUID().GetCounter()
        );

        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 itemGuid = fields[0].Get<uint32>();
                uint8 enhanceLevel = fields[1].Get<uint8>();

                enhanceLevels[itemGuid] = enhanceLevel;

            } while (result->NextRow());

            LOG_INFO("module", "Karazhan: Loaded {} enhanced items for player {}",
                enhanceLevels.size(), player->GetName());
        }

        for (uint8 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
        {
            Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i);
            if (!item)
                continue;

            ItemTemplate const* proto = item->GetTemplate();
            if (!proto)
                continue;

            // Check whether this slot can be enhanced
            if (!sItemKarazhanMgr->CanEnhanceSlot(proto->InventoryType))
                continue;

            uint32 itemGuid = item->GetGUID().GetCounter();

            // Read preloaded enhancement level from memory
            uint8 currentLevel = 0;
            auto itr = enhanceLevels.find(itemGuid);
            if (itr != enhanceLevels.end())
            {
                currentLevel = itr->second;
            }

            LOG_INFO("module", "Karazhan: Item GUID {}, Entry {}, Current Level: {}",
                itemGuid, item->GetEntry(), currentLevel);

            // Check max enhancement level
            uint8 maxLevel = sItemKarazhanMgr->GetMaxEnhanceLevel(proto->InventoryType);

            // Skip items that already reached max level
            if (currentLevel >= maxLevel)
            {
                LOG_INFO("module", "Karazhan: Item GUID {} already at max level {}", itemGuid, maxLevel);
                continue;
            }

            hasItems = true;

            // Resolve localized item name
            std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);

            std::ostringstream itemInfo;
            itemInfo << "[" << slotNames[i] << "] ";

            if (proto->Quality < 8)
                itemInfo << qualityColors[proto->Quality];

            itemInfo << itemName;
            itemInfo << "|r";

            // Show current enhancement level loaded from DB
            if (currentLevel > 0)
            {
                itemInfo << " |cffffcc00+" << uint32(currentLevel) << "|r";
            }

            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, itemInfo.str(), GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + i);

            LOG_INFO("module", "Karazhan: Added to list - Slot {}, Item: {}, Level: +{}",
                i, itemName, currentLevel);
        }

        if (!hasItems)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "강화 가능한 아이템이 없습니다", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
            AddGossipItemFor(player, GOSSIP_ICON_DOT, "(장비를 착용했는지, 최대 강화 단계에 도달하지 않았는지 확인해주세요)", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
        }

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 뒤로 가기", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowTypeSelection(Player* player, Creature* creature, uint8 slot)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
        {
            OnGossipHello(player, creature);
            return;
        }

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
        {
            OnGossipHello(player, creature);
            return;
        }

        uint32 itemGuid = item->GetGUID().GetCounter();

        // ??DB?먯꽌 吏곸젒 議고쉶
        QueryResult result = CharacterDatabase.Query(
            "SELECT enhance_level FROM karazhan_item_enhance WHERE item_guid = {}", itemGuid
        );

        uint8 enhanceLevel = result ? result->Fetch()[0].Get<uint8>() : 0;

        LOG_INFO("module", "Karazhan: ShowTypeSelection - GUID: {}, Level from DB: {}", itemGuid, enhanceLevel);

        uint8 maxLevel = sItemKarazhanMgr->GetMaxEnhanceLevel(proto->InventoryType);
        uint8 currentType = sItemKarazhanMgr->GetItemEnhanceType(item);
        bool lockedType = enhanceLevel > 0 && currentType != ENHANCE_TYPE_NONE;

        std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);

        std::ostringstream titleMsg;
        titleMsg << "=== 타입 선택 ===";
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, titleMsg.str(),
            GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        std::ostringstream itemMsg;
        itemMsg << "아이템: " << itemName;
        if (enhanceLevel > 0)
            itemMsg << " +" << uint32(enhanceLevel);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, itemMsg.str(),
            GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        std::ostringstream enhanceLevelMsg;
        enhanceLevelMsg << "현재 강화: +" << uint32(enhanceLevel) << " / 최대 +" << uint32(maxLevel);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, enhanceLevelMsg.str(),
            GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        if (lockedType)
        {
            std::ostringstream lockMsg;
            lockMsg << "현재 유형: "
                    << sItemKarazhanMgr->GetEnhanceTypeName(currentType)
                    << " (고정)";
            AddGossipItemFor(player, GOSSIP_ICON_DOT, lockMsg.str(),
                GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "------------------------------",
            GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        auto addTypeOption = [&](uint8 type, char const* label)
        {
            uint32 confirmAction = EncodeTypedAction(ACTION_ENHANCE_CONFIRM,
                slot, type);
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, label,
                GOSSIP_SENDER_MAIN, confirmAction);
        };

        if (lockedType)
        {
            switch (currentType)
            {
                case ENHANCE_TYPE_MELEE:
                    addTypeOption(ENHANCE_TYPE_MELEE, "[밀리]");
                    break;
                case ENHANCE_TYPE_CASTER:
                    addTypeOption(ENHANCE_TYPE_CASTER, "[캐스터]");
                    break;
                case ENHANCE_TYPE_HEALER:
                    addTypeOption(ENHANCE_TYPE_HEALER, "[힐러]");
                    break;
                case ENHANCE_TYPE_TANK:
                    addTypeOption(ENHANCE_TYPE_TANK, "[탱커]");
                    break;
                default:
                    break;
            }
        }
        else
        {
            addTypeOption(ENHANCE_TYPE_MELEE, "[밀리]");
            addTypeOption(ENHANCE_TYPE_CASTER, "[캐스터]");
            addTypeOption(ENHANCE_TYPE_HEALER, "[힐러]");
            addTypeOption(ENHANCE_TYPE_TANK, "[탱커]");
        }

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 뒤로 가기", GOSSIP_SENDER_MAIN, ACTION_SHOW_ITEMS);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowEnhanceConfirm(Player* player, Creature* creature, uint8 slot,
        uint8 enhanceType)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
        {
            OnGossipHello(player, creature);
            return;
        }

        ClearGossipMenuFor(player);

        ItemTemplate const* proto = item->GetTemplate();
        uint32 itemGuid = item->GetGUID().GetCounter();

        // ??DB?먯꽌 吏곸젒 議고쉶
        QueryResult result = CharacterDatabase.Query(
            "SELECT enhance_level FROM karazhan_item_enhance WHERE item_guid = {}", itemGuid
        );

        uint8 currentLevel = result ? result->Fetch()[0].Get<uint8>() : 0;
        uint8 targetLevel = currentLevel + 1;
        uint8 maxLevel = sItemKarazhanMgr->GetMaxEnhanceLevel(proto->InventoryType);
        uint8 currentType = sItemKarazhanMgr->GetItemEnhanceType(item);
        bool lockedType = currentLevel > 0 &&
            currentType != ENHANCE_TYPE_NONE;

        if (lockedType && currentType != enhanceType)
        {
            ShowTypeSelection(player, creature, slot);
            return;
        }

        LOG_INFO("module", "Karazhan: ShowEnhanceConfirm - GUID: {}, Current: {}, Target: {}",
            itemGuid, currentLevel, targetLevel);

        uint32 sameAction = EncodeTypedAction(ACTION_ENHANCE_CONFIRM, slot,
            enhanceType);
        uint32 doAction = EncodeTypedAction(ACTION_ENHANCE_DO, slot,
            enhanceType);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "=== 강화 확인 ===",
            GOSSIP_SENDER_MAIN, sameAction);

        // Resolve localized item name
        std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);

        std::ostringstream itemMsg;
        itemMsg << "아이템: " << itemName;
        if (currentLevel > 0)
        {
            itemMsg << " +" << uint32(currentLevel);
        }
        AddGossipItemFor(player, GOSSIP_ICON_DOT, itemMsg.str(),
            GOSSIP_SENDER_MAIN, sameAction);

        std::ostringstream levelMsg;
        levelMsg << "다음 강화: +" << uint32(currentLevel) << " -> +" << uint32(targetLevel) << " (최대: +" << uint32(maxLevel) << ")";
        AddGossipItemFor(player, GOSSIP_ICON_DOT, levelMsg.str(),
            GOSSIP_SENDER_MAIN, sameAction);

        std::ostringstream typeMsg;
        typeMsg << "선택 유형: "
                << sItemKarazhanMgr->GetEnhanceTypeName(enhanceType);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, typeMsg.str(),
            GOSSIP_SENDER_MAIN, sameAction);

        if (lockedType)
        {
            std::ostringstream lockMsg;
            lockMsg << "이 아이템은 "
                    << sItemKarazhanMgr->GetEnhanceTypeName(currentType)
                    << " 계열로 고정됩니다";
            AddGossipItemFor(player, GOSSIP_ICON_DOT, lockMsg.str(),
                GOSSIP_SENDER_MAIN, sameAction);
        }

        KarazhanEnchantConfig const* selectedConfig =
            sItemKarazhanMgr->GetEnchantConfig(targetLevel, enhanceType);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "------------------------------",
            GOSSIP_SENDER_MAIN, sameAction);
        if (selectedConfig)
        {
            AddEnhanceConfigPreview(player, sameAction, selectedConfig);
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "[강화 진행]",
                GOSSIP_SENDER_MAIN, doAction);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "해당 강화 설정을 찾을 수 없습니다",
                GOSSIP_SENDER_MAIN, sameAction);
        }
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 타입 다시 선택",
            GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void DoEnhancement(Player* player, Creature* creature, uint8 slot,
        uint8 enhanceType = ENHANCE_TYPE_NONE)
    {
        Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "아이템을 찾을 수 없습니다!", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "확인", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return;
        }

        sItemKarazhanMgr->RequestEnhancement(player, item, enhanceType);

        CloseGossipMenuFor(player);
    }
};

void Add_SC_npc_item_karazhan()
{
    new npc_item_karazhan();
}
