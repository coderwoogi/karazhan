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
    ACTION_ENHANCE_CONFIRM = 200,
    ACTION_ENHANCE_DO = 300,
    ACTION_ENHANCE_TYPE = 400
};

static uint32 EncodeEnhanceTypeAction(uint8 slot, uint8 enhanceType)
{
    return ACTION_ENHANCE_TYPE + (slot * 10) +
        static_cast<uint32>(enhanceType);
}

static bool DecodeEnhanceTypeAction(uint32 action, uint8& slot,
    uint8& enhanceType)
{
    if (action < ACTION_ENHANCE_TYPE)
        return false;

    uint32 value = action - ACTION_ENHANCE_TYPE;
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
            ShowItemDetails(player, creature, slot);
            return true;
        }

        if (action >= ACTION_ENHANCE_CONFIRM && action < ACTION_ENHANCE_DO)
        {
            uint8 slot = action - ACTION_ENHANCE_CONFIRM;
            ShowEnhanceConfirm(player, creature, slot);
            return true;
        }

        if (action >= ACTION_ENHANCE_DO && action < ACTION_ENHANCE_TYPE)
        {
            uint8 slot = action - ACTION_ENHANCE_DO;
            DoEnhancement(player, creature, slot);
            return true;
        }

        uint8 slot = 0;
        uint8 enhanceType = ENHANCE_TYPE_NONE;
        if (DecodeEnhanceTypeAction(action, slot, enhanceType))
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

    void ShowItemDetails(Player* player, Creature* creature, uint8 slot)
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

        LOG_INFO("module", "Karazhan: ShowItemDetails - GUID: {}, Level from DB: {}", itemGuid, enhanceLevel);

        uint8 maxLevel = sItemKarazhanMgr->GetMaxEnhanceLevel(proto->InventoryType);

        // Resolve localized item name
        std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);

        std::ostringstream titleMsg;
        titleMsg << "=== " << itemName;
        if (enhanceLevel > 0)
        {
            titleMsg << " +" << uint32(enhanceLevel);
        }
        titleMsg << " ===";
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, titleMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        const char* qualityNames[] = {
            "불량", "일반", "고급", "희귀", "영웅", "전설", "유물", "계승품"
        };

        if (proto->Quality < 8)
        {
            std::ostringstream qualityMsg;
            qualityMsg << "품질: " << qualityNames[proto->Quality];
            AddGossipItemFor(player, GOSSIP_ICON_DOT, qualityMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);
        }

        std::ostringstream itemLevelMsg;
        itemLevelMsg << "아이템 레벨: " << proto->ItemLevel;
        AddGossipItemFor(player, GOSSIP_ICON_DOT, itemLevelMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        std::ostringstream enhanceLevelMsg;
        enhanceLevelMsg << "현재 강화: +" << uint32(enhanceLevel) << " / 최대 +" << uint32(maxLevel);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, enhanceLevelMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

        if (enhanceLevel < maxLevel)
        {
            AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "[이 아이템을 강화합니다]", GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "이미 최대 강화 단계입니다", GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);
        }

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 뒤로 가기", GOSSIP_SENDER_MAIN, ACTION_SHOW_ITEMS);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }

    void ShowEnhanceConfirm(Player* player, Creature* creature, uint8 slot)
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

        LOG_INFO("module", "Karazhan: ShowEnhanceConfirm - GUID: {}, Current: {}, Target: {}",
            itemGuid, currentLevel, targetLevel);

        KarazhanEnchantConfig const* config = sItemKarazhanMgr->GetEnchantConfig(targetLevel);
        if (!config)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "강화 설정을 찾을 수 없습니다!", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
            AddGossipItemFor(player, GOSSIP_ICON_TALK, "확인", GOSSIP_SENDER_MAIN, ACTION_MAIN_MENU);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return;
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "=== 강화 확인 ===", GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        // Resolve localized item name
        std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);

        std::ostringstream itemMsg;
        itemMsg << "아이템: " << itemName;
        if (currentLevel > 0)
        {
            itemMsg << " +" << uint32(currentLevel);
        }
        AddGossipItemFor(player, GOSSIP_ICON_DOT, itemMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        std::ostringstream levelMsg;
        levelMsg << "다음 강화: +" << uint32(currentLevel) << " -> +" << uint32(targetLevel) << " (최대: +" << uint32(maxLevel) << ")";
        AddGossipItemFor(player, GOSSIP_ICON_DOT, levelMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        std::ostringstream typeMsg;
        typeMsg << "현재 유형: "
                << sItemKarazhanMgr->GetEnhanceTypeName(currentType);
        AddGossipItemFor(player, GOSSIP_ICON_DOT, typeMsg.str(),
            GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        // goldCost is stored in gold units
        std::ostringstream costMsg;
        costMsg << "강화 비용: ";
        
        if (config->goldCost > 0)
        {
            costMsg << config->goldCost << " 골드";
        }
        else
        {
            costMsg << "무료";
        }

        AddGossipItemFor(player, GOSSIP_ICON_DOT, costMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        // ???щ즺 ?쒖떆 (?쒓?紐?
        if (config->material1 > 0 && config->material1Count > 0)
        {
            std::string materialName = sItemKarazhanMgr->GetItemNameLocale(config->material1, player);
            std::ostringstream materialMsg;
            materialMsg << "필요 재료 1: " << materialName << " x" << config->material1Count;
            AddGossipItemFor(player, GOSSIP_ICON_DOT, materialMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }

        if (config->material2 > 0 && config->material2Count > 0)
        {
            std::string materialName = sItemKarazhanMgr->GetItemNameLocale(config->material2, player);
            std::ostringstream materialMsg;
            materialMsg << "필요 재료 2: " << materialName << " x" << config->material2Count;
            AddGossipItemFor(player, GOSSIP_ICON_DOT, materialMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }

        if (config->material3 > 0 && config->material3Count > 0)
        {
            std::string materialName = sItemKarazhanMgr->GetItemNameLocale(config->material3, player);
            std::ostringstream materialMsg;
            materialMsg << "필요 재료 3: " << materialName << " x" << config->material3Count;
            AddGossipItemFor(player, GOSSIP_ICON_DOT, materialMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }

        std::ostringstream successMsg;
        successMsg << "성공 확률: " << std::fixed << std::setprecision(1) << config->successRate << "%";
        AddGossipItemFor(player, GOSSIP_ICON_DOT, successMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        float destroyRate = 100.0f - config->successRate - config->failRate;
        if (destroyRate > 0)
        {
            std::ostringstream destroyMsg;
            destroyMsg << "파괴 확률: " << std::fixed << std::setprecision(1) << destroyRate << "%";
            AddGossipItemFor(player, GOSSIP_ICON_DOT, destroyMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }

        if (config->failRate > 0)
        {
            std::ostringstream failMsg;
            failMsg << "실패 (단계 유지): " << std::fixed << std::setprecision(1) << config->failRate << "%";
            AddGossipItemFor(player, GOSSIP_ICON_DOT, failMsg.str(), GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
            "------------------------------",
            GOSSIP_SENDER_MAIN, ACTION_ENHANCE_CONFIRM + slot);

        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "[밀리 선택]",
            GOSSIP_SENDER_MAIN,
            EncodeEnhanceTypeAction(slot, ENHANCE_TYPE_MELEE));
        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "[캐스터 선택]",
            GOSSIP_SENDER_MAIN,
            EncodeEnhanceTypeAction(slot, ENHANCE_TYPE_CASTER));
        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "[힐러 선택]",
            GOSSIP_SENDER_MAIN,
            EncodeEnhanceTypeAction(slot, ENHANCE_TYPE_HEALER));
        AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "[탱커 선택]",
            GOSSIP_SENDER_MAIN,
            EncodeEnhanceTypeAction(slot, ENHANCE_TYPE_TANK));
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "<- 취소", GOSSIP_SENDER_MAIN, ACTION_ITEM_SELECT + slot);

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

        std::string itemName = sItemKarazhanMgr->GetItemNameLocale(item->GetEntry(), player);
        uint8 currentLevel = sItemKarazhanMgr->GetItemEnhanceLevel(item->GetGUID().GetCounter());

        sItemKarazhanMgr->RequestEnhancement(player, item, enhanceType);

        CloseGossipMenuFor(player);

        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cff00ff00==============================================|r");
            
            // StringFormat ??
            std::string msg = Acore::StringFormat(
                "|cffffcc00[카라잔 강화]|r {} +{} ({}) 강화를 시작합니다...",
                itemName, currentLevel,
                sItemKarazhanMgr->GetEnhanceTypeName(enhanceType)
            );
            handler.PSendSysMessage(msg.c_str());
            
            handler.PSendSysMessage("|cff00ff00잠시 후 결과가 표시됩니다.|r");
            handler.PSendSysMessage("|cff00ff00==============================================|r");
        }
    }
};

void Add_SC_npc_item_karazhan()
{
    new npc_item_karazhan();
}
