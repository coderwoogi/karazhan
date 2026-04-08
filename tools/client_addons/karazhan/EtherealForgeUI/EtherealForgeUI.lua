local EtherealForge = CreateFrame("Frame", "EtherealForgeUIFrame", UIParent)

local FORGE_NPC_ENTRY = 190014
local ACTION_ITEM_SELECT_BASE = 100
local ACTION_ENHANCE_CONFIRM_BASE = 1000
local ENHANCE_TYPE_MELEE = 1
local ENHANCE_TYPE_CASTER = 2
local ENHANCE_TYPE_HEALER = 3
local ENHANCE_TYPE_TANK = 4

local TYPE_ORDER = {
  "[밀리]",
  "[캐스터]",
  "[힐러]",
  "[탱커]",
}

local SLOT_LAYOUT = {
  { key = "HEAD", label = "머리", slotId = 1, x = 22, y = -126 },
  { key = "NECK", label = "목", slotId = 2, x = 22, y = -168 },
  { key = "SHOULDER", label = "어깨", slotId = 3, x = 22, y = -210 },
  { key = "BACK", label = "등", slotId = 15, x = 22, y = -252 },
  { key = "CHEST", label = "가슴", slotId = 5, x = 22, y = -294 },
  { key = "WRIST", label = "손목", slotId = 9, x = 22, y = -336 },
  { key = "RING1", label = "반지1", slotId = 11, x = 22, y = -378 },
  { key = "TRINKET1", label = "장신구1", slotId = 13, x = 22, y = -420 },
  { key = "MAINHAND", label = "주무기", slotId = 16, x = 22, y = -462 },
  { key = "HANDS", label = "손", slotId = 10, x = 318, y = -126 },
  { key = "WAIST", label = "허리", slotId = 6, x = 318, y = -168 },
  { key = "LEGS", label = "다리", slotId = 7, x = 318, y = -210 },
  { key = "FEET", label = "발", slotId = 8, x = 318, y = -252 },
  { key = "RING2", label = "반지2", slotId = 12, x = 318, y = -294 },
  { key = "TRINKET2", label = "장신구2", slotId = 14, x = 318, y = -336 },
  { key = "OFFHAND", label = "보조무기", slotId = 17, x = 318, y = -378 },
  { key = "RANGED", label = "원거리", slotId = 18, x = 318, y = -420 },
}

local SLOT_LABEL_TO_ID = {}
for _, slotDef in ipairs(SLOT_LAYOUT) do
  SLOT_LABEL_TO_ID[slotDef.label] = slotDef.slotId
end

local ShowForOptions
local SelectAction
local ShowLocalTypeMenu

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

  if UnitExists("target") then
    return UnitName("target")
  end

  return nil
end

local function GetNpcEntryFromUnit(unit)
  if not UnitExists(unit) then
    return nil
  end

  local guid = UnitGUID(unit)
  if not guid then
    return nil
  end

  if string.find(guid, "^0x[Ff]13") or string.find(guid, "^0x[Ff]15") then
    return tonumber(string.sub(guid, 7, 12), 16)
  end

  local unitType, _, _, _, _, npcId = strsplit("-", guid)
  if unitType == "Creature" or unitType == "Vehicle" then
    return tonumber(npcId)
  end

  return nil
end

local function GetNpcEntry()
  local entry = GetNpcEntryFromUnit("npc")
  if entry then
    return entry
  end

  return GetNpcEntryFromUnit("target")
end

local function IsForgeNpc()
  local entry = GetNpcEntry()
  if entry == FORGE_NPC_ENTRY then
    return true
  end

  local name = GetCurrentNpcName()
  if not name then
    return false
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

local function GetResultColor(text)
  if string.find(text or "", "성공", 1, true) then
    return 0.20, 0.90, 0.30
  end

  if string.find(text or "", "실패", 1, true)
    or string.find(text or "", "파괴", 1, true) then
    return 0.95, 0.30, 0.25
  end

  return 0.88, 0.74, 0.30
end

local function GetEnhancementLevel(itemLink)
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
  local texture = GetInventoryItemTexture("player", slotId)
    or nil

  if not itemLink then
    return {
      itemLink = nil,
      texture = texture,
      quality = 0,
      name = "",
      level = 0,
    }
  end

  local name, _, quality = GetItemInfo(itemLink)
  if not name then
    name = itemLink
  end

  return {
    itemLink = itemLink,
    texture = texture,
    quality = quality or 1,
    name = TrimColorCodes(name),
    level = GetEnhancementLevel(itemLink),
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

local function HideDefaultGossip()
  if GossipFrame and GossipFrame:IsShown() then
    GossipFrame:Hide()
  end
end

local function DebugMessage(message)
  if DEFAULT_CHAT_FRAME and message then
    DEFAULT_CHAT_FRAME:AddMessage(
      "|cff33ff99[EtherealForgeUI]|r " .. tostring(message)
    )
  end
end

local function ParsePage(options)
  local hasRoot = false
  local hasEquipment = false
  local hasType = false
  local hasConfirm = false

  for _, option in ipairs(options) do
    local text = option.cleanText
    if text == "장착 중인 장비를 보여주세요" then
      hasRoot = true
    elseif text == "[강화 진행]" then
      hasConfirm = true
    else
      local isType = false
      for _, label in ipairs(TYPE_ORDER) do
        if text == label then
          hasType = true
          isType = true
          break
        end
      end
      if not isType and string.find(text, "^%[.-%]") ~= nil then
        hasEquipment = true
      end
    end
  end

  if hasRoot then
    return "root"
  end
  if hasEquipment then
    return "equipment"
  end
  if hasType then
    return "type"
  end
  if hasConfirm then
    return "confirm"
  end

  return "info"
end

local function IsForgeRootOptions(options)
  local hasInfo = false
  local hasShowItems = false

  for _, option in ipairs(options) do
    local text = option.cleanText
    if string.find(text, "아이템 강화 시스템", 1, true) ~= nil then
      hasInfo = true
    elseif string.find(text, "장착 중인 장비를 보여주세요", 1, true)
      ~= nil then
      hasShowItems = true
    end
  end

  return hasInfo and hasShowItems
end

local function ParseInfoLines(options, skipPredicate)
  local lines = {}
  for _, option in ipairs(options) do
    local text = option.cleanText
    if text ~= ""
      and text ~= "------------------------------"
      and not string.find(text, "^===")
      and not skipPredicate(text) then
      table.insert(lines, text)
    end
  end
  return lines
end

local function BuildOptionsSignature(options)
  local parts = {}
  for _, option in ipairs(options) do
    table.insert(parts, option.cleanText or "")
  end
  return table.concat(parts, "|")
end

local function ScheduleGossipRefresh(expectedPage, previousSignature, triesLeft)
  if triesLeft <= 0 then
    if EtherealForge.state then
      EtherealForge.state.transitioning = false
      EtherealForge.state.autoRouting = false
    end
    return
  end

  C_Timer.After(0.10, function()
    if not EtherealForge.state or not EtherealForge.state.active then
      return
    end

    local count = GetNumGossipOptions() or 0
    if count <= 0 then
      ScheduleGossipRefresh(expectedPage, previousSignature, triesLeft - 1)
      return
    end

    local options = ReadGossipOptions()
    local signature = BuildOptionsSignature(options)
    local page = ParsePage(options)
    DebugMessage("schedule page=" .. tostring(page)
      .. " expected=" .. tostring(expectedPage))
    if signature ~= previousSignature or page == expectedPage then
      EtherealForge.state.transitioning = false
      ShowForOptions(options)
      return
    end

    ScheduleGossipRefresh(expectedPage, previousSignature, triesLeft - 1)
  end)
end

local function TryOpenEquipmentPage(triesLeft)
  if triesLeft <= 0 then
    if EtherealForge.state then
      EtherealForge.state.autoRouting = false
    end
    DebugMessage("equipment page retry exhausted")
    return
  end

  C_Timer.After(0.05, function()
    if not EtherealForge.state or not EtherealForge.state.active then
      return
    end

    local options = ReadGossipOptions()
    local page = ParsePage(options)
    DebugMessage("retry page=" .. tostring(page))
    if page == "equipment" then
      EtherealForge.state.autoRouting = false
      ShowForOptions(options)
      return
    end

    local showItemsIndex = FindOptionIndex(options, function(option)
      return string.find(option.cleanText,
        "장착 중인 장비를 보여주세요", 1, true) ~= nil
    end)

    if showItemsIndex then
      DebugMessage("retry show items action=" .. tostring(showItemsIndex))
      SelectAction(showItemsIndex, "equipment")
    end

    TryOpenEquipmentPage(triesLeft - 1)
  end)
end

local function RefreshFromCurrentGossip()
  if not EtherealForge.state or not EtherealForge.state.active then
    return
  end

  local count = GetNumGossipOptions() or 0
  DebugMessage("refresh count=" .. tostring(count))
  if count <= 0 then
    return
  end

  local options = ReadGossipOptions()
  DebugMessage("refresh page=" .. tostring(ParsePage(options)))
  ShowForOptions(options)
end

SelectAction = function(actionIndex, expectedPage)
  if actionIndex then
    local previousOptions = ReadGossipOptions()
    local previousSignature = BuildOptionsSignature(previousOptions)
    EtherealForge.state.transitioning = true
    DebugMessage("select action=" .. tostring(actionIndex)
      .. " expected=" .. tostring(expectedPage))
    SelectGossipOption(actionIndex)
    C_Timer.After(0.05, RefreshFromCurrentGossip)
    C_Timer.After(0.15, RefreshFromCurrentGossip)
    ScheduleGossipRefresh(expectedPage, previousSignature, 10)
  end
end

local function ResetState()
  EtherealForge.state = {
    active = false,
    autoRouting = false,
    npcName = nil,
    selectedSlotKey = nil,
    selectedSlotId = nil,
    equipmentActions = {},
    transitioning = false,
    statusText = "강화 가능한 장비를 불러오는 중입니다.",
    resultText = "",
  }
end

ResetState()

EtherealForge:SetSize(980, 610)
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

SetSimpleBackdrop(EtherealForge, 0.03, 0.03, 0.05, 0.96,
  0.72, 0.60, 0.28, 0.92)

local close = CreateFrame("Button", nil, EtherealForge, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", EtherealForge, "TOPRIGHT", -8, -8)

local title = CreateLabel(EtherealForge, "GameFontHighlightLarge", 22,
  0.98, 0.88, 0.54, "LEFT")
title:SetPoint("TOPLEFT", EtherealForge, "TOPLEFT", 22, -18)
title:SetText("달라란 강화사 에테르")

local subtitle = CreateLabel(EtherealForge, "GameFontNormal", 12,
  0.78, 0.76, 0.72, "LEFT")
subtitle:SetPoint("TOPLEFT", title, "BOTTOMLEFT", 0, -4)
subtitle:SetWidth(920)
subtitle:SetText(
  "왼쪽에서 장착 장비를 선택하고, 오른쪽에서 강화 유형과 진행 여부를 확인하세요."
)

local leftPane = CreateFrame("Frame", nil, EtherealForge)
leftPane:SetPoint("TOPLEFT", EtherealForge, "TOPLEFT", 18, -58)
leftPane:SetSize(380, 530)
SetSimpleBackdrop(leftPane, 0.07, 0.06, 0.05, 0.84,
  0.46, 0.36, 0.18, 0.85)

local rightPane = CreateFrame("Frame", nil, EtherealForge)
rightPane:SetPoint("TOPRIGHT", EtherealForge, "TOPRIGHT", -18, -58)
rightPane:SetSize(564, 530)
SetSimpleBackdrop(rightPane, 0.04, 0.04, 0.06, 0.88,
  0.70, 0.52, 0.20, 0.90)

local leftHeader = CreateLabel(leftPane, "GameFontHighlight", 14,
  1.0, 0.84, 0.34, "LEFT")
leftHeader:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 20, -18)
leftHeader:SetText("캐릭터 장비")

local leftDivider = leftPane:CreateTexture(nil, "ARTWORK")
leftDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
leftDivider:SetVertexColor(0.88, 0.72, 0.26, 0.85)
leftDivider:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 16, -36)
leftDivider:SetPoint("TOPRIGHT", leftPane, "TOPRIGHT", -16, -36)
leftDivider:SetHeight(8)

local nameText = CreateLabel(leftPane, "GameFontHighlightLarge", 18,
  0.95, 0.92, 0.85, "CENTER")
nameText:SetPoint("TOP", leftPane, "TOP", 0, -54)
nameText:SetWidth(260)

local infoText = CreateLabel(leftPane, "GameFontNormal", 12,
  0.74, 0.74, 0.72, "CENTER")
infoText:SetPoint("TOP", nameText, "BOTTOM", 0, -4)
infoText:SetWidth(260)

local statusLine = CreateLabel(leftPane, "GameFontHighlight", 12,
  0.88, 0.74, 0.30, "CENTER")
statusLine:SetPoint("TOP", infoText, "BOTTOM", 0, -8)
statusLine:SetWidth(300)

local modelPanel = CreateFrame("Frame", nil, leftPane)
modelPanel:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 112, -118)
modelPanel:SetSize(152, 250)
SetSimpleBackdrop(modelPanel, 0.11, 0.08, 0.05, 0.36,
  0.44, 0.32, 0.14, 0.52)

local playerModel = CreateFrame("PlayerModel", nil, modelPanel)
playerModel:SetPoint("TOPLEFT", modelPanel, "TOPLEFT", 4, -4)
playerModel:SetPoint("BOTTOMRIGHT", modelPanel, "BOTTOMRIGHT", -4, 4)

local selectedSlotSummary = CreateLabel(leftPane, "GameFontNormalSmall", 11,
  0.75, 0.73, 0.69, "CENTER")
selectedSlotSummary:SetPoint("TOP", modelPanel, "BOTTOM", 0, -10)
selectedSlotSummary:SetWidth(220)
selectedSlotSummary:SetText(
  "장비 슬롯을 클릭하면 오른쪽에 강화 메뉴가 열립니다."
)

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
SetSimpleBackdrop(itemIconBorder, 0.12, 0.08, 0.05, 0.96,
  0.82, 0.66, 0.24, 0.96)

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

local requirementPanel = CreateFrame("Frame", nil, rightPane)
requirementPanel:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 18, -156)
requirementPanel:SetSize(290, 338)
SetSimpleBackdrop(requirementPanel, 0.08, 0.07, 0.05, 0.88,
  0.38, 0.30, 0.14, 0.88)

local requirementTitle = CreateLabel(requirementPanel, "GameFontHighlight", 13,
  1.0, 0.82, 0.28, "LEFT")
requirementTitle:SetPoint("TOPLEFT", requirementPanel, "TOPLEFT", 14, -10)
requirementTitle:SetText("필요 조건")

local detailLines = {}
for i = 1, 10 do
  local line = CreateLabel(requirementPanel, "GameFontNormal", 13,
    0.92, 0.90, 0.85, "LEFT")
  if i == 1 then
    line:SetPoint("TOPLEFT", requirementPanel, "TOPLEFT", 14, -36)
  else
    line:SetPoint("TOPLEFT", detailLines[i - 1], "BOTTOMLEFT", 0, -10)
  end
  line:SetWidth(262)
  detailLines[i] = line
end

local requirementItemRows = {}
for i = 1, 4 do
  local row = CreateFrame("Frame", nil, requirementPanel)
  row:SetSize(262, 28)
  if i == 1 then
    row:SetPoint("TOPLEFT", requirementPanel, "TOPLEFT", 14, -206)
  else
    row:SetPoint("TOPLEFT", requirementItemRows[i - 1], "BOTTOMLEFT", 0,
      -8)
  end

  row.iconBorder = CreateFrame("Frame", nil, row)
  row.iconBorder:SetPoint("LEFT", row, "LEFT", 0, 0)
  row.iconBorder:SetSize(24, 24)
  SetSimpleBackdrop(row.iconBorder, 0.10, 0.09, 0.07, 0.92,
    0.42, 0.32, 0.16, 0.84)

  row.icon = row.iconBorder:CreateTexture(nil, "ARTWORK")
  row.icon:SetPoint("TOPLEFT", row.iconBorder, "TOPLEFT", 2, -2)
  row.icon:SetPoint("BOTTOMRIGHT", row.iconBorder, "BOTTOMRIGHT", -2, 2)
  row.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  row.name = CreateLabel(row, "GameFontNormal", 12,
    0.92, 0.90, 0.85, "LEFT")
  row.name:SetPoint("LEFT", row.iconBorder, "RIGHT", 8, 0)
  row.name:SetWidth(180)

  row.count = CreateLabel(row, "GameFontHighlight", 12,
    1.0, 0.82, 0.28, "RIGHT")
  row.count:SetPoint("RIGHT", row, "RIGHT", 0, 0)
  row.count:SetWidth(42)

  row:Hide()
  requirementItemRows[i] = row
end

local actionPanel = CreateFrame("Frame", nil, rightPane)
actionPanel:SetPoint("TOPRIGHT", rightPane, "TOPRIGHT", -18, -156)
actionPanel:SetSize(230, 338)
SetSimpleBackdrop(actionPanel, 0.08, 0.07, 0.05, 0.88,
  0.38, 0.30, 0.14, 0.88)

local actionTitle = CreateLabel(actionPanel, "GameFontHighlight", 13,
  1.0, 0.82, 0.28, "LEFT")
actionTitle:SetPoint("TOPLEFT", actionPanel, "TOPLEFT", 14, -10)
actionTitle:SetText("선택")

local actionButtons = {}
local SetStatusText
local SetSelectedItem
local function AcquireActionButton(index)
  if actionButtons[index] then
    return actionButtons[index]
  end

  local button = CreateFrame("Button", nil, actionPanel)
  button:SetHeight(32)
  button:SetWidth(202)
  SetSimpleBackdrop(button, 0.18, 0.11, 0.05, 0.96,
    0.76, 0.58, 0.20, 0.94)

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

local slotButtons = {}
local function AcquireSlotButton(index)
  if slotButtons[index] then
    return slotButtons[index]
  end

  local button = CreateFrame("Button", nil, leftPane)
  button:SetSize(38, 38)
  button:EnableMouse(true)
  button:RegisterForClicks("LeftButtonUp")
  button:SetFrameStrata("DIALOG")
  button:SetFrameLevel(leftPane:GetFrameLevel() + 20)
  SetSimpleBackdrop(button, 0.10, 0.09, 0.07, 0.90,
    0.26, 0.22, 0.14, 0.84)

  button.icon = button:CreateTexture(nil, "ARTWORK")
  button.icon:SetPoint("TOPLEFT", button, "TOPLEFT", 5, -5)
  button.icon:SetPoint("BOTTOMRIGHT", button, "BOTTOMRIGHT", -5, 5)
  button.icon:SetTexture(nil)

  button.levelText = CreateLabel(button, "GameFontHighlightSmall", 11,
    0.98, 0.82, 0.28, "RIGHT")
  button.levelText:SetPoint("BOTTOMRIGHT", button, "BOTTOMRIGHT", -3, 4)
  button.levelText:SetWidth(28)
  button.levelText:SetJustifyV("BOTTOM")

  button:SetScript("OnEnter", function(self)
    if self.isSelectable then
      self:SetBackdropBorderColor(0.88, 0.70, 0.28, 0.96)
    end
    if self.itemLink then
      GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
      GameTooltip:SetHyperlink(self.itemLink)
      GameTooltip:Show()
    end
  end)

  button:SetScript("OnLeave", function(self)
    if self.isSelected then
      self:SetBackdropBorderColor(0.98, 0.82, 0.34, 1.0)
    elseif self.isSelectable then
      self:SetBackdropBorderColor(0.34, 0.28, 0.18, 0.82)
    else
      self:SetBackdropBorderColor(0.18, 0.16, 0.12, 0.48)
    end
    GameTooltip:Hide()
  end)

  button:SetScript("OnClick", function(self)
    local actionIndex = self.actionIndex
    if not actionIndex and self.itemLink then
      actionIndex = ACTION_ITEM_SELECT_BASE + self.slotId
    end

    DebugMessage("slot click slotId=" .. tostring(self.slotId)
      .. " selectable=" .. tostring(self.isSelectable)
      .. " action=" .. tostring(actionIndex))

    if not self.isSelectable or not actionIndex then
      local options = ReadGossipOptions()
      local page = ParsePage(options)
      if page == "root" or page == "info" then
        local showItemsIndex = FindOptionIndex(options, function(option)
          return string.find(option.cleanText,
            "장착 중인 장비를 보여주세요", 1, true) ~= nil
        end)
        if showItemsIndex then
          EtherealForge.state.selectedSlotKey = self.slotLabel
          EtherealForge.state.selectedSlotId = self.slotId
          SetStatusText("장비 목록을 먼저 불러오는 중입니다.")
          SelectAction(showItemsIndex, "equipment")
          return
        end
      end
      return
    end
    local ok, err = pcall(function()
      DebugMessage("slot click continue")
      EtherealForge.state.selectedSlotKey = self.slotLabel
      EtherealForge.state.selectedSlotId = self.slotId
      EtherealForge.state.resultText = ""
      DebugMessage("slot click before selected item")
      SetSelectedItem(self.slotLabel, self.slotId,
        self.slotLabel .. " 장비의 강화 메뉴를 불러오는 중입니다.")
      DebugMessage("slot click after selected item")
      ShowLocalTypeMenu(self.slotLabel, self.slotId)
      DebugMessage("slot click showed local type menu")
    end)
    if not ok then
      DebugMessage("slot handler error=" .. tostring(err))
    end
  end)

  slotButtons[index] = button
  return button
end

local function UpdateCharacterHeader()
  playerModel:SetUnit("player")
  local r, g, b = GetClassColor()
  nameText:SetText(UnitName("player") or "이름 없음")
  nameText:SetTextColor(r, g, b)
  infoText:SetText(string.format("레벨 %d · %s",
    UnitLevel("player") or 0, UnitClass("player") or "-"))
end

SetStatusText = function(text)
  EtherealForge.state.statusText = text or ""
  statusLine:SetText(EtherealForge.state.statusText)
end

local function ResetDetailPane(titleText, subtitleText)
  selectedItemText:SetText(titleText or "장비를 선택해주세요")
  selectedSlotText:SetText(subtitleText
    or "왼쪽 장비 슬롯을 클릭하면 오른쪽에 강화 메뉴가 열립니다.")
  itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  for i = 1, #detailLines do
    detailLines[i]:SetText("")
  end

  HideRequirementItems()

  for i = 1, #actionButtons do
    actionButtons[i]:Hide()
    actionButtons[i].payload = nil
  end

  lastResultText:SetText(EtherealForge.state.resultText or "")
end

local function ApplyDetailLines(lines)
  for i = 1, #detailLines do
    detailLines[i]:SetText(lines[i] or "")
  end
end

local function HideRequirementItems()
  for i = 1, #requirementItemRows do
    requirementItemRows[i]:Hide()
  end
end

local function ApplyRequirementItems(items)
  HideRequirementItems()

  for i = 1, #items do
    local row = requirementItemRows[i]
    local item = items[i]
    if row and item then
      row.icon:SetTexture(item.icon
        or "Interface\\Icons\\INV_Misc_QuestionMark")
      row.name:SetText(item.name or "필요 재료")
      row.count:SetText("x" .. tostring(item.count or 0))
      row:Show()
    end
  end
end

local function ParseRequirementItems(options)
  local items = {}

  for _, option in ipairs(options) do
    local text = option.cleanText or ""
    if string.find(text, "재료:", 1, true) ~= nil then
      local itemId = tonumber(string.match(text, "item:(%d+)"))
      local count = tonumber(string.match(text, " x(%d+)"))
      local name = string.match(text, "%[(.-)%]")

      if itemId and count and name then
        table.insert(items, {
          itemId = itemId,
          count = count,
          name = name,
          icon = GetItemIcon(itemId),
        })
      end
    end
  end

  return items
end

SetSelectedItem = function(slotLabel, slotId, subtitleText)
  local ok, err = pcall(function()
    local item = GetItemDisplay(slotId) or {}
    local quality = tonumber(item.quality) or 1
    local color = QUALITY_COLORS[quality] or QUALITY_COLORS[1]
    local itemName = item.name

    if not itemName or itemName == "" then
      itemName = (slotLabel or "선택한") .. " 장비"
    end

    if selectedItemText then
      selectedItemText:SetText("|c" .. color .. itemName .. "|r")
    end
    if selectedSlotText then
      selectedSlotText:SetText(
        subtitleText or ((slotLabel or "선택한") .. " 장비 강화 메뉴")
      )
    end

    if itemIcon then
      if item.texture then
        itemIcon:SetTexture(item.texture)
      else
        itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
      end
    end

    if selectedSlotSummary then
      selectedSlotSummary:SetText((slotLabel or "선택한") .. " 장비가 선택되었습니다.")
    end
  end)

  if not ok then
    DebugMessage("SetSelectedItem error=" .. tostring(err))
  end
end

local function SetResultText(text)
  EtherealForge.state.resultText = text or ""
  local r, g, b = GetResultColor(text)
  lastResultText:SetTextColor(r, g, b)
  lastResultText:SetText(EtherealForge.state.resultText)
end

local function BuildEquipmentButtons()
  UpdateCharacterHeader()

  for index, def in ipairs(SLOT_LAYOUT) do
  local button = AcquireSlotButton(index)
  local item = GetItemDisplay(def.slotId)
  local slotAction = EtherealForge.state.equipmentActions[def.slotId]

    button:ClearAllPoints()
    button:SetPoint("TOPLEFT", leftPane, "TOPLEFT", def.x, def.y)

    button.slotLabel = def.label
    button.slotId = def.slotId
    button.itemLink = item.itemLink
    button.actionIndex = slotAction and slotAction.actionIndex or nil
    button.isSelectable = item.itemLink ~= nil
    button.isSelected = EtherealForge.state.selectedSlotKey == def.label

    if item.itemLink and item.texture then
      button.icon:SetTexture(item.texture)
      button.icon:SetTexCoord(0.08, 0.92, 0.08, 0.92)
    else
      button.icon:SetTexture("Interface\\PaperDoll\\UI-PaperDoll-Slot-BG")
      button.icon:SetTexCoord(0, 1, 0, 1)
    end

    if button.isSelectable then
      if button.isSelected then
        button:SetBackdropColor(0.22, 0.14, 0.04, 0.94)
        button:SetBackdropBorderColor(0.98, 0.82, 0.34, 1.0)
      else
        button:SetBackdropColor(0.10, 0.09, 0.07, 0.90)
        button:SetBackdropBorderColor(0.34, 0.28, 0.18, 0.82)
      end
    else
      button:SetBackdropColor(0.08, 0.08, 0.08, 0.60)
      button:SetBackdropBorderColor(0.18, 0.16, 0.12, 0.48)
    end

    if item.level > 0 then
      button.levelText:SetText("+" .. item.level)
    else
      button.levelText:SetText("")
    end

    button:Show()
  end
end

local function ShowActionButtons(buttonDefs)
  for i = 1, #actionButtons do
    actionButtons[i]:Hide()
  end

  for index, def in ipairs(buttonDefs) do
    local button = AcquireActionButton(index)
    button:ClearAllPoints()
    if index == 1 then
      button:SetPoint("TOPLEFT", actionPanel, "TOPLEFT", 14, -38)
    else
      button:SetPoint("TOPLEFT", actionButtons[index - 1], "BOTTOMLEFT",
        0, -10)
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

local function ShowEquipmentPage(options)
  EtherealForge.state.autoRouting = false
  EtherealForge.state.transitioning = false
  EtherealForge.state.equipmentActions = {}

  local infoIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "아이템 강화 시스템", 1, true)
      ~= nil
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)

  local hasItems = false
  local mappedCount = 0
  for _, option in ipairs(options) do
    local slotLabel = string.match(option.cleanText, "^%[(.-)%]")
    if slotLabel then
      local slotId = SLOT_LABEL_TO_ID[slotLabel]
      if slotId then
        EtherealForge.state.equipmentActions[slotId] = {
          actionIndex = option.index,
          label = slotLabel,
        }
        hasItems = true
        mappedCount = mappedCount + 1
      end
    end
  end

  DebugMessage("equipment mapped slots=" .. tostring(mappedCount))

  BuildEquipmentButtons()

  if hasItems then
    SetStatusText("왼쪽 장비 슬롯을 클릭하면 오른쪽에서 강화 메뉴를 선택할 수 있습니다.")
    if not EtherealForge.state.selectedSlotKey
      or not EtherealForge.state.equipmentActions[
        EtherealForge.state.selectedSlotId] then
      ResetDetailPane("강화할 장비를 선택해주세요",
        "왼쪽 캐릭터 장비창에서 원하는 장비를 먼저 선택하세요.")
      selectedSlotSummary:SetText(
        "장비 슬롯을 클릭하면 오른쪽에 강화 메뉴가 열립니다."
      )
      ApplyDetailLines({
        "장착 중인 장비만 강화할 수 있습니다.",
        "장비를 선택하면 유형 선택 > 강화 확인 > 강화 진행 순서로 이어집니다.",
        "현재 강화 수치는 각 슬롯 아이콘의 +값으로 표시됩니다.",
      })
    end
  else
    SetStatusText("강화 가능한 아이템이 없습니다.")
    ResetDetailPane("강화 가능한 아이템이 없습니다",
      "장비 착용 여부와 최대 강화 단계 도달 여부를 확인해주세요.")
    selectedSlotSummary:SetText("강화 가능한 장비가 없습니다.")
    ApplyDetailLines({
      "강화 가능한 장비만 왼쪽 장비창에서 활성화됩니다.",
      "이미 최대 강화 단계에 도달한 장비는 목록에서 제외됩니다.",
    })
  end

  ShowActionButtons({
    {
      label = "강화 안내",
      payload = function()
        SelectAction(infoIndex)
      end,
    },
    {
      label = "장비 새로고침",
      payload = function()
        if backIndex then
          SelectAction(backIndex)
          C_Timer.After(0.05, function()
            local current = ReadGossipOptions()
            local showItemsIndex = FindOptionIndex(current, function(option)
              return option.cleanText == "장착 중인 장비를 보여주세요"
            end)
            SelectAction(showItemsIndex)
          end)
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
  EtherealForge.state.autoRouting = false
  EtherealForge.state.transitioning = false
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

local function ShowRootPage(options)
  EtherealForge.state.autoRouting = false
  EtherealForge.state.transitioning = false

  ResetDetailPane("에테르 강화 메뉴",
    "장비 목록을 불러와 원하는 장비를 선택하세요.")
  ApplyDetailLines({
    "왼쪽은 캐릭터 장비 슬롯, 오른쪽은 강화 메뉴입니다.",
    "장착 장비는 서버 gossip 흐름을 그대로 사용해서 강화합니다.",
    "먼저 아래 버튼으로 장비 목록을 불러오세요.",
  })

  local showItemsIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText,
      "장착 중인 장비를 보여주세요", 1, true) ~= nil
  end)
  local infoIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText,
      "아이템 강화 시스템", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "장비 목록 불러오기",
      payload = function()
        SetStatusText("강화 가능한 장비를 불러오는 중입니다.")
        SelectAction(showItemsIndex, "equipment")
      end,
    },
    {
      label = "강화 안내",
      payload = function()
        SelectAction(infoIndex, "info")
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

local function BuildTypedConfirmAction(slotId, enhanceType)
  return ACTION_ENHANCE_CONFIRM_BASE + (slotId * 10) + enhanceType
end

ShowLocalTypeMenu = function(slotLabel, slotId)
  SetStatusText(slotLabel .. " 장비의 강화 유형을 선택하세요.")
  SetSelectedItem(slotLabel, slotId, slotLabel .. " 장비의 강화 유형을 선택하세요.")
  ApplyDetailLines({
    "선택 장비: " .. slotLabel,
    "현재 강화: +" .. tostring(GetEnhancementLevel(
      GetInventoryItemLink("player", slotId)) or 0),
    "다음 단계: 강화 유형 선택 필요",
    "가능 유형: 밀리 / 캐스터 / 힐러 / 탱커",
  })
  HideRequirementItems()

  ShowActionButtons({
    {
      label = "[밀리]",
      payload = function()
        SelectAction(BuildTypedConfirmAction(slotId,
          ENHANCE_TYPE_MELEE), "confirm")
      end,
    },
    {
      label = "[캐스터]",
      payload = function()
        SelectAction(BuildTypedConfirmAction(slotId,
          ENHANCE_TYPE_CASTER), "confirm")
      end,
    },
    {
      label = "[힐러]",
      payload = function()
        SelectAction(BuildTypedConfirmAction(slotId,
          ENHANCE_TYPE_HEALER), "confirm")
      end,
    },
    {
      label = "[탱커]",
      payload = function()
        SelectAction(BuildTypedConfirmAction(slotId,
          ENHANCE_TYPE_TANK), "confirm")
      end,
    },
  })
end

local function ShowTypePage(options)
  EtherealForge.state.autoRouting = false
  EtherealForge.state.transitioning = false
  DebugMessage("render type page")
  local slotKey = EtherealForge.state.selectedSlotKey
  local slotId = EtherealForge.state.selectedSlotId
  if not slotKey or not slotId then
    return
  end

  SetSelectedItem(slotKey, slotId, slotKey .. " 장비의 강화 유형을 선택하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    if string.find(text, "뒤로 가기", 1, true) ~= nil then
      return true
    end
    for _, label in ipairs(TYPE_ORDER) do
      if text == label then
        return true
      end
    end
    return false
  end))

  HideRequirementItems()

  local buttons = {}
  for _, label in ipairs(TYPE_ORDER) do
    local actionIndex = FindOptionIndex(options, function(option)
      return option.cleanText == label
    end)
    if actionIndex then
      table.insert(buttons, {
        label = label,
        payload = function()
          SelectAction(actionIndex, "confirm")
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
      SelectAction(backIndex, "equipment")
    end,
  })

  ShowActionButtons(buttons)
end

local function ShowConfirmPage(options)
  EtherealForge.state.autoRouting = false
  EtherealForge.state.transitioning = false
  DebugMessage("render confirm page")
  local slotKey = EtherealForge.state.selectedSlotKey
  local slotId = EtherealForge.state.selectedSlotId
  if not slotKey or not slotId then
    return
  end

  SetSelectedItem(slotKey, slotId, slotKey .. " 장비의 비용과 확률을 확인하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return text == "[강화 진행]"
      or string.find(text, "다시 선택", 1, true) ~= nil
      or string.find(text, "뒤로 가기", 1, true) ~= nil
  end))

  ApplyRequirementItems(ParseRequirementItems(options))

  local executeIndex = FindOptionIndex(options, function(option)
    return option.cleanText == "[강화 진행]"
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "다시 선택", 1, true) ~= nil
      or string.find(option.cleanText, "뒤로 가기", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "강화 진행",
      payload = function()
        SetResultText("강화 요청을 전송했습니다. 결과를 확인해주세요.")
        SelectAction(executeIndex, nil)
      end,
    },
    {
      label = "유형 다시 선택",
      payload = function()
        SelectAction(backIndex, "type")
      end,
    },
  })
end

ShowForOptions = function(options)
  local page = ParsePage(options)
  local forgeNpc = IsForgeNpc()
  local forgeRoot = IsForgeRootOptions(options)
  local isActiveSession = EtherealForge.state
    and EtherealForge.state.active

  DebugMessage(string.format(
    "page=%s forgeNpc=%s forgeRoot=%s active=%s shown=%s",
    tostring(page),
    tostring(forgeNpc),
    tostring(forgeRoot),
    tostring(isActiveSession),
    tostring(EtherealForge:IsShown())
  ))

  if not forgeNpc and not forgeRoot and not isActiveSession then
    if EtherealForge:IsShown() or EtherealForge.state.active then
      DebugMessage("non-forge gossip detected, resetting state")
      EtherealForge:Hide()
      ResetState()
    end
    return false
  end

  if page == "root" and forgeRoot then
    EtherealForge.state.active = true
    EtherealForge.state.npcName = GetCurrentNpcName()
    DebugMessage("root detected, opening forge ui")
    EtherealForge:Show()
    DebugMessage("frame shown=" .. tostring(EtherealForge:IsShown()))
    HideDefaultGossip()
    SetStatusText("강화 메뉴를 준비했습니다.")
    ShowRootPage(options)
    return true
  end

  if not EtherealForge.state.active then
    return false
  end

  EtherealForge:Show()
  DebugMessage("showing frame for page=" .. tostring(page)
    .. " shown=" .. tostring(EtherealForge:IsShown()))
  HideDefaultGossip()

  if page == "equipment" or page == "type" or page == "confirm" then
    EtherealForge.state.autoRouting = false
    EtherealForge.state.transitioning = false
  end

  if EtherealForge.state.autoRouting then
    return true
  end

  if page == "equipment" then
    ShowEquipmentPage(options)
  elseif page == "type" then
    ShowTypePage(options)
  elseif page == "confirm" then
    ShowConfirmPage(options)
  else
    ShowInfoPage(options)
  end

  return true
end

EtherealForge:RegisterEvent("GOSSIP_SHOW")
EtherealForge:RegisterEvent("GOSSIP_CLOSED")
EtherealForge:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")
EtherealForge:RegisterEvent("CHAT_MSG_SYSTEM")

EtherealForge:SetScript("OnEvent", function(self, event, ...)
  if event == "GOSSIP_SHOW" then
    DebugMessage("event=GOSSIP_SHOW")
    ShowForOptions(ReadGossipOptions())
    return
  end

  if event == "GOSSIP_CLOSED" then
    DebugMessage("event=GOSSIP_CLOSED")
    if self.state.autoRouting or self.state.transitioning then
      DebugMessage("ignoring close during transition")
      return
    end
    DebugMessage("keeping forge ui open on gossip close")
    return
  end

  if event == "PLAYER_EQUIPMENT_CHANGED" then
    if self:IsShown() then
      BuildEquipmentButtons()
      UpdateCharacterHeader()
    end
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
      SetStatusText(
        "강화 결과를 확인했습니다. NPC와 다시 대화하면 장비 목록을 갱신합니다."
      )
    end
  end
end)

EtherealForge:SetScript("OnShow", function(self)
  UpdateCharacterHeader()
  BuildEquipmentButtons()
  statusLine:SetText(self.state.statusText or "")
  if not self.state.selectedSlotKey then
    ResetDetailPane("강화할 장비를 선택해주세요",
      "왼쪽 캐릭터 장비 슬롯을 클릭하면 오른쪽에 강화 메뉴가 열립니다.")
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
