local EtherealForge = CreateFrame("Frame", "EtherealForgeUIFrame", UIParent)

local FORGE_NPC_ENTRY = 190014
local FORGE_UI_PREFIX = "KARAZHAN_FORGE_UI"
local FORGE_CMD_PREFIX = "KARAZHAN_FORGE_CMD"
local FORGE_DEBUG = false
local ROOT_SHOW_ITEMS_TEXT = "장착 중인 장비를 보여주세요"
local ACTION_MAIN_MENU = 1
local ACTION_INFO = 2
local ACTION_SHOW_ITEMS = 3
local ACTION_GOODBYE = 4

local ENHANCE_TYPE_BY_KEY = {
  melee = 1,
  caster = 2,
  healer = 3,
  tank = 4,
}


local TYPE_LABELS = {
  ["[밀리]"] = true,
  ["[캐스터]"] = true,
  ["[힐러]"] = true,
  ["[탱커]"] = true,
}

local TYPE_LABEL_BY_KEY = {
  melee = "[밀리]",
  caster = "[캐스터]",
  healer = "[힐러]",
  tank = "[탱커]",
}

local CLIENT_TO_SERVER_SLOT = {
  [1] = 0,
  [2] = 1,
  [3] = 2,
  [5] = 4,
  [6] = 5,
  [7] = 6,
  [8] = 7,
  [9] = 8,
  [10] = 9,
  [11] = 10,
  [12] = 11,
  [13] = 12,
  [14] = 13,
  [15] = 14,
  [16] = 15,
  [17] = 16,
  [18] = 17,
}

local SERVER_TO_CLIENT_SLOT = {}
for clientSlotId, serverSlotId in pairs(CLIENT_TO_SERVER_SLOT) do
  SERVER_TO_CLIENT_SLOT[serverSlotId] = clientSlotId
end

if RegisterAddonMessagePrefix then
  RegisterAddonMessagePrefix(FORGE_UI_PREFIX)
  RegisterAddonMessagePrefix(FORGE_CMD_PREFIX)
end

local function IsTypeLabel(text)
  if not text or text == "" then
    return false
  end

  return string.find(text, "밀리", 1, true) ~= nil
    or string.find(text, "캐스터", 1, true) ~= nil
    or string.find(text, "힐러", 1, true) ~= nil
    or string.find(text, "탱커", 1, true) ~= nil
end

local function GetTypeKey(text)
  if not text then
    return nil
  end
  if string.find(text, "밀리", 1, true) ~= nil then
    return "melee"
  end
  if string.find(text, "캐스터", 1, true) ~= nil then
    return "caster"
  end
  if string.find(text, "힐러", 1, true) ~= nil then
    return "healer"
  end
  if string.find(text, "탱커", 1, true) ~= nil then
    return "tank"
  end
  return nil
end

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

local SLOT_LABEL_TO_ID = {}
for _, slotDef in ipairs(SLOT_LAYOUT) do
  SLOT_LABEL_TO_ID[slotDef.label] = slotDef.slotId
end

local function ToServerSlotId(clientSlotId)
  return CLIENT_TO_SERVER_SLOT[clientSlotId] or clientSlotId
end

local function ToClientSlotId(serverSlotId)
  return SERVER_TO_CLIENT_SLOT[serverSlotId] or serverSlotId
end

local timerFrame = CreateFrame("Frame")
local timers = {}

local function After(delay, callback)
  table.insert(timers, { remaining = delay or 0, callback = callback })
  timerFrame:Show()
end

timerFrame:Hide()
timerFrame:SetScript("OnUpdate", function(self, elapsed)
  for i = #timers, 1, -1 do
    local timer = timers[i]
    timer.remaining = timer.remaining - elapsed
    if timer.remaining <= 0 then
      table.remove(timers, i)
      if timer.callback then
        timer.callback()
      end
    end
  end

  if #timers == 0 then
    self:Hide()
  end
end)

local function DebugMessage(text)
  if FORGE_DEBUG and DEFAULT_CHAT_FRAME and text then
    DEFAULT_CHAT_FRAME:AddMessage(
      "|cff33ff99[EtherealForgeUI]|r " .. tostring(text)
    )
  end
end

local alertOverlay
local resultOverlay
local alertTitle
local alertMessage

local function IsForgeFailureMessage(message)
  if not message or message == "" then
    return false
  end

  return string.find(message, "강화 실패", 1, true) ~= nil
    or string.find(message, "아이템 파괴", 1, true) ~= nil
    or string.find(message, "재료 부족", 1, true) ~= nil
    or string.find(message, "강화할 수", 1, true) ~= nil
    or string.find(message, "아이템을 찾을 수 없습니다", 1, true) ~= nil
    or string.find(message, "인벤토리 공간이 부족", 1, true) ~= nil
    or string.find(message, "소유하고 있지 않습니다", 1, true) ~= nil
    or string.find(message, "유형으로 고정", 1, true) ~= nil
end

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

local function SplitText(text, delim)
  local parts = {}
  if not text or text == "" then
    return parts
  end

  local pattern = "([^" .. delim .. "]+)"
  string.gsub(text, pattern, function(part)
    table.insert(parts, part)
  end)
  return parts
end

local function SendForgeCommand(...)
  local payload = table.concat({ ... }, "\t")
  DebugMessage("send=[" .. payload .. "]")
  SendAddonMessage(FORGE_CMD_PREFIX, payload, "WHISPER", UnitName("player"))
end

local function ParseItemId(itemLink)
  if not itemLink then
    return nil
  end

  return tonumber(string.match(itemLink, "item:(%d+)"))
end

local function GetNpcName()
  if UnitExists("npc") then
    return UnitName("npc")
  end

  if UnitExists("target") then
    return UnitName("target")
  end

  return nil
end

local function GetNpcEntryFromGuid(guid)
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

local function GetCurrentNpcEntry()
  local guid = UnitExists("npc") and UnitGUID("npc") or nil
  local entry = GetNpcEntryFromGuid(guid)
  if entry then
    return entry
  end

  guid = UnitExists("target") and UnitGUID("target") or nil
  return GetNpcEntryFromGuid(guid)
end

local function ReadGossipOptions()
  local raw = { GetGossipOptions() }
  local count = GetNumGossipOptions() or 0
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

local function BuildOptionsSignature(options)
  local parts = {}
  for _, option in ipairs(options) do
    table.insert(parts, option.cleanText or "")
  end
  return table.concat(parts, "|")
end

local function FindOptionIndex(options, predicate)
  for _, option in ipairs(options) do
    if predicate(option) then
      return option.index, option
    end
  end

  return nil, nil
end

local function ParsePage(options)
  local hasRootInfo = false
  local hasShowItems = false
  local hasEquipment = false
  local hasType = false
  local hasConfirm = false

  for _, option in ipairs(options) do
    local text = option.cleanText
    if string.find(text, "아이템 강화 시스템에 대해 알려주세요", 1, true) ~= nil then
      hasRootInfo = true
    elseif string.find(text, "장착 중인 장비를 보여주세요", 1, true)
      ~= nil then
      hasShowItems = true
    elseif text == "[강화 진행]" then
      hasConfirm = true
    elseif IsTypeLabel(text) then
      hasType = true
    elseif string.match(text, "^%[(.-)%]") then
      hasEquipment = true
    end
  end

  if hasRootInfo and hasShowItems then
    return "root"
  end
  if hasConfirm then
    return "confirm"
  end
  if hasType then
    return "type"
  end
  if hasEquipment then
    return "equipment"
  end

  return "info"
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

local function ParseRequirementItems(options)
  local items = {}
  for _, option in ipairs(options) do
    local raw = option.text or ""
    local clean = option.cleanText or ""
    if string.find(clean, "재료:", 1, true) ~= nil then
      local itemId = tonumber(string.match(raw, "item:(%d+)"))
      local count = tonumber(string.match(clean, " x(%d+)"))
      local name = string.match(clean, "%[(.-)%]")
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

local function HideDefaultGossip()
  if GossipFrame and GossipFrame:IsShown() then
    if HideUIPanel then
      HideUIPanel(GossipFrame)
    else
      GossipFrame:Hide()
    end
  end
end

local function CloseHiddenGossip()
  if CloseGossip then
    EtherealForge.state.ignoreNextGossipClosed = true
    CloseGossip()
  end

  HideDefaultGossip()
end

local function ReleaseGossipState()
  if CloseGossip then
    CloseGossip()
  end

  HideDefaultGossip()
end

local function ResetState()
  EtherealForge.state = {
    active = false,
    transitioning = false,
    autoRouting = false,
    selectedSlotId = nil,
    selectedServerSlotId = nil,
    selectedSlotLabel = nil,
    selectedSlotActionIndex = nil,
    pendingSlotId = nil,
    pendingSlotLabel = nil,
    pendingTypeLabel = nil,
    pendingTypeKey = nil,
    selectedTypeKey = nil,
    selectedItemLink = nil,
    equipmentActions = {},
    rootShowItemsIndex = nil,
    rootInfoIndex = nil,
    ignoreNextGossipClosed = false,
    statusText = "강화 가능한 장비를 불러오는 중입니다.",
    resultText = "",
  }

  if resultOverlay then
    resultOverlay:Hide()
  end
  if alertOverlay then
    alertOverlay:Hide()
  end
end

ResetState()

local function CloseForgeWindow()
  EtherealForge:Hide()
  ResetState()
  ReleaseGossipState()
end

local function HandleForgeEscape()
  if alertOverlay and alertOverlay:IsShown() then
    alertOverlay:Hide()
    return
  end

  CloseForgeWindow()
end

local function IsForgeOptions(options)
  local page = ParsePage(options)
  if page == "root" then
    local entry = GetCurrentNpcEntry()
    if entry == FORGE_NPC_ENTRY then
      return true
    end

    local npcName = GetNpcName()
    return npcName and string.find(npcName, "에테르", 1, true) ~= nil
  end

  return EtherealForge.state and EtherealForge.state.active
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

  if not itemLink then
    return {
      itemLink = nil,
      texture = nil,
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

local function FindEquipmentActionForSlot(slotId, itemLink, options)
  local mapped = EtherealForge.state.equipmentActions[slotId]
  if mapped and mapped.actionIndex then
    return mapped.actionIndex
  end

  local itemName = nil
  if itemLink then
    local name = GetItemInfo(itemLink)
    if name then
      itemName = TrimColorCodes(name)
    end
  end

  if not itemName or itemName == "" then
    return nil
  end

  for _, option in ipairs(options) do
    if string.match(option.cleanText, "^%[(.-)%]")
      and string.find(option.cleanText, itemName, 1, true) ~= nil then
      return option.index
    end
  end

  return nil
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

local function ShowForgeAlert(titleText, bodyText)
  if not alertOverlay then
    return
  end

  alertTitle:SetText(titleText or "강화 알림")
  alertMessage:SetText(bodyText or "")
  alertOverlay:Show()
end

EtherealForge:SetSize(980, 610)
EtherealForge:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
EtherealForge:SetFrameStrata("DIALOG")
EtherealForge:SetClampedToScreen(true)
EtherealForge:EnableMouse(true)
EtherealForge:EnableKeyboard(false)
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
  "왼쪽에서 장비를 선택하고, 오른쪽에서 강화 유형과 진행 여부를 확인하세요."
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
  "장비를 선택하면 오른쪽에 강화 메뉴가 표시됩니다."
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
for i = 1, 8 do
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
    row:SetPoint("TOPLEFT", requirementPanel, "TOPLEFT", 14, -202)
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

resultOverlay = CreateFrame("Frame", nil, EtherealForge)
resultOverlay:SetAllPoints(EtherealForge)
resultOverlay:SetFrameStrata("FULLSCREEN_DIALOG")
resultOverlay:SetFrameLevel(EtherealForge:GetFrameLevel() + 50)
resultOverlay:EnableMouse(true)
resultOverlay:EnableKeyboard(false)
resultOverlay:Hide()
SetSimpleBackdrop(resultOverlay, 0.02, 0.02, 0.03, 0.97,
  0.76, 0.60, 0.22, 0.98)

local resultTitle = CreateLabel(resultOverlay, "GameFontHighlightLarge", 26,
  1.0, 0.88, 0.40, "CENTER")
resultTitle:SetPoint("TOP", resultOverlay, "TOP", 0, -80)
resultTitle:SetWidth(760)

local resultItemName = CreateLabel(resultOverlay, "GameFontHighlightLarge", 22,
  0.96, 0.92, 0.85, "CENTER")
resultItemName:SetPoint("TOP", resultTitle, "BOTTOM", 0, -28)
resultItemName:SetWidth(760)

local resultLevelText = CreateLabel(resultOverlay, "GameFontHighlight", 18,
  1.0, 0.82, 0.28, "CENTER")
resultLevelText:SetPoint("TOP", resultItemName, "BOTTOM", 0, -20)
resultLevelText:SetWidth(760)

local resultTypeText = CreateLabel(resultOverlay, "GameFontNormal", 16,
  0.82, 0.80, 0.76, "CENTER")
resultTypeText:SetPoint("TOP", resultLevelText, "BOTTOM", 0, -14)
resultTypeText:SetWidth(760)

local resultMessageText = CreateLabel(resultOverlay, "GameFontNormalLarge", 18,
  0.92, 0.90, 0.85, "CENTER")
resultMessageText:SetPoint("TOP", resultTypeText, "BOTTOM", 0, -36)
resultMessageText:SetWidth(760)

local resultSubMessageText = CreateLabel(resultOverlay, "GameFontNormal", 14,
  0.76, 0.74, 0.70, "CENTER")
resultSubMessageText:SetPoint("TOP", resultMessageText, "BOTTOM", 0, -18)
resultSubMessageText:SetWidth(760)

local resultCloseButton = CreateFrame("Button", nil, resultOverlay, "UIPanelButtonTemplate")
resultCloseButton:SetSize(180, 32)
resultCloseButton:SetPoint("BOTTOM", resultOverlay, "BOTTOM", 0, 42)
resultCloseButton:SetText("강화 메뉴로 돌아가기")

alertOverlay = CreateFrame("Frame", nil, EtherealForge)
alertOverlay:SetPoint("CENTER", EtherealForge, "CENTER", 0, 0)
alertOverlay:SetSize(420, 220)
alertOverlay:SetFrameStrata("FULLSCREEN_DIALOG")
alertOverlay:SetFrameLevel(resultOverlay:GetFrameLevel() + 10)
alertOverlay:EnableMouse(true)
alertOverlay:EnableKeyboard(false)
alertOverlay:Hide()
SetSimpleBackdrop(alertOverlay, 0.04, 0.03, 0.03, 0.98,
  0.82, 0.32, 0.22, 0.95)

alertTitle = CreateLabel(alertOverlay, "GameFontHighlightLarge", 22,
  1.0, 0.78, 0.36, "CENTER")
alertTitle:SetPoint("TOP", alertOverlay, "TOP", 0, -26)
alertTitle:SetWidth(340)
alertTitle:SetText("강화 알림")

alertMessage = CreateLabel(alertOverlay, "GameFontNormalLarge", 16,
  0.96, 0.92, 0.88, "CENTER")
alertMessage:SetPoint("TOP", alertTitle, "BOTTOM", 0, -26)
alertMessage:SetWidth(340)

local alertConfirmButton = CreateFrame("Button", nil, alertOverlay, "UIPanelButtonTemplate")
alertConfirmButton:SetSize(120, 30)
alertConfirmButton:SetPoint("BOTTOM", alertOverlay, "BOTTOM", 0, 24)
alertConfirmButton:SetText("확인")

local actionTitle = CreateLabel(actionPanel, "GameFontHighlight", 13,
  1.0, 0.82, 0.28, "LEFT")
actionTitle:SetPoint("TOPLEFT", actionPanel, "TOPLEFT", 14, -10)
actionTitle:SetText("선택")

local slotButtons = {}
local actionButtons = {}

local function UpdateCharacterHeader()
  playerModel:SetUnit("player")
  local r, g, b = GetClassColor()
  nameText:SetText(UnitName("player") or "이름 없음")
  nameText:SetTextColor(r, g, b)
  infoText:SetText(string.format("레벨 %d · %s",
    UnitLevel("player") or 0, UnitClass("player") or "-"))
end

local function SetStatusText(text)
  EtherealForge.state.statusText = text or ""
  statusLine:SetText(EtherealForge.state.statusText)
end

local function SetResultText(text)
  EtherealForge.state.resultText = text or ""
  local r, g, b = GetResultColor(text)
  lastResultText:SetTextColor(r, g, b)
  lastResultText:SetText(EtherealForge.state.resultText)
end

local function HideRequirementItems()
  for i = 1, #requirementItemRows do
    requirementItemRows[i]:Hide()
  end
end

local function ApplyDetailLines(lines)
  for i = 1, #detailLines do
    detailLines[i]:SetText(lines[i] or "")
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

local function ResetDetailPane(titleText, subtitleText)
  selectedItemText:SetText(titleText or "장비를 선택해주세요")
  selectedSlotText:SetText(subtitleText
    or "왼쪽 장비칸을 클릭하면 오른쪽에 강화 메뉴가 표시됩니다.")
  itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
  ApplyDetailLines({})
  HideRequirementItems()

  for i = 1, #actionButtons do
    actionButtons[i]:Hide()
    actionButtons[i].payload = nil
  end

  lastResultText:SetText(EtherealForge.state.resultText or "")
end

local function SetSelectedItem(slotLabel, slotId, subtitleText)
  local item = GetItemDisplay(slotId) or {}
  local quality = tonumber(item.quality) or 1
  local color = QUALITY_COLORS[quality] or QUALITY_COLORS[1]
  local itemName = item.name

  if not itemName or itemName == "" then
    itemName = (slotLabel or "선택") .. " 장비"
  end

  selectedItemText:SetText("|c" .. color .. itemName .. "|r")
  selectedSlotText:SetText(subtitleText
    or ((slotLabel or "선택") .. " 장비 강화 메뉴"))

  if item.texture then
    itemIcon:SetTexture(item.texture)
  else
    itemIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
  end

  selectedSlotSummary:SetText((slotLabel or "선택") .. " 장비가 선택되었습니다.")
  EtherealForge.state.selectedItemLink = item.itemLink
end

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
      if not self.payload then
        return
      end

      DebugMessage("action button click=" .. tostring(def.label))
      local ok, err = pcall(function()
        self.payload()
      end)
      if not ok then
        DebugMessage("action button error=" .. tostring(err))
      end
    end)
    button:Show()
  end
end

local function BuildSlotSelectAction(slotId)
  return 100 + slotId
end

local SelectAction
local ShowCurrentPage
local HandleSlotSelection
local RequestConfirmForType

RequestConfirmForType = function(typeLabel, explicitTypeKey)
  EtherealForge.state.pendingTypeLabel = typeLabel
  EtherealForge.state.pendingTypeKey = explicitTypeKey or GetTypeKey(typeLabel)
  EtherealForge.state.selectedTypeKey = EtherealForge.state.pendingTypeKey
  if not EtherealForge.state.selectedSlotId
    or not EtherealForge.state.selectedServerSlotId
    or not EtherealForge.state.pendingTypeKey then
    DebugMessage("type key missing label=" .. tostring(typeLabel)
      .. " explicit=" .. tostring(explicitTypeKey))
    return
  end

  SetStatusText((typeLabel or "선택한 유형") .. " 강화 확인을 불러오는 중입니다.")
  SendForgeCommand("TYPE", tostring(EtherealForge.state.selectedServerSlotId),
    EtherealForge.state.pendingTypeKey)
end

SelectAction = function(actionIndex, expectedPage)
  if not actionIndex then
    return
end

  local previousOptions = ReadGossipOptions()
  local previousSignature = BuildOptionsSignature(previousOptions)
  EtherealForge.state.transitioning = true
  DebugMessage("select action=" .. tostring(actionIndex)
    .. " expected=" .. tostring(expectedPage))

  local ok, err = pcall(function()
    SelectGossipOption(actionIndex)
  end)
  if not ok then
    EtherealForge.state.transitioning = false
    DebugMessage("SelectGossipOption error=" .. tostring(err))
    return
  end

  local function poll(triesLeft)
    if triesLeft <= 0 then
      EtherealForge.state.transitioning = false
      return
    end

    After(0.10, function()
      if not EtherealForge.state.active then
        return
      end

      local count = GetNumGossipOptions() or 0
      if count <= 0 then
        poll(triesLeft - 1)
        return
      end

      local options = ReadGossipOptions()
      local signature = BuildOptionsSignature(options)
      local page = ParsePage(options)
      DebugMessage("schedule page=" .. tostring(page)
        .. " expected=" .. tostring(expectedPage))

      if signature ~= previousSignature
        or not expectedPage
        or page == expectedPage then
        EtherealForge.state.transitioning = false
        EtherealForge.state.autoRouting = false
        if IsForgeOptions(options) then
          HideDefaultGossip()
          EtherealForge:Show()
          ShowCurrentPage(options)
        end
        return
      end

      poll(triesLeft - 1)
    end)
  end

  poll(10)
end

HandleSlotSelection = function(self)
  local actionIndex = self.actionIndex
  if not actionIndex and self.itemLink then
    actionIndex = FindEquipmentActionForSlot(self.slotId, self.itemLink,
      ReadGossipOptions())
  end

  DebugMessage("slot click slotId=" .. tostring(self.slotId)
    .. " selectable=" .. tostring(self.isSelectable)
    .. " action=" .. tostring(actionIndex))

  if not self.itemLink then
    return
  end

  if not actionIndex then
    EtherealForge.state.selectedSlotId = self.slotId
    EtherealForge.state.selectedServerSlotId = ToServerSlotId(self.slotId)
    EtherealForge.state.selectedSlotLabel = self.slotLabel
    EtherealForge.state.selectedSlotActionIndex = nil
    EtherealForge.state.pendingSlotId = self.slotId
    EtherealForge.state.pendingSlotLabel = self.slotLabel
    SetSelectedItem(self.slotLabel, self.slotId,
      self.slotLabel .. " 장비를 선택했습니다.")
    ApplyDetailLines({
      "선택 장비: " .. self.slotLabel,
      "현재 강화: +" .. tostring(GetEnhancementLevel(self.itemLink) or 0),
      "다음 단계: 강화 유형 선택",
      "오른쪽에서 원하는 강화 유형을 선택하세요.",
    })
    HideRequirementItems()
    SetStatusText(self.slotLabel .. " 장비의 강화 유형을 선택하세요.")
    ShowActionButtons({
      {
        label = "[밀리]",
        payload = function()
          RequestConfirmForType("[밀리]", "melee")
        end,
      },
      {
        label = "[캐스터]",
        payload = function()
          RequestConfirmForType("[캐스터]", "caster")
        end,
      },
      {
        label = "[힐러]",
        payload = function()
          RequestConfirmForType("[힐러]", "healer")
        end,
      },
      {
        label = "[탱커]",
        payload = function()
          RequestConfirmForType("[탱커]", "tank")
        end,
      },
    })
    SendForgeCommand("SLOT", tostring(EtherealForge.state.selectedServerSlotId))
    return
  end

  EtherealForge.state.selectedSlotId = self.slotId
  EtherealForge.state.selectedServerSlotId = ToServerSlotId(self.slotId)
  EtherealForge.state.selectedSlotLabel = self.slotLabel
  EtherealForge.state.selectedSlotActionIndex = actionIndex
  EtherealForge.state.resultText = ""
  EtherealForge.state.pendingTypeLabel = nil
  EtherealForge.state.pendingTypeKey = nil
  EtherealForge.state.selectedTypeKey = nil
  SetSelectedItem(self.slotLabel, self.slotId,
    self.slotLabel .. " 장비를 선택했습니다.")
  ApplyDetailLines({
    "선택 장비: " .. self.slotLabel,
    "현재 강화: +" .. tostring(GetEnhancementLevel(self.itemLink) or 0),
    "다음 단계: 강화 유형 선택",
    "오른쪽에서 원하는 유형 버튼을 선택하세요.",
  })
  HideRequirementItems()
  SetStatusText(self.slotLabel .. " 장비의 강화 유형을 선택하세요.")

  ShowActionButtons({
    {
      label = "[밀리]",
      payload = function()
          RequestConfirmForType("[밀리]", "melee")
      end,
    },
    {
      label = "[캐스터]",
      payload = function()
          RequestConfirmForType("[캐스터]", "caster")
      end,
    },
    {
      label = "[힐러]",
      payload = function()
          RequestConfirmForType("[힐러]", "healer")
      end,
    },
    {
      label = "[탱커]",
      payload = function()
          RequestConfirmForType("[탱커]", "tank")
      end,
    },
  })
end

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

  button.levelText = CreateLabel(button, "GameFontHighlightSmall", 11,
    0.98, 0.82, 0.28, "RIGHT")
  button.levelText:SetPoint("BOTTOMRIGHT", button, "BOTTOMRIGHT", -3, 4)
  button.levelText:SetWidth(28)

  button:SetScript("OnEnter", function(self)
    if self.itemLink then
      GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
      GameTooltip:SetHyperlink(self.itemLink)
      GameTooltip:Show()
    end
  end)

  button:SetScript("OnLeave", function()
    GameTooltip:Hide()
  end)

  button:SetScript("OnClick", function(self)
    HandleSlotSelection(self)
  end)

  slotButtons[index] = button
  return button
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

    if item.itemLink and item.texture then
      button.icon:SetTexture(item.texture)
      button.icon:SetTexCoord(0.08, 0.92, 0.08, 0.92)
    else
      button.icon:SetTexture("Interface\\PaperDoll\\UI-PaperDoll-Slot-BG")
      button.icon:SetTexCoord(0, 1, 0, 1)
    end

    if item.level > 0 then
      button.levelText:SetText("+" .. item.level)
    else
      button.levelText:SetText("")
    end

    button:Show()
  end
end

local function ShowRootPage(options)
  for _, option in ipairs(options) do
    DebugMessage("root option index=" .. tostring(option.index)
      .. " text=" .. tostring(option.cleanText))
  end

  ResetDetailPane("에테르 강화 메뉴",
    "장비 목록을 불러와 강화할 장비를 선택하세요.")
  ApplyDetailLines({
    "왼쪽은 캐릭터 장비, 오른쪽은 강화 메뉴입니다.",
    "장착 중인 장비를 선택하면 강화 유형을 고를 수 있습니다.",
    "아래 버튼으로 장비 목록을 불러오세요.",
  })
  HideRequirementItems()

  local showItemsIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText,
      "장착 중인 장비를 보여주세요", 1, true) ~= nil
  end)
  local infoIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText,
      "아이템 강화 시스템에 대해 알려주세요", 1, true) ~= nil
  end)

  EtherealForge.state.rootShowItemsIndex = showItemsIndex
  EtherealForge.state.rootInfoIndex = infoIndex

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
        ResetState()
        CloseGossip()
      end,
    },
  })
end

local function ShowInfoPage(options)
  ResetDetailPane("강화 안내", "강화 규칙과 주의 사항을 확인하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return string.find(text, "<- 뒤로 가기", 1, true) ~= nil
  end))
  HideRequirementItems()

  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "<- 뒤로 가기", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "장비 목록으로",
      payload = function()
        SelectAction(backIndex, "root")
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        ResetState()
        CloseGossip()
      end,
    },
  })
end

local function ShowEquipmentPage(options)
  EtherealForge.state.equipmentActions = {}

  for _, option in ipairs(options) do
    local slotLabel = string.match(option.cleanText, "^%[(.-)%]")
    local slotId = slotLabel and SLOT_LABEL_TO_ID[slotLabel] or nil
    if slotId then
      EtherealForge.state.equipmentActions[slotId] = {
        actionIndex = option.index,
        label = slotLabel,
      }
    end
  end

  DebugMessage("equipment page selectedSlotId="
    .. tostring(EtherealForge.state.selectedSlotId)
    .. " pendingSlotId=" .. tostring(EtherealForge.state.pendingSlotId)
    .. " pendingType=" .. tostring(EtherealForge.state.pendingTypeLabel)
    .. " mappedAction="
    .. tostring(
      EtherealForge.state.selectedSlotId
      and EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId]
      and EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId].actionIndex
      or nil
    ))

  BuildEquipmentButtons()

  if EtherealForge.state.pendingTypeLabel
    and EtherealForge.state.selectedSlotId
    and EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId]
  then
    DebugMessage("equipment pendingType branch action="
      .. tostring(EtherealForge.state.equipmentActions[
        EtherealForge.state.selectedSlotId].actionIndex))
    EtherealForge.state.selectedSlotActionIndex =
      EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId].actionIndex
    SetStatusText((EtherealForge.state.pendingTypeLabel or "선택한 유형")
      .. " 강화 확인을 불러오는 중입니다.")
    local nextAction = EtherealForge.state.selectedSlotActionIndex
    After(0.05, function()
      if not EtherealForge.state.active then
        return
      end
      SelectAction(nextAction, "type")
    end)
    return
  end

  if EtherealForge.state.pendingSlotId
    and EtherealForge.state.equipmentActions[EtherealForge.state.pendingSlotId]
  then
    local pendingSlotId = EtherealForge.state.pendingSlotId
    EtherealForge.state.pendingSlotId = nil
    EtherealForge.state.pendingSlotLabel = nil
    EtherealForge.state.selectedSlotActionIndex =
      EtherealForge.state.equipmentActions[pendingSlotId].actionIndex

    if EtherealForge.state.pendingTypeLabel then
      SetSelectedItem(EtherealForge.state.selectedSlotLabel, pendingSlotId,
        EtherealForge.state.selectedSlotLabel .. " 장비의 강화 유형을 불러오는 중입니다.")
      local nextAction =
        EtherealForge.state.equipmentActions[pendingSlotId].actionIndex
      After(0.05, function()
        if not EtherealForge.state.active then
          return
        end
        SelectAction(nextAction, "type")
      end)
      return
    end

    for _, button in ipairs(slotButtons) do
      if button.slotId == pendingSlotId then
        button.actionIndex =
          EtherealForge.state.equipmentActions[pendingSlotId].actionIndex
        After(0.05, function()
          if not EtherealForge.state.active then
            return
          end
          HandleSlotSelection(button)
        end)
        return
      end
    end
  end

  if EtherealForge.state.selectedSlotId
    and EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId]
    and not EtherealForge.state.pendingTypeLabel
  then
    EtherealForge.state.selectedSlotActionIndex =
      EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId].actionIndex

    for _, button in ipairs(slotButtons) do
      if button.slotId == EtherealForge.state.selectedSlotId then
        button.actionIndex =
          EtherealForge.state.equipmentActions[EtherealForge.state.selectedSlotId].actionIndex
        After(0.05, function()
          if not EtherealForge.state.active then
            return
          end
          HandleSlotSelection(button)
        end)
        return
      end
    end
  end

  if not EtherealForge.state.selectedSlotId then
    ResetDetailPane("강화할 장비를 선택해주세요",
      "왼쪽 장비칸 중 강화할 아이템을 선택해 주세요.")
    ApplyDetailLines({
      "강화 가능한 장착 장비만 선택할 수 있습니다.",
      "장비 선택 후 강화 유형을 고르고 강화 확인 후 진행합니다.",
      "현재 강화 수치는 장비 아이콘의 +값으로 표시됩니다.",
    })
    HideRequirementItems()
  end

  local infoIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText,
      "아이템 강화 시스템에 대해 알려주세요", 1, true) ~= nil
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "<- 뒤로 가기", 1, true) ~= nil
  end)

  ShowActionButtons({
    {
      label = "강화 안내",
      payload = function()
        SelectAction(infoIndex, "info")
      end,
    },
    {
      label = "장비 목록으로",
      payload = function()
        SelectAction(backIndex, "root")
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        ResetState()
        CloseGossip()
      end,
    },
  })
end

local function ShowTypePage(options)
  local slotLabel = EtherealForge.state.selectedSlotLabel
  local slotId = EtherealForge.state.selectedSlotId
  if not slotLabel or not slotId then
    return
  end

  SetSelectedItem(slotLabel, slotId, slotLabel .. " 장비의 강화 유형을 선택하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return IsTypeLabel(text)
      or string.find(text, "<- 뒤로 가기", 1, true) ~= nil
  end))
  HideRequirementItems()

  local buttons = {}
  local pendingTypeIndex = nil
  for _, option in ipairs(options) do
    if IsTypeLabel(option.cleanText) then
      if EtherealForge.state.pendingTypeKey
        and GetTypeKey(option.cleanText) == EtherealForge.state.pendingTypeKey then
        pendingTypeIndex = option.index
      end
      table.insert(buttons, {
        label = option.cleanText,
        payload = function()
          local typeKey = GetTypeKey(option.cleanText)
          EtherealForge.state.selectedTypeKey = typeKey
          SelectAction(option.index, "confirm")
        end,
      })
    end
  end

  DebugMessage("type page slotId=" .. tostring(slotId)
    .. " pendingType=" .. tostring(EtherealForge.state.pendingTypeLabel)
    .. " pendingTypeIndex=" .. tostring(pendingTypeIndex))

  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "<- 뒤로 가기", 1, true) ~= nil
  end)
  table.insert(buttons, {
    label = "장비 목록으로",
    payload = function()
      SelectAction(backIndex, "equipment")
    end,
  })

  if pendingTypeIndex then
    local label = EtherealForge.state.pendingTypeLabel
    local typeKey = EtherealForge.state.pendingTypeKey
    EtherealForge.state.pendingTypeLabel = nil
    EtherealForge.state.pendingTypeKey = nil
    EtherealForge.state.selectedTypeKey = typeKey
    SetStatusText((label or "선택한 유형") .. " 강화 확인을 불러오는 중입니다.")
    After(0.05, function()
      if not EtherealForge.state.active then
        return
      end
      SelectAction(pendingTypeIndex, "confirm")
    end)
    return
  end

  ShowActionButtons(buttons)
end

local function ShowConfirmPage(options)
  local slotLabel = EtherealForge.state.selectedSlotLabel
  local slotId = EtherealForge.state.selectedSlotId
  if not slotLabel or not slotId then
    return
  end

  SetSelectedItem(slotLabel, slotId, slotLabel .. " 장비의 비용과 성공 확률을 확인하세요.")
  ApplyDetailLines(ParseInfoLines(options, function(text)
    return text == "[강화 진행]"
      or string.find(text, "<- 타입 다시 선택", 1, true) ~= nil
      or string.find(text, "<- 뒤로 가기", 1, true) ~= nil
      or string.find(text, "재료:", 1, true) ~= nil
  end))
  ApplyRequirementItems(ParseRequirementItems(options))

  local executeIndex = FindOptionIndex(options, function(option)
    return option.cleanText == "[강화 진행]"
  end)
  local backIndex = FindOptionIndex(options, function(option)
    return string.find(option.cleanText, "<- 타입 다시 선택", 1, true) ~= nil
      or string.find(option.cleanText, "<- 뒤로 가기", 1, true) ~= nil
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

ShowCurrentPage = function(options)
  local page = ParsePage(options)
  DebugMessage("render page=" .. tostring(page))

  if page == "root" then
    ShowRootPage(options)
  elseif page == "equipment" then
    ShowEquipmentPage(options)
  elseif page == "type" then
    ShowTypePage(options)
  elseif page == "confirm" then
    ShowConfirmPage(options)
  else
    ShowInfoPage(options)
  end
end

local function ShowBridgeTypePage(parts)
  local serverSlotId = tonumber(parts[2])
  local slotId = serverSlotId and ToClientSlotId(serverSlotId) or nil
  local itemName = parts[3] or "알 수 없는 장비"
  local currentLevel = tonumber(parts[4]) or 0
  local maxLevel = tonumber(parts[5]) or 0
  local allowedKeys = SplitText(parts[6] or "", ",")
  local summary = parts[7] or ""

  if not slotId then
    return
  end

  EtherealForge.state.selectedSlotId = slotId
  EtherealForge.state.selectedServerSlotId = serverSlotId
  EtherealForge.state.selectedSlotActionIndex = slotId
  SetSelectedItem(EtherealForge.state.selectedSlotLabel or "", slotId,
    "강화 유형을 선택하세요.")
  ApplyDetailLines({
    "선택 장비: " .. itemName,
    "현재 강화: +" .. tostring(currentLevel),
    "최대 강화: +" .. tostring(maxLevel),
    summary,
  })
  HideRequirementItems()

  local buttons = {}
  for _, key in ipairs(allowedKeys) do
    local label = TYPE_LABEL_BY_KEY[key]
    if label then
      table.insert(buttons, {
        label = label,
        payload = function()
          RequestConfirmForType(label, key)
        end,
      })
    end
  end

  if #buttons == 0 then
    table.insert(buttons, {
      label = "선택 가능한 강화 유형 없음",
      payload = function()
      end,
    })
  end

  ShowActionButtons(buttons)
  SetStatusText("강화 유형을 선택하세요.")
end

local function ShowBridgeConfirmPage(parts)
  local serverSlotId = tonumber(parts[2])
  local slotId = serverSlotId and ToClientSlotId(serverSlotId) or nil
  local itemName = parts[3] or "알 수 없는 장비"
  local currentLevel = tonumber(parts[4]) or 0
  local targetLevel = tonumber(parts[5]) or 0
  local maxLevel = tonumber(parts[6]) or 0
  local typeKey = parts[7] or ""
  local typeName = parts[8] or ""
  local successRate = parts[9] or "0.0"
  local goldCost = tonumber(parts[10]) or 0
  local materials = {
    {
      itemId = tonumber(parts[11]) or 0,
      count = tonumber(parts[12]) or 0,
      name = parts[13] or "",
    },
    {
      itemId = tonumber(parts[14]) or 0,
      count = tonumber(parts[15]) or 0,
      name = parts[16] or "",
    },
    {
      itemId = tonumber(parts[17]) or 0,
      count = tonumber(parts[18]) or 0,
      name = parts[19] or "",
    },
  }

  if not slotId then
    return
  end

  EtherealForge.state.selectedSlotId = slotId
  EtherealForge.state.selectedServerSlotId = serverSlotId
  EtherealForge.state.selectedTypeKey = typeKey
  SetSelectedItem(EtherealForge.state.selectedSlotLabel or "", slotId,
    "강화 조건과 재료를 확인하세요.")
  ApplyDetailLines({
    "선택 장비: " .. itemName,
    "다음 강화: +" .. tostring(currentLevel) .. " -> +" .. tostring(targetLevel),
    "최대 강화: +" .. tostring(maxLevel),
    "선택 유형: " .. typeName,
    "성공 확률: " .. tostring(successRate) .. "%",
    "비용: " .. tostring(goldCost) .. " 골드",
  })

  local requirementItems = {}
  for _, material in ipairs(materials) do
    if material.itemId > 0 and material.count > 0 then
      table.insert(requirementItems, {
        itemId = material.itemId,
        count = material.count,
        name = material.name,
        icon = GetItemIcon(material.itemId),
      })
    end
  end
  ApplyRequirementItems(requirementItems)

  ShowActionButtons({
    {
      label = "강화 진행",
      payload = function()
        if EtherealForge.state.selectedSlotId and EtherealForge.state.selectedTypeKey then
          SetResultText("강화 요청을 전송했습니다. 결과를 확인해주세요.")
          SendForgeCommand("DO", tostring(EtherealForge.state.selectedServerSlotId),
            EtherealForge.state.selectedTypeKey)
        end
      end,
    },
    {
      label = "유형 다시 선택",
      payload = function()
        if EtherealForge.state.selectedServerSlotId then
          SendForgeCommand("SLOT", tostring(EtherealForge.state.selectedServerSlotId))
        end
      end,
    },
  })
  SetStatusText("강화 진행 여부를 선택하세요.")
end

local function ShowBridgeInfoPage(parts)
  ResetDetailPane("강화 안내", "강화 규칙과 주의 사항을 확인하세요.")
  ApplyDetailLines(SplitText(parts[2] or "", "^"))
  HideRequirementItems()
  ShowActionButtons({
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        ResetState()
        CloseGossip()
      end,
    },
  })
end

local function ReturnToForgeHome(message)
  if resultOverlay then
    resultOverlay:Hide()
  end
  if alertOverlay then
    alertOverlay:Hide()
  end

  EtherealForge.state.selectedSlotId = nil
  EtherealForge.state.selectedServerSlotId = nil
  EtherealForge.state.selectedSlotLabel = nil
  EtherealForge.state.selectedSlotActionIndex = nil
  EtherealForge.state.pendingSlotId = nil
  EtherealForge.state.pendingSlotLabel = nil
  EtherealForge.state.pendingTypeLabel = nil
  EtherealForge.state.pendingTypeKey = nil
  EtherealForge.state.selectedTypeKey = nil

  BuildEquipmentButtons()
  ResetDetailPane("강화할 장비를 선택해주세요",
    "왼쪽 장비칸 중 강화할 아이템을 선택해 주세요.")
  ApplyDetailLines({
    "강화 가능한 장착 장비만 선택할 수 있습니다.",
    "장비 선택 후 강화 유형을 고르고 강화 확인 후 진행합니다.",
    "현재 강화 수치는 장비 아이콘의 +값으로 표시됩니다.",
  })
  HideRequirementItems()
  SetResultText(message or "")
  SetStatusText(message or "강화 결과를 확인했습니다.")
  ShowActionButtons({
    {
      label = "강화 안내",
      payload = function()
        SendForgeCommand("INFO")
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        CloseForgeWindow()
      end,
    },
  })
  EtherealForge:Show()
end

local function ShowResultOverlay(parts)
  local resultType = parts[2] or "UNKNOWN"
  local itemName = parts[3] or "알 수 없는 장비"
  local currentLevel = tonumber(parts[4]) or 0
  local targetLevel = tonumber(parts[5]) or 0
  local typeName = parts[6] or ""
  local message = parts[7] or ""

  local titleColor = { 1.0, 0.88, 0.40 }
  local titleText = "강화 결과"
  local levelText = ""

  if resultType == "REQUESTED" then
    SetStatusText("강화 요청을 전송했습니다. 결과를 기다리는 중입니다.")
    return
  end

  if resultType == "SUCCESS" then
    titleText = "강화 성공"
    titleColor = { 0.30, 0.95, 0.45 }
    levelText = "+" .. tostring(currentLevel) .. " -> +" .. tostring(targetLevel)
  elseif resultType == "FAIL" then
    titleText = "강화 실패"
    titleColor = { 0.95, 0.52, 0.28 }
    levelText = "+" .. tostring(currentLevel) .. " 유지"
  elseif resultType == "DESTROYED" then
    titleText = "아이템 파괴"
    titleColor = { 0.95, 0.26, 0.26 }
    levelText = "+" .. tostring(currentLevel) .. " -> 파괴"
  end

  resultTitle:SetTextColor(titleColor[1], titleColor[2], titleColor[3])
  resultTitle:SetText(titleText)
  resultItemName:SetText(itemName)
  resultLevelText:SetText(levelText)
  resultTypeText:SetText(typeName ~= "" and ("선택 유형: " .. typeName) or "")
  resultMessageText:SetText(message ~= "" and message or "강화 결과를 확인했습니다.")
  resultSubMessageText:SetText("결과를 확인한 뒤 강화 메뉴로 돌아갈 수 있습니다.")
  resultOverlay:Show()

end

local function ApplyForgePayload(message)
  local parts = SplitText(message or "", "\t")
  local payloadType = parts[1]
  DebugMessage("recv=[" .. tostring(message) .. "]")
  if not payloadType then
    return
  end

  if payloadType == "TYPE" then
    ShowBridgeTypePage(parts)
    return
  end

  if payloadType == "CONFIRM" then
    ShowBridgeConfirmPage(parts)
    return
  end

  if payloadType == "INFO" then
    ShowBridgeInfoPage(parts)
    return
  end

  if payloadType == "RESULT" then
    ShowResultOverlay(parts)
    return
  end

  if payloadType == "ERROR" then
    local errorMessage = parts[2] or "강화 메뉴를 불러오지 못했습니다."
    SetStatusText(errorMessage)
    ShowForgeAlert("강화 알림", errorMessage)
    return
  end
end

local function OpenForOptions(options)
  if not IsForgeOptions(options) then
    EtherealForge:Hide()
    ResetState()
    return
  end

  ResetState()
  EtherealForge.state.active = true
  HideDefaultGossip()
  EtherealForge:Show()
  ResetDetailPane("강화할 장비를 선택해주세요",
    "왼쪽 장비칸을 클릭하면 오른쪽에 강화 메뉴가 표시됩니다.")
  ApplyDetailLines({
    "장비 슬롯을 선택하면 서버에서 강화 유형을 불러옵니다.",
    "유형 선택 후 강화 조건과 재료를 확인할 수 있습니다.",
    "강화 진행 결과는 채팅과 오른쪽 결과 영역에 표시됩니다.",
  })
  HideRequirementItems()
  ShowActionButtons({
    {
      label = "강화 안내",
      payload = function()
        SendForgeCommand("INFO")
      end,
    },
    {
      label = "창 닫기",
      payload = function()
        EtherealForge:Hide()
        ResetState()
        CloseGossip()
      end,
    },
  })
  SetStatusText("강화할 장비를 선택하세요.")
  SendForgeCommand("OPEN")
  CloseHiddenGossip()
end

EtherealForge:RegisterEvent("GOSSIP_SHOW")
EtherealForge:RegisterEvent("GOSSIP_CLOSED")
EtherealForge:RegisterEvent("PLAYER_EQUIPMENT_CHANGED")
EtherealForge:RegisterEvent("CHAT_MSG_SYSTEM")
EtherealForge:RegisterEvent("CHAT_MSG_ADDON")
EtherealForge:RegisterEvent("CHAT_MSG_WHISPER")
EtherealForge:RegisterEvent("PLAYER_LOGIN")

EtherealForge:SetScript("OnEvent", function(self, event, ...)
  if event == "PLAYER_LOGIN" then
    if RegisterAddonMessagePrefix then
      RegisterAddonMessagePrefix(FORGE_UI_PREFIX)
      RegisterAddonMessagePrefix(FORGE_CMD_PREFIX)
    end
    return
  end

  if event == "GOSSIP_SHOW" then
    return OpenForOptions(ReadGossipOptions())
  end

  if event == "GOSSIP_CLOSED" then
    if self.state and self.state.ignoreNextGossipClosed then
      self.state.ignoreNextGossipClosed = false
      return
    end

    self:Hide()
    ResetState()
    return
  end

  if event == "CHAT_MSG_ADDON" then
    local prefix, message = ...
    if prefix == FORGE_UI_PREFIX and type(message) == "string" then
      ApplyForgePayload(message)
    end
    return
  end

  if event == "CHAT_MSG_WHISPER" then
    local message = ...
    if type(message) == "string" then
      local prefix = FORGE_UI_PREFIX .. "\t"
      if string.sub(message, 1, string.len(prefix)) == prefix then
        ApplyForgePayload(string.sub(message, string.len(prefix) + 1))
        return
      end

      local payloadType = SplitText(message, "\t")[1]
      if payloadType == "TYPE"
        or payloadType == "CONFIRM"
        or payloadType == "INFO"
        or payloadType == "ERROR"
        or payloadType == "OPEN"
        or payloadType == "RESULT" then
        ApplyForgePayload(message)
        return
      end
    end
  end

  if event == "PLAYER_EQUIPMENT_CHANGED" and self:IsShown() then
    BuildEquipmentButtons()
    UpdateCharacterHeader()
    return
  end

  if event == "CHAT_MSG_SYSTEM" then
    return
  end
end)

EtherealForge:SetScript("OnShow", function(self)
  UpdateCharacterHeader()
  BuildEquipmentButtons()
  statusLine:SetText(self.state.statusText or "")
  if not self.state.selectedSlotId then
    ResetDetailPane("강화할 장비를 선택해주세요",
      "왼쪽 장비칸을 클릭하면 오른쪽에 강화 메뉴가 표시됩니다.")
  end
end)

EtherealForge:SetScript("OnHide", function(self)
  self:EnableKeyboard(false)
  ReleaseGossipState()
  ResetState()
end)

local function ForgeSystemMessageFilter(_, _, message)
  if IsForgeFailureMessage(message) then
    ShowForgeAlert("강화 알림", message)
    return true
  end

  return false
end

if ChatFrame_AddMessageEventFilter then
  ChatFrame_AddMessageEventFilter("CHAT_MSG_SYSTEM", ForgeSystemMessageFilter)
end

close:SetScript("OnClick", function()
  CloseForgeWindow()
end)

resultCloseButton:SetScript("OnClick", function()
  ReturnToForgeHome()
end)

alertConfirmButton:SetScript("OnClick", function()
  alertOverlay:Hide()
end)






