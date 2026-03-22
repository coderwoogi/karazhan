local AIO = AIO or require("AIO")

if not AIO.IsMainState() then
    return
end

local BonusMissionHandlers = AIO.AddHandlers("KarazhanBonusMission", {})

local function BuildMissionState(player)
    if not player then
        return nil
    end

    local mapId = player:GetMapId()
    local instanceId = player:GetInstanceId()
    if not mapId or not instanceId or instanceId == 0 then
        return nil
    end

    local query = WorldDBQuery(string.format(
        "SELECT theme_name, theme_key, title, target_label, target_count, current_count, time_limit_sec, start_time, expire_time, completed, failed, briefing, announcement, mission_type FROM instance_bonus_mission_live WHERE instance_id = %u AND map_id = %u",
        instanceId,
        mapId
    ))

    if not query then
        return nil
    end

    local expireTime = query:GetUInt32(8)
    local remainingSec = 0
    if expireTime > 0 then
        local now = os.time()
        if expireTime > now then
            remainingSec = expireTime - now
        end
    end

    local targetCount = query:GetUInt32(4)
    local currentCount = query:GetUInt32(5)
    local progressText = string.format("%u / %u", currentCount, targetCount)
    local statusText = "진행 중"
    local completed = query:GetUInt32(9) == 1
    local failed = query:GetUInt32(10) == 1

    if completed then
        statusText = "완료"
    elseif failed then
        statusText = "실패"
    end

    return {
        active = true,
        map_id = mapId,
        instance_id = instanceId,
        theme_name = query:GetString(0) or "",
        theme_key = query:GetString(1) or "",
        title = query:GetString(2) or "",
        target_label = query:GetString(3) or "",
        target_count = targetCount,
        current_count = currentCount,
        time_limit_sec = query:GetUInt32(6),
        start_time = query:GetUInt32(7),
        expire_time = expireTime,
        remaining_sec = remainingSec,
        completed = completed,
        failed = failed,
        briefing = query:GetString(11) or "",
        announcement = query:GetString(12) or "",
        mission_type = query:GetUInt32(13),
        progress_text = progressText,
        status_text = statusText,
    }
end

function BonusMissionHandlers.RequestState(player)
    local state = BuildMissionState(player)
    if not state then
        AIO.Handle(player, "KarazhanBonusMission", "HideFrame")
        return
    end

    AIO.Handle(player, "KarazhanBonusMission", "ReceiveState", state)
end