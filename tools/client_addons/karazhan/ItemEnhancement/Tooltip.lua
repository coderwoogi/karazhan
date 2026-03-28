-- ============================================================
-- 툴팁 강화 정보 표시 (테두리 색상 적용) - 완전 수정 버전
-- ============================================================
ItemEnhancement = ItemEnhancement or {}
ItemEnhancement.Tooltip = {}

-- 중복 방지를 위한 플래그
local currentTooltipData = {
    itemId = nil,
    slot = nil,
    processed = false
}

-- 강화 레벨별 색상 (RGB)
local function GetEnhancementColor(level)
    if level >= 10 then
        return 1.0, 0.4, 0.0  -- 주황색 (전설)
    elseif level >= 7 then
        return 0.64, 0.21, 0.93  -- 보라색 (영웅)
    elseif level >= 4 then
        return 0.0, 0.44, 0.87  -- 파란색 (희귀)
    else
        return 0.12, 1.0, 0.0  -- 녹색 (고급)
    end
end

-- ★★★ 아이템 아이콘 테두리 Glow 생성 및 색상 적용 ★★★
local function SetIconBorderGlow(button, level)
    if not button then return end
    
    -- 버튼의 Glow 텍스처 가져오기 또는 생성
    if not button.EnhancementGlow then
        local glow = button:CreateTexture(nil, "OVERLAY", nil, 7)
        glow:SetTexture("Interface\\Buttons\\UI-ActionButton-Border")
        glow:SetBlendMode("ADD")
        glow:SetAlpha(0.9)
        glow:SetWidth(button:GetWidth() * 1.65)
        glow:SetHeight(button:GetHeight() * 1.65)
        glow:SetPoint("CENTER", button, "CENTER", 0, 0)
        button.EnhancementGlow = glow
    end
    
    if level <= 0 then
        button.EnhancementGlow:Hide()
        return
    end
    
    local r, g, b = GetEnhancementColor(level)
    button.EnhancementGlow:SetVertexColor(r, g, b, 1.0)
    button.EnhancementGlow:Show()
end

-- ItemLink에서 ItemID 추출
local function GetItemIdFromLink(itemLink)
    if not itemLink then return nil end
    return tonumber(string.match(itemLink, "item:(%d+)"))
end

-- ItemID로 강화 데이터 찾기
local function GetEnhancementByItemId(itemId)
    if not itemId then return nil end
    return ItemEnhancement.Data:GetEnhancementByItemId(itemId)
end

-- ★★★ 가방 슬롯 → 프레임 번호 변환 (WoW 3.3.5a 기준) ★★★
local function GetContainerFrameId(bag)
    -- bag = 0 (배낭) → ContainerFrame5
    -- bag = 1 → ContainerFrame4
    -- bag = 2 → ContainerFrame3
    -- bag = 3 → ContainerFrame2
    -- bag = 4 → ContainerFrame1
    if bag == 0 then
        return 5
    else
        return 5 - bag
    end
end

-- ★★★ 가방 아이템 아이콘 업데이트 ★★★
local function UpdateBagItemGlow(bag, slot)
    local frameId = GetContainerFrameId(bag)
    local buttonName = "ContainerFrame" .. frameId .. "Item" .. slot
    local button = _G[buttonName]
    
    if not button then return end
    
    local itemLink = GetContainerItemLink(bag, slot)
    
    if itemLink then
        local itemId = GetItemIdFromLink(itemLink)
        local data = GetEnhancementByItemId(itemId)
        
        if data and data.level > 0 then
            SetIconBorderGlow(button, data.level)
        else
            SetIconBorderGlow(button, 0)
        end
    else
        SetIconBorderGlow(button, 0)
    end
end

-- ★★★ 장착 슬롯 아이콘 업데이트 ★★★
local function UpdateEquipmentSlotGlow(slot)
    local slotNames = {
        [1] = "CharacterHeadSlot",
        [2] = "CharacterNeckSlot",
        [3] = "CharacterShoulderSlot",
        [5] = "CharacterChestSlot",
        [6] = "CharacterWaistSlot",
        [7] = "CharacterLegsSlot",
        [8] = "CharacterFeetSlot",
        [9] = "CharacterWristSlot",
        [10] = "CharacterHandsSlot",
        [11] = "CharacterFinger0Slot",
        [12] = "CharacterFinger1Slot",
        [13] = "CharacterTrinket0Slot",
        [14] = "CharacterTrinket1Slot",
        [15] = "CharacterBackSlot",
        [16] = "CharacterMainHandSlot",
        [17] = "CharacterSecondaryHandSlot",
        [18] = "CharacterRangedSlot"
    }
    
    local slotName = slotNames[slot]
    if not slotName then return end
    
    local button = _G[slotName]
    if not button then return end
    
    local itemLink = GetInventoryItemLink("player", slot)
    
    if itemLink then
        local itemId = GetItemIdFromLink(itemLink)
        local data = GetEnhancementByItemId(itemId)
        
        if data and data.level > 0 then
            SetIconBorderGlow(button, data.level)
        else
            SetIconBorderGlow(button, 0)
        end
    else
        SetIconBorderGlow(button, 0)
    end
end

-- ★★★ 모든 장착 슬롯 Glow 업데이트 ★★★
local function UpdateAllEquipmentGlows()
    for slot = 1, 18 do
        UpdateEquipmentSlotGlow(slot)
    end
end

-- ★★★ 모든 가방 아이템 Glow 업데이트 ★★★
local function UpdateAllBagGlows()
    for bag = 0, 4 do
        local numSlots = GetContainerNumSlots(bag)
        for slot = 1, numSlots do
            UpdateBagItemGlow(bag, slot)
        end
    end
end

-- ★★★ 이벤트 프레임 생성 ★★★
local eventFrame = CreateFrame("Frame")
eventFrame:RegisterEvent("PLAYER_LOGIN")
eventFrame:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")
eventFrame:RegisterEvent("BAG_UPDATE_DELAYED")
eventFrame:SetScript("OnEvent", function(self, event, ...)
    if event == "PLAYER_LOGIN" then
        -- 로그인 시 모든 아이콘 Glow 업데이트
        C_Timer.After(1, function()
            UpdateAllEquipmentGlows()
            UpdateAllBagGlows()
        end)
    elseif event == "PLAYER_EQUIPMENT_CHANGED" then
        local slot = ...
        UpdateEquipmentSlotGlow(slot)
    elseif event == "BAG_UPDATE_DELAYED" then
        UpdateAllBagGlows()
    end
end)

-- ============================================================
-- 툴팁 훅
-- ============================================================

-- 아이템 이름에 강화 레벨 추가
local function AddEnhancementToItemName(tooltip, data)
    local itemName = _G[tooltip:GetName().."TextLeft1"]
    if not itemName then return end
    
    local currentText = itemName:GetText()
    if not currentText then return end
    
    if data and data.level > 0 and not string.find(currentText, "%+%d+") then
        local r, g, b = GetEnhancementColor(data.level)
        local colorCode = string.format("|cff%02x%02x%02x", r*255, g*255, b*255)
        currentText = currentText .. " " .. colorCode .. "+" .. data.level .. "|r"
    end
    
    itemName:SetText(currentText)
end

-- 툴팁에 강화 정보 추가
local function AddEnhancementStats(tooltip, data)
    if not data or data.level <= 0 then return end
    if currentTooltipData.processed then return end
    
    tooltip:AddLine(" ")
    
    local r, g, b = GetEnhancementColor(data.level)
    local colorCode = string.format("|cff%02x%02x%02x", r*255, g*255, b*255)
    tooltip:AddLine(colorCode .. "강화 레벨: +" .. data.level .. "|r", 1, 1, 1)
    
    if data.bonus > 0 then
        tooltip:AddLine("|cff00ccff추가 능력치: +" .. data.bonus .. "|r", 0.8, 0.8, 1)
    end
    
    if data.level < 15 then
        local nextBonus = math.floor((data.level + 1) * 2.5)
        tooltip:AddLine("|cffaaaaaa다음 레벨: +" .. nextBonus .. " 능력치|r", 0.7, 0.7, 0.7)
    else
        tooltip:AddLine("|cffffcc00최대 강화 달성!|r", 1, 0.8, 0)
    end
    
    tooltip:Show()
    
    currentTooltipData.processed = true
end

-- 툴팁 초기화
local function ResetTooltipData()
    currentTooltipData.itemId = nil
    currentTooltipData.slot = nil
    currentTooltipData.processed = false
end

-- GameTooltip 훅
local function OnTooltipSetItem(tooltip)
    ResetTooltipData()
    
    if not tooltip then return end
    
    local _, itemLink = tooltip:GetItem()
    if not itemLink then return end
    
    local itemId = GetItemIdFromLink(itemLink)
    if not itemId then return end
    
    currentTooltipData.itemId = itemId
    
    local data = GetEnhancementByItemId(itemId)
    
    if data then
        AddEnhancementToItemName(tooltip, data)
        AddEnhancementStats(tooltip, data)
    end
end

GameTooltip:HookScript("OnTooltipCleared", ResetTooltipData)
GameTooltip:HookScript("OnTooltipSetItem", OnTooltipSetItem)

DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r Tooltip module loaded (Fixed version)")