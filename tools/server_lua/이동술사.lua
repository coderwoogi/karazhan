local NPC_ENTRY = 190000
local PREFIX = "TELEPORT_MASTER_UI"
local ADDON_EVENT_ON_MESSAGE = 30
local CHAT_MSG_WHISPER = 7
local PAGE_SIZE = 6

local playerState = {}
local npcMeta = nil

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

    player:SendAddonMessage(
        PREFIX,
        table.concat(parts, "\t"),
        CHAT_MSG_WHISPER,
        player
    )
end

local function Trim(text)
    return (text or ""):gsub("^%s+", ""):gsub("%s+$", "")
end

local function DecodeOptionText(text)
    local raw = text or ""
    local icon = raw:match("|T([^:|]+)")
    if icon then
        icon = icon:gsub("/", "\\")
        if not icon:find("^Interface\\") then
            icon = "Interface\\" .. icon
        end
    end

    local cleaned = raw
    cleaned = cleaned:gsub("|T.-|t", "")
    cleaned = cleaned:gsub("|c%x%x%x%x%x%x%x%x", "")
    cleaned = cleaned:gsub("|r", "")
    cleaned = Trim(cleaned)

    return cleaned, icon
end

local function GetPlayerKey(player)
    return player and player:GetGUIDLow() or 0
end

local function GetNpcMeta()
    if npcMeta then
        return npcMeta
    end

    local query = WorldDBQuery(string.format([[
        SELECT name, subname, gossip_menu_id
        FROM creature_template
        WHERE entry = %d
    ]], NPC_ENTRY))

    if not query then
        npcMeta = {
            name = "이동술사",
            subname = "The Karazhan",
            menuId = 0,
        }
        return npcMeta
    end

    npcMeta = {
        name = query:GetString(0) or "이동술사",
        subname = query:GetString(1) or "",
        menuId = query:GetUInt32(2) or 0,
    }

    return npcMeta
end

local function GetMenuOptions(menuId)
    local options = {}
    local query = WorldDBQuery(string.format([[
        SELECT OptionID, OptionText, ActionMenuID
        FROM gossip_menu_option
        WHERE MenuID = %d
        ORDER BY OptionID
    ]], menuId))

    if not query then
        return options
    end

    repeat
        table.insert(options, {
            optionId = query:GetUInt32(0),
            optionText = query:GetString(1) or "",
            actionMenuId = query:GetUInt32(2) or 0,
        })
    until not query:NextRow()

    return options
end

local function GetTeleportAction(menuId, optionId)
    local query = WorldDBQuery(string.format([[
        SELECT action_type, action_param1, target_x, target_y, target_z, target_o
        FROM smart_scripts
        WHERE source_type = 0
          AND entryorguid = %d
          AND event_type = 62
          AND event_param1 = %d
          AND event_param2 = %d
        LIMIT 1
    ]], NPC_ENTRY, menuId, optionId))

    if not query then
        return nil
    end

    return {
        actionType = query:GetUInt32(0) or 0,
        mapId = query:GetUInt32(1) or 0,
        x = query:GetFloat(2) or 0,
        y = query:GetFloat(3) or 0,
        z = query:GetFloat(4) or 0,
        o = query:GetFloat(5) or 0,
    }
end

local function BuildMenuItems(menuId, page)
    local items = {}
    local options = GetMenuOptions(menuId)
    local currentPage = tonumber(page) or 1
    local totalPages = math.max(1, math.ceil(#options / PAGE_SIZE))

    if currentPage < 1 then
        currentPage = 1
    end

    if currentPage > totalPages then
        currentPage = totalPages
    end

    local startIndex = ((currentPage - 1) * PAGE_SIZE) + 1
    local endIndex = math.min(#options, startIndex + PAGE_SIZE - 1)

    if currentPage > 1 then
        table.insert(items, {
            id = string.format("PAGE:%d:%d", menuId, currentPage - 1),
            label = "이전 페이지",
            desc = "현재 목록의 이전 항목을 표시합니다.",
            icon = "Interface\\ICONS\\Ability_Hunter_Pet_Gorilla",
        })
    end

    for index = startIndex, endIndex do
        local option = options[index]
        local label, icon = DecodeOptionText(option.optionText)
        local desc = ""
        local action = "TELEPORT"

        if option.actionMenuId and option.actionMenuId > 0 then
            action = "MENU"
            if option.optionId == 0 then
                desc = "이전 화면으로 돌아갑니다."
            else
                desc = "세부 목적지 목록을 엽니다."
            end
        else
            desc = "선택한 지역으로 즉시 이동합니다."
        end

        table.insert(items, {
            id = string.format("%s:%d:%d", action, menuId, option.optionId),
            label = label,
            desc = desc,
            icon = icon or "Interface\\ICONS\\Spell_Arcane_PortalDalaran",
        })
    end

    if currentPage < totalPages then
        table.insert(items, {
            id = string.format("PAGE:%d:%d", menuId, currentPage + 1),
            label = "다음 페이지",
            desc = "남은 목적지 목록을 계속 표시합니다.",
            icon = "Interface\\ICONS\\INV_Misc_Map_01",
        })
    end

    return items, currentPage, totalPages
end

local function OpenTeleportUi(player, menuId, page)
    local meta = GetNpcMeta()
    local key = GetPlayerKey(player)
    local currentMenuId = menuId or meta.menuId
    local items, currentPage, totalPages = BuildMenuItems(currentMenuId, page)

    playerState[key] = {
        menuId = currentMenuId,
        page = currentPage,
    }

    SendUi(player, "CLEAR")
    SendUi(
        player,
        "HEADER",
        meta.name,
        meta.subname,
        "PORTRAIT_NPC"
    )
    SendUi(
        player,
        "BODY",
        string.format(
            "원하는 목적지를 선택하세요. 하위 분류는 화면 안에서 계속 탐색할 수 있습니다. 현재 %d / %d 페이지입니다.",
            currentPage,
            totalPages
        )
    )
    SendUi(player, "SECTION", "이동 가능한 지역")
    SendUi(player, "CONTROL", "닫기", "새로고침")

    for _, item in ipairs(items) do
        SendUi(player, "ITEM", item.id, item.label, item.desc, item.icon)
    end

    SendUi(player, "SHOW")
end

local function HandleAction(player, payload)
    local action, menuId, optionId = string.match(
        tostring(payload or ""),
        "^([^:]+):(%d+):(%d+)$"
    )
    if not action then
        return
    end

    local menu = tonumber(menuId)
    local option = tonumber(optionId)
    if not menu or not option then
        return
    end

    if action == "PAGE" then
        OpenTeleportUi(player, menu, option)
        return
    end

    local options = GetMenuOptions(menu)
    local selected = nil
    for _, entry in ipairs(options) do
        if entry.optionId == option then
            selected = entry
            break
        end
    end

    if not selected then
        player:SendBroadcastMessage("이동 항목을 찾을 수 없습니다.")
        return
    end

    if selected.actionMenuId and selected.actionMenuId > 0 then
        OpenTeleportUi(player, selected.actionMenuId)
        return
    end

    local teleport = GetTeleportAction(menu, option)
    if not teleport or teleport.actionType ~= 62 then
        player:SendBroadcastMessage("이동 정보를 찾을 수 없습니다.")
        return
    end

    player:Teleport(
        teleport.mapId,
        teleport.x,
        teleport.y,
        teleport.z,
        teleport.o
    )
    SendUi(player, "CLOSE")
end

local function OnGossipHello(event, player, creature)
    if not player or not creature then
        return
    end

    OpenTeleportUi(player)
    player:GossipComplete()
end

local function OnAddonMessage(event, sender, msgType, prefix, msg, target)
    if prefix ~= PREFIX or not sender then
        return
    end

    local command, argument = string.match(msg or "", "^([^\t]+)\t?(.*)$")
    if command == "ACT" then
        HandleAction(sender, argument)
        return false
    end

    if command == "REFRESH" then
        local state = playerState[GetPlayerKey(sender)]
        OpenTeleportUi(
            sender,
            state and state.menuId or nil,
            state and state.page or nil
        )
        return false
    end
end

RegisterCreatureGossipEvent(NPC_ENTRY, 1, OnGossipHello)
RegisterServerEvent(ADDON_EVENT_ON_MESSAGE, OnAddonMessage)
