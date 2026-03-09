/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
 */

#ifndef ITEM_KARAZHAN_H
#define ITEM_KARAZHAN_H

#include "Common.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include <queue>
#include <string>
#include <unordered_map>

class Item;
class Player;

// Constants
#define MAX_ITEM_PROTO_SOCKETS 3
#define KARAZHAN_ENCHANT_SLOTS 11

// Enhancement result state
enum class EnhanceResult : uint8
{
    SUCCESS = 0,
    FAIL = 1,
    DESTROYED = 2,
    FAILED = 3
};

// Slot configuration
struct KarazhanSlotConfig
{
    uint8 slotId;
    std::string slotName;
    std::string slotNameKo;
    bool canEnhance;
    uint8 maxEnhanceLevel;
    bool enabled;
    std::string comment;
};

// Enhancement level configuration
struct KarazhanEnchantConfig
{
    uint8 enchantLevel;
    uint32 spellId;
    int32 randomPropertyId;
    float successRate;
    float failRate;
    uint32 goldCost;
    uint32 material1;
    uint32 material1Count;
    uint32 material2;
    uint32 material2Count;
    uint32 material3;
    uint32 material3Count;
};

// Item enhancement data
struct KarazhanItemEnhance
{
    uint32 itemGuid;
    uint32 ownerGuid;
    uint32 itemEntry;
    uint8 enhanceLevel;
    uint32 totalSuccess;
    uint32 totalFail;
    uint64 totalGoldSpent;
};

// Item state backup
struct ItemStateBackup
{
    uint32 entry;
    uint8 bag;
    uint8 slot;

    struct EnchantmentData
    {
        uint32 enchantId;
        uint32 duration;
        uint32 charges;
    };
    EnchantmentData enchantments[KARAZHAN_ENCHANT_SLOTS];

    uint32 gems[MAX_ITEM_PROTO_SOCKETS];

    uint32 durability;
    uint32 maxDurability;

    bool soulbound;
    ObjectGuid owner;

    int32 randomPropertyId;
    uint32 randomSuffix;
};

// Queued enhancement request
struct PendingEnhancement
{
    ObjectGuid playerGuid;
    uint32 oldItemGuid;
    uint32 itemEntry;
    uint8 bag;
    uint8 slot;
    uint8 currentLevel;
    uint8 targetLevel;
    uint8 enhanceType;
    EnhanceResult result;
    ItemStateBackup backup;
    KarazhanEnchantConfig config;
    float randomValue;
};

class ItemKarazhanMgr
{
private:
    ItemKarazhanMgr();
    ~ItemKarazhanMgr() = default;

public:
    static ItemKarazhanMgr* instance();

    void Initialize();
    void Update(uint32 diff);  // Process queued enhancement jobs

    void LoadItemEnhance(uint32 itemGuid);

    bool CanEnhanceSlot(uint8 inventoryType) const;
    uint8 GetMaxEnhanceLevel(uint8 inventoryType) const;
    bool CanEnhance(Item const* item, Player* player) const;

    uint8 GetItemEnhanceLevel(uint32 itemGuid) const;
    void SetItemEnhanceLevel(uint32 itemGuid, uint32 ownerGuid, uint32 itemEntry, uint8 level);
    void DeleteItemEnhance(uint32 itemGuid);

    KarazhanEnchantConfig const* GetEnchantConfig(uint8 level) const;
    KarazhanSlotConfig const* GetSlotConfig(uint8 slotId) const;

    // Queue an enhancement request
    void RequestEnhancement(Player* player, Item* item, uint8 enhanceType);

    std::string GetItemNameLocale(uint32 itemEntry, Player* player) const;
    uint8 GetItemEnhanceType(Item const* item) const;
    char const* GetEnhanceTypeName(uint8 enhanceType) const;
    uint32 GetEnhanceSpellId(uint8 level, uint8 enhanceType) const;

private:
    void LoadSlotConfigs();
    void LoadEnchantConfigs();

    void ApplyEnchantToItem(Player* player, Item* item, uint32 spellId);

    bool CheckAndConsumeMaterials(Player* player, KarazhanEnchantConfig const* config);
    void UpdateItemEnhanceStats(uint32 itemGuid, EnhanceResult result, uint32 goldCost);
    void LogEnhance(Player* player, Item* item, uint8 levelBefore, uint8 levelAfter,
        uint8 targetLevel, EnhanceResult result, float successRate, float failRate,
        float randomValue, KarazhanEnchantConfig const* config, std::string const& errorMsg = "");

    // Process a queued enhancement request
    void ProcessPendingEnhancement(PendingEnhancement const& pending);

    bool ApplyRandomProperty(Player* player, Item* item, int32 randomPropertyId);

    ItemStateBackup BackupItemState(Item const* item);
    Item* CreateItemWithRandomProperty(Player* player, uint32 entry, int32 randomPropertyId);
    bool RestoreItemState(Item* newItem, ItemStateBackup const& backup, Player* player);

    std::unordered_map<uint8, KarazhanSlotConfig> _slotConfigs;
    std::unordered_map<uint8, KarazhanEnchantConfig> _enchantConfigs;
    std::unordered_map<uint32, KarazhanItemEnhance> _itemEnhances;

    // Pending enhancement queue
    std::queue<PendingEnhancement> _pendingQueue;
};

#define sItemKarazhanMgr ItemKarazhanMgr::instance()

#endif
