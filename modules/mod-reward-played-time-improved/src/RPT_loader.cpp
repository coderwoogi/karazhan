/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

void AddRewardPlayedTimeScripts();

void Addmod_reward_played_time_improvedScripts()
{
    AddRewardPlayedTimeScripts();
    // PlaytimeRewardLog는 유틸리티 클래스이므로 별도 등록 불필요
}

