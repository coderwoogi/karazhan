// modules/mod-blackmarket/src/BlackMarketSystem.cpp

#include "BlackMarketSystem.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "World.h"
#include "Chat.h"
#include "MapMgr.h"
#include "Creature.h"
#include "Item.h"
#include "GameTime.h"
#include "DBCStores.h"
#include "WorldPacket.h"
#include "Language.h"
#include "Map.h"
#include "WorldSession.h"
#include "StringFormat.h"

BlackMarketSystem* BlackMarketSystem::instance()
{
    static BlackMarketSystem instance;
    return &instance;
}

BlackMarketSystem::BlackMarketSystem()
    : _enabled(false), _isActive(false), _spawnCycle(30), _itemCount(5),
      _announceGlobal(false), _announceZone(true), _npcEntry(999999),
      _currentSessionId(0), _currentSpawnPointId(0)
{
    _dialogues.push_back("시간은 금이고, 기회는 짧지...");
    _dialogues.push_back("오늘은 당신에게 운이 따를지도 모르겠군.");
    _dialogues.push_back("거래는 여기에서만 유효하다.");
    _dialogues.push_back("원하는 것이 있다면 서두르는 게 좋을 거야.");
    _dialogues.push_back("어떤 물건을 찾고 있지?");
    _dialogues.push_back("이런 물건은 아무 데서나 구할 수 없지.");
    _dialogues.push_back("빨리 말해, 난 오래 머물지 않는다.");
    _dialogues.push_back("이 물건은 당신을 기다려주지 않을 거다.");
}

BlackMarketSystem::~BlackMarketSystem()
{
}

void BlackMarketSystem::Initialize()
{
    _enabled = sConfigMgr->GetOption<bool>("BlackMarket.Enable", true);
    
    if (!_enabled)
        return;
    
    _spawnCycle = sConfigMgr->GetOption<uint32>("BlackMarket.SpawnCycle", 30);
    _announceGlobal = sConfigMgr->GetOption<bool>("BlackMarket.Announce.Global", false);
    _announceZone = sConfigMgr->GetOption<bool>("BlackMarket.Announce.Zone", true);
    _itemCount = sConfigMgr->GetOption<uint8>("BlackMarket.ItemCount", 5);
    _npcEntry = sConfigMgr->GetOption<uint32>("BlackMarket.NpcEntry", 999999);
    
    LoadSpawnPoints();
    LoadItemPool();
    LoadCurrentState();

    _events.CancelEvent(EVENT_BLACKMARKET_CYCLE);
    _events.CancelEvent(EVENT_BLACKMARKET_ANNOUNCE);

    // Always start a fresh random session on server startup.
    if (_isActive || !_currentMerchantGUID.IsEmpty() || !_currentItems.empty())
        DespawnMerchant();

    _events.ScheduleEvent(EVENT_BLACKMARKET_CYCLE, Seconds(1));
}

void BlackMarketSystem::LoadSpawnPoints()
{
    QueryResult result = WorldDatabase.Query(
        "SELECT id, map, zone_id, position_x, position_y, position_z, orientation, comment "
        "FROM blackmarket_spawn_points");
    
    if (!result)
        return;
    
    do
    {
        Field* fields = result->Fetch();
        
        BlackMarketSpawnPoint point;
        point.id = fields[0].Get<uint32>();
        point.map = fields[1].Get<uint16>();
        point.zoneId = fields[2].Get<uint32>();
        point.x = fields[3].Get<float>();
        point.y = fields[4].Get<float>();
        point.z = fields[5].Get<float>();
        point.o = fields[6].Get<float>();
        point.comment = fields[7].Get<std::string>();
        
        _spawnPoints.push_back(point);
        
    } while (result->NextRow());
    
}

void BlackMarketSystem::LoadItemPool()
{
    QueryResult result = WorldDatabase.Query(
        "SELECT id, item_entry, price_gold, price_item, price_item_count, "
        "rarity, weight, max_per_spawn "
        "FROM blackmarket_item_pool");
    
    if (!result)
        return;
    
    do
    {
        Field* fields = result->Fetch();
        
        BlackMarketItem item;
        item.id = fields[0].Get<uint32>();
        item.itemEntry = fields[1].Get<uint32>();
        item.goldPrice = fields[2].Get<uint32>();
        item.currencyItemEntry = fields[3].Get<uint32>();
        item.currencyCount = fields[4].Get<uint32>();
        item.rarity = fields[5].Get<uint8>();
        item.weight = fields[6].Get<uint32>();
        item.maxCount = fields[7].Get<uint8>();
        
        _itemPool.push_back(item);
        
    } while (result->NextRow());
    
}

void BlackMarketSystem::LoadCurrentState()
{
    QueryResult result = WorldDatabase.Query(
        "SELECT is_active, spawn_point_id, spawn_time, despawn_time, creature_guid "
        "FROM blackmarket_state WHERE id = 1");
    
    if (!result)
    {
        WorldDatabase.Execute("INSERT INTO blackmarket_state (id, is_active) VALUES (1, 0)");
        return;
    }
    
    Field* fields = result->Fetch();
    _isActive = fields[0].Get<uint8>() != 0;
    _currentSpawnPointId = fields[1].Get<uint32>();
    uint32 spawnTime = fields[2].Get<uint32>();
    uint64 creatureGuid = fields[4].Get<uint64>();
    
    if (_isActive)
    {
        _currentMerchantGUID.Set(creatureGuid);
        _currentSessionId = spawnTime;

        QueryResult itemResult = WorldDatabase.Query(
            "SELECT id, item_entry, price_gold, price_item, price_item_count, remaining_count "
            "FROM blackmarket_vendor_items ORDER BY id");

        if (itemResult)
        {
            do
            {
                Field* itemFields = itemResult->Fetch();

                BlackMarketVendorItem vendorItem;
                vendorItem.id = itemFields[0].Get<uint32>();
                vendorItem.itemEntry = itemFields[1].Get<uint32>();
                vendorItem.goldPrice = itemFields[2].Get<uint32>();
                vendorItem.currencyItemEntry = itemFields[3].Get<uint32>();
                vendorItem.currencyCount = itemFields[4].Get<uint32>();
                vendorItem.remainingCount = itemFields[5].Get<uint8>();

                _currentItems.push_back(vendorItem);

            } while (itemResult->NextRow());
        }

    }
}

void BlackMarketSystem::Update(uint32 diff)
{
    if (!_enabled)
        return;
    
    _events.Update(diff);
    
    while (uint32 eventId = _events.ExecuteEvent())
    {
        switch (eventId)
        {
            case EVENT_BLACKMARKET_CYCLE:
                SpawnMerchant();
                break;
                
            case EVENT_BLACKMARKET_ANNOUNCE:
                AnnounceSpawn();
                break;
        }
    }
}

void BlackMarketSystem::SpawnMerchant()
{
    if (_isActive)
    {
        LOG_WARN("module", "BlackMarket already active. Duplicate spawn prevented.");
        return;
    }

    QueryResult stateCheck = WorldDatabase.Query(
        "SELECT is_active FROM blackmarket_state WHERE id = 1");
    
    if (stateCheck)
    {
        Field* fields = stateCheck->Fetch();
        if (fields[0].Get<uint8>() != 0)
        {
            LOG_ERROR("module", "BlackMarket recorded as active in DB. Stopping spawn.");
            _isActive = true;
            return;
        }
    }

    if (_spawnPoints.empty())
    {
        LOG_ERROR("module", "BlackMarket spawn failed - No spawn points");
        return;
    }

    if (!_currentMerchantGUID.IsEmpty())
    {
        LOG_WARN("module", "BlackMarket existing NPC GUID found. Respawning after cleanup.");
        DespawnMerchant();
    }

    uint32 randIndex = urand(0, _spawnPoints.size() - 1);
    BlackMarketSpawnPoint const& point = _spawnPoints[randIndex];
    _currentSpawnPointId = point.id;
    
    Map* map = sMapMgr->CreateBaseMap(point.map);
    if (!map)
    {
        LOG_ERROR("module", "BlackMarket map creation failed {}", point.map);
        return;
    }
    
    Creature* merchant = new Creature();
    if (!merchant->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, 1, _npcEntry, 0,
        point.x, point.y, point.z, point.o))
    {
        delete merchant;
        LOG_ERROR("module", "BlackMarket NPC creation failed");
        return;
    }
    
    merchant->SetHomePosition(point.x, point.y, point.z, point.o);
    merchant->SetReactState(REACT_PASSIVE);
    merchant->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
    merchant->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_PC);
    merchant->SetUnitFlag(UNIT_FLAG_IMMUNE_TO_NPC);
    
    if (!map->AddToMap(merchant))
    {
        delete merchant;
        LOG_ERROR("module", "BlackMarket failed to add to map");
        return;
    }
    
    _currentMerchantGUID = merchant->GetGUID();
    _isActive = true;
    _currentSessionId = uint32(GameTime::GetGameTime().count());
    
    ShuffleItems();
    
    _events.ScheduleEvent(EVENT_BLACKMARKET_ANNOUNCE, Seconds(2));
    
    SaveState();
    
    LOG_INFO("module", "BlackMarket merchant spawned (GUID: {}, Pos: {:.2f}/{:.2f}/{:.2f}, Map: {}, Items: {})",
        _currentMerchantGUID.GetCounter(), point.x, point.y, point.z, point.map, _currentItems.size());
}

void BlackMarketSystem::DespawnMerchant()
{
    if (!_isActive && _currentMerchantGUID.IsEmpty())
    {
        LOG_WARN("module", "BlackMarket already inactive. Duplicate despawn prevented.");
        return;
    }

    LOG_INFO("module", "BlackMarket merchant despawn started (GUID: {})", _currentMerchantGUID.GetCounter());

    bool removed = false;
    
    if (!_currentMerchantGUID.IsEmpty())
    {
        for (auto const& point : _spawnPoints)
        {
            Map* map = sMapMgr->FindMap(point.map, 0);
            if (map)
            {
                if (Creature* merchant = map->GetCreature(_currentMerchantGUID))
                {
                    merchant->AddObjectToRemoveList();
                    removed = true;
                    LOG_INFO("module", "BlackMarket NPC removed (Map {})", point.map);
                    break;
                }
            }
        }

        if (!removed)
        {
            LOG_WARN("module", "BlackMarket NPC not found (GUID: {}). Possibly already despawned.", 
                _currentMerchantGUID.GetCounter());
        }
    }
    
    _currentMerchantGUID.Clear();
    _isActive = false;
    _currentItems.clear();
    
    WorldDatabase.Execute("UPDATE blackmarket_state SET is_active = 0, creature_guid = 0 WHERE id = 1");
    WorldDatabase.Execute("DELETE FROM blackmarket_vendor_items");
    
    LOG_INFO("module", "BlackMarket merchant despawn complete");
}

void BlackMarketSystem::ShuffleItems()
{
    _currentItems.clear();
    
    if (_itemPool.empty())
        return;
    
    std::vector<BlackMarketItem> selectedItems; 
    std::vector<uint32> usedIndices;
    
    for ( uint8 i = 0; i < _itemCount && selectedItems.size() < _itemPool.size(); ++i )
    {
        uint32 totalWeight = 0;
        for ( size_t j = 0; j < _itemPool.size(); ++j )
        {
            if ( std::find( usedIndices.begin(), usedIndices.end(), j ) == usedIndices.end() )
                totalWeight += _itemPool[j].weight;
        }
        
        if ( totalWeight == 0 )
            break;
        
        uint32 roll = urand(0, totalWeight - 1);
        uint32 currentWeight = 0;
        
        for ( size_t j = 0; j < _itemPool.size(); ++j )
        {
            if ( std::find( usedIndices.begin(), usedIndices.end(), j ) != usedIndices.end() )
                continue;
            
            currentWeight += _itemPool[j].weight;
            if ( roll < currentWeight )
            {
                selectedItems.push_back( _itemPool[j] );
                usedIndices.push_back( j );
                break;
            }
        }
    }
    
    WorldDatabase.Execute("DELETE FROM blackmarket_vendor_items");
    
    for ( size_t i = 0; i < selectedItems.size(); ++i )
    {
        BlackMarketItem const& poolItem = selectedItems[i];
        
        BlackMarketVendorItem vendorItem;
        vendorItem.id = i + 1;
        vendorItem.itemEntry = poolItem.itemEntry;
        vendorItem.goldPrice = poolItem.goldPrice;
        vendorItem.currencyItemEntry = poolItem.currencyItemEntry;
        vendorItem.currencyCount = poolItem.currencyCount;
        vendorItem.remainingCount = poolItem.maxCount;
        
        _currentItems.push_back( vendorItem );
        
        WorldDatabase.Execute(
            "INSERT INTO blackmarket_vendor_items "
            "(id, item_entry, price_gold, price_item, price_item_count, remaining_count) "
            "VALUES ({}, {}, {}, {}, {}, {})",
            vendorItem.id, vendorItem.itemEntry, vendorItem.goldPrice,
            vendorItem.currencyItemEntry, vendorItem.currencyCount, vendorItem.remainingCount);
    }
    
    LOG_INFO("module", "BlackMarket selected {} items", _currentItems.size());
}

void BlackMarketSystem::SaveState()
{
    uint32 now = uint32(GameTime::GetGameTime().count());

    WorldDatabase.Execute(
        "UPDATE blackmarket_state SET "
        "is_active = {}, "
        "spawn_point_id = {}, "
        "spawn_time = {}, "
        "despawn_time = {}, "
        "creature_guid = {} "
        "WHERE id = 1",
        _isActive ? 1 : 0,
        _currentSpawnPointId,
        now,
        0,
        _currentMerchantGUID.GetCounter()
    );

    LOG_DEBUG("module", "BlackMarket state saved (Active: {}, GUID: {}, SpawnPoint: {})",
        _isActive, _currentMerchantGUID.GetCounter(), _currentSpawnPointId);
}

void BlackMarketSystem::AnnounceSpawn()
{
    if (!_isActive || _spawnPoints.empty())
        return;
    
    auto it = std::find_if(_spawnPoints.begin(), _spawnPoints.end(),
        [this](BlackMarketSpawnPoint const& p) { return p.id == _currentSpawnPointId; });
    
    if (it == _spawnPoints.end())
        return;
    
    std::string zoneName = "Unknown Location";
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(it->zoneId))
        zoneName = area->area_name[0];
    
    if (_announceGlobal)
    {
        std::ostringstream ss;
        ss << "|cFF9D9D9D수상한 기운이 감돕니다. 암상인이 모습을 드러냈습니다. 위치: " << zoneName << "...|r";
        std::string message = ss.str();
        
        HashMapHolder<Player>::MapType const& onlinePlayers = ObjectAccessor::GetPlayers();
        for (HashMapHolder<Player>::MapType::const_iterator itr = onlinePlayers.begin(); itr != onlinePlayers.end(); ++itr)
        {
            if (Player* player = itr->second)
            {
                if (player->IsInWorld())
                {
                    ChatHandler(player->GetSession()).SendSysMessage(message.c_str());
                }
            }
        }
    }
    else if (_announceZone)
    {
        Map* map = sMapMgr->FindMap(it->map, 0);
        if (map)
        {
            std::ostringstream ss;
            ss << "|cFF9D9D9D근처에서 수상한 움직임이 느껴집니다...|r";
            std::string message = ss.str();
            
            Map::PlayerList const& players = map->GetPlayers();
            for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            {
                if (Player* player = itr->GetSource())
                {
                    if (player->GetZoneId() == it->zoneId)
                    {
                        ChatHandler(player->GetSession()).SendSysMessage(message.c_str());
                    }
                }
            }
        }
    }
}

bool BlackMarketSystem::CanPurchaseItem(Player* player, uint32 itemEntry)
{
    BlackMarketVendorItem const* item = nullptr;
    for (auto const& vendorItem : _currentItems)
    {
        if (vendorItem.itemEntry == itemEntry)
        {
            item = &vendorItem;
            break;
        }
    }

    if (!item)  
    {
        ChatHandler(player->GetSession()).SendSysMessage("해당 아이템을 찾을 수 없습니다.");
        return false;
    }

    if (item->remainingCount == 0)
    {
        ChatHandler(player->GetSession()).SendSysMessage("모두 팔렸습니다.");
        return false;
    }

    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry);
    if (!itemTemplate)
    {
        ChatHandler(player->GetSession()).SendSysMessage("잘못된 아이템입니다.");
        return false;
    }

    if (itemTemplate->MaxCount > 0)
    {
        uint32 currentCount = player->GetItemCount(itemEntry, true);
        if (currentCount >= uint32(itemTemplate->MaxCount))
        {
            std::string itemName = GetLocalizedItemName(itemEntry, player->GetSession()->GetSessionDbcLocale());
            
            std::string msg = Acore::StringFormat(
                "|cFFFF0000이미 최대 개수의 아이템을 보유 중입니다. [{}] (현재: {}, 최대: {})|r",
                itemName, currentCount, itemTemplate->MaxCount
            );
            
            ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());
            return false;
        }
    }

    uint64 playerMoney = player->GetMoney();
    if (item->goldPrice > 0 && playerMoney < item->goldPrice)
    {
        std::string msg = Acore::StringFormat(
            "|cFFFF0000골드가 충분하지 않습니다. (현재: {}g, 필요: {}g)|r",
            playerMoney / 10000,
            item->goldPrice / 10000
        );
        
        ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());
        return false;
    }

    if (item->currencyItemEntry > 0 && item->currencyCount > 0)
    {
        uint32 ownedCount = player->GetItemCount(item->currencyItemEntry, false);
        if (ownedCount < item->currencyCount)
        {
            std::string currencyName = GetLocalizedItemName(item->currencyItemEntry, player->GetSession()->GetSessionDbcLocale());
            
            std::string msg = Acore::StringFormat(
                "|cFFFF0000재화가 충분하지 않습니다. [{}] (현재: {}, 필요: {})|r",
                currencyName, ownedCount, item->currencyCount
            );
            
            ChatHandler(player->GetSession()).SendSysMessage(msg.c_str());
            return false;
        }
    }

    return true;
}

bool BlackMarketSystem::PurchaseItem(Player* player, uint32 itemEntry)
{
    if (!CanPurchaseItem(player, itemEntry))
        return false;

    BlackMarketVendorItem* item = nullptr;
    for (auto& vendorItem : _currentItems)
    {
        if (vendorItem.itemEntry == itemEntry)
        {
            item = &vendorItem;
            break;
        }
    }   

    if (!item)
        return false;

    if (item->goldPrice > 0)
    {
        if (!player->ModifyMoney(-static_cast<int64>(item->goldPrice)))
        {
            LOG_ERROR("module", "Black Market: Failed to deduct {} copper from player", item->goldPrice);
            return false;
        }
    }

    if (item->currencyItemEntry > 0 && item->currencyCount > 0)
    {
        player->DestroyItemCount(item->currencyItemEntry, item->currencyCount, true);
    }

    ItemPosCountVec dest;
    if (player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item->itemEntry, 1) == EQUIP_ERR_OK)
    {
        if (Item* newItem = player->StoreNewItem(dest, item->itemEntry, true))
        {
            player->SendNewItem(newItem, 1, true, false);
        }
    }

    --item->remainingCount;

    WorldDatabase.Execute(
        "UPDATE blackmarket_vendor_items SET remaining_count = {} WHERE item_entry = {}",
        item->remainingCount, item->itemEntry);

    LogPurchase(player, item->itemEntry);

    // ?숈쟻 ?ㅽ룿 濡쒖쭅: 援щℓ 利됱떆 ?대룞
    DespawnMerchant();
    SpawnMerchant();

    return true;
}

void BlackMarketSystem::LogPurchase(Player* player, uint32 itemEntry)
{
    CharacterDatabase.Execute(
        "INSERT INTO blackmarket_purchase_log "
        "(account_id, character_guid, item_entry, purchase_time) "
        "VALUES ({}, {}, {}, {})",
        player->GetSession()->GetAccountId(),
        player->GetGUID().GetCounter(),
        itemEntry,
        uint32(GameTime::GetGameTime().count()));
}

std::string BlackMarketSystem::GetRandomDialogue() const
{
    if (_dialogues.empty())
        return "...";
    
    uint32 randIndex = urand(0, _dialogues.size() - 1);
    return _dialogues[randIndex];
}

void BlackMarketSystem::Shutdown()
{
    if (_isActive)
    {
        SaveState();
    }
    
    LOG_INFO("module", "BlackMarket system shutting down...");
}

std::string BlackMarketSystem::GetLocalizedItemName(uint32 itemEntry, LocaleConstant locale) const
{
    ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry);
    if (!itemTemplate)
        return "Unknown Item";

    if (locale != LOCALE_koKR)
        return itemTemplate->Name1;

    ItemLocale const* itemLocale = sObjectMgr->GetItemLocale(itemEntry);
    if (itemLocale && itemLocale->Name.size() > uint8(locale))
    {
        std::string localizedName = itemLocale->Name[locale];
        if (!localizedName.empty())
            return localizedName;
    }

    return itemTemplate->Name1;
}

void BlackMarketSystem::SendVendorInventory(Player* player, Creature* creature)
{
    if (!_isActive || _currentItems.empty())
    {
        ChatHandler(player->GetSession()).SendSysMessage("현재 판매 중인 항목이 없습니다.");
        return;
    }

    WorldPacket data(SMSG_LIST_INVENTORY);
    data << creature->GetGUID().WriteAsPacked();

    uint8 itemCount = 0;
    size_t countPos = data.wpos();
    data << uint8(itemCount);

    for (auto const& vendorItem : _currentItems)
    {
        if (vendorItem.remainingCount == 0)
            continue;

        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(vendorItem.itemEntry);
        if (!itemTemplate)
            continue;

        data << uint32(itemCount + 1);                  // slot
        data << uint32(vendorItem.itemEntry);           // item entry
        data << uint32(itemTemplate->DisplayInfoID);    // display id
        data << int32(vendorItem.remainingCount);       // stock count
        data << uint32(vendorItem.goldPrice);           // price
        data << uint32(itemTemplate->MaxDurability);    // max durability
        data << uint32(itemTemplate->BuyCount);         // buy count
        data << uint32(0);                              // extended cost

        ++itemCount;
    }

    data.put<uint8>(countPos, itemCount);
    player->GetSession()->SendPacket(&data);
}

bool BlackMarketSystem::HandleVendorBuy(Player* player, uint32 itemEntry)
{
    bool isBlackMarketItem = false;
    for (auto const& item : _currentItems)
    {
        if (item.itemEntry == itemEntry && item.remainingCount > 0)
        {
            isBlackMarketItem = true;
            break;
        }
    }

    if (!isBlackMarketItem)
        return false;

    if (PurchaseItem(player, itemEntry))
    {
        ChatHandler(player->GetSession()).SendSysMessage("|cFF00FF00구매 성공!|r");
        return true;
    }
    else
    {
        ChatHandler(player->GetSession()).SendSysMessage("|cFFFF0000구매 실패|r");
        return false;
    }
}

void BlackMarketSystem::SetEnabled(bool enabled)
{
    if (_enabled == enabled)
    {
        LOG_INFO("module", ">> BlackMarket system is already {}", enabled ? "enabled" : "disabled");
        return;
    }
    
    _enabled = enabled;
    
    if (!_enabled)
    {
        if (_isActive)
        {
            DespawnMerchant();
            LOG_INFO("module", ">> BlackMarket system disabled: Current NPC despawned");
        }
        
        _events.CancelEvent(EVENT_BLACKMARKET_CYCLE);
        _events.CancelEvent(EVENT_BLACKMARKET_ANNOUNCE);
        
        LOG_INFO("module", ">> BlackMarket system has been disabled");
    }
    else
    {
        if (!_isActive)
        {
            _events.CancelEvent(EVENT_BLACKMARKET_CYCLE);
            _events.ScheduleEvent(EVENT_BLACKMARKET_CYCLE, Seconds(1));
            LOG_INFO("module", ">> BlackMarket system enabled: Initial random spawn scheduled");
        }
        else
        {
            LOG_INFO("module", ">> BlackMarket system enabled: Current session maintained");
        }
    }
}

void BlackMarketSystem::Enable()
{
    SetEnabled(true);
}

void BlackMarketSystem::Disable()
{
    SetEnabled(false);
}
