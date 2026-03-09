// modules/mod-blackmarket/src/BlackMarketSystem.h

#ifndef BLACKMARKET_SYSTEM_H
#define BLACKMARKET_SYSTEM_H

#include "Common.h"
#include "ObjectGuid.h"
#include "EventMap.h"
#include <vector>
#include <string>

class Player;
class Creature;

struct BlackMarketSpawnPoint
{
    uint32 id;
    uint16 map;
    uint32 zoneId;
    float x, y, z, o;
    std::string comment;
};

struct BlackMarketItem
{
    uint32 id;
    uint32 itemEntry;
    uint32 goldPrice;
    uint32 currencyItemEntry;
    uint32 currencyCount;
    uint8 rarity;
    uint32 weight;
    uint8 maxCount;
};

struct BlackMarketVendorItem
{
    uint32 id;
    uint32 itemEntry;
    uint32 goldPrice;
    uint32 currencyItemEntry;
    uint32 currencyCount;
    uint8 remainingCount;
};

enum BlackMarketEvents
{
    EVENT_BLACKMARKET_CYCLE         = 1,
    EVENT_BLACKMARKET_DESPAWN       = 2,
    EVENT_BLACKMARKET_ANNOUNCE      = 3,
};

class BlackMarketSystem
{
public:
    static BlackMarketSystem* instance();

    void Initialize();
    void Update(uint32 diff);
    void Shutdown();

    bool IsEnabled() const { return _enabled; }
    bool IsActive() const { return _isActive; }
    
    // Runtime enable/disable methods
    void SetEnabled(bool enabled);
    void Enable();
    void Disable();
    
    ObjectGuid GetCurrentMerchantGUID() const { return _currentMerchantGUID; }
    uint32 GetCurrentSessionId() const { return _currentSessionId; }
    uint32 GetNpcEntry() const { return _npcEntry; }
    
    // Vendor 지원 함수
    void SendVendorInventory(Player* player, Creature* creature);
    bool HandleVendorBuy(Player* player, uint32 itemEntry);
    
    std::vector<BlackMarketVendorItem> const& GetCurrentItems() const { return _currentItems; }
    std::string GetRandomDialogue() const;
    std::string GetLocalizedItemName(uint32 itemEntry, LocaleConstant locale) const;

private:
    BlackMarketSystem();
    ~BlackMarketSystem();
    
    void LoadSpawnPoints();
    void LoadItemPool();
    void LoadCurrentState();
    
    void SpawnMerchant();
    void DespawnMerchant();
    void ShuffleItems();
    void SaveState();
    void AnnounceSpawn();
    
    bool CanPurchaseItem(Player* player, uint32 itemEntry);
    bool PurchaseItem(Player* player, uint32 itemEntry);
    void LogPurchase(Player* player, uint32 itemEntry);
    
    bool _enabled;
    bool _isActive;
    uint32 _spawnCycle;
    uint8 _itemCount;
    bool _announceGlobal;
    bool _announceZone;
    uint32 _npcEntry;
    
    EventMap _events;
    ObjectGuid _currentMerchantGUID;
    uint32 _currentSessionId;
    uint32 _currentSpawnPointId;
    
    std::vector<BlackMarketSpawnPoint> _spawnPoints;
    std::vector<BlackMarketItem> _itemPool;
    std::vector<BlackMarketVendorItem> _currentItems;
    
    std::vector<std::string> _dialogues;
};

#define sBlackMarket BlackMarketSystem::instance()

#endif
