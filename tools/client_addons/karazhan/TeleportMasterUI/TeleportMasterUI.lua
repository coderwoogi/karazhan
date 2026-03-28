local addonName = ...

local PREFIX = "TELEPORT_MASTER_UI"

local function Split(input, sep)
  local parts = {}
  local start = 1
  local index = string.find(input, sep, start, true)
  while index do
    table.insert(parts, string.sub(input, start, index - 1))
    start = index + string.len(sep)
    index = string.find(input, sep, start, true)
  end
  table.insert(parts, string.sub(input, start))
  return parts
end

local function SendCommand(command, value)
  local playerName = UnitName("player")
  if not playerName then
    return
  end

  local payload = command
  if value and value ~= "" then
    payload = payload .. "\t" .. tostring(value)
  end

  SendAddonMessage(PREFIX, payload, "WHISPER", playerName)
end

local function CreateLabel(parent, template, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", template)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local Frame = CreateFrame("Frame", "TeleportMasterUIFrame", UIParent)
Frame:SetSize(860, 540)
Frame:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
Frame:SetClampedToScreen(true)
Frame:EnableMouse(true)
Frame:SetMovable(true)
Frame:RegisterForDrag("LeftButton")
Frame:SetScript("OnDragStart", Frame.StartMoving)
Frame:SetScript("OnDragStop", Frame.StopMovingOrSizing)
Frame:Hide()
table.insert(UISpecialFrames, Frame:GetName())

Frame:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
  edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
  tile = true,
  tileSize = 32,
  edgeSize = 32,
  insets = { left = 11, right = 12, top = 12, bottom = 11 },
})
Frame:SetBackdropColor(0.04, 0.04, 0.04, 0.96)

Frame.state = {
  title = "이동술사",
  subtitle = "The Karazhan",
  body = "",
  npcDisplayId = 0,
  section = "이동 가능한 지역",
  closeText = "닫기",
  refreshText = "새로고침",
  items = {},
}

local title = CreateLabel(
  Frame,
  "GameFontHighlightLarge",
  20,
  0.96,
  0.84,
  0.30,
  "CENTER"
)
title:SetPoint("TOP", Frame, "TOP", 0, -18)

local subtitle = CreateLabel(
  Frame,
  "GameFontNormal",
  12,
  0.72,
  0.72,
  0.72,
  "CENTER"
)
subtitle:SetPoint("TOP", title, "BOTTOM", 0, -4)

local close = CreateFrame("Button", nil, Frame, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", Frame, "TOPRIGHT", -10, -10)

local leftPane = CreateFrame("Frame", nil, Frame)
leftPane:SetPoint("TOPLEFT", Frame, "TOPLEFT", 24, -54)
leftPane:SetSize(268, 452)

local leftHeader = CreateLabel(
  leftPane,
  "GameFontHighlight",
  14,
  1.0,
  0.84,
  0.25
)
leftHeader:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 6, 0)
leftHeader:SetText("지역 선택")

local leftDivider = leftPane:CreateTexture(nil, "ARTWORK")
leftDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
leftDivider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
leftDivider:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 0, -22)
leftDivider:SetPoint("TOPRIGHT", leftPane, "TOPRIGHT", 0, -22)
leftDivider:SetHeight(8)

local leftBg = leftPane:CreateTexture(nil, "BACKGROUND")
leftBg:SetTexture("Interface\\Buttons\\WHITE8x8")
leftBg:SetVertexColor(0.02, 0.02, 0.02, 0.55)
leftBg:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 0, -30)
leftBg:SetPoint("BOTTOMRIGHT", leftPane, "BOTTOMRIGHT", 0, 0)

local leftScroll = CreateFrame(
  "ScrollFrame",
  "TeleportMasterUILeftScroll",
  leftPane,
  "UIPanelScrollFrameTemplate"
)
leftScroll:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 4, -34)
leftScroll:SetPoint("BOTTOMRIGHT", leftPane, "BOTTOMRIGHT", -28, 4)

local leftContent = CreateFrame("Frame", nil, leftScroll)
leftContent:SetSize(236, 1)
leftScroll:SetScrollChild(leftContent)

local rightPane = CreateFrame("Frame", nil, Frame)
rightPane:SetPoint("TOPRIGHT", Frame, "TOPRIGHT", -24, -54)
rightPane:SetSize(534, 452)

local rightBg = rightPane:CreateTexture(nil, "BACKGROUND")
rightBg:SetTexture("Interface\\Buttons\\WHITE8x8")
rightBg:SetVertexColor(0.08, 0.03, 0.03, 0.62)
rightBg:SetAllPoints(rightPane)

local rightBorder = CreateFrame("Frame", nil, rightPane)
rightBorder:SetAllPoints(rightPane)
rightBorder:SetBackdrop({
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 12,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
rightBorder:SetBackdropBorderColor(0.45, 0.30, 0.10, 0.80)

local stageBadge = CreateFrame("Frame", nil, rightPane)
stageBadge:SetSize(52, 52)
stageBadge:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 18, -16)
stageBadge:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 12,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
stageBadge:SetBackdropColor(0.12, 0.08, 0.02, 0.95)
stageBadge:SetBackdropBorderColor(0.88, 0.70, 0.22, 0.90)

local stageBadgeText = CreateLabel(
  stageBadge,
  "GameFontHighlightLarge",
  18,
  1.0,
  0.84,
  0.25,
  "CENTER"
)
stageBadgeText:SetPoint("CENTER", stageBadge, "CENTER", 0, 0)
stageBadgeText:SetText("N")

local rightTitle = CreateLabel(
  rightPane,
  "GameFontHighlightLarge",
  22,
  0.96,
  0.92,
  0.86
)
rightTitle:SetPoint("TOPLEFT", stageBadge, "TOPRIGHT", 14, -2)
rightTitle:SetWidth(420)

local rightMeta = CreateLabel(
  rightPane,
  "GameFontNormal",
  12,
  0.86,
  0.76,
  0.34
)
rightMeta:SetPoint("TOPLEFT", rightTitle, "BOTTOMLEFT", 0, -6)
rightMeta:SetWidth(420)

local rightDivider = rightPane:CreateTexture(nil, "ARTWORK")
rightDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
rightDivider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
rightDivider:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -84)
rightDivider:SetPoint("TOPRIGHT", rightPane, "TOPRIGHT", -16, -84)
rightDivider:SetHeight(8)

local npcModel = CreateFrame("PlayerModel", nil, rightPane)
npcModel:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 20, -104)
npcModel:SetPoint("BOTTOMRIGHT", rightPane, "BOTTOMRIGHT", -20, 126)
npcModel:SetCamDistanceScale(1)
npcModel:SetPosition(0, 0, 0)

local bodyText = CreateLabel(
  rightPane,
  "GameFontNormal",
  13,
  0.95,
  0.82,
  0.24
)
bodyText:SetPoint("TOPLEFT", npcModel, "BOTTOMLEFT", 0, -14)
bodyText:SetPoint("TOPRIGHT", npcModel, "BOTTOMRIGHT", 0, -14)
bodyText:SetJustifyH("LEFT")

local refreshButton = CreateFrame(
  "Button",
  nil,
  rightPane,
  "UIPanelButtonTemplate"
)
refreshButton:SetSize(120, 28)
refreshButton:SetPoint("BOTTOMRIGHT", rightPane, "BOTTOMRIGHT", -18, 16)
refreshButton:SetScript("OnClick", function()
  SendCommand("REFRESH", "")
end)

local closeButton = CreateFrame(
  "Button",
  nil,
  rightPane,
  "UIPanelButtonTemplate"
)
closeButton:SetSize(120, 28)
closeButton:SetPoint("RIGHT", refreshButton, "LEFT", -10, 0)
closeButton:SetScript("OnClick", function()
  Frame:Hide()
end)

Frame.buttons = {}

local function CreateListButton(index)
  local button = CreateFrame("Button", nil, leftContent)
  button:SetSize(236, 52)
  button:SetPoint("TOPLEFT", leftContent, "TOPLEFT", 0, -((index - 1) * 56))
  button:SetBackdrop({
    bgFile = "Interface\\Buttons\\WHITE8x8",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    edgeSize = 10,
    insets = { left = 2, right = 2, top = 2, bottom = 2 },
  })
  button:SetBackdropColor(0.05, 0.05, 0.05, 0.82)
  button:SetBackdropBorderColor(0.18, 0.18, 0.18, 0.90)
  button:Hide()

  button.iconBg = button:CreateTexture(nil, "ARTWORK")
  button.iconBg:SetTexture("Interface\\Buttons\\UI-Quickslot2")
  button.iconBg:SetSize(32, 32)
  button.iconBg:SetPoint("LEFT", button, "LEFT", 10, 0)

  button.icon = button:CreateTexture(nil, "OVERLAY")
  button.icon:SetPoint("TOPLEFT", button.iconBg, "TOPLEFT", 4, -4)
  button.icon:SetPoint("BOTTOMRIGHT", button.iconBg, "BOTTOMRIGHT", -4, 4)
  button.icon:SetTexture("Interface\\Icons\\Spell_Arcane_PortalDalaran")

  button.name = CreateLabel(
    button,
    "GameFontHighlight",
    14,
    1.0,
    0.84,
    0.25
  )
  button.name:SetPoint("TOPLEFT", button.iconBg, "TOPRIGHT", 10, -2)
  button.name:SetWidth(170)

  button.meta = CreateLabel(
    button,
    "GameFontNormalSmall",
    11,
    0.72,
    0.72,
    0.72
  )
  button.meta:SetPoint("TOPLEFT", button.name, "BOTTOMLEFT", 0, -5)
  button.meta:SetWidth(170)

  button:SetScript("OnClick", function(self)
    if self.actionId then
      SendCommand("ACT", tostring(self.actionId))
    end
  end)

  Frame.buttons[index] = button
  return button
end

for i = 1, 12 do
  CreateListButton(i)
end

local function UpdateNpcModel()
  if UnitExists("npc") then
    npcModel:SetUnit("npc")
    return
  end

  if UnitExists("target") then
    npcModel:SetUnit("target")
    return
  end

  if Frame.state.npcDisplayId and Frame.state.npcDisplayId > 0
      and npcModel.SetDisplayInfo then
    npcModel:SetDisplayInfo(Frame.state.npcDisplayId)
    return
  end

  npcModel:ClearModel()
end

local function ResetState()
  Frame.state.title = "이동술사"
  Frame.state.subtitle = "The Karazhan"
  Frame.state.body = ""
  Frame.state.npcDisplayId = 0
  Frame.state.section = "이동 가능한 지역"
  Frame.state.closeText = "닫기"
  Frame.state.refreshText = "새로고침"
  Frame.state.items = {}
end

local function RefreshList()
  local count = #Frame.state.items
  if count < 1 then
    count = 1
  end
  leftContent:SetHeight((count * 56) - 4)

  for index, button in ipairs(Frame.buttons) do
    local item = Frame.state.items[index]
    if item then
      button.actionId = item.id
      button.icon:SetTexture(
        item.icon or "Interface\\Icons\\Spell_Arcane_PortalDalaran"
      )
      button.name:SetText(item.label or "")
      button.meta:SetText(item.desc or "")
      button:Show()
    else
      button.actionId = nil
      button:Hide()
    end
  end
end

local function Refresh()
  title:SetText(Frame.state.title or "이동술사")
  subtitle:SetText(Frame.state.subtitle or "")
  rightTitle:SetText(Frame.state.title or "이동술사")
  rightMeta:SetText(Frame.state.section or "이동 가능한 지역")
  bodyText:SetText(Frame.state.body or "")
  closeButton:SetText(Frame.state.closeText or "닫기")
  refreshButton:SetText(Frame.state.refreshText or "새로고침")
  UpdateNpcModel()
  RefreshList()
end

Frame:RegisterEvent("PLAYER_LOGIN")
Frame:RegisterEvent("CHAT_MSG_ADDON")
Frame:SetScript("OnEvent", function(self, event, prefix, message)
  if event == "PLAYER_LOGIN" then
    if RegisterAddonMessagePrefix then
      RegisterAddonMessagePrefix(PREFIX)
    end
    return
  end

  if prefix ~= PREFIX or type(message) ~= "string" then
    return
  end

  local parts = Split(message, "\t")
  local kind = parts[1]

  if kind == "CLEAR" then
    ResetState()
    Refresh()
    return
  end

  if kind == "HEADER" then
    Frame.state.title = parts[2] or Frame.state.title
    Frame.state.subtitle = parts[3] or ""
    Frame.state.npcDisplayId = tonumber(parts[5]) or 0
    Refresh()
    return
  end

  if kind == "BODY" then
    Frame.state.body = parts[2] or ""
    Refresh()
    return
  end

  if kind == "SECTION" then
    Frame.state.section = parts[2] or Frame.state.section
    Refresh()
    return
  end

  if kind == "CONTROL" then
    Frame.state.closeText = parts[2] or Frame.state.closeText
    Frame.state.refreshText = parts[3] or Frame.state.refreshText
    Refresh()
    return
  end

  if kind == "ITEM" then
    table.insert(Frame.state.items, {
      id = parts[2] or "",
      label = parts[3] or "",
      desc = parts[4] or "",
      icon = parts[5] or "Interface\\Icons\\Spell_Arcane_PortalDalaran",
    })
    Refresh()
    return
  end

  if kind == "SHOW" then
    Refresh()
    Frame:Show()
    return
  end

  if kind == "CLOSE" then
    Frame:Hide()
    return
  end
end)

ResetState()
Refresh()
