#include "RandomQuestSystem.h"
#include "ScriptedGossip.h"
#include "QuestDef.h"
#include "Creature.h"
#include "Player.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Chat.h"
#include "ObjectMgr.h"
#include "World.h"
#include "GameTime.h"
#include "Random.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

RandomQuestSystem* RandomQuestSystem::instance()
{
    static RandomQuestSystem instance;
    return &instance;
}

RandomQuestSystem::RandomQuestSystem() = default;
RandomQuestSystem::~RandomQuestSystem() = default;

void RandomQuestSystem::Initialize()
{
}

void RandomQuestSystem::HandleQuestReward(Player* player, Quest const* quest)
{
    // 랜덤 퀘스트 범위인지 확인
    uint32 questId = quest->GetQuestId();
    if (questId < QUEST_POOL_START || questId > QUEST_POOL_END)
        return;

    DailyQuestState state;
    LoadPlayerDailyState(player, state);

    // 리셋 체크
    if (ShouldResetDailyState(state.lastResetTime))
    {
        ResetPlayerDailyState(player);
        LoadPlayerDailyState(player, state);
    }

    state.questsCompletedToday++;
    state.completedQuests.push_back(questId);
    SavePlayerDailyState(player, state);

    LOG_INFO("module.randomquest", "Player {} completed random quest {}. Today: {}/{}",
        player->GetName(), questId, state.questsCompletedToday, MAX_DAILY_QUESTS);

    if (state.questsCompletedToday == MAX_DAILY_QUESTS)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFF00FF00[축하합니다] 오늘의 랜덤 퀘스트 3개를 모두 완료하셨습니다!|r"
        );
        // 여기에 추가 보상 로직을 넣을 수 있습니다.
        // 예: player->AddItem(ITEM_ID, COUNT);
    }

    ChatHandler(player->GetSession()).PSendSysMessage(
        "|cFF00FF00랜덤 퀘스트 완료! (오늘: {}/{})|r",
        state.questsCompletedToday, MAX_DAILY_QUESTS
    );
}

void RandomQuestSystem::HandleQuestAbandon(Player* player, uint32 questId)
{
    if (questId < QUEST_POOL_START || questId > QUEST_POOL_END)
        return;

    // 포기 시 로직 (필요하다면 구현)
    // 예: 횟수 차감 등. 현재는 로그만 남김.
    LOG_INFO("module.randomquest", "Player {} abandoned random quest {}.", player->GetName(), questId);
}

void RandomQuestSystem::Shutdown()
{
    _playerStateCache.clear();
}

uint32 RandomQuestSystem::GetCurrentResetPeriod()
{
    time_t now = GameTime::GetGameTime().count();
    struct tm timeinfo;

#ifdef _WIN32
    localtime_s(&timeinfo, &now);
#else
    localtime_r(&now, &timeinfo);
#endif

    if (timeinfo.tm_hour < RESET_HOUR)
    {
        timeinfo.tm_mday -= 1;
    }

    timeinfo.tm_hour = RESET_HOUR;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;

    return static_cast<uint32>(mktime(&timeinfo));
}

std::string RandomQuestSystem::GetResetPeriodString(uint32 timestamp)
{
    time_t t = timestamp;
    struct tm timeinfo;

#ifdef _WIN32
    localtime_s(&timeinfo, &t);
#else
    localtime_r(&t, &timeinfo);
#endif

    std::ostringstream oss;
    oss << std::put_time(&timeinfo, "%Y-%m-%d_%H:%M");
    return oss.str();
}

bool RandomQuestSystem::ShouldResetDailyState(uint32 lastResetTime)
{
    uint32 currentResetPeriod = GetCurrentResetPeriod();
    return lastResetTime < currentResetPeriod;
}

void RandomQuestSystem::ResetPlayerDailyState(Player* player)
{
    uint64 playerGuid = player->GetGUID().GetCounter();
    uint32 currentResetPeriod = GetCurrentResetPeriod();

    CharacterDatabase.Execute(
        "UPDATE character_random_quest_state SET "
        "last_reset_time = {}, quests_given_today = 0, quests_completed_today = 0, given_quest_ids = '' "
        "WHERE player_guid = {}",
        currentResetPeriod, playerGuid
    );

    DailyQuestState& state = _playerStateCache[playerGuid];
    state.lastResetTime = currentResetPeriod;
    state.questsGivenToday = 0;
    state.questsCompletedToday = 0;
    state.givenQuests.clear();

    LogDailyReset(player);

    LOG_INFO("module.randomquest",
        "Daily state reset for player {} (GUID: {}) at reset period {}",
        player->GetName(), playerGuid, GetResetPeriodString(currentResetPeriod));
}

void RandomQuestSystem::LoadPlayerDailyState(Player* player, DailyQuestState& state)
{
    uint64 playerGuid = player->GetGUID().GetCounter();

    auto itr = _playerStateCache.find(playerGuid);
    if (itr != _playerStateCache.end())
    {
        state = itr->second;
        return;
    }

    QueryResult result = CharacterDatabase.Query(
        "SELECT last_reset_time, quests_given_today, quests_completed_today, given_quest_ids, completed_quest_ids "
        "FROM character_random_quest_state WHERE player_guid = {}", playerGuid
    );

    if (!result)
    {
        uint32 currentResetPeriod = GetCurrentResetPeriod();

        CharacterDatabase.Execute(
            "INSERT INTO character_random_quest_state "
            "(player_guid, last_reset_time, quests_given_today, quests_completed_today, given_quest_ids, completed_quest_ids) "
            "VALUES ({}, {}, 0, 0, '', '')",
            playerGuid, currentResetPeriod
        );

        state.lastResetTime = currentResetPeriod;
        state.questsGivenToday = 0;
        state.questsCompletedToday = 0;
        state.givenQuests.clear();
        state.completedQuests.clear();
    }
    else
    {
        Field* fields = result->Fetch();
        state.lastResetTime = fields[0].Get<uint32>();
        state.questsGivenToday = fields[1].Get<uint8>();
        state.questsCompletedToday = fields[2].Get<uint8>();

        std::string givenQuestIdsStr = fields[3].Get<std::string>();
        if (!givenQuestIdsStr.empty())
        {
            std::istringstream iss(givenQuestIdsStr);
            std::string token;
            while (std::getline(iss, token, ','))
            {
                state.givenQuests.push_back(std::stoul(token));
            }
        }

        std::string completedQuestIdsStr = fields[4].Get<std::string>();
        if (!completedQuestIdsStr.empty())
        {
            std::istringstream iss(completedQuestIdsStr);
            std::string token;
            while (std::getline(iss, token, ','))
            {
                state.completedQuests.push_back(std::stoul(token));
            }
        }
    }

    _playerStateCache[playerGuid] = state;
}

void RandomQuestSystem::SavePlayerDailyState(Player* player, DailyQuestState const& state)
{
    uint64 playerGuid = player->GetGUID().GetCounter();

    std::ostringstream oss;
    for (size_t i = 0; i < state.givenQuests.size(); ++i)
    {
        if (i > 0) oss << ",";
        oss << state.givenQuests[i];
    }
    std::string givenQuestIdsStr = oss.str();

    std::ostringstream oss2;
    for (size_t i = 0; i < state.completedQuests.size(); ++i)
    {
        if (i > 0) oss2 << ",";
        oss2 << state.completedQuests[i];
    }
    std::string completedQuestIdsStr = oss2.str();

    CharacterDatabase.Execute(
        "UPDATE character_random_quest_state SET "
        "last_reset_time = {}, quests_given_today = {}, quests_completed_today = {}, given_quest_ids = '{}', completed_quest_ids = '{}' "
        "WHERE player_guid = {}",
        state.lastResetTime, state.questsGivenToday, state.questsCompletedToday, givenQuestIdsStr, completedQuestIdsStr, playerGuid
    );

    _playerStateCache[playerGuid] = state;
}

bool RandomQuestSystem::CanGiveQuest(Player* player, DailyQuestState const& state)
{
    if (state.questsGivenToday >= MAX_DAILY_QUESTS)
    {
        LogQuestDenied(player, "Daily limit reached (3/3)");
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFF0000오늘은 이미 최대 {}개의 퀘스트를 받으셨습니다. 내일 새벽 6시에 다시 오세요.|r",
            MAX_DAILY_QUESTS
        );
        return false;
    }

    return true;
}

uint32 RandomQuestSystem::SelectRandomQuest(Player* player, DailyQuestState const& state)
{
    std::vector<uint32> availableQuests;

    for (uint32 questId = QUEST_POOL_START; questId <= QUEST_POOL_END; ++questId)
    {
        if (std::find(state.givenQuests.begin(), state.givenQuests.end(), questId) != state.givenQuests.end())
            continue;

        if (!sObjectMgr->GetQuestTemplate(questId))
            continue;

        availableQuests.push_back(questId);
    }

    if (availableQuests.empty())
    {
        LogQuestDenied(player, "No available quests in pool");
        ChatHandler(player->GetSession()).SendSysMessage(
            "|cFFFF0000현재 지급 가능한 퀘스트가 없습니다.|r"
        );
        return 0;
    }

    uint32 randomIndex = urand(0, availableQuests.size() - 1);
    return availableQuests[randomIndex];
}

void RandomQuestSystem::GiveQuestToPlayer(Player* player, uint32 questId, DailyQuestState& state)
{
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
    {
        LOG_ERROR("module.randomquest", "Invalid quest ID: {}", questId);
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFF0000[오류] 퀘스트 ID {}가 존재하지 않습니다.|r", questId
        );
        return;
    }

    // 디버그: 퀘스트 정보 로그
    LOG_INFO("module.randomquest",
        "Attempting to give quest {} to player {} (Level: {}, Class: {}, Race: {})",
        questId, player->GetName(), player->GetLevel(), player->getClass(), player->getRace()
    );

    // 상세 조건 체크
    if (player->GetQuestStatus(questId) != QUEST_STATUS_NONE)
    {
        LogQuestDenied(player, "Quest already in progress or completed");
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFF0000이미 진행 중이거나 완료한 퀘스트입니다.|r"
        );
        return;
    }

    if (player->CanTakeQuest(quest, false))
    {
        player->AddQuestAndCheckCompletion(quest, nullptr);

        state.questsGivenToday++;
        state.givenQuests.push_back(questId);
        SavePlayerDailyState(player, state);

        LogQuestGive(player, questId, NPC_RANDOM_QUEST_GIVER);

        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFF00FF00랜덤 퀘스트가 지급되었습니다! (오늘: {}/{})|r",
            state.questsGivenToday, MAX_DAILY_QUESTS
        );

        LOG_INFO("module.randomquest",
            "Quest {} given to player {} (GUID: {}) - Daily count: {}/{}",
            questId, player->GetName(), player->GetGUID().GetCounter(),
            state.questsGivenToday, MAX_DAILY_QUESTS
        );
    }
    else
    {
        // 상세 실패 원인 로그
        LOG_ERROR("module.randomquest",
            "CanTakeQuest failed for quest {} - Player: {}, Level: {}/{}, Class: {}, Race: {}",
            questId, player->GetName(),
            player->GetLevel(), quest->GetMinLevel(),
            player->getClass(), player->getRace()
        );

        LogQuestDenied(player, "CanTakeQuest check failed");

        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFF0000퀘스트 조건을 만족하지 못했습니다.|r"
        );
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFFFF00- 퀘스트 ID: {}|r", questId
        );
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cFFFFFF00- 필요 레벨: {} (현재: {})|r",
            quest->GetMinLevel(), player->GetLevel()
        );
    }
}

void RandomQuestSystem::HandleQuestGiverInteraction(Player* player)
{
    if (!player)
        return;

    uint32 const maxPlayerLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    if (player->GetLevel() < maxPlayerLevel)
    {
        ChatHandler(player->GetSession()).PSendSysMessage(
            "랜덤 퀘스트는 만렙({}) 달성 후 이용할 수 있습니다.", maxPlayerLevel);
        return;
    }

    DailyQuestState state;
    LoadPlayerDailyState(player, state);

    if (ShouldResetDailyState(state.lastResetTime))
    {
        ResetPlayerDailyState(player);
        LoadPlayerDailyState(player, state);
    }

    if (!CanGiveQuest(player, state))
        return;

    uint32 questId = SelectRandomQuest(player, state);
    if (questId == 0)
        return;

    GiveQuestToPlayer(player, questId, state);
}

void RandomQuestSystem::HandleQuestCompleterInteraction(Player* player)
{
    if (!player)
        return;

    ChatHandler(player->GetSession()).SendSysMessage(
        "퀘스트 완료 NPC입니다. (기본 퀘스트 UI 사용)"
    );
}

void RandomQuestSystem::LogQuestGive(Player* player, uint32 questId, uint32 npcEntry)
{
    uint64 playerGuid = player->GetGUID().GetCounter();
    uint32 giveTime = static_cast<uint32>(GameTime::GetGameTime().count());
    std::string resetPeriod = GetResetPeriodString(GetCurrentResetPeriod());

    CharacterDatabase.Execute(
        "INSERT INTO character_random_quest_log "
        "(player_guid, quest_id, npc_entry, give_time, reset_period, action) "
        "VALUES ({}, {}, {}, {}, '{}', 'GIVE')",
        playerGuid, questId, npcEntry, giveTime, resetPeriod
    );

    LOG_INFO("module.randomquest",
        "[QUEST_GIVE] Player: {} (GUID: {}), Quest: {}, NPC: {}, Period: {}",
        player->GetName(), playerGuid, questId, npcEntry, resetPeriod
    );
}

void RandomQuestSystem::LogDailyReset(Player* player)
{
    uint64 playerGuid = player->GetGUID().GetCounter();
    uint32 resetTime = static_cast<uint32>(GameTime::GetGameTime().count());
    std::string resetPeriod = GetResetPeriodString(GetCurrentResetPeriod());

    CharacterDatabase.Execute(
        "INSERT INTO character_random_quest_log "
        "(player_guid, quest_id, npc_entry, give_time, reset_period, action) "
        "VALUES ({}, 0, 0, {}, '{}', 'RESET')",
        playerGuid, resetTime, resetPeriod
    );

    LOG_INFO("module.randomquest",
        "[DAILY_RESET] Player: {} (GUID: {}), Period: {}",
        player->GetName(), playerGuid, resetPeriod
    );
}

void RandomQuestSystem::LogQuestDenied(Player* player, std::string const& reason)
{
    uint64 playerGuid = player->GetGUID().GetCounter();
    uint32 denyTime = static_cast<uint32>(GameTime::GetGameTime().count());
    std::string resetPeriod = GetResetPeriodString(GetCurrentResetPeriod());

    CharacterDatabase.Execute(
        "INSERT INTO character_random_quest_log "
        "(player_guid, quest_id, npc_entry, give_time, reset_period, action, deny_reason) "
        "VALUES ({}, 0, {}, {}, '{}', 'DENY', '{}')",
        playerGuid, NPC_RANDOM_QUEST_GIVER, denyTime, resetPeriod, reason
    );

    LOG_WARN("module.randomquest",
        "[QUEST_DENIED] Player: {} (GUID: {}), Reason: {}, Period: {}",
        player->GetName(), playerGuid, reason, resetPeriod
    );
}

// 파일 끝에 추가

uint8 RandomQuestSystem::GetPlayerDailyQuestCount(Player* player)
{
    if (!player)
        return 0;

    DailyQuestState state;
    LoadPlayerDailyState(player, state);

    // 리셋 체크
    if (ShouldResetDailyState(state.lastResetTime))
    {
        return 0; // 리셋 대상이면 0 반환
    }

    return state.questsGivenToday;
}

uint8 RandomQuestSystem::GetPlayerDailyQuestCompletedCount(Player* player)
{
    if (!player)
        return 0;

    DailyQuestState state;
    LoadPlayerDailyState(player, state);

    // 리셋 체크
    if (ShouldResetDailyState(state.lastResetTime))
    {
        return 0; // 리셋 대상이면 0 반환
    }

    return state.questsCompletedToday;
}

void RandomQuestSystem::ShowDailyStatusGossip(Player* player, Creature* creature)
{
    if (!player || !creature)
        return;

    ClearGossipMenuFor(player);

    DailyQuestState state;
    LoadPlayerDailyState(player, state);

    // 리셋 체크
    if (ShouldResetDailyState(state.lastResetTime))
    {
        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "오늘은 아직 퀘스트를 받지 않으셨습니다.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return;
    }

    // 헤더 (Gossip Menu Title로 대체하거나 첫 항목으로 표시)
    // AddGossipItemFor(player, GOSSIP_ICON_CHAT, "=== 오늘의 랜덤 퀘스트 현황 ===", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

    if (!state.givenQuests.empty())
    {
        for (uint32 questId : state.givenQuests)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (quest)
            {
                std::string questTitle = quest->GetTitle();
                
                // 상태 확인 로직 개선 (completedQuests 목록 확인)
                bool isCompletedToday = std::find(state.completedQuests.begin(), state.completedQuests.end(), questId) != state.completedQuests.end();
                QuestStatus status = player->GetQuestStatus(questId);

                std::string statusStr;
                uint32 icon = GOSSIP_ICON_DOT;

                if (isCompletedToday)
                {
                    statusStr = "|cFF00FF00[완료]|r"; // 녹색
                    icon = GOSSIP_ICON_INTERACT_1;
                }
                else if (status == QUEST_STATUS_COMPLETE)
                {
                    statusStr = "|cFFFFFF00[보상 대기]|r"; // 노란색
                    icon = GOSSIP_ICON_INTERACT_1;
                }
                else if (status == QUEST_STATUS_INCOMPLETE)
                {
                    statusStr = "|cFFCCCCCC[진행 중]|r"; // 회색
                    icon = GOSSIP_ICON_TALK;
                }
                else // QUEST_STATUS_NONE (포기함 or 시작 안함)
                {
                    statusStr = "|cFFFF0000[포기함]|r"; // 빨간색
                    icon = GOSSIP_ICON_BATTLE;
                }

                std::string menuText = Acore::StringFormat("{} {}", statusStr, questTitle);
                
                // 클릭해도 아무 동작 안하도록 GOSSIP_ACTION_INFO_DEF 사용 (NPC 스크립트에서 처리 안하면 닫힘/무시)
                AddGossipItemFor(player, icon, menuText, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
            }
        }
    }
    else
    {
         AddGossipItemFor(player, GOSSIP_ICON_CHAT, "받은 퀘스트 기록이 없습니다.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);
    }

    // 닫기 버튼 추가
    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "[돌아가기]", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 99);

    SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
}
