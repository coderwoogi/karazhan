/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#include "ItemKarazhan.h"
#include "Item.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Log.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "Chat.h"
#include "WorldSession.h"
#include "World.h"
#include "DatabaseEnv.h"
#include "StringFormat.h"

namespace
{
    constexpr uint8 ENHANCE_TYPE_NONE = 0;
    constexpr uint8 ENHANCE_TYPE_MELEE = 1;
    constexpr uint8 ENHANCE_TYPE_CASTER = 2;
    constexpr uint8 ENHANCE_TYPE_HEALER = 3;
    constexpr uint8 ENHANCE_TYPE_TANK = 4;
    constexpr uint8 ENHANCE_TYPE_ALL_STATS = 5;
}

ItemKarazhanMgr::ItemKarazhanMgr()
{
    LOG_INFO("module", "Karazhan: Item enhancement system initialized");
}

ItemKarazhanMgr* ItemKarazhanMgr::instance()
{
    static ItemKarazhanMgr instance;
    return &instance;
}

void ItemKarazhanMgr::Initialize()
{
    LOG_INFO("module", "Karazhan: Loading enhancement configurations...");

    LoadSlotConfigs();
    LoadEnchantConfigs();

    LOG_INFO("module", "Karazhan: Loaded {} slot configs", _slotConfigs.size());
    LOG_INFO("module", "Karazhan: Loaded {} enchant configs", _enchantConfigs.size());

    for (auto const& [slotId, config] : _slotConfigs)
    {
        if (config.canEnhance)
        {
            LOG_INFO("module", "Karazhan: Slot {} ({}) - Max Level: {}",
                slotId, config.slotNameKo, config.maxEnhanceLevel);
        }
    }

    LOG_INFO("module", "Karazhan: Enhancement system ready");
}

void ItemKarazhanMgr::Update(uint32 /*diff*/)
{
    // ?????ш린 ?쒗븳 (硫붾え由???컻 諛⑹?)
    if (_pendingQueue.size() > 1000)
    {
        LOG_ERROR("module", "Karazhan: Queue overflow! Size: {}, clearing old requests", _pendingQueue.size());
        
        // ???ㅻ옒???붿껌 ?뺣━
        while (_pendingQueue.size() > 100)
        {
            _pendingQueue.pop();
        }
    }

    if (_pendingQueue.empty())
        return;

    // Process up to 10 queued jobs per update
    uint32 processedCount = 0;
    const uint32 MAX_PROCESS_PER_UPDATE = 10;

    while (!_pendingQueue.empty() && processedCount < MAX_PROCESS_PER_UPDATE)
    {
        PendingEnhancement pending = _pendingQueue.front();
        _pendingQueue.pop();

        try
        {
            ProcessPendingEnhancement(pending);
            processedCount++;
        }
        catch (std::exception const& e)
        {
            LOG_ERROR("module", "Karazhan: Exception in ProcessPendingEnhancement - {}", e.what());
        }
        catch (...)
        {
            LOG_ERROR("module", "Karazhan: Unknown exception in ProcessPendingEnhancement!");
        }
    }

    if (processedCount > 0)
    {
        LOG_DEBUG("module", "Karazhan: Processed {} enhancements, {} remaining in queue",
                  processedCount, _pendingQueue.size());
    }
}

void ItemKarazhanMgr::LoadSlotConfigs()
{
    _slotConfigs.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT slot_id, slot_name, slot_name_ko, can_enhance, max_enhance_level, enabled, comment "
        "FROM karazhan_enchant_slots"
    );

    if (!result)
    {
        LOG_ERROR("module", "Karazhan: karazhan_enchant_slots table is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        KarazhanSlotConfig config;
        config.slotId = fields[0].Get<uint8>();
        config.slotName = fields[1].Get<std::string>();
        config.slotNameKo = fields[2].Get<std::string>();
        config.canEnhance = fields[3].Get<bool>();
        config.maxEnhanceLevel = fields[4].Get<uint8>();
        config.enabled = fields[5].Get<bool>();
        config.comment = fields[6].Get<std::string>();

        _slotConfigs[config.slotId] = config;
        count++;

    } while (result->NextRow());

    LOG_INFO("module", "Karazhan: Loaded {} slot configurations", count);
}

void ItemKarazhanMgr::LoadEnchantConfigs()
{
    _enchantConfigs.clear();

    QueryResult result = WorldDatabase.Query(
        "SELECT enchant_level, spell_id, random_property_id, success_rate, fail_rate, "
        "gold_cost, material_1, material_1_count, material_2, material_2_count, "
        "material_3, material_3_count FROM karazhan_enchant_config"
    );

    if (!result)
    {
        LOG_ERROR("module", "Karazhan: karazhan_enchant_config table is empty!");
        return;
    }

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();

        KarazhanEnchantConfig config;
        config.enchantLevel = fields[0].Get<uint8>();
        config.spellId = fields[1].Get<uint32>();
        config.randomPropertyId = fields[2].Get<int32>();
        config.successRate = fields[3].Get<float>();
        config.failRate = fields[4].Get<float>();
        config.goldCost = fields[5].Get<uint32>(); // Stored in gold units in DB
        config.material1 = fields[6].Get<uint32>();
        config.material1Count = fields[7].Get<uint32>();
        config.material2 = fields[8].Get<uint32>();
        config.material2Count = fields[9].Get<uint32>();
        config.material3 = fields[10].Get<uint32>();
        config.material3Count = fields[11].Get<uint32>();

        _enchantConfigs[config.enchantLevel] = config;
        count++;

    } while (result->NextRow());

    LOG_INFO("module", "Karazhan: Loaded {} enchant level configurations", count);
}

bool ItemKarazhanMgr::CanEnhanceSlot(uint8 inventoryType) const
{
    auto itr = _slotConfigs.find(inventoryType);
    if (itr == _slotConfigs.end())
        return false;

    return itr->second.canEnhance && itr->second.enabled;
}

uint8 ItemKarazhanMgr::GetMaxEnhanceLevel(uint8 inventoryType) const
{
    auto itr = _slotConfigs.find(inventoryType);
    if (itr == _slotConfigs.end())
        return 0;

    return itr->second.maxEnhanceLevel;
}

bool ItemKarazhanMgr::CanEnhance(Item const* item, Player* player) const
{
    if (!item || !player)
        return false;

    ItemTemplate const* proto = item->GetTemplate();
    if (!proto)
        return false;

    if (!CanEnhanceSlot(proto->InventoryType))
        return false;

    uint8 currentLevel = GetItemEnhanceLevel(item->GetGUID().GetCounter());
    uint8 maxLevel = GetMaxEnhanceLevel(proto->InventoryType);

    return currentLevel < maxLevel;
}

uint8 ItemKarazhanMgr::GetItemEnhanceLevel(uint32 itemGuid) const
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT enhance_level FROM karazhan_item_enhance WHERE item_guid = {}", itemGuid
    );

    if (!result)
        return 0;

    return result->Fetch()[0].Get<uint8>();
}

void ItemKarazhanMgr::SetItemEnhanceLevel(uint32 itemGuid, uint32 ownerGuid, uint32 itemEntry, uint8 level)
{
    CharacterDatabase.Execute(
        "INSERT INTO karazhan_item_enhance "
        "(item_guid, owner_guid, item_entry, enhance_level, total_success, total_fail, total_gold_spent) "
        "VALUES ({}, {}, {}, {}, 0, 0, 0) "
        "ON DUPLICATE KEY UPDATE "
        "enhance_level = {}, owner_guid = {}, item_entry = {}",
        itemGuid, ownerGuid, itemEntry, level,
        level, ownerGuid, itemEntry
    );
}

void ItemKarazhanMgr::DeleteItemEnhance(uint32 itemGuid)
{
    CharacterDatabase.Execute("DELETE FROM karazhan_item_enhance WHERE item_guid = {}", itemGuid);
    _itemEnhances.erase(itemGuid);
}

void ItemKarazhanMgr::LoadItemEnhance(uint32 itemGuid)
{
    QueryResult result = CharacterDatabase.Query(
        "SELECT item_guid, owner_guid, item_entry, enhance_level, total_success, total_fail, total_gold_spent "
        "FROM karazhan_item_enhance WHERE item_guid = {}", itemGuid
    );

    if (!result)
    {
        _itemEnhances.erase(itemGuid);
        return;
    }

    Field* fields = result->Fetch();

    KarazhanItemEnhance enhance;
    enhance.itemGuid = fields[0].Get<uint32>();
    enhance.ownerGuid = fields[1].Get<uint32>();
    enhance.itemEntry = fields[2].Get<uint32>();
    enhance.enhanceLevel = fields[3].Get<uint8>();
    enhance.totalSuccess = fields[4].Get<uint32>();
    enhance.totalFail = fields[5].Get<uint32>();
    enhance.totalGoldSpent = fields[6].Get<uint64>();

    _itemEnhances[itemGuid] = enhance;
}

KarazhanEnchantConfig const* ItemKarazhanMgr::GetEnchantConfig(uint8 level) const
{
    auto itr = _enchantConfigs.find(level);
    return (itr != _enchantConfigs.end()) ? &itr->second : nullptr;
}

KarazhanSlotConfig const* ItemKarazhanMgr::GetSlotConfig(uint8 slotId) const
{
    auto itr = _slotConfigs.find(slotId);
    return (itr != _slotConfigs.end()) ? &itr->second : nullptr;
}

bool ItemKarazhanMgr::CheckAndConsumeMaterials(Player* player, KarazhanEnchantConfig const* config)
{
    if (!player || !config)
        return false;

    // Convert DB gold value to copper
    uint32 goldInCopper = config->goldCost * 10000;

    // ========================================
    // 1. Check gold
    // ========================================
    if (player->GetMoney() < goldInCopper)
    {
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cffff0000==============================================|r");
            handler.PSendSysMessage("|cffff0000[재료 부족]|r");
            
            std::string msg = Acore::StringFormat(
                "골드가 부족합니다! (필요: |cffffcc00{} 골드|r)",
                config->goldCost
            );
            handler.PSendSysMessage(msg.c_str());
            
            handler.PSendSysMessage("|cffff0000==============================================|r");
        }
        
        LOG_WARN("module", "Karazhan: Player {} lacks gold - Need: {}, Has: {}",
                 player->GetName(), config->goldCost, player->GetMoney() / 10000);
        
        return false;
    }

    // ========================================
    // 2. Check material 1
    // ========================================
    if (config->material1 > 0 && config->material1Count > 0)
    {
        if (!player->HasItemCount(config->material1, config->material1Count))
        {
            std::string materialName = GetItemNameLocale(config->material1, player);
            
            if (WorldSession* session = player->GetSession())
            {
                ChatHandler handler(session);
                handler.PSendSysMessage("|cffff0000==============================================|r");
                handler.PSendSysMessage("|cffff0000[재료 부족]|r");
                
                std::string msg = Acore::StringFormat(
                    "재료가 부족합니다! (필요: |cffffcc00{} x {}|r)",
                    materialName, config->material1Count
                );
                handler.PSendSysMessage(msg.c_str());
                
                handler.PSendSysMessage("|cffff0000==============================================|r");
            }
            
            LOG_WARN("module", "Karazhan: Player {} lacks material - Item: {}, Need: {}",
                     player->GetName(), config->material1, config->material1Count);
            
            return false;
        }
    }

    // ========================================
    // 3. Check material 2
    // ========================================
    if (config->material2 > 0 && config->material2Count > 0)
    {
        if (!player->HasItemCount(config->material2, config->material2Count))
        {
            std::string materialName = GetItemNameLocale(config->material2, player);
            
            if (WorldSession* session = player->GetSession())
            {
                ChatHandler handler(session);
                handler.PSendSysMessage("|cffff0000==============================================|r");
                handler.PSendSysMessage("|cffff0000[재료 부족]|r");
                
                std::string msg = Acore::StringFormat(
                    "재료가 부족합니다! (필요: |cffffcc00{} x {}|r)",
                    materialName, config->material2Count
                );
                handler.PSendSysMessage(msg.c_str());
                
                handler.PSendSysMessage("|cffff0000==============================================|r");
            }
            
            LOG_WARN("module", "Karazhan: Player {} lacks material - Item: {}, Need: {}",
                     player->GetName(), config->material2, config->material2Count);
            
            return false;
        }
    }

    // ========================================
    // 4. Check material 3
    // ========================================
    if (config->material3 > 0 && config->material3Count > 0)
    {
        if (!player->HasItemCount(config->material3, config->material3Count))
        {
            std::string materialName = GetItemNameLocale(config->material3, player);
            
            if (WorldSession* session = player->GetSession())
            {
                ChatHandler handler(session);
                handler.PSendSysMessage("|cffff0000==============================================|r");
                handler.PSendSysMessage("|cffff0000[재료 부족]|r");
                
                std::string msg = Acore::StringFormat(
                    "재료가 부족합니다! (필요: |cffffcc00{} x {}|r)",
                    materialName, config->material3Count
                );
                handler.PSendSysMessage(msg.c_str());
                
                handler.PSendSysMessage("|cffff0000==============================================|r");
            }
            
            LOG_WARN("module", "Karazhan: Player {} lacks material - Item: {}, Need: {}",
                     player->GetName(), config->material3, config->material3Count);
            
            return false;
        }
    }

    // ========================================
    // 5. Consume all costs
    // ========================================
    player->ModifyMoney(-static_cast<int32>(goldInCopper));

    if (config->material1 > 0 && config->material1Count > 0)
        player->DestroyItemCount(config->material1, config->material1Count, true);

    if (config->material2 > 0 && config->material2Count > 0)
        player->DestroyItemCount(config->material2, config->material2Count, true);

    if (config->material3 > 0 && config->material3Count > 0)
        player->DestroyItemCount(config->material3, config->material3Count, true);

    LOG_INFO("module", "Karazhan: Player {} consumed materials - Gold: {}, Mat1: {}x{}, Mat2: {}x{}, Mat3: {}x{}",
             player->GetName(), config->goldCost,
             config->material1, config->material1Count,
             config->material2, config->material2Count,
             config->material3, config->material3Count);

    return true;
}

void ItemKarazhanMgr::UpdateItemEnhanceStats(uint32 itemGuid, EnhanceResult result, uint32 goldCost)
{
    // Convert DB gold value to copper
    uint32 goldInCopper = goldCost * 10000;

    if (result == EnhanceResult::SUCCESS)
    {
        CharacterDatabase.Execute(
            "UPDATE karazhan_item_enhance SET "
            "total_success = total_success + 1, "
            "total_gold_spent = total_gold_spent + {} "
            "WHERE item_guid = {}",
            goldInCopper, itemGuid
        );
    }
    else if (result == EnhanceResult::FAIL)
    {
        CharacterDatabase.Execute(
            "UPDATE karazhan_item_enhance SET "
            "total_fail = total_fail + 1, "
            "total_gold_spent = total_gold_spent + {} "
            "WHERE item_guid = {}",
            goldInCopper, itemGuid
        );
    }
}

void ItemKarazhanMgr::ApplyEnchantToItem(Player* player, Item* item, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return;

    uint32 enchantId = 0;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_ENCHANT_ITEM ||
            spellInfo->Effects[i].Effect == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
        {
            enchantId = spellInfo->Effects[i].MiscValue;
            break;
        }
    }

    if (enchantId > 0)
    {
        SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchantId);
        if (enchantEntry)
        {
            bool isEquipped = item->IsEquipped();
            uint8 itemSlot = item->GetSlot();

            if (isEquipped)
                player->_ApplyItemMods(item, itemSlot, false);

            item->ClearEnchantment(TEMP_ENCHANTMENT_SLOT);
            item->SetEnchantment(TEMP_ENCHANTMENT_SLOT, enchantId, 0, 0, player->GetGUID());

            if (isEquipped)
                player->_ApplyItemMods(item, itemSlot, true);
        }
    }
}

bool ItemKarazhanMgr::ApplyRandomProperty(Player* player, Item* item, int32 randomPropertyId)
{
    if (!player || !item)
        return false;

    ItemStateBackup backup = BackupItemState(item);
    Item* newItem = CreateItemWithRandomProperty(player, backup.entry, randomPropertyId);
    
    if (!newItem)
        return false;

    if (!RestoreItemState(newItem, backup, player))
    {
        delete newItem;
        return false;
    }

    return true;
}

ItemStateBackup ItemKarazhanMgr::BackupItemState(Item const* item)
{
    ItemStateBackup backup = {};

    backup.entry = item->GetEntry();
    backup.bag = item->GetBagSlot();
    backup.slot = item->GetSlot();

    for (uint8 i = 0; i < KARAZHAN_ENCHANT_SLOTS; ++i)
    {
        backup.enchantments[i].enchantId = item->GetEnchantmentId(static_cast<EnchantmentSlot>(i));
        backup.enchantments[i].duration = item->GetEnchantmentDuration(static_cast<EnchantmentSlot>(i));
        backup.enchantments[i].charges = item->GetEnchantmentCharges(static_cast<EnchantmentSlot>(i));
    }

    backup.durability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
    backup.maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    backup.soulbound = item->IsSoulBound();
    backup.owner = item->GetOwnerGUID();
    backup.randomPropertyId = item->GetItemRandomPropertyId();
    backup.randomSuffix = item->GetItemSuffixFactor();

    return backup;
}

Item* ItemKarazhanMgr::CreateItemWithRandomProperty(Player* player, uint32 entry, int32 randomPropertyId)
{
    if (!player)
        return nullptr;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(entry);
    if (!proto)
        return nullptr;

    Item* newItem = Item::CreateItem(entry, 1, player);
    if (!newItem)
        return nullptr;

    if (randomPropertyId != 0)
    {
        newItem->SetInt32Value(ITEM_FIELD_RANDOM_PROPERTIES_ID, randomPropertyId);
        newItem->SetUInt32Value(ITEM_FIELD_PROPERTY_SEED, 0);
        newItem->SetState(ITEM_CHANGED, player);
    }

    return newItem;
}

bool ItemKarazhanMgr::RestoreItemState(Item* newItem, ItemStateBackup const& backup, Player* player)
{
    if (!newItem || !player)
        return false;

    for (uint8 i = 0; i < KARAZHAN_ENCHANT_SLOTS; ++i)
    {
        if (i == TEMP_ENCHANTMENT_SLOT)
            continue;

        if (backup.enchantments[i].enchantId != 0)
        {
            newItem->SetEnchantment(
                static_cast<EnchantmentSlot>(i),
                backup.enchantments[i].enchantId,
                backup.enchantments[i].duration,
                backup.enchantments[i].charges,
                player->GetGUID()
            );
        }
    }

    newItem->SetUInt32Value(ITEM_FIELD_DURABILITY, backup.durability);
    newItem->SetUInt32Value(ITEM_FIELD_MAXDURABILITY, backup.maxDurability);

    if (backup.soulbound)
    {
        newItem->SetBinding(true);
        newItem->SetOwnerGUID(backup.owner);
    }

    newItem->SetState(ITEM_CHANGED, player);
    return true;
}

std::string ItemKarazhanMgr::GetItemNameLocale(uint32 itemEntry, Player* player) const
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemEntry);
    if (!proto)
        return "Unknown";

    ItemLocale const* locale = sObjectMgr->GetItemLocale(itemEntry);
    if (locale && player)
    {
        LocaleConstant loc = player->GetSession()->GetSessionDbLocaleIndex();
        ObjectMgr::GetLocaleString(locale->Name, loc, const_cast<std::string&>(proto->Name1));
    }

    return proto->Name1;
}

uint8 ItemKarazhanMgr::GetItemEnhanceType(Item const* item) const
{
    if (!item)
        return ENHANCE_TYPE_NONE;

    uint32 enchantId = item->GetEnchantmentId(
        static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT + 3));

    if (enchantId >= 3884 && enchantId <= 3983)
        return ENHANCE_TYPE_ALL_STATS;

    if (enchantId >= 3984 && enchantId <= 4023)
    {
        switch ((enchantId - 3984) % 4)
        {
            case 0:
                return ENHANCE_TYPE_MELEE;
            case 1:
                return ENHANCE_TYPE_CASTER;
            case 2:
                return ENHANCE_TYPE_HEALER;
            case 3:
                return ENHANCE_TYPE_TANK;
            default:
                break;
        }
    }

    return ENHANCE_TYPE_NONE;
}

char const* ItemKarazhanMgr::GetEnhanceTypeName(uint8 enhanceType) const
{
    switch (enhanceType)
    {
        case ENHANCE_TYPE_MELEE:
            return "밀리";
        case ENHANCE_TYPE_CASTER:
            return "캐스터";
        case ENHANCE_TYPE_HEALER:
            return "힐러";
        case ENHANCE_TYPE_TANK:
            return "탱커";
        case ENHANCE_TYPE_ALL_STATS:
            return "모능";
        default:
            return "미설정";
    }
}

uint32 ItemKarazhanMgr::GetEnhanceSpellId(uint8 level, uint8 enhanceType) const
{
    if (level == 0)
        return 0;

    switch (enhanceType)
    {
        case ENHANCE_TYPE_ALL_STATS:
            return 201180 + level;
        case ENHANCE_TYPE_MELEE:
        case ENHANCE_TYPE_CASTER:
        case ENHANCE_TYPE_HEALER:
        case ENHANCE_TYPE_TANK:
            return 201201 + ((level - 1) * 4) +
                (static_cast<uint32>(enhanceType) - 1);
        default:
            return 0;
    }
}

void ItemKarazhanMgr::RequestEnhancement(Player* player, Item* item,
    uint8 enhanceType)
{
    // ========================================
    // STEP 0: Basic validation
    // ========================================
    if (!player || !item)
    {
        LOG_ERROR("module", "Karazhan: RequestEnhancement - NULL player or item!");
        return;
    }

    // Validate player state
    if (!player->IsInWorld() || !player->IsAlive())
    {
        LOG_ERROR("module", "Karazhan: Player {} is not in world or dead!", player->GetName());
        
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cffff0000강화할 수 없는 상태입니다.|r");
        }
        return;
    }

    // Validate item ownership
    if (item->GetOwnerGUID() != player->GetGUID())
    {
        LOG_ERROR("module", "Karazhan: Item GUID:{} is not owned by player {}!",
                  item->GetGUID().GetCounter(), player->GetName());
        
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cffff0000해당 아이템을 소유하고 있지 않습니다!|r");
        }
        return;
    }

    // Validate item inventory position
    uint8 bag = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    
    Item* foundItem = player->GetItemByPos(bag, slot);
    if (!foundItem || foundItem->GetGUID() != item->GetGUID())
    {
        LOG_ERROR("module", "Karazhan: Item GUID:{} not found in player inventory at Bag:{}, Slot:{}!",
                  item->GetGUID().GetCounter(), bag, slot);
        
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cffff0000아이템을 찾을 수 없습니다!|r");
        }
        return;
    }

    // Validate enhancement eligibility
    if (!CanEnhance(item, player))
    {
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cffff0000해당 아이템은 더 이상 강화할 수 없습니다!|r");
        }
        return;
    }

    // Collect item state
    uint32 oldItemGuid = item->GetGUID().GetCounter();
    uint32 itemEntry = item->GetEntry();
    uint8 currentLevel = GetItemEnhanceLevel(oldItemGuid);
    uint8 targetLevel = currentLevel + 1;

    LOG_INFO("module", "Karazhan: RequestEnhancement - Player: {}, Item: {}, GUID: {}, Bag: {}, Slot: {}, Level: {} -> {}, Type: {}",
             player->GetName(), itemEntry, oldItemGuid, bag, slot, currentLevel,
             targetLevel, GetEnhanceTypeName(enhanceType));

    if (enhanceType != ENHANCE_TYPE_MELEE &&
        enhanceType != ENHANCE_TYPE_CASTER &&
        enhanceType != ENHANCE_TYPE_HEALER &&
        enhanceType != ENHANCE_TYPE_TANK)
    {
        if (WorldSession* session = player->GetSession())
            ChatHandler(session).PSendSysMessage(
                "|cffff0000강화 타입을 먼저 선택해야 합니다.|r");

        return;
    }

    // Load target enhancement config
    KarazhanEnchantConfig const* config = GetEnchantConfig(targetLevel);
    if (!config)
    {
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            std::string msg = Acore::StringFormat("강화 설정을 찾을 수 없습니다! (단계 {})", targetLevel);
            handler.PSendSysMessage(msg.c_str());
        }
        return;
    }

    // ???щ즺 ?뺤씤 諛??뚮퉬
    if (!CheckAndConsumeMaterials(player, config))
        return;

    // Roll enhancement result
    float randomValue = frand(0.0f, 100.0f);
    EnhanceResult result;

    if (randomValue <= config->successRate)
        result = EnhanceResult::SUCCESS;
    else if (randomValue <= (config->successRate + config->failRate))
        result = EnhanceResult::FAIL;
    else
        result = EnhanceResult::DESTROYED;

    LOG_INFO("module", "Karazhan: Enhancement result calculated - Random: {:.2f}, Success: {:.2f}, Fail: {:.2f}, Result: {}",
             randomValue, config->successRate, config->failRate, static_cast<int>(result));

    // Backup current item state
    ItemStateBackup backup = BackupItemState(item);

    // ???湲??먯뿉 異붽?
    PendingEnhancement pending;
    pending.playerGuid = player->GetGUID();
    pending.oldItemGuid = oldItemGuid;
    pending.itemEntry = itemEntry;
    pending.bag = bag;
    pending.slot = slot;
    pending.currentLevel = currentLevel;
    pending.targetLevel = targetLevel;
    pending.enhanceType = enhanceType;
    pending.result = result;
    pending.backup = backup;
    pending.config = *config;
    pending.randomValue = randomValue;

    _pendingQueue.push(pending);

    LOG_INFO("module", "Karazhan: Enhancement queued - Queue size: {}", _pendingQueue.size());

    // Notify player
    if (WorldSession* session = player->GetSession())
    {
        ChatHandler handler(session);
        handler.PSendSysMessage("|cff00ff00==============================================|r");
        handler.PSendSysMessage("|cffffcc00[카라잔 강화]|r 강화를 시작합니다...");
        handler.PSendSysMessage("|cff00ff00잠시 후 결과가 표시됩니다.|r");
        handler.PSendSysMessage("|cff00ff00==============================================|r");
    }
}

void ItemKarazhanMgr::ProcessPendingEnhancement(PendingEnhancement const& pending)
{
    // ========================================
    // STEP 1: Validate player
    // ========================================
    Player* player = ObjectAccessor::FindPlayer(pending.playerGuid);
    if (!player || !player->IsInWorld() || !player->GetSession())
    {
        LOG_ERROR("module", "Karazhan: Player not available!");
        return;
    }

    Item* oldItem = player->GetItemByPos(pending.bag, pending.slot);
    if (!oldItem || oldItem->GetGUID().GetCounter() != pending.oldItemGuid)
    {
        LOG_ERROR("module", "Karazhan: Item GUID mismatch!");
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler(session).PSendSysMessage("|cffff0000아이템을 찾을 수 없습니다!|r");
        }
        return;
    }

    std::string itemName = GetItemNameLocale(pending.itemEntry, player);

    // ========================================
    // SUCCESS
    // ========================================
    if (pending.result == EnhanceResult::SUCCESS)
    {
        LOG_INFO("module", "Karazhan: ===== ENHANCEMENT START =====");

        bool wasEquipped = oldItem->IsEquipped();
        uint8 originalSlot = oldItem->GetSlot();

        // ========================================
        // STEP 2: Unequip
        // ========================================
        if (wasEquipped)
        {
            LOG_INFO("module", "Karazhan: Unequipping item...");

            uint32 hpBefore = player->GetHealth();
            uint32 maxHpBefore = player->GetMaxHealth();

            ItemPosCountVec unequipDest;
            if (player->CanStoreItem(NULL_BAG, NULL_SLOT, unequipDest, oldItem, false) != EQUIP_ERR_OK)
            {
                LOG_ERROR("module", "Karazhan: No inventory space!");
                if (WorldSession* session = player->GetSession())
                {
                    ChatHandler(session).PSendSysMessage("|cffff0000인벤토리 공간이 부족합니다!|r");
                }
                return;
            }

            player->_ApplyItemMods(oldItem, originalSlot, false);
            player->RemoveItem(pending.bag, pending.slot, false);
            oldItem = player->StoreItem(unequipDest, oldItem, true);

            if (!oldItem)
            {
                LOG_ERROR("module", "Karazhan: Unequip failed!");
                return;
            }

            uint32 hpAfter = player->GetHealth();
            uint32 maxHpAfter = player->GetMaxHealth();

            if (hpAfter == 0 || hpAfter > maxHpAfter)
            {
                if (maxHpBefore > 0 && maxHpAfter > 0)
                {
                    float percent = (float)hpBefore / (float)maxHpBefore;
                    player->SetHealth(std::max(1u, (uint32)(maxHpAfter * percent)));
                }
                else
                {
                    player->SetHealth(std::max(1u, maxHpAfter / 2));
                }
            }

            LOG_INFO("module", "Karazhan: Unequipped successfully");
        }

        // ========================================
        // STEP 3: Capture current position
        // ========================================
        uint8 currentBag = oldItem->GetBagSlot();
        uint8 currentSlot = oldItem->GetSlot();

        // ========================================
        // STEP 4: 諛깆뾽
        // ========================================
        uint32 itemEntry = oldItem->GetEntry();
        bool isSoulbound = oldItem->IsSoulBound();
        uint32 durability = oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY);
        uint32 maxDurability = oldItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

        struct EnchantData {
            uint32 id;
            uint32 duration;
            uint32 charges;
        } enchants[MAX_ENCHANTMENT_SLOT];

        for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            enchants[i].id = oldItem->GetEnchantmentId(static_cast<EnchantmentSlot>(i));
            enchants[i].duration = oldItem->GetEnchantmentDuration(static_cast<EnchantmentSlot>(i));
            enchants[i].charges = oldItem->GetEnchantmentCharges(static_cast<EnchantmentSlot>(i));
        }

        LOG_INFO("module", "Karazhan: Backup complete");

        // ========================================
        // STEP 5: ??젣
        // ========================================
        player->DestroyItem(currentBag, currentSlot, true);
        LOG_INFO("module", "Karazhan: Old item destroyed - GUID:{}", pending.oldItemGuid);

        // ========================================
        // STEP 6: ?끸쁾???щ컮瑜?StoreNewItem ?몄텧 ?끸쁾??
        // ========================================
        int32 randomPropId = 2164 + pending.targetLevel;

        ItemPosCountVec dest;
        // ??CanStoreItem ?щ컮瑜??쒓렇?덉쿂: (bag, slot, dest, entry, count)
        InventoryResult storeResult = player->CanStoreItem(currentBag, currentSlot, dest, itemEntry, 1);

        if (storeResult != EQUIP_ERR_OK)
        {
            storeResult = player->CanStoreItem(NULL_BAG, NULL_SLOT, dest, itemEntry, 1);
            
            if (storeResult != EQUIP_ERR_OK)
            {
                LOG_FATAL("module", "Karazhan: CRITICAL - No space!");
                return;
            }
        }

        // ?끸쁾??StoreNewItem: (dest, entry, update, randomPropertyId) ?끸쁾??
        Item* newItem = player->StoreNewItem(dest, itemEntry, true, randomPropId);

        if (!newItem)
        {
            LOG_FATAL("module", "Karazhan: StoreNewItem failed!");
            return;
        }

        uint32 newGuid = newItem->GetGUID().GetCounter();
        LOG_INFO("module", "Karazhan: New item created - GUID:{}, RandomProp:{}",
                 newGuid, randomPropId);

        // ========================================
        // STEP 7: 蹂듭썝
        // ========================================
        if (isSoulbound && !newItem->IsSoulBound())
        {
            newItem->SetBinding(true);
        }

        newItem->SetUInt32Value(ITEM_FIELD_DURABILITY, durability);
        newItem->SetUInt32Value(ITEM_FIELD_MAXDURABILITY, maxDurability);

        for (uint8 i = 0; i < MAX_ENCHANTMENT_SLOT; ++i)
        {
            if (i == TEMP_ENCHANTMENT_SLOT || 
                (i >= SOCK_ENCHANTMENT_SLOT && i < SOCK_ENCHANTMENT_SLOT + MAX_ITEM_PROTO_SOCKETS))
            {
                continue;
            }

            if (enchants[i].id > 0)
            {
                newItem->SetEnchantment(
                    static_cast<EnchantmentSlot>(i),
                    enchants[i].id,
                    enchants[i].duration,
                    enchants[i].charges,
                    player->GetGUID()
                );
            }
        }

        // ========================================
        // STEP 8: Apply enhancement enchant
        // ========================================
        uint32 spellId = GetEnhanceSpellId(pending.targetLevel,
            pending.enhanceType);
        if (spellId > 0)
        {
            if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
            {
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                {
                    if (spellInfo->Effects[i].Effect == SPELL_EFFECT_ENCHANT_ITEM ||
                        spellInfo->Effects[i].Effect == SPELL_EFFECT_ENCHANT_ITEM_TEMPORARY)
                    {
                        uint32 enchantId = spellInfo->Effects[i].MiscValue;
                        if (enchantId > 0)
                        {
                            EnchantmentSlot slot = static_cast<EnchantmentSlot>(SOCK_ENCHANTMENT_SLOT + 3);
                            newItem->SetEnchantment(slot, enchantId, 0, 0, player->GetGUID());
                            LOG_INFO("module", "Karazhan: Enhancement enchant {} applied for type {}",
                                enchantId, GetEnhanceTypeName(pending.enhanceType));
                        }
                        break;
                    }
                }
            }
        }

        newItem->SetState(ITEM_CHANGED, player);

        // ========================================
        // STEP 9: Update database
        // ========================================
        DeleteItemEnhance(pending.oldItemGuid);
        SetItemEnhanceLevel(newGuid, player->GetGUID().GetCounter(), 
                           itemEntry, pending.targetLevel);

        LOG_INFO("module", "Karazhan: Enhancement data updated");

        // ========================================
        // STEP 10: ?ㅽ꺈 ?ш퀎??
        // ========================================
        player->UpdateAllStats();

        if (player->GetHealth() > player->GetMaxHealth())
        {
            player->SetHealth(player->GetMaxHealth());
        }

        // ========================================
        // STEP 11: 硫붿떆吏
        // ========================================
        if (WorldSession* session = player->GetSession())
        {
            ChatHandler handler(session);
            handler.PSendSysMessage("|cff00ff00==============================================|r");
            handler.PSendSysMessage("|cff00ff00[강화 성공!]|r");
            
            std::string msg = Acore::StringFormat(
                "|cffffcc00{}|r |cff00ff00+{}|r -> |cffff00ff+{}|r ({})",
                itemName, pending.currentLevel, pending.targetLevel,
                GetEnhanceTypeName(pending.enhanceType)
            );
            handler.PSendSysMessage(msg.c_str());

            if (wasEquipped)
            {
                handler.PSendSysMessage("|cffffcc00강화된 아이템이 가방으로 이동했습니다.|r");
            }

            handler.PSendSysMessage("|cff00ff00==============================================|r");
        }

        LOG_INFO("module", "Karazhan: ===== ENHANCEMENT SUCCESS =====");
    }
    // ========================================
    // FAIL
    // ========================================
    else if (pending.result == EnhanceResult::FAIL)
    {
        if (pending.currentLevel == 0)
        {
            SetItemEnhanceLevel(pending.oldItemGuid, player->GetGUID().GetCounter(),
                               pending.itemEntry, pending.currentLevel);
        }

        UpdateItemEnhanceStats(pending.oldItemGuid, EnhanceResult::FAIL, pending.config.goldCost);

        if (WorldSession* session = player->GetSession())
        {
            ChatHandler(session).PSendSysMessage("|cffff0000[강화 실패] 단계 유지|r");
        }

        LOG_INFO("module", "Karazhan: Enhancement FAIL");
    }
    // ========================================
    // DESTROYED
    // ========================================
    else if (pending.result == EnhanceResult::DESTROYED)
    {
        player->DestroyItem(pending.bag, pending.slot, true);
        DeleteItemEnhance(pending.oldItemGuid);

        if (WorldSession* session = player->GetSession())
        {
            ChatHandler(session).PSendSysMessage("|cffff0000[아이템 파괴!]|r");
        }

        LOG_WARN("module", "Karazhan: Enhancement DESTROYED");
    }
}

void Add_SC_item_karazhan_mgr()
{
    // ?붾? ?⑥닔
}
