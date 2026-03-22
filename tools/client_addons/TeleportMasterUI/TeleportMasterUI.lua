local addonName = ...

local PREFIX = "TELEPORT_MASTER_UI"

local frame = CreateFrame("Frame", "TeleportMasterUIFrame", UIParent)
frame:SetSize(430, 636)
frame:SetPoint("RIGHT", UIParent, "RIGHT", -84, -6)
frame:SetScale(0.72)
frame:SetFrameStrata("DIALOG")
frame:SetToplevel(true)
frame:SetClampedToScreen(true)
frame:EnableMouse(true)
frame:SetMovable(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", frame.StartMoving)
frame:SetScript("OnDragStop", frame.StopMovingOrSizing)
frame:Hide()
table.insert(UISpecialFrames, frame:GetName())

frame.state = {
  title = "이동술사",
  subtitle = "The Karazhan",
  body = "",
  npcDisplayId = 0,
  section = "이동 가능한 지역",
  closeText = "닫기",
  refreshText = "새로고침",
  items = {},
}

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

local function CreateText(parent, layer, template, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, layer or "OVERLAY", template)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local PortraitUpdater = CreateFrame("Frame")
PortraitUpdater.queue = {}

local function SetSafePortraitTexture(textureObject, unit)
  if not textureObject or not unit or not UnitExists(unit) then
    return false
  end

  if IsUnitModelReadyForUI and IsUnitModelReadyForUI(unit) then
    PortraitUpdater.queue[textureObject] = nil
    SetPortraitTexture(textureObject, unit)
    return true
  end

  PortraitUpdater.queue[textureObject] = unit
  PortraitUpdater.t = 0
  PortraitUpdater:SetScript("OnUpdate", function(self, elapsed)
    self.t = self.t + elapsed
    if self.t < 0.35 then
      return
    end

    self.t = 0
    self:SetScript("OnUpdate", nil)
    for texture, queuedUnit in pairs(self.queue) do
      if texture:IsVisible() and UnitExists(queuedUnit) then
        SetPortraitTexture(texture, queuedUnit)
      end
    end
    wipe(self.queue)
  end)

  return true
end

local function SkinMainButton(button, texturePath)
  button:SetNormalTexture("")
  button:SetPushedTexture("")
  button:SetHighlightTexture("")
  button:SetDisabledTexture("")

  button.bg = button:CreateTexture(nil, "BACKGROUND")
  button.bg:SetAllPoints(button)
  button.bg:SetTexture(texturePath)

  button.hl = button:CreateTexture(nil, "HIGHLIGHT")
  button.hl:SetAllPoints(button)
  button.hl:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\ButtonHighlight-Add.tga"
  )
  button.hl:SetBlendMode("ADD")
  button.hl:SetAlpha(0.26)

  local text = button:GetFontString()
  if text then
    text:SetFont(STANDARD_TEXT_FONT, 14, "")
    text:SetTextColor(0.95, 0.92, 0.84)
    text:SetShadowOffset(1, -1)
  end
end

local function SkinListButton(button)
  button:SetNormalTexture("")
  button:SetPushedTexture("")
  button:SetHighlightTexture("")
  button:SetDisabledTexture("")

  button.bg = button:CreateTexture(nil, "BACKGROUND")
  button.bg:SetAllPoints(button)
  button.bg:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownOptionBackgroundCommon.tga"
  )
  button.bg:SetAlpha(0.88)

  button.hl = button:CreateTexture(nil, "HIGHLIGHT")
  button.hl:SetAllPoints(button)
  button.hl:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\RewardChoice-Highlight.tga"
  )
  button.hl:SetBlendMode("ADD")
  button.hl:SetAlpha(0.18)
end

frame.bg = frame:CreateTexture(nil, "BACKGROUND")
frame.bg:SetAllPoints(frame)
frame.bg:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownParchmentFull.tga"
)
frame.bg:SetTexCoord(0.0, 494/512, 34/1024, 1019/1024)

frame.close = CreateFrame("Button", nil, frame)
frame.close:SetSize(26, 26)
frame.close:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -20, -18)
frame.close.tex = frame.close:CreateTexture(nil, "ARTWORK")
frame.close.tex:SetAllPoints(frame.close)
frame.close.tex:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\WidgetCloseButtonRed.tga"
)
frame.close:SetHighlightTexture(
  "Interface\\Buttons\\UI-Common-MouseHilight", "ADD"
)
frame.close:SetScript("OnClick", function()
  frame:Hide()
end)

frame.iconBorder = CreateFrame("Frame", nil, frame)
frame.iconBorder:SetSize(56, 56)
frame.iconBorder:SetPoint("TOPLEFT", frame, "TOPLEFT", 28, -26)

frame.icon = frame.iconBorder:CreateTexture(nil, "ARTWORK")
frame.icon:SetPoint("TOPLEFT", frame.iconBorder, "TOPLEFT", 8, -8)
frame.icon:SetPoint("BOTTOMRIGHT", frame.iconBorder, "BOTTOMRIGHT", -8, 8)
frame.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

frame.iconFront = frame.iconBorder:CreateTexture(nil, "OVERLAY")
frame.iconFront:SetAllPoints(frame.iconBorder)
frame.iconFront:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownPortraitFrame.tga"
)

frame.subtitle = CreateText(
  frame,
  "OVERLAY",
  "GameFontNormal",
  13,
  0.44,
  0.35,
  0.24
)
frame.subtitle:SetPoint("TOPLEFT", frame.iconBorder, "TOPRIGHT", 14, -2)
frame.subtitle:SetPoint("RIGHT", frame, "RIGHT", -50, 0)

frame.title = CreateText(
  frame,
  "OVERLAY",
  "GameFontHighlightLarge",
  24,
  0.18,
  0.13,
  0.08
)
frame.title:SetPoint("TOPLEFT", frame.subtitle, "BOTTOMLEFT", 0, -2)
frame.title:SetPoint("RIGHT", frame, "RIGHT", -50, 0)

frame.divider = frame:CreateTexture(nil, "ARTWORK")
frame.divider:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownDivider.tga"
)
frame.divider:SetPoint("TOPLEFT", frame, "TOPLEFT", 26, -94)
frame.divider:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -26, -94)
frame.divider:SetHeight(10)

frame.body = CreateText(
  frame,
  "OVERLAY",
  "GameFontNormal",
  14,
  0.21,
  0.16,
  0.11
)
frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 34, -118)
frame.body:SetWidth(352)
frame.body:SetSpacing(7)

frame.section = CreateText(
  frame,
  "OVERLAY",
  "GameFontHighlight",
  15,
  0.34,
  0.25,
  0.14
)
frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -24)

frame.options = CreateFrame("Frame", nil, frame)
frame.options:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", 0, -16)
frame.options:SetSize(362, 268)

frame.optionButtons = {}
for index = 1, 6 do
  local button = CreateFrame("Button", nil, frame.options, "UIPanelButtonTemplate")
  button:SetSize(362, 40)
  if index == 1 then
    button:SetPoint("TOPLEFT", frame.options, "TOPLEFT", 0, 0)
  else
    button:SetPoint(
      "TOPLEFT",
      frame.optionButtons[index - 1],
      "BOTTOMLEFT",
      0,
      -8
    )
  end
  SkinListButton(button)

  button.iconBg = button:CreateTexture(nil, "ARTWORK")
  button.iconBg:SetSize(28, 28)
  button.iconBg:SetPoint("LEFT", button, "LEFT", 10, 0)
  button.iconBg:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownItemButtonBackground.tga"
  )

  button.icon = button:CreateTexture(nil, "BORDER")
  button.icon:SetSize(16, 16)
  button.icon:SetPoint("CENTER", button.iconBg, "CENTER", 0, 0)
  button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  button.iconBorder = button:CreateTexture(nil, "OVERLAY")
  button.iconBorder:SetSize(32, 32)
  button.iconBorder:SetPoint("CENTER", button.iconBg, "CENTER", 0, 0)
  button.iconBorder:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownRewardChoiceItemBorder.tga"
  )

  button.label = CreateText(
    button,
    "OVERLAY",
    "GameFontNormal",
    13,
    0.20,
    0.16,
    0.10
  )
  button.label:SetPoint("TOPLEFT", button.iconBg, "TOPRIGHT", 12, -5)
  button.label:SetPoint("RIGHT", button, "RIGHT", -14, 0)

  button.desc = CreateText(
    button,
    "OVERLAY",
    "GameFontDisableSmall",
    11,
    0.45,
    0.35,
    0.24
  )
  button.desc:SetPoint("TOPLEFT", button.label, "BOTTOMLEFT", 0, -2)
  button.desc:SetPoint("RIGHT", button, "RIGHT", -14, 0)

  button:SetScript("OnClick", function(self)
    if self.actionId then
      SendCommand("ACT", tostring(self.actionId))
    end
  end)

  button:Hide()
  frame.optionButtons[index] = button
end

frame.footerDivider = frame:CreateTexture(nil, "ARTWORK")
frame.footerDivider:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownDivider.tga"
)
frame.footerDivider:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 26, 118)
frame.footerDivider:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -26, 118)
frame.footerDivider:SetHeight(10)

frame.closeButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
frame.closeButton:SetSize(162, 38)
frame.closeButton:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 34, 28)
SkinMainButton(
  frame.closeButton,
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownOptionBackgroundCommon.tga"
)
frame.closeButton:SetText("닫기")
frame.closeButton:SetScript("OnClick", function()
  frame:Hide()
end)

frame.refreshButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
frame.refreshButton:SetSize(162, 38)
frame.refreshButton:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -34, 28)
SkinMainButton(
  frame.refreshButton,
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownOptionBackgroundGrey.tga"
)
frame.refreshButton:SetText("새로고침")
frame.refreshButton:SetScript("OnClick", function()
  SendCommand("REFRESH", "")
end)

local function CreateHotkeyBadge(parent, text)
  local badge = CreateFrame("Frame", nil, parent)
  badge:SetSize(28, 20)
  badge:SetPoint("LEFT", parent, "LEFT", 12, 0)

  badge.bg = badge:CreateTexture(nil, "BACKGROUND")
  badge.bg:SetAllPoints(badge)
  badge.bg:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownHotkeyBackground.tga"
  )

  badge.text = CreateText(
    badge,
    "OVERLAY",
    "GameFontHighlightSmall",
    10,
    0.95,
    0.93,
    0.86,
    "CENTER"
  )
  badge.text:SetPoint("CENTER", badge, "CENTER", 0, 0)
  badge.text:SetText(text)
  return badge
end

frame.closeKey = CreateHotkeyBadge(frame.closeButton, "Esc")
frame.refreshKey = CreateHotkeyBadge(frame.refreshButton, "R")
frame.closeButton:GetFontString():SetPoint("LEFT", frame.closeKey, "RIGHT", 8, 0)
frame.refreshButton:GetFontString():SetPoint(
  "LEFT",
  frame.refreshKey,
  "RIGHT",
  8,
  0
)

local function UpdateHeaderIcon()
  if SetSafePortraitTexture(frame.icon, "npc") then
    return
  end

  if SetSafePortraitTexture(frame.icon, "target") then
    return
  end

  if frame.state.npcDisplayId and frame.state.npcDisplayId > 0
      and SetPortraitTextureFromCreatureDisplayID then
    SetPortraitTextureFromCreatureDisplayID(
      frame.icon,
      frame.state.npcDisplayId
    )
  else
    frame.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
  end
end

local function ResetState()
  frame.state.title = "이동술사"
  frame.state.subtitle = "The Karazhan"
  frame.state.body = ""
  frame.state.npcDisplayId = 0
  frame.state.section = "이동 가능한 지역"
  frame.state.closeText = "닫기"
  frame.state.refreshText = "새로고침"
  frame.state.items = {}
end

local function Refresh()
  UpdateHeaderIcon()
  frame.subtitle:SetText(frame.state.subtitle or "")
  frame.title:SetText(frame.state.title or "이동술사")
  frame.body:SetText(frame.state.body or "")
  frame.section:SetText(frame.state.section or "이동 가능한 지역")
  frame.closeButton:SetText(frame.state.closeText or "닫기")
  frame.refreshButton:SetText(frame.state.refreshText or "새로고침")

  for index, button in ipairs(frame.optionButtons) do
    local item = frame.state.items[index]
    if item then
      button.actionId = item.id
      button.icon:SetTexture(item.icon or "Interface\\Icons\\INV_Misc_QuestionMark")
      button.label:SetText(item.label or "")
      button.desc:SetText(item.desc or "")
      button:Show()
    else
      button.actionId = nil
      button:Hide()
    end
  end
end

frame:RegisterEvent("PLAYER_LOGIN")
frame:RegisterEvent("CHAT_MSG_ADDON")
frame:SetScript("OnEvent", function(self, event, prefix, message)
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
    frame.state.title = parts[2] or frame.state.title
    frame.state.subtitle = parts[3] or ""
    frame.state.npcDisplayId = tonumber(parts[5]) or 0
    Refresh()
    return
  end

  if kind == "BODY" then
    frame.state.body = parts[2] or ""
    Refresh()
    return
  end

  if kind == "SECTION" then
    frame.state.section = parts[2] or frame.state.section
    Refresh()
    return
  end

  if kind == "CONTROL" then
    frame.state.closeText = parts[2] or frame.state.closeText
    frame.state.refreshText = parts[3] or frame.state.refreshText
    Refresh()
    return
  end

  if kind == "ITEM" then
    table.insert(frame.state.items, {
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
    frame:Show()
    return
  end

  if kind == "CLOSE" then
    frame:Hide()
    return
  end
end)

ResetState()
Refresh()
