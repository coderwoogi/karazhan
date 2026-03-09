#ifndef RANDOM_QUEST_SYSTEM_H
#define RANDOM_QUEST_SYSTEM_H

#include "Define.h"
#include "ObjectGuid.h"
#include <vector>
#include <unordered_map>
#include <string>

class Player;
class Quest;
class Creature;

constexpr uint32 NPC_RANDOM_QUEST_GIVER = 190017;
constexpr uint32 NPC_RANDOM_QUEST_COMPLETER = 190018;

struct DailyQuestState
{
    uint32 lastResetTime;
    uint8 questsGivenToday;
    uint8 questsCompletedToday;
    std::vector<uint32> givenQuests;
    std::vector<uint32> completedQuests;
};

class RandomQuestSystem
{
public:
    static RandomQuestSystem* instance();

    void Initialize();
    void Shutdown();

    void HandleQuestGiverInteraction(Player* player);
    void HandleQuestCompleterInteraction(Player* player);

    void HandleQuestReward(Player* player, Quest const* quest);
    void HandleQuestAbandon(Player* player, uint32 questId);

    // 추가: 플레이어 일일 상태 조회
    uint8 GetPlayerDailyQuestCount(Player* player);
    uint8 GetPlayerDailyQuestCompletedCount(Player* player);
    void ShowDailyStatusGossip(Player* player, Creature* creature);

private:
    RandomQuestSystem();
    ~RandomQuestSystem();

    bool ShouldResetDailyState(uint32 lastResetTime);
    void ResetPlayerDailyState(Player* player);
    void LoadPlayerDailyState(Player* player, DailyQuestState& state);
    void SavePlayerDailyState(Player* player, DailyQuestState const& state);

    bool CanGiveQuest(Player* player, DailyQuestState const& state);
    uint32 SelectRandomQuest(Player* player, DailyQuestState const& state);
    void GiveQuestToPlayer(Player* player, uint32 questId, DailyQuestState& state);

    void LogQuestGive(Player* player, uint32 questId, uint32 npcEntry);
    void LogDailyReset(Player* player);
    void LogQuestDenied(Player* player, std::string const& reason);

    uint32 GetCurrentResetPeriod();
    std::string GetResetPeriodString(uint32 timestamp);

    static constexpr uint8 MAX_DAILY_QUESTS = 3;
    static constexpr uint32 QUEST_POOL_START = 900002;
    static constexpr uint32 QUEST_POOL_END = 900013;
    static constexpr uint8 RESET_HOUR = 6;

    std::unordered_map<uint64, DailyQuestState> _playerStateCache;
};

#define sRandomQuestSystem RandomQuestSystem::instance()

#endif
