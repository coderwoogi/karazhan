-- ============================================================
-- 강화 데이터 저장소
-- ============================================================
ItemEnhancement = ItemEnhancement or {}
ItemEnhancement.Data = {}

-- 슬롯별 강화 정보 (장착된 아이템)
ItemEnhancement.EnhancementData = {}

-- GUID별 강화 정보 (인벤토리 아이템)
ItemEnhancement.EnhancementByGUID = {}

-- ============================================================
-- 초기화
-- ============================================================
function ItemEnhancement.Data:Initialize()
    ItemEnhancementDB = ItemEnhancementDB or {}
    self:ClearCache()
    DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[ItemEnhancement]|r Data.lua initialized")
end

-- ============================================================
-- 캐시 초기화
-- ============================================================
function ItemEnhancement.Data:ClearCache()
    ItemEnhancement.EnhancementData = {}
    ItemEnhancement.EnhancementByGUID = {}
end

-- ============================================================
-- 강화 데이터 저장
-- ============================================================
function ItemEnhancement.Data:SetEnhancement(slot, data)
    if not slot or not data then return end
    
    local enhanceData = {
        level = tonumber(data.level) or 0,
        bonus = tonumber(data.bonus) or 0,
        itemId = tonumber(data.itemId) or 0,
        class = tonumber(data.class) or 0,
        baseStats = tonumber(data.baseStats) or 0,
        guid = tonumber(data.guid) or 0
    }
    
    -- 슬롯별 저장 (장착된 아이템)
    ItemEnhancement.EnhancementData[slot] = enhanceData
    
    -- GUID별 저장 (인벤토리 아이템)
    if enhanceData.guid > 0 then
        ItemEnhancement.EnhancementByGUID[enhanceData.guid] = enhanceData
    end
    
    -- 디버그 출력
    DEFAULT_CHAT_FRAME:AddMessage(
        string.format("|cff00ff00[ItemEnhancement]|r Slot %d: +%d (Bonus: %d, GUID: %d)", 
        slot, enhanceData.level, enhanceData.bonus, enhanceData.guid)
    )
end

-- ============================================================
-- 슬롯으로 조회 (장착된 아이템)
-- ============================================================
function ItemEnhancement.Data:GetEnhancement(slot)
    return ItemEnhancement.EnhancementData[slot]
end

-- ============================================================
-- GUID로 조회 (인벤토리 아이템)
-- ============================================================
function ItemEnhancement.Data:GetEnhancementByGUID(guid)
    return ItemEnhancement.EnhancementByGUID[guid]
end

-- ============================================================
-- 아이템 링크에서 GUID 추출 (WoW 3.3.5 한계로 사용 불가)
-- 대신 itemId로 매칭 시도
-- ============================================================
function ItemEnhancement.Data:GetEnhancementByItemId(itemId)
    -- itemId로 검색 (동일 아이템이 여러 개일 경우 첫 번째 반환)
    for guid, data in pairs(ItemEnhancement.EnhancementByGUID) do
        if data.itemId == itemId then
            return data
        end
    end
    return nil
end

-- ============================================================
-- 슬롯에 강화 정보가 있는지 확인
-- ============================================================
function ItemEnhancement.Data:HasEnhancement(slot)
    local data = self:GetEnhancement(slot)
    return data and data.level > 0
end

-- ============================================================
-- 모든 강화 데이터 조회
-- ============================================================
function ItemEnhancement.Data:GetAllEnhancements()
    return ItemEnhancement.EnhancementData
end