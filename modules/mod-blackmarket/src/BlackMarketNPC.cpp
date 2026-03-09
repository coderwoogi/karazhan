// modules/mod-blackmarket/src/BlackMarketNPC.cpp

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "Creature.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "BlackMarketSystem.h"

class npc_blackmarket_merchant : public CreatureScript
{
public:
    npc_blackmarket_merchant() : CreatureScript("npc_blackmarket_merchant") { }

    bool OnGossipHello(Player* player, Creature* creature) override 
    {
        if (!sBlackMarket->IsActive())
        {
            ChatHandler(player->GetSession()).SendSysMessage("암상인이 아직 거래 중이 아닙니다.");
            player->PlayerTalkClass->SendCloseGossip();
            return true;
        }

        ShowItemList(player, creature);
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender == GOSSIP_SENDER_MAIN)
        {
            if (action == 999) // ?リ린
            {
                player->PlayerTalkClass->SendCloseGossip();
                return true;
            }
            else if (action >= 100 && action < 200) // ?꾩씠??援щℓ
            {
                uint32 itemIndex = action - 100;
                std::vector<BlackMarketVendorItem> const& items = sBlackMarket->GetCurrentItems();
                
                if (itemIndex < items.size())
                {
                    uint32 itemEntry = items[itemIndex].itemEntry;
                    
                    // ??援щℓ ?ㅽ뙣 ?먯씤蹂?NPC ???
                    if (sBlackMarket->HandleVendorBuy(player, itemEntry))
                    {
                        creature->Say("누군가 암상인과 거래를 성사시켰습니다. 암상인은 자취를 감추고 새로운 장소로 이동합니다.", LANG_UNIVERSAL);
                        player->PlayerTalkClass->SendCloseGossip();
                        return true;
                    }
                    else
                    {
                        // ?ㅽ뙣 ?먯씤 ?뚯븙
                        std::string failReason = GetPurchaseFailReason(player, itemEntry);
                        creature->Say(failReason.c_str(), LANG_UNIVERSAL);
                    }
                }
                
                ShowItemList(player, creature);
                return true;
            }
        }

        return false;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_blackmarket_merchantAI(creature);
    }

    struct npc_blackmarket_merchantAI : public ScriptedAI
    {
        npc_blackmarket_merchantAI(Creature* creature) : ScriptedAI(creature) 
        {
            _dialogueTimer = urand(8000, 12000);
        }

        void Reset() override 
        { 
            _dialogueTimer = urand(8000, 12000);
        }

        void UpdateAI(uint32 diff) override 
        { 
            if (_dialogueTimer <= diff)
            {
                if (urand(0, 100) < 10)
                {
                    me->Say(sBlackMarket->GetRandomDialogue(), LANG_UNIVERSAL);
                }
                
                _dialogueTimer = urand(8000, 12000);
            }
            else
            {
                _dialogueTimer -= diff;
            }
        }

        void MoveInLineOfSight(Unit* who) override
        {
            if (!who || !who->IsInWorld())
                return;

            Player* player = who->ToPlayer();
            if (!player)
                return;

            if (me->IsWithinDist(player, 5.0f, false) && !me->IsWithinDist(player, 4.5f, false))
            {
                if (urand(0, 100) < 30)
                {
                    me->Say(sBlackMarket->GetRandomDialogue(), LANG_UNIVERSAL);
                }
            }
        }

    private:
        uint32 _dialogueTimer;
    };

private:
    // ??援щℓ ?ㅽ뙣 ?먯씤 ?먮떒
    std::string GetPurchaseFailReason(Player* player, uint32 itemEntry)
    {
        std::vector<BlackMarketVendorItem> const& items = sBlackMarket->GetCurrentItems();
        BlackMarketVendorItem const* item = nullptr;
        
        for (auto const& vendorItem : items)
        {
            if (vendorItem.itemEntry == itemEntry)
            {
                item = &vendorItem;
                break;
            }
        }
        
        if (!item || item->remainingCount == 0)
            return "이건 이미 팔렸어. 다음 기회를 노려라.";
        
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemEntry);
        if (!itemTemplate)
            return "잘못된 물건이다.";
        
        // 怨좎쑀 ?꾩씠??泥댄겕
        if (itemTemplate->MaxCount > 0)
        {
            uint32 currentCount = player->GetItemCount(itemEntry, true);
            if (currentCount >= uint32(itemTemplate->MaxCount))
                return "너는 그 물건을 이미 충분히 가지고 있잖아.";
        }
        
        // ?몃깽?좊━ 泥댄겕
        ItemPosCountVec dest;
        InventoryResult msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemEntry, 1);
        if (msg != EQUIP_ERR_OK)
        {
            if (msg == EQUIP_ERR_INVENTORY_FULL)
                return "짐이 너무 많은 것 같은데?";
            else
                return "그건 더 가질 수 없어.";
        }
        
        // 怨⑤뱶 泥댄겕
        if (item->goldPrice > 0 && player->GetMoney() < item->goldPrice)
            return "돈이 부족한가? 다음에 다시 오게.";
        
        // ?ы솕 泥댄겕
        if (item->currencyItemEntry > 0 && item->currencyCount > 0)
        {
            if (player->GetItemCount(item->currencyItemEntry, false) < item->currencyCount)
                return "필요한 재화가 부족해.";
        }
        
        return "지금 거래할 텐가?";
    }

    void ShowItemList(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);
        
        std::vector<BlackMarketVendorItem> const& items = sBlackMarket->GetCurrentItems();
        
        if (items.empty())
        {
            ChatHandler(player->GetSession()).SendSysMessage("현재 판매 중인 물품이 없습니다.");
            CloseGossipMenuFor(player);
            return;
        }

        for (size_t i = 0; i < items.size(); ++i)
        {
            BlackMarketVendorItem const& vendorItem = items[i];
            
            if (vendorItem.remainingCount == 0)
                continue;
            
            ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(vendorItem.itemEntry);
            if (!itemTemplate)
                continue;
            
            // ?쒓?紐?媛?몄삤湲?
            std::string itemName = sBlackMarket->GetLocalizedItemName(
                vendorItem.itemEntry, 
                player->GetSession()->GetSessionDbcLocale()
            );
            
            std::ostringstream ss;
            
            // ?ш????됱긽
            switch (itemTemplate->Quality)
            {
                case ITEM_QUALITY_POOR:
                    ss << "|cFF9d9d9d";
                    break;
                case ITEM_QUALITY_NORMAL:
                    ss << "|cFFffffff";
                    break;
                case ITEM_QUALITY_UNCOMMON:
                    ss << "|cFF1eff00";
                    break;
                case ITEM_QUALITY_RARE:
                    ss << "|cFF0070dd";
                    break;
                case ITEM_QUALITY_EPIC:
                    ss << "|cFFa335ee";
                    break;
                case ITEM_QUALITY_LEGENDARY:
                    ss << "|cFFff8000";
                    break;
                case ITEM_QUALITY_ARTIFACT:
                    ss << "|cFFe6cc80";
                    break;
                default:
                    break;
            }
            
            ss << itemName << "|r";
            
            // 媛寃??뺣낫
            if (vendorItem.goldPrice > 0)
            {
                uint32 gold = vendorItem.goldPrice / 10000;
                uint32 silver = (vendorItem.goldPrice % 10000) / 100;
                uint32 copper = vendorItem.goldPrice % 100;
                
                ss << " - ";
                if (gold > 0) ss << gold << "|cFFFFD700g|r ";
                if (silver > 0) ss << silver << "|cFFC0C0C0s|r ";
                if (copper > 0) ss << copper << "|cFFCD7F32c|r";
            }
            
            // 異붽? ?ы솕
            if (vendorItem.currencyItemEntry > 0)
            {
                std::string currencyName = sBlackMarket->GetLocalizedItemName(
                    vendorItem.currencyItemEntry, 
                    player->GetSession()->GetSessionDbcLocale()
                );
                ss << " |cFFFF0000+ " << vendorItem.currencyCount << "x " << currencyName << "|r";
            }
            
            // ?ш퀬
            ss << " |cFF808080[재고: " << uint32(vendorItem.remainingCount) << "]|r";
            
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, ss.str(), GOSSIP_SENDER_MAIN, 100 + i);
        }
        
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "|cFF808080[닫기]|r", GOSSIP_SENDER_MAIN, 999);
        
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
    }
};

void AddBlackMarketNPCScripts()
{
    new npc_blackmarket_merchant();
}

