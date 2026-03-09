/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"

bool modRptiEnabled;
bool modRptiAnnounce;
bool modRptiAddToBag;
uint32 modRptiRewardIntervalMinutes;

std::vector<uint32> modRptiItems;
std::unordered_map<ObjectGuid, uint32> modRptiTimers;

class mod_reward_time_played_conf : public WorldScript
{
public:
    mod_reward_time_played_conf() : WorldScript("mod_reward_time_played_conf", {}) { }

    // Load Configuration Settings
    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        modRptiEnabled = sConfigMgr->GetOption<bool>("RewardPlayedTime.Enable", true);
        modRptiAnnounce = sConfigMgr->GetOption<bool>("RewardPlayedTime.Announce", true);
        modRptiAddToBag = sConfigMgr->GetOption<bool>("RewardPlayedTime.AddToBag", true);
        modRptiRewardIntervalMinutes = sConfigMgr->GetOption<uint32>("RewardPlayedTime.RewardInterval", 3600);

        // Get reward list
        std::string itemList = sConfigMgr->GetOption<std::string>("RewardPlayedTime.RewardItems", "");
        std::stringstream ss(itemList);
        std::string token;
        modRptiItems.clear();
        while (std::getline(ss, token, ','))
        {
            modRptiItems.push_back(std::stoul(token));
        }
        LOG_INFO("module", "[RewardPlayedTime]: Loaded " + std::to_string(modRptiItems.size()) + " rewards.");
    }
};

class RewardPlayedTime : public PlayerScript
{
public:
    RewardPlayedTime() : PlayerScript("RewardPlayedTime") { }

    std::string mail_subject = "RewardPlayedTime";
    std::string mail_body = "축하합니다! 접속 보상이 지급됐습니다.";

    void OnPlayerLogin(Player* player) override
    {
        if (!modRptiEnabled)
        {
            return;
        }
        if (modRptiAnnounce)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("RPT Start");
        }

        modRptiTimers[player->GetGUID()] = 0;
    }

    void OnPlayerBeforeUpdate(Player* player, uint32 p_time) override
    {
        if (!modRptiEnabled)
        {
            return;
        }

        uint32 intervalMs = modRptiRewardIntervalMinutes * SECOND * IN_MILLISECONDS;

        auto player_timer = modRptiTimers.find(player->GetGUID());
        if (player_timer == modRptiTimers.end())
        {
            return; // player not tracked
        }
        if (player->isAFK())
        {
            return;
        }

        player_timer->second += p_time;

        if (player_timer->second < intervalMs)
        {
            return;
        }
        if (modRptiItems.empty())
        {
            LOG_WARN("module", "[RewardPlayedTime]: RewardItems list is empty. Check your config!");
            return; // no items configured
        }

        uint32 rewardItemId = RollReward();
        SendRewardToPlayer(player, rewardItemId, 1);

        player_timer->second = 0; // reset timer
    }

    void OnPlayerLogout(Player* player) override
    {
        if (!modRptiEnabled) {
            return;
        }

        modRptiTimers.erase(player->GetGUID());
    }

    uint32 RollReward()
    {
        int32 roll = urand(0, modRptiItems.size() - 1);
        return modRptiItems[roll];
    }

    void SendRewardToPlayer(Player* receiver, uint32 itemId, uint32 count)
    {
        if (!ValidateItemId(itemId, count))
        {
            return;
        }

        ChatHandler(receiver->GetSession()).PSendSysMessage("[|cff4CFF00카라잔 시스템 공지|r] 축하드립니다. 장기 접속 보상이 지급됐습니다.");

        if (modRptiAddToBag) {
            if (receiver->IsInWorld() && receiver->AddItem(itemId, 1))
            {
                return;
            }
            ChatHandler(receiver->GetSession()).PSendSysMessage("[|cff4CFF00카라잔 시스템 공지|r] 걱정안하셔도 됩니다. 우편으로 지급도비니다.");
        } else {
            ChatHandler(receiver->GetSession()).PSendSysMessage("[|cff4CFF00카라잔 시스템 공지|r] 보상이 우편으로 지급됐습니다.");
        }

        MailDraft draft(mail_subject, mail_body);
        CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

        if (Item* item = Item::CreateItem(itemId, count))
        {
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
        draft.SendMailTo(trans, MailReceiver(receiver), MailSender(receiver));

        CharacterDatabase.CommitTransaction(trans);
    }

    bool ValidateItemId(uint32 itemId, uint32 count)
    {
        ItemTemplate const* item_template = sObjectMgr->GetItemTemplate(itemId);
        if (!item_template)
        {
            LOG_ERROR("module", "[RewardPlayedTime]: The itemId is invalid: {}", itemId);
            return false;
        }
        if (count < 1 || (item_template->MaxCount > 0 && count > uint32(item_template->MaxCount)))
        {
            LOG_ERROR("module", "[RewardPlayedTime]: The item count is invalid: {} : {}", itemId, count);
            return false;
        }

        return true;
    }
};

void AddRewardPlayedTimeScripts()
{
    new mod_reward_time_played_conf();
    new RewardPlayedTime();
}
