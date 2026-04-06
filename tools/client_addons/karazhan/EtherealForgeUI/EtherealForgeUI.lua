local ADDON_NAME = ...

local EtherealForge = CreateFrame("Frame", "EtherealForgeUIFrame", UIParent)
local FORGE_NPC_NAMES = {
  ["달라란 강화사 에테르"] = true,
  ["에테르"] = true,
}
local TYPE_ORDER = {
  "[밀리]",
  "[캐스터]",
  "[힐러]",
  "[탱커]",
}
local QUALITY_COLORS = {
  [0] = "ff9d9d9d",
  [1] = "ffffffff",
  [2] = "ff1eff00",
  [3] = "ff0070dd",
  [4] = "ffa335ee",
  [5] = "ffff8000",
  [6] = "ffe6cc80",
  [7] = "ffe6cc80",
}
local SLOT_DEFS = {
  { key = "머리", slotId = 1 },
  { key = "목", slotId = 2 },
  { key = "어깨", slotId = 3 },
  { key = "가슴", slotId = 5 },
  { key = "허리", slotId = 6 },
  { key = "다리", slotId = 7 },
  { key = "발", slotId = 8 },
  { key = "손목", slotId = 9 },
  { key = "손", slotId = 10 },
  { key = "반지1", slotId = 11 },
  { key = "반지2", slotId = 12 },
  { key = "장신구1", slotId = 13 },
  { key = "장신구2", slotId = 14 },
  { key = "등", slotId = 15 },
  { key = "주무기", slotId = 16 },
  { key = "보조무기", slotId = 17 },
  { key = "원거리", slotId = 18 },
}

local function CreateLabel(parent, template, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", template)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  if fs.SetWordWrap then
    fs:SetWordWrap(true)
  end
  return fs
end

local function SetSimpleBackdrop(frame, bgR, bgG, bgB, bgA,
  borderR, borderG, borderB, borderA)
  frame:SetBackdrop({
    bgFile = "Interface\\Buttons\\WHITE8x8",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    edgeSize = 12,
    insets = { left = 3, right = 3, top = 3, bottom = 3 },
  })
  frame:SetBackdropColor(bgR, bgG, bgB, bgA)
  frame:SetBackdropBorderColor(borderR, borderG, borderB, borderA)
end

local function TrimColorCodes(text)
  if not text then
    return ""
  end

  local clean = string.gsub(text, "|c%x%x%x%x%x%x%x%x", "")
  clean = string.gsub(clean, "|r", "")
  return clean
end

local function ParseItemId(itemLink)
  if not itemLink then
    return nil
  end

  return tonumber(string.match(itemLink, "item:(%d+)"))
end

local function GetCurrentNpcName()
  if UnitExists("npc") then
    return UnitName("npc")
  end

  return nil
end

local function IsForgeNpcName(name)
  if not name or name == "" then
    return false
  end

  if FORGE_NPC_NAMES[name] then
    return true
  end

  return string.find(name, "에테르", 1, true) ~= nil
end

local function GetClassColor()
  local _, class = UnitClass("player")
  local color = class and RAID_CLASS_COLORS and RAID_CLASS_COLORS[class]
  if color then
    return color.r, color.g, color.b
  end

  return 0.85, 0.82, 0.78
end

local function GetDifficultyColor(text)
  if string.find(text or "", "성공") then
    return 0.20, 0.90, 0.30
  end
  if string.find(text or "", "실패") or string.find(text or "", "파괴") then
    return 0.95, 0.30, 0.25
  end

  return 0.88, 0.74, 0.30
end

local function GetEnhancementLevelForLink(itemLink)
  if not itemLink or not ItemEnhancement or not ItemEnhancement.Data then
    return 0
  end

  local itemId = ParseItemId(itemLink)
  if not itemId then
    return 0
  end

  local data = ItemEnhancement.Data:GetEnhancementByItemId(itemId)
  return data and tonumber(data.level) or 0
end

local function GetItemDisplay(slotId)
  local itemLink = GetInventoryItemLink("player", slotId)
  local itemTexture = GetInventoryItemTexture("player", slotId)
    or "Interface\\Icons\\INV_Misc_QuestionMark"

  if not itemLink then
    return {
      itemLink = nil,
      texture = itemTexture,
      quality = 0,
      name = "장착 아이템 없음",
      enhanceLevel = 0,
    }
  end

  local name, _, quality = GetItemInfo(itemLink)
  if not name then
    name = itemLink
  end

  return {
    itemLink = itemLink,
    texture = itemTexture,
    quality = quality or 1,
    name = TrimColorCodes(name),
    enhanceLevel = GetEnhancementLevelForLink(itemLink),
  }
end

local function ReadGossipOptions()
  local count = GetNumGossipOptions() or 0
  local raw = { GetGossipOptions() }
  local options = {}
  local optionIndex = 1

  for i = 1, count * 2, 2 do
    local text = raw[i]
    if text then
      table.insert(options, {
        index = optionIndex,
        text = text,
        cleanText = TrimColorCodes(text),
        icon = raw[i + 1],
      })
      optionIndex = optionIndex + 1
    end
  end

  return options
end

local function FindOptionIndex(options, predicate)
  for _, option in ipairs(options) do
    if predicate(option) then
      return option.index, option
    end
  end

  return nil, nil
end

local function IsTypeButton(text)
  for _, label in ipairs(TYPE_ORDER) do
    if text == label then
      return true
    end
  end

  return false
end

local function LooksLikeEquipmentOption(text)
  return string.find(text or "", "^%[.-%]") ~= nil
end

local function IsInfoPage(options)
  for _, option in ipairs(options) do
    if string.find(option.cleanText, "카라잔 강화 시스템", 1, true) then
      return true
    end
  end

  return false
end

local function HideDefaultGossip()
  if GossipFrame and GossipFrame:IsShown() then
    GossipFrame:Hide()
  end
end

local function ResetState()
  EtherealForge.state = {
    isActive = false,
    autoRouting = false,
    npcName = nil,
    selectedSlotKey = nil,
    selectedSlotId = nil,
    selectedActionIndex = nil,
    equipmentActions = {},
    detailTitle = "강화 메뉴",
    detailSubtitle = "왼쪽 장비를 선택하면 이곳에 강화 정보가 표시됩니다.",
    detailLines = {},
    actionButtons = {},
    statusText = "강화 가능한 장비를 불러오는 중입니다.",
    lastResultText = "",
    currentBackAction = nil,
    refreshAction = nil,
    infoAction = nil,
  }
end

ResetState()

EtherealForge:SetSize(980, 600)
EtherealForge:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
EtherealForge:SetFrameStrata("DIALOG")
EtherealForge:SetClampedToScreen(true)
EtherealForge:EnableMouse(true)
EtherealForge:SetMovable(true)
EtherealForge:RegisterForDrag("LeftButton")
EtherealForge:SetScript("OnDragStart", EtherealForge.StartMoving)
EtherealForge:SetScript("OnDragStop", EtherealForge.StopMovingOrSizing)
EtherealForge:Hide()
tinsert(UISpecialFrames, "EtherealForgeUIFrame")

SetSimpleBackdrop(EtherealForge, 0.03, 0.03, 0.05, 0.96, 0.72, 0.60, 0.28,
  0.92)

local close = CreateFrame("Button", nil, EtherealForge, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", EtherealForge, "TOPRIGHT", -8, -8)

local title = CreateLabel(EtherealForge, "GameFontHighlightLarge", 22,
  0.98, 0.88, 0.54, "LEFT")
title:SetPoint("TOPLEFT", EtherealForge, "TOPLEFT", 22, -18)
title:SetText("달라란 강화사 에테르")

local subtitle = CreateLabel(EtherealForge, "GameFontNormal", 12,
  0.78, 0.76, 0.72, "LEFT")
subtitle:SetPoint("TOPLEFT", title, "BOTTOMLEFT", 0, -4)
subtitle:SetWidth(900)
subtitle:SetText(
  "캐릭터 장비를 선택해 강화 계열과 확률을 확인하고 바로 진행할 수 있습니다."
)

local leftPane = CreateFrame("Frame", nil, EtherealForge)
leftPane:SetPoint("TOPLEFT", EtherealForge, "TOPLEFT", 18, -58)
leftPane:SetSize(372, 520)
SetSimpleBackdrop(leftPane, 0.07, 0.06, 0.05, 0.84, 0.46, 0.36, 0.18, 0.85)

local rightPane = CreateFrame("Frame", nil, EtherealForge)
rightPane:SetPoint("TOPRIGHT", EtherealForge, "TOPRIGHT", -18, -58)
rightPane:SetSize(570, 520)
SetSimpleBackdrop(rightPane, 0.04, 0.04, 0.06, 0.88, 0.70, 0.52, 0.20, 0.90)

local portraitBorder = CreateFrame("Frame", nil, leftPane)
portraitBorder:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 18, -18)
portraitBorder:SetSize(74, 74)
SetSimpleBackdrop(portraitBorder, 0.10, 0.08, 0.05, 0.96, 0.82, 0.66, 0.24,
  0.95)

local portrait = portraitBorder:CreateTexture(nil, "ARTWORK")
portrait:SetPoint("TOPLEFT", portraitBorder, "TOPLEFT", 6, -6)
portrait:SetPoint("BOTTOMRIGHT", portraitBorder, "BOTTOMRIGHT", -6, 6)
portrait:SetTexture("Interface\\CharacterFrame\\TempPortrait")

local nameText = CreateLabel(leftPane, "GameFontHighlightLarge", 19,
  0.95, 0.92, 0.85, "LEFT")
nameText:SetPoint("TOPLEFT", portraitBorder, "TOPRIGHT", 14, -2)
nameText:SetWidth(230)

local infoText = CreateLabel(leftPane, "GameFontNormal", 12,
  0.74, 0.74, 0.72, "LEFT")
infoText:SetPoint("TOPLEFT", nameText, "BOTTOMLEFT", 0, -5)
infoText:SetWidth(230)

local statusLine = CreateLabel(leftPane, "GameFontHighlight", 12,
  0.88, 0.74, 0.30, "LEFT")
statusLine:SetPoint("TOPLEFT", infoText, "BOTTOMLEFT", 0, -8)
statusLine:SetWidth(230)

local leftHeader = CreateLabel(leftPane, "GameFontHighlight", 14,
  1.0, 0.84, 0.34, "LEFT")
leftHeader:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 20, -112)
leftHeader:SetText("장착 장비")

local leftDivider = leftPane:CreateTexture(nil, "ARTWORK")
leftDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
leftDivider:SetVertexColor(0.88, 0.72, 0.26, 0.85)
leftDivider:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 16, -130)
leftDivider:SetPoint("TOPRIGHT", leftPane, "TOPRIGHT", -16, -130)
leftDivider:SetHeight(8)

local leftScroll = CreateFrame("ScrollFrame", "EtherealForgeLeftScroll",
  leftPane, "UIPanelScrollFrameTemplate")
leftScroll:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 14, -142)
leftScroll:SetPoint("BOTTOMRIGHT", leftPane, "BOTTOMRIGHT", -30, 16)

local leftContent = CreateFrame("Frame", nil, leftScroll)
leftContent:SetWidth(320)
leftContent:SetHeight(1)
leftScroll:SetScrollChild(leftContent)

local detailHeader = CreateLabel(rightPane, "GameFontHighlight", 14,
  1.0, 0.84, 0.34, "LEFT")
detailHeader:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 20, -16)
detailHeader:SetText("강화 메뉴")

local detailDivider = rightPane:CreateTexture(nil, "ARTWORK")
detailDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
detailDivider:SetVertexColor(0.88, 0.72, 0.26, 0.85)
detailDivider:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -34)
detailDivider:SetPoint("TOPRIGHT", rightPane, "TOPRIGHT", -16, -34)
detailDivider:SetHeight(8)

local itemIconBorder = CreateFrame("Frame", nil, rightPane)
itemIconBorder:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 22, -52)
itemIconBorder:SetSize(86, 86)
SetSimpleBackdrop(itemIconBorder, 0.12, 0.08, 0.05, 0.96, 0.82, 0.66, 0.24,
  0.96)

local itemIcon = itemIconBorder:CreateTexture(nil, "ARTWORK")
itemIcon:SetPoint("TOPLEFT", itemIconBorder, "TOPLEFT", 6, -6)
itemIcon:SetPoint("BOTTOMRIGHT", itemIconBorder, "BOTTOMRIGHT", -6, 6)
itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

local selectedItemText = CreateLabel(rightPane, "GameFontHighlightLarge", 20,
  0.96, 0.92, 0.85, "LEFT")
selectedItemText:SetPoint("TOPLEFT", itemIconBorder, "TOPRIGHT", 14, -4)
selectedItemText:SetWidth(410)

local selectedSlotText = CreateLabel(rightPane, "GameFontNormal", 12,
  0.78, 0.76, 0.72, "LEFT")
selectedSlotText:SetPoint("TOPLEFT", selectedItemText, "BOTTOMLEFT", 0, -4)
selectedSlotText:SetWidth(410)

local lastResultText = CreateLabel(rightPane, "GameFontHighlight", 12,
  0.90, 0.74, 0.32, "LEFT")
lastResultText:SetPoint("TOPLEFT", selectedSlotText, "BOTTOMLEFT", 0, -8)
lastResultText:SetWidth(410)

local detailLines = {}
for i = 1, 10 do
  local line = CreateLabel(rightPane, "GameFontNormal", 13,
    0.92, 0.90, 0.85, "LEFT")
  if i == 1 then
    line:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 24, -156)
  else
    line:SetPoint("TOPLEFT", detailLines[i - 1], "BOTTOMLEFT", 0, -10)
  end
  line:SetWidth(520)
  detailLines[i] = line
end

local actionPanel = CreateFrame("Frame", nil, rightPane)
actionPanel:SetPoint("BOTTOMLEFT", rightPane, "BOTTOMLEFT", 18, 18)
actionPanel:SetPoint("BOTTOMRIGHT", rightPane, "BOTTOMRIGHT", -18, 18)
actionPanel:SetHeight(118)
SetSimpleBackdrop(actionPanel, 0.08, 0.07, 0.05, 0.88, 0.38, 0.30, 0.14,
  0.88)

local actionTitle = CreateLabel(actionPanel, "GameFontHighlight", 13,
  1.0, 0.82, 0.28, "LEFT")
actionTitle:SetPoint("TOPLEFT", actionPanel, "TOPLEFT", 14, -10)
actionTitle:SetText("실행")

local actionButtons = {}
local function AcquireActionButton(index)
  if actionButtons[index] then
    return actionButtons[index]
  end

  local button = CreateFrame("Button", nil, actionPanel)
  button:SetHeight(28)
  button:SetWidth(160)
  SetSimpleBackdrop(button, 0.18, 0.11, 0.05, 0.96, 0.76, 0.58, 0.20, 0.94)

  button.text = CreateLabel(button, "GameFontHighlight", 13,
    0.98, 0.90, 0.74, "CENTER")
  button.text:SetPoint("CENTER", button, "CENTER", 0, 0)

  button:SetScript("OnEnter", function(self)
    self:SetBackdropColor(0.28, 0.16, 0.05, 0.98)
  end)
  button:SetScript("OnLeave", function(self)
    self:SetBackdropColor(0.18, 0.11, 0.05, 0.96)
  end)

  actionButtons[index] = button
  return button
end

local slotRows = {}
local function AcquireSlotRow(index)
  if slotRows[index] then
    return slotRows[index]
  end

  local row = CreateFrame("Button", nil, leftContent)
  row:SetHeight(46)
  row:SetWidth(322)
  SetSimpleBackdrop(row, 0.10, 0.09, 0.07, 0.82, 0.26, 0.22, 0.14, 0.78)

  row.iconBorder = CreateFrame("Frame", nil, row)
  row.iconBorder:SetPoint("LEFT", row, "LEFT", 8, 0)
  row.iconBorder:SetSize(32, 32)
  SetSimpleBackdrop(row.iconBorder, 0.08, 0.08, 0.08, 0.96, 0.46, 0.38, 0.18,
    0.90)

  row.icon = row.iconBorder:CreateTexture(nil, "ARTWORK")
  row.icon:SetAllPoints(row.iconBorder)
  row.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  row.slotText = CreateLabel(row, "GameFontHighlight", 12,
    0.95, 0.84, 0.56, "LEFT")
  row.slotText:SetPoint("TOPLEFT", row, "TOPLEFT", 50, -8)
  row.slotText:SetWidth(72)

  row.itemText = CreateLabel(row, "GameFontNormal", 12,
    0.92, 0.90, 0.86, "LEFT")
  row.itemText:SetPoint("TOPLEFT", row, "TOPLEFT", 124, -8)
  row.itemText:SetWidth(148)
  row.itemText:SetHeight(30)

  row.levelText = CreateLabel(row, "GameFontNormalSmall", 11,
    0.98, 0.76, 0.28, "RIGHT")
  row.levelText:SetPoint("RIGHT", row, "RIGHT", -12, 0)
  row.levelText:SetWidth(54)

  row:SetScript("OnEnter", function(self)
    if self.isSelectable then
      self:SetBackdropBorderColor(0.88, 0.70, 0.28, 0.96)
      if self.itemLink then
        GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
        GameTooltip:SetHyperlink(self.itemLink)
        GameTooltip:Show()
      end
    end
  end)
  row:SetScript("OnLeave", function(self)
    if self.isSelected then
      self:SetBackdropBorderColor(0.98, 0.82, 0.34, 1.0)
    else
      self:SetBackdropBorderColor(0.26, 0.22, 0.14, 0.78)
    end
    GameTooltip:Hide()
  end)
  row:SetScript("OnClick", function(self)
    if not self.isSelectable or not self.actionIndex then
      return
    end

    EtherealForge.state.selectedSlotKey = self.slotKey
    EtherealForge.state.selectedSlotId = self.slotId
    EtherealForge.state.selectedActionIndex = self.actionIndex
    EtherealForge.state.lastResultText = ""
    SelectGossipOption(self.actionIndex)
  end)

  slotRows[index] = row
  return row
end

local function UpdateCharacterHeader()
  SetPortraitTexture(portrait, "player")
  local r, g, b = GetClassColor()
  nameText:SetText(UnitName("player") or "알 수 없음")
  nameText:SetTextColor(r, g, b)
  infoText:SetText(string.format("레벨 %d · %s",
    UnitLevel("player") or 0, UnitClass("player") or "-"))
end

local function SetStatusText(text)
  EtherealForge.state.statusText = text or ""
  statusLine:SetText(EtherealForge.state.statusText)
end

local function ResetDetailPane(titleText, subtitleText)
  selectedItemText:SetText(titleText or "장비를 선택해주세요")
  selectedSlotText:SetText(subtitleText
    or "왼쪽 장비 목록에서 강화할 아이템을 선택하면 상세 정보가 열립니다.")
  itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  for i = 1, #detailLines do
    detailLines[i]:SetText("")
  end

  for i = 1, #actionButtons do
    actionButtons[i]:Hide()
    actionButtons[i].payload = nil
  end

  lastResultText:SetText(EtherealForge.state.lastResultText or "")
end

local function ApplyDetailLines(lines)
  for i = 1, #detailLines do
    detailLines[i]:SetText(lines[i] or "")
  end
end

local function SetSelectedItem(slotKey, slotId, subtitleText)
  local item = GetItemDisplay(slotId)
  local color = QUALITY_COLORS[item.quality] or QUALITY_COLORS[1]
  selectedItemText:SetText("|c" .. color .. item.name .. "|r")
  selectedSlotText:SetText(subtitleText or (slotKey .. " 장비의 강화 메뉴입니다."))
  itemIcon:SetTexture(item.texture)
end

local function SetResultText(text)
  EtherealForge.state.lastResultText = text or ""
  local r, g, b = GetDifficultyColor(text)
  lastResultText:SetTextColor(r, g, b)
  lastResultText:SetText(EtherealForge.state.lastResultText)
end

local function BuildEquipmentRows()
  UpdateCharacterHeader()

  local offsetY = 0
  for index, def in ipairs(SLOT_DEFS) do
    local row = AcquireSlotRow(index)
    row:ClearAllPoints()
    row:SetPoint("TOPLEFT", leftContent, "TOPLEFT", 0, -offsetY)
    offsetY = offsetY + 50

    local item = GetItemDisplay(def.slotId)
    local slotAction = EtherealForge.state.equipmentActions[def.key]

    row.slotKey = def.key
    row.slotId = def.slotId
    row.itemLink = item.itemLink
    row.actionIndex = slotAction and slotAction.actionIndex or nil
    row.isSelectable = row.actionIndex ~= nil
    row.isSelected = EtherealForge.state.selectedSlotKey == def.key

    row.icon:SetTexture(item.texture)
    row.slotText:SetText(def.key)

    if row.isSelectable then
      local color = QUALITY_COLORS[item.quality] or QUALITY_COLORS[1]
      row.itemText:SetText("|c" .. color .. item.name .. "|r")
      if row.isSelected then
        row:SetBackdropBorderColor(0.98, 0.82, 0.34, 1.0)
      else
        row:SetBackdropBorderColor(0.34, 0.28, 0.18, 0.82)
      end
    else
      row.itemText:SetText("|cff8a8a8a" .. item.name .. "|r")
      row:SetBackdropBorderColor(0.18, 0.16, 0.12, 0.48)
    end

    if item.enhanceLevel > 0 then
      row.levelText:SetText("+" .. item.enhanceLevel)
    else
      row.levelText:SetText("-")
    end

    row:Show()
  end

  leftContent:SetHeight(offsetY)
end

local function ParseInfoLines(options, skipPredicate)
  local lines = {}

  for _, option in ipairs(options) do
    local clean = option.cleanText
    if clean ~= "" and clean ~= "------------------------------"
      and not string.find(clean, "^===")
      and not skipPredicate(clean) then
      table.insert(lines, clean)
    end
  end

  return lines
end

local function ShowActionButtons(buttonDefs)
  for index, def in ipairs(buttonDefs) do
    local button = AcquireActionButton(index)
    button:ClearAllPoints()

    if index == 1 then
      button:SetPoint("TOPLEFT", actionPanel, "TOPLEFT", 14, -36)
    elseif index <= 3 then
      button:SetPoint("LEFT", actionButtons[index - 1], "RIGHT", 10, 0)
    else
      button:SetPoint("TOPLEFT", actionPanel, "TOPLEFT",
        14 + ((index - 4) * 170), -72)
    end

    button.text:SetText(def.label)
    button.payload = def.payload
    button:SetScript("OnClick", function(self)
      if self.payload then
        self.payload()
      end
    end)
    button:Show()
  end
end

local function SelectAction(actionIndex)
  if actionIndex then
    SelectGossipOption(actionIndex)
  end
end

local function ShowEquipmentPage(options)
  local infoIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "강화 시스템", 1, true) ~= nil
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)

  EtherealForge.state.equipmentActions = {}
  EtherealForge.state.refreshAction = backIndex
  EtherealForge.state.infoAction = infoIndex

  local hasItems = false
  for _, option in ipairs(options) do
    if LooksLikeEquipmentOption(option.cleanText) then
      local slotKey = string.match(option.cleanText, "^%[(.-)%]")
      if slotKey then
        EtherealForge.state.equipmentActions[slotKey] = {
          actionIndex = option.index,
          text = option.cleanText,
        }
        hasItems = true
      end
    end
  end

  BuildEquipmentRows()

  if hasItems then
    SetStatusText("장비를 클릭하면 오른쪽에서 강화 계열과 비용을 확인할 수 있습니다.")
    if not EtherealForge.state.selectedSlotKey
      or not EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotKey] then
      ResetDetailPane("강화할 장비를 선택해주세요",
        "왼쪽 장착 장비 중 강화 가능한 항목을 선택해 주세요.")
      ApplyDetailLines({
        "강화는 기존 서버 로직을 그대로 사용합니다.",
        "장비 선택 → 유형 선택 → 강화 확인 → 강화 진행 순서로 이어집니다.",
        "현재 장비의 강화 수치는 왼쪽 목록 끝의 +값으로 표시됩니다.",
      })
    else
      ResetDetailPane("강화할 장비를 선택해주세요",
        "왼쪽 장착 장비 목록에서 강화할 아이템을 선택해 주세요.")
      SetSelectedItem(EtherealForge.state.selectedSlotKey,
        EtherealForge.state.selectedSlotId,
        EtherealForge.state.selectedSlotKey .. " 장비를 다시 선택하거나 유형을 이어서 고를 수 있습니다.")
      ApplyDetailLines({
        "이전 단계에서 선택한 장비입니다.",
        "왼쪽 목록에서 다른 장비를 선택하면 즉시 해당 장비 메뉴로 이동합니다.",
        "강화 안내 버튼으로 전체 규칙을 다시 확인할 수 있습니다.",
      })
    end
  else
    SetStatusText("강화 가능한 장비가 없습니다.")
    ResetDetailPane("강화 가능한 장비가 없습니다",
      "장비 착용 여부와 최대 강화 도달 여부를 확인해 주세요.")
    ApplyDetailLines({
      "강화 가능한 부위만 목록에 활성화됩니다.",
      "이미 최대 단계인 장비는 서버에서 목록에 제외됩니다.",
    })
  end

  ShowActionButtons({
    {
      label = "장비 새로고침",
      payload = function()
        SelectAction(backIndex or infoIndex)
        if backIndex then
          C_Timer.After(0.05, function()
            SelectAction(FindOptionIndex(ReadGossipOptions(), function(option)
              return option.cleanText == "장착 중인 장비를 보여주세요"
            end))
          end)
        end
      end,
    },
    {
      label = "강화 안내",
      payload = function()
        if infoIndex then
          SelectAction(infoIndex)
        end
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        CloseGossip()
      end,
    },
  })
end

local function ShowInfoPage(options)
  ResetDetailPane("강화 안내", "강화 규칙과 주의 사항을 확인하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return string.find(text, "뒤로 가기", 1, true) ~= nil
  end))

  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "장비 목록으로",
      payload = function()
        SelectAction(backIndex)
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        CloseGossip()
      end,
    },
  })
end

local function ShowTypePage(options)
  local slotKey = EtherealForge.state.selectedSlotKey
  local slotId = EtherealForge.state.selectedSlotId
  if not slotKey or not slotId then
    return
  end

  SetSelectedItem(slotKey, slotId, slotKey .. " 장비의 강화 계열을 선택하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return IsTypeButton(text) or string.find(text, "뒤로 가기", 1, true) ~= nil
  end))

  local buttons = {}
  for _, label in ipairs(TYPE_ORDER) do
    local actionIndex = FindOptionIndex(options, function(option)
      return option.cleanText == label
    end)
    if actionIndex then
      table.insert(buttons, {
        label = label,
        payload = function()
          SelectAction(actionIndex)
        end,
      })
    end
  end

  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)
  table.insert(buttons, {
    label = "장비 목록으로",
    payload = function()
      SelectAction(backIndex)
    end,
  })

  ShowActionButtons(buttons)
end

local function ShowConfirmPage(options)
  local slotKey = EtherealForge.state.selectedSlotKey
  local slotId = EtherealForge.state.selectedSlotId
  if not slotKey or not slotId then
    return
  end

  SetSelectedItem(slotKey, slotId, slotKey .. " 장비의 강화 비용과 성공 확률을 확인하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return text == "[강화 진행]"
      or string.find(text, "타입 다시 선택", 1, true) ~= nil
      or string.find(text, "뒤로 가기", 1, true) ~= nil
  end))

  local executeIndex = FindOptionIndex(options, function(option)
    return option.cleanText == "[강화 진행]"
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "타입 다시 선택", 1, true) ~= nil
      or string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "강화 진행",
      payload = function()
        SetResultText("강화 요청을 전송했습니다. 결과를 확인해 주세요.")
        SelectAction(executeIndex)
      end,
    },
    {
      label = "유형 다시 선택",
      payload = function()
        SelectAction(backIndex)
      end,
    },
  })
end

local function ParsePage(options)
  local hasRoot = FindOptionIndex(options, function(option)
    return option.cleanText == "장착 중인 장비를 보여주세요"
  end) ~= nil
  local hasEquipment = false
  local hasTypes = false
  local hasConfirm = false

  for _, option in ipairs(options) do
    if LooksLikeEquipmentOption(option.cleanText) then
      hasEquipment = true
    elseif IsTypeButton(option.cleanText) then
      hasTypes = true
    elseif option.cleanText == "[강화 진행]" then
      hasConfirm = true
    end
  end

  if hasRoot then
    return "root"
  end
  if hasEquipment then
    return "equipment"
  end
  if hasTypes then
    return "type"
  end
  if hasConfirm then
    return "confirm"
  end
  if IsInfoPage(options) then
    return "info"
  end

  return "unknown"
end

local function ShouldHandleOptions(options)
  local npcName = GetCurrentNpcName()
  local page = ParsePage(options)

  if page == "root" and IsForgeNpcName(npcName) then
    EtherealForge.state.npcName = npcName
    return true
  end

  if EtherealForge.state.isActive and EtherealForge.state.npcName
    and IsForgeNpcName(npcName or EtherealForge.state.npcName) then
    return true
  end

  return false
end

local function RouteToEquipment(options)
  local showItemsIndex = FindOptionIndex(options, function(option)
    return option.cleanText == "장착 중인 장비를 보여주세요"
  end)
  if not showItemsIndex then
    return
  end

  EtherealForge.state.autoRouting = true
  SetStatusText("강화 가능한 장비를 불러오는 중입니다.")
  C_Timer.After(0.05, function()
    if GetNumGossipOptions and (GetNumGossipOptions() or 0) > 0 then
      SelectGossipOption(showItemsIndex)
    end
    EtherealForge.state.autoRouting = false
  end)
end

local function ShowForOptions(options)
  if not ShouldHandleOptions(options) then
    return false
  end

  EtherealForge.state.isActive = true
  EtherealForge:Show()
  HideDefaultGossip()

  if EtherealForge.state.autoRouting then
    return true
  end

  local page = ParsePage(options)
  if page == "root" then
    RouteToEquipment(options)
    return true
  end

  if page == "equipment" then
    ShowEquipmentPage(options)
  elseif page == "type" then
    ShowTypePage(options)
  elseif page == "confirm" then
    ShowConfirmPage(options)
  elseif page == "info" then
    ShowInfoPage(options)
  else
    ResetDetailPane("강화 메뉴", "서버 강화 메뉴를 불러오는 중입니다.")
    ApplyDetailLines({
      "현재 메뉴 상태를 판별하지 못했습니다.",
      "다시 선택하거나 NPC와 다시 대화해 주세요.",
    })
    ShowActionButtons({
      {
        label = "창 닫기",
        payload = function()
          EtherealForge:Hide()
          CloseGossip()
        end,
      },
    })
  end

  return true
end

local function RefreshVisibleRows()
  if not EtherealForge:IsShown() then
    return
  end

  BuildEquipmentRows()
  UpdateCharacterHeader()
end

EtherealForge:RegisterEvent("GOSSIP_SHOW")
EtherealForge:RegisterEvent("GOSSIP_CLOSED")
EtherealForge:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")
EtherealForge:RegisterEvent("CHAT_MSG_SYSTEM")

EtherealForge:SetScript("OnEvent", function(self, event, ...)
  if event == "GOSSIP_SHOW" then
    ShowForOptions(ReadGossipOptions())
    return
  end

  if event == "GOSSIP_CLOSED" then
    if self.state.autoRouting then
      return
    end

    self.state.currentBackAction = nil
    if self:IsShown() then
      SetStatusText("강화 대화가 종료되었습니다. 다시 대화하면 목록을 새로 불러옵니다.")
    end
    return
  end

  if event == "PLAYER_EQUIPMENT_CHANGED" then
    RefreshVisibleRows()
    return
  end

  if event == "CHAT_MSG_SYSTEM" then
    local message = ...
    if message and (
      string.find(message, "%[강화 성공%]")
      or string.find(message, "%[강화 실패%]")
      or string.find(message, "파괴", 1, true)
      or string.find(message, "강화", 1, true)
    ) then
      SetResultText(message)
      SetStatusText("강화 결과를 확인했습니다. NPC와 다시 대화하면 장비 목록을 갱신합니다.")
    end
  end
end)

EtherealForge:SetScript("OnShow", function(self)
  UpdateCharacterHeader()
  BuildEquipmentRows()
  statusLine:SetText(self.state.statusText or "")
  if not self.state.selectedSlotKey then
    ResetDetailPane("강화 메뉴",
      "왼쪽 장착 장비 목록에서 강화할 아이템을 선택해 주세요.")
  end
end)

EtherealForge:SetScript("OnHide", function()
  if GossipFrame and GossipFrame:IsShown() then
    GossipFrame:Hide()
  end
end)

close:SetScript("OnClick", function()
  EtherealForge:Hide()
  if GossipFrame and GossipFrame:IsShown() then
    CloseGossip()
  end
end)

DEFAULT_CHAT_FRAME:AddMessage(
  "|cff33ff99[EtherealForgeUI]|r 강화 전용 UI가 로드되었습니다."
)
