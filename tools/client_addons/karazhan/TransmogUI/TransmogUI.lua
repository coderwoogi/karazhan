local TransmogUI = CreateFrame("Frame", "KarazhanTransmogFrame", UIParent)

local SLOT_LAYOUT = {
  { label = "머리", slotId = 1, x = 22, y = -126 },
  { label = "목", slotId = 2, x = 22, y = -168 },
  { label = "어깨", slotId = 3, x = 22, y = -210 },
  { label = "등", slotId = 15, x = 22, y = -252 },
  { label = "가슴", slotId = 5, x = 22, y = -294 },
  { label = "손목", slotId = 9, x = 22, y = -336 },
  { label = "반지1", slotId = 11, x = 22, y = -378 },
  { label = "장신구1", slotId = 13, x = 22, y = -420 },
  { label = "주무기", slotId = 16, x = 22, y = -462 },
  { label = "손", slotId = 10, x = 318, y = -126 },
  { label = "허리", slotId = 6, x = 318, y = -168 },
  { label = "다리", slotId = 7, x = 318, y = -210 },
  { label = "발", slotId = 8, x = 318, y = -252 },
  { label = "반지2", slotId = 12, x = 318, y = -294 },
  { label = "장신구2", slotId = 14, x = 318, y = -336 },
  { label = "보조무기", slotId = 17, x = 318, y = -378 },
  { label = "원거리", slotId = 18, x = 318, y = -420 },
}

local SLOT_ALIASES = {
  ["머리"] = true, ["Head"] = true,
  ["목"] = true, ["Neck"] = true,
  ["어깨"] = true, ["Shoulders"] = true, ["Shoulder"] = true,
  ["등"] = true, ["Back"] = true, ["Cloak"] = true,
  ["가슴"] = true, ["Chest"] = true,
  ["손목"] = true, ["Wrists"] = true, ["Wrist"] = true,
  ["손"] = true, ["Hands"] = true, ["Hand"] = true, ["Gloves"] = true,
  ["허리"] = true, ["Waist"] = true,
  ["다리"] = true, ["Legs"] = true,
  ["발"] = true, ["Feet"] = true,
  ["반지1"] = true, ["반지2"] = true, ["Finger"] = true, ["Ring"] = true,
  ["장신구1"] = true, ["장신구2"] = true, ["Trinket"] = true,
  ["주무기"] = true, ["Main Hand"] = true, ["Main-Hand"] = true,
  ["보조무기"] = true, ["Off Hand"] = true, ["Off-Hand"] = true,
  ["원거리"] = true, ["Ranged"] = true,
}

local TRANSMOG_SLOT_ORDER = {
  "머리",
  "어깨",
  nil,
  "가슴",
  "허리",
  "다리",
  "발",
  "손목",
  "손",
  "등",
  "주무기",
  "보조무기",
  "원거리",
}

local QUALITY_COLORS = {
  [0] = "ff9d9d9d",
  [1] = "ffffffff",
  [2] = "ff1eff00",
  [3] = "ff0070dd",
  [4] = "ffa335ee",
  [5] = "ffff8000",
}

local slotButtons = {}
local listButtons = {}

local function SetBackdrop(frame, r, g, b, a, br, bg, bb, ba)
  frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    tile = true,
    tileSize = 16,
    edgeSize = 14,
    insets = { left = 4, right = 4, top = 4, bottom = 4 },
  })
  frame:SetBackdropColor(r, g, b, a)
  frame:SetBackdropBorderColor(br, bg, bb, ba)
end

local function CreateText(parent, template, size, r, g, b, justify)
  local text = parent:CreateFontString(nil, "OVERLAY", template)
  text:SetFont("Fonts\\2002.TTF", size, "")
  text:SetTextColor(r, g, b)
  text:SetJustifyH(justify or "LEFT")
  return text
end

local function StripTextures(text)
  text = tostring(text or "")
  text = string.gsub(text, "|T.-|t", "")
  return string.gsub(text, "^%s+", "")
end

local function StripLinks(text)
  text = StripTextures(text)
  text = string.gsub(text, "|c%x%x%x%x%x%x%x%x", "")
  text = string.gsub(text, "|r", "")
  text = string.gsub(text, "|H.-|h%[(.-)%]|h", "%1")
  return text
end

local function ParseItemId(text)
  return tonumber(string.match(text or "", "item:(%d+)"))
end

local function ParseItemName(text)
  return string.match(text or "", "|h%[(.-)%]|h")
    or string.match(StripLinks(text), "%[(.-)%]")
    or StripLinks(text)
end

local function GetItemQualityColor(itemLink)
  local _, _, quality = GetItemInfo(itemLink or "")
  return QUALITY_COLORS[quality or 1] or QUALITY_COLORS[1]
end

local function GetGossipOptionsSafe()
  local options = {}
  if not GetGossipOptions or not GetNumGossipOptions then
    return options
  end

  local raw = { GetGossipOptions() }
  local count = GetNumGossipOptions() or 0
  for i = 1, count do
    local text = raw[(i - 1) * 2 + 1] or ""
    table.insert(options, {
      index = i,
      text = text,
      clean = StripLinks(text),
      itemId = ParseItemId(text),
    })
  end
  return options
end

local IsListOption

local function IsTransmogRoot(options)
  local slotCount = 0
  local plainCount = 0
  local hasTransmogText = false
  for _, option in ipairs(options) do
    if SLOT_ALIASES[option.clean] then
      slotCount = slotCount + 1
    end
    if not option.itemId and not IsListOption(option) then
      plainCount = plainCount + 1
    end
    if string.find(option.clean, "형상", 1, true)
      or string.find(option.clean, "변환", 1, true)
      or string.find(option.clean, "Transmog", 1, true)
      or string.find(option.clean, "Transmogrification", 1, true)
      or string.find(option.clean, "Remove Transmogrification", 1, true)
      or string.find(option.clean, "Update menu", 1, true) then
      hasTransmogText = true
    end
  end
  return slotCount >= 5 or plainCount >= 8 or hasTransmogText
end

local function IsRootUtilityOption(option)
  return option.itemId
    or IsListOption(option)
    or string.find(option.clean, "How", 1, true)
    or string.find(option.clean, "작동", 1, true)
    or string.find(option.clean, "안내", 1, true)
    or string.find(option.clean, "Manage", 1, true)
    or string.find(option.clean, "세트", 1, true)
end

local function BuildRootSlotOptionMap(options)
  local mapped = {}
  local orderIndex = 1

  for _, option in ipairs(options or {}) do
    if not IsRootUtilityOption(option) then
      local label = option.clean
      local mappedLabel = SLOT_ALIASES[label] and label
        or TRANSMOG_SLOT_ORDER[orderIndex]
      if mappedLabel then
        mapped[mappedLabel] = option
      end
      orderIndex = orderIndex + 1
    end
  end

  return mapped
end

function IsListOption(option)
  if option.itemId then
    return true
  end
  return string.find(option.clean, "Next", 1, true)
    or string.find(option.clean, "Previous", 1, true)
    or string.find(option.clean, "이전", 1, true)
    or string.find(option.clean, "다음", 1, true)
    or string.find(option.clean, "검색", 1, true)
    or string.find(option.clean, "Search", 1, true)
    or string.find(option.clean, "숨기", 1, true)
    or string.find(option.clean, "Hide", 1, true)
    or string.find(option.clean, "제거", 1, true)
    or string.find(option.clean, "Remove", 1, true)
    or string.find(option.clean, "뒤로", 1, true)
    or string.find(option.clean, "Back", 1, true)
end

local function ConcealDefaultPanels()
  if GossipFrame and GossipFrame:IsShown() then
    GossipFrame:SetAlpha(0)
    GossipFrame:EnableMouse(false)
  end
  if MerchantFrame and MerchantFrame:IsShown() then
    MerchantFrame:SetAlpha(0)
    MerchantFrame:EnableMouse(false)
  end
end

local function RestoreDefaultPanels()
  if GossipFrame then
    GossipFrame:SetAlpha(1)
    GossipFrame:EnableMouse(true)
  end
  if MerchantFrame then
    MerchantFrame:SetAlpha(1)
    MerchantFrame:EnableMouse(true)
  end
end

local function CloseNpcWindows()
  RestoreDefaultPanels()
  if CloseGossip then
    CloseGossip()
  end
  if CloseMerchant then
    CloseMerchant()
  end
end

local function SelectGossip(index)
  if index and SelectGossipOption then
    SelectGossipOption(index)
  end
end

local function ClearList()
  for _, button in ipairs(listButtons) do
    button:Hide()
  end
end

local function SetStatus(message)
  TransmogUI.status:SetText(message or "")
end

TransmogUI:SetSize(980, 610)
TransmogUI:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
TransmogUI:SetFrameStrata("DIALOG")
TransmogUI:SetClampedToScreen(true)
TransmogUI:EnableMouse(true)
TransmogUI:SetMovable(true)
TransmogUI:RegisterForDrag("LeftButton")
TransmogUI:SetScript("OnDragStart", TransmogUI.StartMoving)
TransmogUI:SetScript("OnDragStop", TransmogUI.StopMovingOrSizing)
TransmogUI:Hide()
tinsert(UISpecialFrames, "KarazhanTransmogFrame")

SetBackdrop(TransmogUI, 0.03, 0.035, 0.04, 0.96, 0.36, 0.58, 0.64, 0.92)

local close = CreateFrame("Button", nil, TransmogUI, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", TransmogUI, "TOPRIGHT", -8, -8)

local title = CreateText(TransmogUI, "GameFontHighlightLarge", 22, 0.66, 0.94, 1.0, "LEFT")
title:SetPoint("TOPLEFT", TransmogUI, "TOPLEFT", 22, -18)
title:SetText("형상변환")

local subtitle = CreateText(TransmogUI, "GameFontNormal", 12, 0.78, 0.86, 0.88, "LEFT")
subtitle:SetPoint("TOPLEFT", title, "BOTTOMLEFT", 0, -4)
subtitle:SetText("왼쪽 장비칸을 선택하고 오른쪽에서 적용할 형상 아이템을 고르세요.")

local leftPane = CreateFrame("Frame", nil, TransmogUI)
leftPane:SetPoint("TOPLEFT", TransmogUI, "TOPLEFT", 18, -58)
leftPane:SetSize(380, 530)
SetBackdrop(leftPane, 0.04, 0.055, 0.06, 0.88, 0.28, 0.50, 0.56, 0.88)

local rightPane = CreateFrame("Frame", nil, TransmogUI)
rightPane:SetPoint("TOPRIGHT", TransmogUI, "TOPRIGHT", -18, -58)
rightPane:SetSize(564, 530)
SetBackdrop(rightPane, 0.025, 0.035, 0.045, 0.92, 0.30, 0.54, 0.62, 0.88)

local leftHeader = CreateText(leftPane, "GameFontHighlight", 14, 0.70, 0.94, 1.0, "LEFT")
leftHeader:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 20, -18)
leftHeader:SetText("장비칸")

local modelPanel = CreateFrame("Frame", nil, leftPane)
modelPanel:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 112, -92)
modelPanel:SetSize(152, 294)
modelPanel:SetFrameLevel(leftPane:GetFrameLevel() + 1)
SetBackdrop(modelPanel, 0.02, 0.04, 0.05, 0.42, 0.20, 0.48, 0.56, 0.58)

local playerModel = CreateFrame("PlayerModel", nil, modelPanel)
playerModel:SetPoint("TOPLEFT", modelPanel, "TOPLEFT", 4, -4)
playerModel:SetPoint("BOTTOMRIGHT", modelPanel, "BOTTOMRIGHT", -4, 4)
playerModel:EnableMouse(false)

local playerNameText = CreateText(leftPane, "GameFontHighlight", 13, 0.70, 0.94, 1.0, "CENTER")
playerNameText:SetPoint("TOP", modelPanel, "BOTTOM", 0, -8)
playerNameText:SetWidth(220)

local selectedSlotText = CreateText(leftPane, "GameFontNormalSmall", 11, 0.78, 0.86, 0.88, "CENTER")
selectedSlotText:SetPoint("TOP", playerNameText, "BOTTOM", 0, -5)
selectedSlotText:SetWidth(250)
selectedSlotText:SetText("장비칸을 선택하세요.")

local rightHeader = CreateText(rightPane, "GameFontHighlight", 14, 0.70, 0.94, 1.0, "LEFT")
rightHeader:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 20, -18)
rightHeader:SetText("형상변환 아이템 목록")

TransmogUI.status = CreateText(rightPane, "GameFontNormal", 12, 0.82, 0.86, 0.86, "LEFT")
TransmogUI.status:SetPoint("TOPLEFT", rightHeader, "BOTTOMLEFT", 0, -8)
TransmogUI.status:SetWidth(520)
TransmogUI.status:SetText("형상변환 NPC와 대화하면 목록이 표시됩니다.")

local function UpdatePlayerModel()
  playerModel:SetUnit("player")
  playerNameText:SetText(UnitName("player") or "")
end

local function BuildSlotButtons(options)
  UpdatePlayerModel()

  if options and IsTransmogRoot(options) then
    TransmogUI.rootSlotOptions = BuildRootSlotOptionMap(options)
  end
  local optionByLabel = TransmogUI.rootSlotOptions or {}

  for i, data in ipairs(SLOT_LAYOUT) do
    local button = slotButtons[i]
    if not button then
      button = CreateFrame("Button", nil, leftPane)
      button:SetSize(40, 40)
      button:SetFrameLevel(leftPane:GetFrameLevel() + 5)
      button.icon = button:CreateTexture(nil, "ARTWORK")
      button.icon:SetAllPoints(button)
      button.border = button:CreateTexture(nil, "OVERLAY")
      button.border:SetTexture("Interface\\Buttons\\UI-ActionButton-Border")
      button.border:SetBlendMode("ADD")
      button.border:SetPoint("CENTER")
      button.border:SetSize(68, 68)
      button.label = CreateText(leftPane, "GameFontNormalSmall", 11, 0.78, 0.86, 0.88, "CENTER")
      button.label:SetPoint("TOP", button, "BOTTOM", 0, -2)
      button:SetScript("OnClick", function(self)
        if not self.optionIndex then
          SetStatus("이 장비칸은 현재 형상변환할 수 없습니다.")
          return
        end
        TransmogUI.selectedSlot = self.slotLabel
        selectedSlotText:SetText(self.slotLabel .. " 선택됨")
        SetStatus(self.slotLabel .. " 형상 목록을 불러오는 중입니다.")
        SelectGossip(self.optionIndex)
      end)
      slotButtons[i] = button
    end

    button:SetPoint("TOPLEFT", leftPane, "TOPLEFT", data.x, data.y)
    button.slotLabel = data.label
    button.label:SetText(data.label)

    local texture = GetInventoryItemTexture("player", data.slotId)
    button.icon:SetTexture(texture or "Interface\\PaperDoll\\UI-Backpack-EmptySlot")

    local option = optionByLabel[data.label]
    button.optionIndex = option and option.index or nil
    if button.optionIndex then
      button.icon:SetVertexColor(1, 1, 1, 1)
      button.border:SetVertexColor(0.2, 0.85, 1.0, 0.75)
    else
      button.icon:SetVertexColor(0.35, 0.35, 0.35, 0.85)
      button.border:SetVertexColor(0.35, 0.35, 0.35, 0.35)
    end
    button:Show()
  end
end

local function AcquireListButton(index)
  local button = listButtons[index]
  if button then
    return button
  end

  button = CreateFrame("Button", nil, rightPane)
  button:SetSize(514, 34)
  button.bg = button:CreateTexture(nil, "BACKGROUND")
  button.bg:SetAllPoints(button)
  button.bg:SetTexture(0.05, 0.08, 0.09, 0.86)
  button.icon = button:CreateTexture(nil, "ARTWORK")
  button.icon:SetPoint("LEFT", button, "LEFT", 8, 0)
  button.icon:SetSize(26, 26)
  button.text = CreateText(button, "GameFontNormal", 12, 0.88, 0.92, 0.92, "LEFT")
  button.text:SetPoint("LEFT", button.icon, "RIGHT", 10, 0)
  button.text:SetWidth(440)
  button:SetScript("OnEnter", function(self)
    self.bg:SetTexture(0.10, 0.18, 0.20, 0.95)
    if self.itemLink then
      GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
      GameTooltip:SetHyperlink(self.itemLink)
      GameTooltip:Show()
    end
  end)
  button:SetScript("OnLeave", function(self)
    self.bg:SetTexture(0.05, 0.08, 0.09, 0.86)
    GameTooltip:Hide()
  end)
  button:SetScript("OnClick", function(self)
    if self.vendorIndex and BuyMerchantItem then
      BuyMerchantItem(self.vendorIndex, 1)
      return
    end
    SelectGossip(self.optionIndex)
  end)
  listButtons[index] = button
  return button
end

local function RenderGossipList(options)
  ClearList()
  local visible = 0
  for _, option in ipairs(options or {}) do
    if IsListOption(option) and not SLOT_ALIASES[option.clean] then
      visible = visible + 1
      local button = AcquireListButton(visible)
      local itemId = option.itemId
      local name = ParseItemName(option.text)
      local link = itemId and select(2, GetItemInfo(itemId)) or nil

      button:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 24, -58 - ((visible - 1) * 38))
      button.optionIndex = option.index
      button.vendorIndex = nil
      button.itemLink = link
      button.icon:SetTexture(itemId and GetItemIcon(itemId) or "Interface\\ICONS\\INV_Enchant_Disenchant")
      button.text:SetText("|c" .. GetItemQualityColor(link) .. name .. "|r")
      button:Show()
    end
  end

  if visible == 0 then
    SetStatus("왼쪽 장비칸을 선택하면 사용 가능한 형상 목록이 표시됩니다.")
  else
    SetStatus((TransmogUI.selectedSlot or "선택한 장비") .. "에 적용 가능한 형상입니다.")
  end
end

local function RenderMerchantList()
  ClearList()
  local count = GetMerchantNumItems and GetMerchantNumItems() or 0
  for i = 1, count do
    local name, texture, price, quantity, _, _, _, _, _, _, _, _, _, itemId =
      GetMerchantItemInfo(i)
    local link = GetMerchantItemLink and GetMerchantItemLink(i) or nil
    local button = AcquireListButton(i)
    button:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 24, -58 - ((i - 1) * 38))
    button.optionIndex = nil
    button.vendorIndex = i
    button.itemLink = link
    button.icon:SetTexture(texture or (itemId and GetItemIcon(itemId)) or "Interface\\ICONS\\INV_Enchant_Disenchant")
    button.text:SetText("|c" .. GetItemQualityColor(link) .. (name or "형상 아이템") .. "|r")
    button:Show()
  end
  SetStatus(count > 0 and "상점형 형상 목록입니다. 클릭하면 적용합니다."
    or "표시할 형상 아이템이 없습니다.")
end

local function OpenForGossip()
  local options = GetGossipOptionsSafe()
  if not IsTransmogRoot(options) and not TransmogUI:IsShown() then
    return
  end

  TransmogUI:Show()
  selectedSlotText:SetText("장비칸을 선택하세요.")
  BuildSlotButtons(options)
  RenderGossipList(options)
  ConcealDefaultPanels()
end

close:SetScript("OnClick", function()
  TransmogUI:Hide()
end)

TransmogUI:SetScript("OnHide", function()
  CloseNpcWindows()
  TransmogUI.selectedSlot = nil
  selectedSlotText:SetText("장비칸을 선택하세요.")
end)

TransmogUI:RegisterEvent("GOSSIP_SHOW")
TransmogUI:RegisterEvent("GOSSIP_CLOSED")
TransmogUI:RegisterEvent("MERCHANT_SHOW")
TransmogUI:RegisterEvent("MERCHANT_CLOSED")
TransmogUI:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")

TransmogUI:SetScript("OnEvent", function(self, event)
  if event == "GOSSIP_SHOW" then
    OpenForGossip()
    return
  end

  if event == "MERCHANT_SHOW" and self:IsShown() then
    ConcealDefaultPanels()
    RenderMerchantList()
    return
  end

  if event == "GOSSIP_CLOSED" or event == "MERCHANT_CLOSED" then
    if self:IsShown() then
      self:Hide()
    end
    return
  end

  if event == "PLAYER_EQUIPMENT_CHANGED" and self:IsShown() then
    BuildSlotButtons(GetGossipOptionsSafe())
  end
end)
