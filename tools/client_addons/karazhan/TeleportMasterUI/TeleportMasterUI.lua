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
Frame:SetSize(320, 520)
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
  subtitle = "",
  closeText = "닫기",
  refreshText = "",
  items = {},
}

local title = CreateLabel(
  Frame,
  "GameFontHighlightLarge",
  18,
  0.96,
  0.84,
  0.30,
  "CENTER"
)
title:SetPoint("TOP", Frame, "TOP", 0, -18)

local subtitle = CreateLabel(
  Frame,
  "GameFontNormal",
  11,
  0.72,
  0.72,
  0.72,
  "CENTER"
)
subtitle:SetPoint("TOP", title, "BOTTOM", 0, -4)
subtitle:SetWidth(240)

local close = CreateFrame("Button", nil, Frame, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", Frame, "TOPRIGHT", -10, -10)

local listHeader = CreateLabel(
  Frame,
  "GameFontHighlight",
  13,
  1.0,
  0.84,
  0.25
)
listHeader:SetPoint("TOPLEFT", Frame, "TOPLEFT", 18, -56)
listHeader:SetText("목적지 선택")

local divider = Frame:CreateTexture(nil, "ARTWORK")
divider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
divider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
divider:SetPoint("TOPLEFT", Frame, "TOPLEFT", 12, -76)
divider:SetPoint("TOPRIGHT", Frame, "TOPRIGHT", -12, -76)
divider:SetHeight(8)

local listBg = Frame:CreateTexture(nil, "BACKGROUND")
listBg:SetTexture("Interface\\Buttons\\WHITE8x8")
listBg:SetVertexColor(0.02, 0.02, 0.02, 0.55)
listBg:SetPoint("TOPLEFT", Frame, "TOPLEFT", 14, -84)
listBg:SetPoint("BOTTOMRIGHT", Frame, "BOTTOMRIGHT", -14, 54)

local scroll = CreateFrame(
  "ScrollFrame",
  "TeleportMasterUILeftScroll",
  Frame,
  "UIPanelScrollFrameTemplate"
)
scroll:SetPoint("TOPLEFT", Frame, "TOPLEFT", 18, -88)
scroll:SetPoint("BOTTOMRIGHT", Frame, "BOTTOMRIGHT", -38, 58)

local content = CreateFrame("Frame", nil, scroll)
content:SetSize(248, 1)
scroll:SetScrollChild(content)

local closeButton = CreateFrame(
  "Button",
  nil,
  Frame,
  "UIPanelButtonTemplate"
)
closeButton:SetSize(110, 26)
closeButton:SetPoint("BOTTOM", Frame, "BOTTOM", 0, 18)
closeButton:SetScript("OnClick", function()
  Frame:Hide()
end)

Frame.buttons = {}

local function CreateListButton(index)
  local button = CreateFrame("Button", nil, content)
  button:SetSize(248, 56)
  button:SetPoint("TOPLEFT", content, "TOPLEFT", 0, -((index - 1) * 60))
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
  button.iconBg:SetSize(30, 30)
  button.iconBg:SetPoint("LEFT", button, "LEFT", 10, 0)

  button.icon = button:CreateTexture(nil, "OVERLAY")
  button.icon:SetPoint("TOPLEFT", button.iconBg, "TOPLEFT", 4, -4)
  button.icon:SetPoint("BOTTOMRIGHT", button.iconBg, "BOTTOMRIGHT", -4, 4)
  button.icon:SetTexture("Interface\\Icons\\Spell_Arcane_PortalDalaran")

  button.name = CreateLabel(
    button,
    "GameFontHighlight",
    13,
    1.0,
    0.84,
    0.25
  )
  button.name:SetPoint("TOPLEFT", button.iconBg, "TOPRIGHT", 10, -2)
  button.name:SetWidth(190)
  button.name:SetHeight(26)
  if button.name.SetWordWrap then
    button.name:SetWordWrap(true)
  end

  button.meta = CreateLabel(
    button,
    "GameFontNormalSmall",
    10,
    0.72,
    0.72,
    0.72
  )
  button.meta:SetPoint("TOPLEFT", button.name, "BOTTOMLEFT", 0, -2)
  button.meta:SetWidth(190)
  button.meta:SetHeight(22)
  if button.meta.SetWordWrap then
    button.meta:SetWordWrap(true)
  end

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

local function ResetState()
  Frame.state.title = "이동술사"
  Frame.state.subtitle = ""
  Frame.state.closeText = "닫기"
  Frame.state.refreshText = ""
  Frame.state.items = {}
end

local function RefreshList()
  local count = #Frame.state.items
  if count < 1 then
    count = 1
  end
  content:SetHeight((count * 60) - 4)

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
  closeButton:SetText(Frame.state.closeText or "닫기")
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
    Refresh()
    return
  end

  if kind == "CONTROL" then
    Frame.state.closeText = parts[2] or Frame.state.closeText
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
