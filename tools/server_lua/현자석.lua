local ITEM_ID = 600000
local PREFIX = "HERO_STONE_UI"
local ADDON_EVENT_ON_MESSAGE = 30
local CHAT_MSG_WHISPER = 7
local MAX_LEVEL = 80
local HERO_STONE_MAX_LEVEL_BUFF_COST = 500000

local HEROIC_DUNGEON_MAPS = {
    33, 34, 36, 43, 47, 48, 70, 90, 109, 129, 169, 189, 209, 229, 230,
    269, 289, 309, 329, 349, 389, 409, 429, 469, 489, 509, 529, 531, 532,
    534, 540, 542, 543, 544, 545, 546, 547, 548, 550, 552, 553, 554, 555,
    556, 557, 558, 559, 560, 562, 564, 565, 566, 568, 572, 574, 575, 576,
    578, 580, 585, 595, 599, 600, 601, 602, 604, 607, 608, 619
}

local ALL_DUNGEON_MAPS = {
    30, 33, 34, 36, 43, 47, 48, 70, 90, 109, 129, 169, 189, 209, 229, 230,
    269, 289, 309, 329, 349, 389, 409, 429, 469, 489, 509, 529, 531, 532,
    534, 540, 542, 543, 544, 545, 546, 547, 548, 550, 552, 553, 554, 555,
    556, 557, 558, 559, 560, 562, 564, 565, 566, 568, 572, 574, 575, 576,
    578, 580, 585, 595, 599, 600, 601, 602, 604, 607, 608, 619
}

local function SafeText(value)
    local text = tostring(value or "")
    text = text:gsub("\t", " ")
    text = text:gsub("\r", " ")
    text = text:gsub("\n", " ")
    return text
end

local function SendUi(player, ...)
    if not player then
        return
    end

    local parts = { ... }
    for index = 1, #parts do
        parts[index] = SafeText(parts[index])
    end

    local payload = table.concat(parts, "\t")
    player:SendAddonMessage(PREFIX, payload, CHAT_MSG_WHISPER, player)
end

local function GetEnhancedStoneSubscription(player)
    if not player then
        return false, 0, ""
    end

    local accountId = player:GetAccountId()
    if not accountId or accountId <= 0 then
        return false, 0, ""
    end

    local query = string.format([[
        SELECT
            GREATEST(TIMESTAMPDIFF(DAY, NOW(), expires_at), 0) AS remain_days,
            DATE_FORMAT(expires_at, '%%Y-%%m-%%d %%H:%%i:%%s') AS expires_at
        FROM update.web_feature_subscriptions
        WHERE user_id = %d
          AND feature_code IN (
              'enhanced_enchant_stone',
              'shining_hero_stone',
              'bright_hero_stone',
              'hero_stone',
              'enhanced_stone'
          )
          AND expires_at > NOW()
        ORDER BY expires_at DESC
        LIMIT 1
    ]], accountId)

    local row = AuthDBQuery(query)
    if not row then
        return false, 0, ""
    end

    local remainDays = row:GetInt32(0) or 0
    local expiresAt = row:GetString(1) or ""
    return remainDays > 0, remainDays, expiresAt
end

local function OpenMailbox(player)
    local ok = pcall(function()
        player:SendShowMailBox(player:GetGUIDLow())
    end)

    if not ok then
        player:SendBroadcastMessage("우편함을 열 수 없습니다. 잠시 후 다시 시도해 주세요.")
    end
end

local function DungeonHeroic(player)
    if player:IsInCombat() then
        player:SendBroadcastMessage("전투 중에는 사용할 수 없습니다.")
        return
    end

    for _, mapId in ipairs(HEROIC_DUNGEON_MAPS) do
        player:UnbindInstance(mapId, 1)
    end

    player:SendBroadcastMessage("현자석: 영웅 던전이 초기화되었습니다.")
end

local function DungeonAll(player)
    if player:IsInCombat() then
        player:SendBroadcastMessage("전투 중에는 사용할 수 없습니다.")
        return
    end

    for _, mapId in ipairs(ALL_DUNGEON_MAPS) do
        player:UnbindInstance(mapId)
        player:UnbindInstance(mapId, 1)
    end

    player:SendBroadcastMessage("현자석: 모든 던전이 초기화되었습니다.")
end

local function PayBuff(player)
    if player:GetCoinage() < 100000 then
        player:SendBroadcastMessage("골드가 부족합니다.")
        return
    end

    player:ModifyMoney(-100000)
    player:AddAura(48074, player)
    player:AddAura(48170, player)
    player:AddAura(43002, player)
    player:AddAura(25898, player)
    player:AddAura(48470, player)
    player:AddAura(48162, player)
    player:AddAura(48942, player)
    player:AddAura(75447, player)
    player:SendBroadcastMessage("현자석: 유료 버프가 적용되었습니다.")
end

local function FreeBuff(player)
    if player:GetLevel() >= MAX_LEVEL then
        if player:GetCoinage() < HERO_STONE_MAX_LEVEL_BUFF_COST then
            player:SendBroadcastMessage(
                "50골드가 필요합니다. 소지금이 부족합니다.")
            return
        end

        player:ModifyMoney(-HERO_STONE_MAX_LEVEL_BUFF_COST)
    end

    player:AddAura(48074, player)
    player:AddAura(48170, player)
    player:AddAura(43002, player)
    player:AddAura(25898, player)
    player:AddAura(48470, player)
    player:AddAura(48162, player)
    player:AddAura(48942, player)
    player:AddAura(75447, player)
    if player:GetLevel() >= MAX_LEVEL then
        player:SendBroadcastMessage(
            "현자석: 유료 버프가 적용되었습니다. (50골드 소모)")
    else
        player:SendBroadcastMessage("현자석: 무료 버프가 적용되었습니다.")
    end
end

local function BuildMenuItems(player, isSubscriber, remainDays)
    local items = {
        {
            id = 1,
            label = "달라란 이동",
            desc = "달라란 상인 지구로 즉시 이동합니다.",
            icon = "Interface\\ICONS\\INV_Misc_Map09",
        }
    }

    if isSubscriber then
        table.insert(items, {
            id = 106,
            label = "모든 던전 초기화",
            desc = "일반과 영웅 던전을 모두 초기화합니다.",
            icon = "Interface\\ICONS\\Ability_Rogue_FeignDeath",
        })
        table.insert(items, {
            id = 103,
            label = "유료 버프",
            desc = "10골드를 소모하고 주요 버프를 받습니다.",
            icon = "Interface\\ICONS\\Ability_Mage_MissileBarrage",
        })
        table.insert(items, {
            id = 100,
            label = "개인 은행",
            desc = "개인 은행 창을 엽니다.",
            icon = "Interface\\ICONS\\INV_Crate_03",
        })
        table.insert(items, {
            id = 107,
            label = "우편함",
            desc = "현 위치에서 우편함을 엽니다.",
            icon = "Interface\\ICONS\\INV_Letter_15",
        })
    else
        table.insert(items, {
            id = 102,
            label = "영웅 던전 초기화",
            desc = "영웅 난이도 던전만 초기화합니다.",
            icon = "Interface\\ICONS\\Ability_Rogue_FeignDeath",
        })
        table.insert(items, {
            id = 105,
            label = player:GetLevel() >= MAX_LEVEL and "유료 버프(50골드)" or
                "무료 버프",
            desc = player:GetLevel() >= MAX_LEVEL and
                "만렙 캐릭터는 50골드를 소모하고 주요 버프를 받습니다." or
                "79레벨 이하 캐릭터만 무료로 사용할 수 있습니다.",
            icon = "Interface\\ICONS\\Ability_Mage_MissileBarrage",
        })
    end

    if player:IsGM() then
        table.insert(items, {
            id = 300,
            label = "이벤트 스탯",
            desc = "힘 수치를 1000으로 설정합니다.",
            icon = "Interface\\ICONS\\Ability_Warrior_BattleShout",
        })
        table.insert(items, {
            id = 301,
            label = "Lua 재로드",
            desc = "ALE 스크립트를 다시 불러옵니다.",
            icon = "Interface\\ICONS\\INV_Misc_EngGizmos_17",
        })
    end

    return items
end

local function OpenHeroStoneUi(player)
    if not player then
        return
    end

    local isSubscriber, remainDays = GetEnhancedStoneSubscription(player)
    local subtitle
    if isSubscriber then
        subtitle = string.format("구독 영웅석 · 남은 기간 %d일", remainDays)
    else
        subtitle = "일반 영웅석"
    end

    local body = "원하는 기능을 선택하세요. 구독 상태에 따라 사용할 수 있는 기능이 달라집니다."
    local items = BuildMenuItems(player, isSubscriber, remainDays)

    SendUi(player, "CLEAR")
    SendUi(
        player,
        "HEADER",
        "영웅석",
        subtitle,
        "Interface\\AddOns\\HeroStoneUI\\Art\\INV_Misc_Rune_100.tga"
    )
    SendUi(player, "BODY", body)
    SendUi(player, "SECTION", "사용 가능한 기능")
    SendUi(player, "CONTROL", "닫기", "새로고침")

    for _, item in ipairs(items) do
        SendUi(player, "ITEM", item.id, item.label, item.desc, item.icon)
    end

    SendUi(player, "SHOW")
end

local function HandleAction(player, actionId)
    if not player or not actionId then
        return
    end

    local isSubscriber = GetEnhancedStoneSubscription(player)

    if player:IsInCombat() then
        player:SendBroadcastMessage("전투 중에는 현자석을 사용할 수 없습니다.")
        return
    end

    if actionId == 1 then
        player:Teleport(571, 5832.27, 494.367, 657.71, 4.51377)
        SendUi(player, "CLOSE")
        return
    end

    if actionId == 100 then
        if not isSubscriber then
            player:SendBroadcastMessage("구독 영웅석 전용 기능입니다.")
            return
        end
        player:SendShowBank(player)
        return
    end

    if actionId == 102 then
        DungeonHeroic(player)
        return
    end

    if actionId == 103 then
        if not isSubscriber then
            player:SendBroadcastMessage("구독 영웅석 전용 기능입니다.")
            return
        end
        PayBuff(player)
        return
    end

    if actionId == 105 then
        FreeBuff(player)
        return
    end

    if actionId == 106 then
        if not isSubscriber then
            player:SendBroadcastMessage("구독 영웅석 전용 기능입니다.")
            return
        end
        DungeonAll(player)
        return
    end

    if actionId == 107 then
        if not isSubscriber then
            player:SendBroadcastMessage("구독 영웅석 전용 기능입니다.")
            return
        end
        OpenMailbox(player)
        return
    end

    if actionId == 300 then
        if player:IsGM() then
            player:SetStat(STAT_STRENGTH, 1000)
            player:SendBroadcastMessage("현자석: 테스트 스탯을 적용했습니다.")
        end
        return
    end

    if actionId == 301 then
        if player:IsGM() then
            Global:ReloadALE()
            player:SendBroadcastMessage("현자석: Lua 스크립트를 다시 불러왔습니다.")
        end
        return
    end
end

local function OnGossipHello(event, player, object)
    if not player then
        return
    end

    OpenHeroStoneUi(player)
    player:GossipComplete()
end

local function OnHeroStoneAddonMessage(event, sender, msgType, prefix, msg, target)
    if prefix ~= PREFIX or not sender then
        return
    end

    local command, argument = string.match(msg or "", "^([^\t]+)\t?(.*)$")
    if command == "ACT" then
        HandleAction(sender, tonumber(argument))
        return false
    end

    if command == "REFRESH" then
        OpenHeroStoneUi(sender)
        return false
    end
end

RegisterItemGossipEvent(ITEM_ID, 1, OnGossipHello)
RegisterServerEvent(ADDON_EVENT_ON_MESSAGE, OnHeroStoneAddonMessage)
