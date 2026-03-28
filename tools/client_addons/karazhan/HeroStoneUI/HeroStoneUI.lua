local addonName = ...

local PREFIX = "HERO_STONE_UI"

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

local Frame = CreateFrame("Frame", "HeroStoneUIFrame", UIParent)
Frame:SetSize(600, 378)
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
  title = "Hero Stone",
  subtitle = "",
  body = "",
  icon = "Interface\\Icons\\INV_Misc_Rune_01",
  section = "Available Features",
  closeText = "Close",
  refreshText = "Refresh",
  items = {},
  isSubscriber = false,
  remainDays = 0,
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
leftPane:SetPoint("TOPLEFT", Frame, "TOPLEFT", 20, -52)
leftPane:SetSize(220, 300)

local leftHeader = CreateLabel(
  leftPane,
  "GameFontHighlight",
  14,
  1.0,
  0.84,
  0.25
)
leftHeader:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 6, 0)
leftHeader:SetText("Feature List")

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
  "HeroStoneUILeftScroll",
  leftPane,
  "UIPanelScrollFrameTemplate"
)
leftScroll:SetPoint("TOPLEFT", leftPane, "TOPLEFT", 4, -34)
leftScroll:SetPoint("BOTTOMRIGHT", leftPane, "BOTTOMRIGHT", -28, 4)

local leftContent = CreateFrame("Frame", nil, leftScroll)
leftContent:SetSize(188, 1)
leftScroll:SetScrollChild(leftContent)

local rightPane = CreateFrame("Frame", nil, Frame)
rightPane:SetPoint("TOPRIGHT", Frame, "TOPRIGHT", -20, -52)
rightPane:SetSize(336, 300)

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

local iconFrame = CreateFrame("Frame", nil, rightPane)
iconFrame:SetSize(66, 66)
iconFrame:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -16)
iconFrame:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 12,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
iconFrame:SetBackdropColor(0.12, 0.08, 0.02, 0.95)
iconFrame:SetBackdropBorderColor(0.88, 0.70, 0.22, 0.90)

local iconTexture = iconFrame:CreateTexture(nil, "ARTWORK")
iconTexture:SetPoint("TOPLEFT", iconFrame, "TOPLEFT", 8, -8)
iconTexture:SetPoint("BOTTOMRIGHT", iconFrame, "BOTTOMRIGHT", -8, 8)
iconTexture:SetTexture(Frame.state.icon)

local infoTitle = CreateLabel(
  rightPane,
  "GameFontHighlightLarge",
  18,
  0.96,
  0.92,
  0.86
)
infoTitle:SetPoint("TOPLEFT", iconFrame, "TOPRIGHT", 12, -2)
infoTitle:SetWidth(236)

local infoSubtitle = CreateLabel(
  rightPane,
  "GameFontNormal",
  12,
  0.86,
  0.76,
  0.34
)
infoSubtitle:SetPoint("TOPLEFT", infoTitle, "BOTTOMLEFT", 0, -4)
infoSubtitle:SetWidth(236)

local gaugeLabel = CreateLabel(
  rightPane,
  "GameFontHighlight",
  12,
  1.0,
  0.84,
  0.25
)
gaugeLabel:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -94)
gaugeLabel:SetText("Subscription")

local gaugeBg = CreateFrame("Frame", nil, rightPane)
gaugeBg:SetSize(304, 18)
gaugeBg:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -116)
gaugeBg:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 10,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
gaugeBg:SetBackdropColor(0.05, 0.05, 0.05, 0.92)
gaugeBg:SetBackdropBorderColor(0.30, 0.24, 0.12, 0.90)

local gaugeFill = gaugeBg:CreateTexture(nil, "ARTWORK")
gaugeFill:SetTexture("Interface\\TARGETINGFRAME\\UI-StatusBar")
gaugeFill:SetVertexColor(0.92, 0.70, 0.18, 0.95)
gaugeFill:SetPoint("TOPLEFT", gaugeBg, "TOPLEFT", 2, -2)
gaugeFill:SetPoint("BOTTOMLEFT", gaugeBg, "BOTTOMLEFT", 2, 2)
gaugeFill:SetWidth(1)

local gaugeText = CreateLabel(
  gaugeBg,
  "GameFontNormalSmall",
  11,
  1.0,
  1.0,
  1.0,
  "CENTER"
)
gaugeText:SetPoint("CENTER", gaugeBg, "CENTER", 0, 0)

local sectionTitle = CreateLabel(
  rightPane,
  "GameFontHighlight",
  13,
  1.0,
  0.84,
  0.25
)
sectionTitle:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -146)

local divider = rightPane:CreateTexture(nil, "ARTWORK")
divider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
divider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
divider:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -166)
divider:SetPoint("TOPRIGHT", rightPane, "TOPRIGHT", -16, -166)
divider:SetHeight(8)

local bodyText = CreateLabel(
  rightPane,
  "GameFontNormal",
  12,
  0.95,
  0.82,
  0.24
)
bodyText:SetPoint("TOPLEFT", rightPane, "TOPLEFT", 16, -182)
bodyText:SetPoint("TOPRIGHT", rightPane, "TOPRIGHT", -16, -182)
bodyText:SetJustifyH("LEFT")

local closeButton = CreateFrame(
  "Button",
  nil,
  rightPane,
  "UIPanelButtonTemplate"
)
closeButton:SetSize(110, 26)
closeButton:SetPoint("BOTTOMLEFT", rightPane, "BOTTOMLEFT", 16, 14)
closeButton:SetScript("OnClick", function()
  Frame:Hide()
end)

local refreshButton = CreateFrame(
  "Button",
  nil,
  rightPane,
  "UIPanelButtonTemplate"
)
refreshButton:SetSize(110, 26)
refreshButton:SetPoint("LEFT", closeButton, "RIGHT", 10, 0)
refreshButton:SetScript("OnClick", function()
  SendCommand("REFRESH", "")
end)

Frame.buttons = {}

local function CreateListButton(index)
  local button = CreateFrame("Button", nil, leftContent)
  button:SetSize(188, 48)
  button:SetPoint("TOPLEFT", leftContent, "TOPLEFT", 0, -((index - 1) * 52))
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
  button.iconBg:SetSize(28, 28)
  button.iconBg:SetPoint("LEFT", button, "LEFT", 8, 0)

  button.icon = button:CreateTexture(nil, "OVERLAY")
  button.icon:SetPoint("TOPLEFT", button.iconBg, "TOPLEFT", 4, -4)
  button.icon:SetPoint("BOTTOMRIGHT", button.iconBg, "BOTTOMRIGHT", -4, 4)
  button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  button.name = CreateLabel(
    button,
    "GameFontHighlight",
    13,
    1.0,
    0.84,
    0.25
  )
  button.name:SetPoint("TOPLEFT", button.iconBg, "TOPRIGHT", 10, -2)
  button.name:SetWidth(134)

  button.meta = CreateLabel(
    button,
    "GameFontNormalSmall",
    10,
    0.72,
    0.72,
    0.72
  )
  button.meta:SetPoint("TOPLEFT", button.name, "BOTTOMLEFT", 0, -4)
  button.meta:SetWidth(134)

  button:SetScript("OnClick", function(self)
    if self.actionId then
      SendCommand("ACT", tostring(self.actionId))
    end
  end)

  Frame.buttons[index] = button
  return button
end

for i = 1, 10 do
  CreateListButton(i)
end

local function ParseSubscription()
  local remainDays = tonumber(string.match(Frame.state.subtitle or "", "(%d+)"))
    or 0
  local isSubscriber = remainDays > 0
  Frame.state.remainDays = remainDays
  Frame.state.isSubscriber = isSubscriber and remainDays > 0
end

local function UpdateGauge()
  local value = 0
  local maxDays = 30

  if Frame.state.isSubscriber then
    value = math.min(Frame.state.remainDays, maxDays)
    gaugeFill:SetVertexColor(0.92, 0.70, 0.18, 0.95)
    gaugeText:SetText(string.format("%d days remaining", Frame.state.remainDays))
  else
    value = 0
    gaugeFill:SetVertexColor(0.45, 0.12, 0.12, 0.95)
    gaugeText:SetText("No subscription")
  end

  local totalWidth = 300
  local width = math.max(1, math.floor((value / maxDays) * totalWidth))
  if value <= 0 then
    width = 1
  end
  gaugeFill:SetWidth(width)
end

local function ResetState()
  Frame.state.title = "Hero Stone"
  Frame.state.subtitle = ""
  Frame.state.body = ""
  Frame.state.icon = "Interface\\Icons\\INV_Misc_Rune_01"
  Frame.state.section = "Available Features"
  Frame.state.closeText = "Close"
  Frame.state.refreshText = "Refresh"
  Frame.state.items = {}
  Frame.state.isSubscriber = false
  Frame.state.remainDays = 0
end

local function RefreshList()
  local count = #Frame.state.items
  if count < 1 then
    count = 1
  end
  leftContent:SetHeight((count * 52) - 4)

  for index, button in ipairs(Frame.buttons) do
    local item = Frame.state.items[index]
    if item then
      button.actionId = item.id
      button.icon:SetTexture(
        item.icon or "Interface\\Icons\\INV_Misc_QuestionMark"
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
  ParseSubscription()
  iconTexture:SetTexture(
    Frame.state.icon or "Interface\\Icons\\INV_Misc_QuestionMark"
  )
  title:SetText(Frame.state.title or "Hero Stone")
  subtitle:SetText(Frame.state.subtitle or "")
  infoTitle:SetText(Frame.state.title or "Hero Stone")
  infoSubtitle:SetText(Frame.state.subtitle or "")
  sectionTitle:SetText(Frame.state.section or "Available Features")
  bodyText:SetText(Frame.state.body or "")
  closeButton:SetText(Frame.state.closeText or "Close")
  refreshButton:SetText(Frame.state.refreshText or "Refresh")
  UpdateGauge()
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
    Frame.state.icon = parts[4] or Frame.state.icon
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
      icon = parts[5] or "Interface\\Icons\\INV_Misc_QuestionMark",
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
