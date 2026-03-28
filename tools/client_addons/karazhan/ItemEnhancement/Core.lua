-- ============================================================
-- 서버 메시지 핸들러
-- ============================================================
ItemEnhancement = ItemEnhancement or {}
ItemEnhancement.Core = {}
ItemEnhancement.IsReady = false  -- ★★★ 준비 상태 플래그 ★★★

-- ... (기존 파싱 함수 등 유지) ...

local function ParseMessage(message)
    local data = {}
    for pair in string.gmatch(message, "([^|]+)") do
        local key, value = string.match(pair, "([^:]+):(.+)")
        if key and value then
            data[key] = value
        end
    end
    return data
end

local function HandleEnhanceUpdate(message)
    local data = ParseMessage(message)
    if data.slot then
        ItemEnhancement.Data:SetEnhancement(tonumber(data.slot), data)
        if GameTooltip:IsShown() then
            GameTooltip:Hide()
            GameTooltip:Show()
        end
    end
end

local function HandleSyncComplete()
    DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r 강화 데이터 동기화 완료!")
    if CharacterFrame and CharacterFrame:IsShown() then
        PaperDollFrame_UpdateStats()
    end
end

local function HandleEnhanceResult(message)
    local data = ParseMessage(message)
    local resultText = {
        ["0"] = "|cff00ff00성공!|r",
        ["1"] = "|cffff0000실패!|r",
        ["2"] = "|cffff0000파괴됨!|r",
        ["3"] = "|cffffff00에러|r"
    }
    local result = data.result or "3"
    local itemName = data.itemName or "Unknown"
    local level = tonumber(data.level) or 0
    DEFAULT_CHAT_FRAME:AddMessage(
        string.format("|cff00ff00[강화]|r %s - %s (Lv.%d)", 
        itemName, resultText[result], level)
    )
end

local function HandleStatRefresh()
    if CharacterFrame and CharacterFrame:IsShown() then
        PaperDollFrame_UpdateStats()
    end
end

local function OnChatMessage(self, event, message, ...)
    if not message or not string.find(message, "^%[IENHANCE%]") then
        return false
    end
    
    message = string.gsub(message, "^%[IENHANCE%]", "")
    
    if string.find(message, "^ENHANCE_UPDATE") then
        local data = string.match(message, "ENHANCE_UPDATE|(.+)")
        if data then
            HandleEnhanceUpdate(data)
        end
    elseif message == "ENHANCE_SYNC_COMPLETE" then
        HandleSyncComplete()
    elseif string.find(message, "^ENHANCE_RESULT") then
        local data = string.match(message, "ENHANCE_RESULT|(.+)")
        if data then
            HandleEnhanceResult(data)
        end
    elseif message == "ENHANCE_STAT_REFRESH" then
        HandleStatRefresh()
    end
    
    return true
end

-- ★★★ 즉시 필터 등록 ★★★
ChatFrame_AddMessageEventFilter("CHAT_MSG_SYSTEM", OnChatMessage)

-- ============================================================
-- ★★★ 서버에 준비 완료 신호 전송 함수 ★★★
-- ============================================================
local function NotifyServerReady()
    -- .enhance ready 명령어로 서버에 신호
    SendChatMessage(".enhance ready", "GUILD")
    DEFAULT_CHAT_FRAME:AddMessage("|cffff6600[ItemEnhancement]|r 서버에 준비 완료 신호 전송!")
end

-- ============================================================
-- 이벤트 프레임
-- ============================================================
local EventFrame = CreateFrame("Frame")
EventFrame:RegisterEvent("ADDON_LOADED")
EventFrame:RegisterEvent("PLAYER_LOGIN")
EventFrame:RegisterEvent("PLAYER_ENTERING_WORLD")

EventFrame:SetScript("OnEvent", function(self, event, ...)
    if event == "ADDON_LOADED" then
        local addonName = ...
        if addonName == "ItemEnhancement" then
            ItemEnhancement.Data:Initialize()
            DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r 애드온 로드됨")
        end
        
    elseif event == "PLAYER_LOGIN" then
        ItemEnhancement.Data:ClearCache()
        DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r 플레이어 로그인")
        
    elseif event == "PLAYER_ENTERING_WORLD" then
        -- ★★★ 월드 입장 1초 후 서버에 준비 완료 신호 ★★★
        if not ItemEnhancement.IsReady then
            ItemEnhancement.IsReady = true
            C_Timer.After(1, function()
                NotifyServerReady()
            end)
        end
    end
end)

DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r Core.lua 로드 완료")