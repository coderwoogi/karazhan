local addonName = ...

local VALID_PREFIXES = {
  HERO_STONE_UI = true,
  TELEPORT_MASTER_UI = true,
}

local activePrefix = "HERO_STONE_UI"

local frame = CreateFrame("Frame", "HeroStoneUIFrame", UIParent)
frame:SetSize(438, 608)
frame:SetPoint("RIGHT", UIParent, "RIGHT", -110, 0)
frame:SetScale(0.70)
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
  title = "Hero Stone",
  subtitle = "",
  body = "",
  icon = "Interface\\AddOns\\HeroStoneUI\\Art\\INV_Misc_Rune_100.tga",
  npcDisplayId = 0,
  theme = "dark",
  section = "Options",
  closeText = "Close",
  refreshText = "Refresh",
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

  SendAddonMessage(activePrefix, payload, "WHISPER", playerName)
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
    if self.t < 0.4 then
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

local function SkinActionButton(button)
  button:SetNormalTexture("")
  button:SetPushedTexture("")
  button:SetHighlightTexture("")
  button:SetDisabledTexture("")

  button.bg = button:CreateTexture(nil, "BACKGROUND")
  button.bg:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\OptionBackground-Common.tga"
  )
  button.bg:SetAllPoints(button)

  button.front = button:CreateTexture(nil, "BORDER")
  button.front:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\ButtonHighlight-Front.tga"
  )
  button.front:SetAllPoints(button)
  button.front:SetAlpha(0.16)

  button.hl = button:CreateTexture(nil, "HIGHLIGHT")
  button.hl:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\ButtonHighlight-Add.tga"
  )
  button.hl:SetAllPoints(button)
  button.hl:SetBlendMode("ADD")
  button.hl:SetAlpha(0.34)

  local fs = button:GetFontString()
  if fs then
    fs:SetFont(STANDARD_TEXT_FONT, 13, "")
    fs:SetTextColor(0.92, 0.91, 0.86)
    fs:SetShadowOffset(1, -1)
  end
end

local function SetButtonTheme(button, theme, isSecondary)
  local common = "Interface\\AddOns\\HeroStoneUI\\Art\\OptionBackground-Common.tga"
  local grey = "Interface\\AddOns\\HeroStoneUI\\Art\\OptionBackground-Common.tga"
  local hotkey = "Interface\\AddOns\\HeroStoneUI\\Art\\HotkeyBackground.tga"
  local textColor = { 0.92, 0.91, 0.86 }

  if theme == "brown" then
    common =
      "Interface\\AddOns\\HeroStoneUI\\Art\\BrownOptionBackgroundCommon.tga"
    grey =
      "Interface\\AddOns\\HeroStoneUI\\Art\\BrownOptionBackgroundGrey.tga"
    hotkey =
      "Interface\\AddOns\\HeroStoneUI\\Art\\BrownHotkeyBackground.tga"
    textColor = isSecondary and { 0.78, 0.75, 0.70 } or { 0.94, 0.89, 0.82 }
  end

  button.bg:SetTexture(isSecondary and grey or common)
  if button.front then
    button.front:SetAlpha(theme == "brown" and 0.10 or 0.16)
  end
  if button.hl then
    button.hl:SetAlpha(theme == "brown" and 0.22 or 0.34)
  end

  local fs = button:GetFontString()
  if fs then
    fs:SetTextColor(textColor[1], textColor[2], textColor[3])
  end

  if button.hotkeyBadge and button.hotkeyBadge.bg then
    button.hotkeyBadge.bg:SetTexture(hotkey)
  end
end

frame.bg = frame:CreateTexture(nil, "BACKGROUND")
frame.bg:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\GenericFrame-Tiled-Large.tga"
)
frame.bg:SetAllPoints(frame)

frame.brownTop = frame:CreateTexture(nil, "BACKGROUND")
frame.brownTop:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\BrownParchmentTop.tga"
)
frame.brownTop:SetPoint("TOPLEFT", frame, "TOPLEFT", 0, 0)
frame.brownTop:SetPoint("TOPRIGHT", frame, "TOPRIGHT", 0, 0)
frame.brownTop:SetHeight(116)
frame.brownTop:Hide()

frame.brownBottom = frame:CreateTexture(nil, "BACKGROUND")
frame.brownBottom:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\BrownParchmentBottom.tga"
)
frame.brownBottom:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 0, 0)
frame.brownBottom:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", 0, 0)
frame.brownBottom:SetHeight(116)
frame.brownBottom:Hide()

frame.brownMid = frame:CreateTexture(nil, "BACKGROUND")
frame.brownMid:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\BrownParchmentMid.tga"
)
frame.brownMid:SetPoint("TOPLEFT", frame.brownTop, "BOTTOMLEFT", 0, 0)
frame.brownMid:SetPoint("BOTTOMRIGHT", frame.brownBottom, "TOPRIGHT", 0, 0)
frame.brownMid:Hide()

frame.shadow = frame:CreateTexture(nil, "BORDER")
frame.shadow:SetTexture("Interface\\DialogFrame\\UI-DialogBox-Background-Dark")
frame.shadow:SetPoint("TOPLEFT", frame, "TOPLEFT", 28, -28)
frame.shadow:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -28, 64)
frame.shadow:SetVertexColor(0, 0, 0, 0.18)

frame.close = CreateFrame("Button", nil, frame)
frame.close:SetSize(24, 24)
frame.close:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -12, -14)
frame.close.tex = frame.close:CreateTexture(nil, "ARTWORK")
frame.close.tex:SetAllPoints(frame.close)
frame.close.tex:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\WidgetCloseButton.tga"
)
frame.close:SetHighlightTexture(
  "Interface\\Buttons\\UI-Common-MouseHilight", "ADD"
)
frame.close:SetScript("OnClick", function()
  frame:Hide()
end)

frame.iconBorder = CreateFrame("Frame", nil, frame)
frame.iconBorder:SetSize(56, 56)
frame.iconBorder:SetPoint("TOPLEFT", frame, "TOPLEFT", 34, -32)

frame.icon = frame.iconBorder:CreateTexture(nil, "ARTWORK")
frame.icon:SetPoint("TOPLEFT", frame.iconBorder, "TOPLEFT", 8, -8)
frame.icon:SetPoint("BOTTOMRIGHT", frame.iconBorder, "BOTTOMRIGHT", -8, 8)
frame.icon:SetTexture(frame.state.icon)

frame.iconFront = frame.iconBorder:CreateTexture(nil, "OVERLAY")
frame.iconFront:SetAllPoints(frame.iconBorder)
frame.iconFront:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\ParchmentPortraitFrame.tga"
)

local function UpdateHeaderIcon()
  if frame.state.icon == "PORTRAIT_NPC" then
    if SetSafePortraitTexture(frame.icon, "npc") then
      return
    end

    if SetSafePortraitTexture(frame.icon, "target") then
      return
    end

    if frame.state.npcDisplayId and frame.state.npcDisplayId > 0 and SetPortraitTextureFromCreatureDisplayID then
      SetPortraitTextureFromCreatureDisplayID(frame.icon, frame.state.npcDisplayId)
    else
      frame.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
    end
    return
  end

  frame.icon:SetTexture(frame.state.icon)
end

frame.title = CreateText(
  frame,
  "OVERLAY",
  "GameFontHighlightLarge",
  23,
  0.95,
  0.95,
  0.92
)
frame.title:SetPoint("TOPLEFT", frame.iconBorder, "TOPRIGHT", 16, -2)
frame.title:SetPoint("RIGHT", frame, "RIGHT", -48, 0)
frame.title:SetText(frame.state.title)

frame.subtitle = CreateText(
  frame,
  "OVERLAY",
  "GameFontNormal",
  13,
  0.72,
  0.72,
  0.68
)
frame.subtitle:SetPoint("TOPLEFT", frame.title, "BOTTOMLEFT", 0, -8)
frame.subtitle:SetPoint("RIGHT", frame, "RIGHT", -48, 0)
frame.subtitle:SetText(frame.state.subtitle)

frame.divider = frame:CreateTexture(nil, "ARTWORK")
frame.divider:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\ParchmentDivider.tga"
)
frame.divider:SetPoint("TOPLEFT", frame, "TOPLEFT", 34, -106)
frame.divider:SetPoint("TOPRIGHT", frame, "TOPRIGHT", -34, -106)
frame.divider:SetHeight(14)
frame.divider:SetAlpha(0.95)

frame.body = CreateText(
  frame,
  "OVERLAY",
  "GameFontNormal",
  14,
  0.88,
  0.88,
  0.84
)
frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 36, -126)
frame.body:SetWidth(366)
frame.body:SetSpacing(6)
frame.body:SetText(frame.state.body)

frame.section = CreateText(
  frame,
  "OVERLAY",
  "GameFontHighlight",
  14,
  0.96,
  0.94,
  0.86
)
frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -24)
frame.section:SetText(frame.state.section)

frame.sectionBg = frame:CreateTexture(nil, "ARTWORK")
frame.sectionBg:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\LabelBackground.tga"
)
frame.sectionBg:SetPoint("TOPLEFT", frame.section, "TOPLEFT", -10, 8)
frame.sectionBg:SetPoint("BOTTOMRIGHT", frame.section, "BOTTOMRIGHT", 18, -8)
frame.sectionBg:SetAlpha(0.65)

frame.options = CreateFrame("Frame", nil, frame)
frame.options:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", 0, -10)
frame.options:SetSize(370, 290)

frame.optionButtons = {}

for index = 1, 8 do
  local button = CreateFrame("Button", nil, frame.options, "UIPanelButtonTemplate")
  button:SetSize(368, 40)
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
  SkinActionButton(button)

  button.icon = button:CreateTexture(nil, "ARTWORK")
  button.icon:SetSize(20, 20)
  button.icon:SetPoint("LEFT", button, "LEFT", 12, 0)
  button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  button.iconBg = button:CreateTexture(nil, "BACKGROUND")
  button.iconBg:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\ItemButtonBackground.tga"
  )
  button.iconBg:SetSize(28, 28)
  button.iconBg:SetPoint("CENTER", button.icon, "CENTER", 0, 0)

  button.iconBorder = button:CreateTexture(nil, "BORDER")
  button.iconBorder:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\ItemBorder.tga"
  )
  button.iconBorder:SetSize(32, 32)
  button.iconBorder:SetPoint("CENTER", button.icon, "CENTER", 0, 0)

  button.label = CreateText(
    button,
    "OVERLAY",
    "GameFontNormal",
    13,
    0.96,
    0.96,
    0.93
  )
  button.label:SetPoint("TOPLEFT", button.icon, "TOPRIGHT", 10, -2)
  button.label:SetPoint("RIGHT", button, "RIGHT", -14, 0)
  button.label:SetText("")

  button.desc = CreateText(
    button,
    "OVERLAY",
    "GameFontDisableSmall",
    11,
    0.68,
    0.68,
    0.66
  )
  button.desc:SetPoint("TOPLEFT", button.label, "BOTTOMLEFT", 0, -3)
  button.desc:SetPoint("RIGHT", button, "RIGHT", -14, 0)
  button.desc:SetText("")

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
  "Interface\\AddOns\\HeroStoneUI\\Art\\SubHeaderBackground.tga"
)
frame.footerDivider:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 34, 82)
frame.footerDivider:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -34, 82)
frame.footerDivider:SetHeight(16)
frame.footerDivider:SetAlpha(0.85)

frame.footerGlow = frame:CreateTexture(nil, "BORDER")
frame.footerGlow:SetTexture(
  "Interface\\AddOns\\HeroStoneUI\\Art\\RewardChoice-Highlight.tga"
)
frame.footerGlow:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 26, 18)
frame.footerGlow:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -26, 18)
frame.footerGlow:SetHeight(84)
frame.footerGlow:SetAlpha(0.08)

frame.closeButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
frame.closeButton:SetSize(150, 34)
frame.closeButton:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 34, 30)
SkinActionButton(frame.closeButton)
frame.closeButton:SetText("Close")
frame.closeButton:SetScript("OnClick", function()
  frame:Hide()
end)

frame.refreshButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
frame.refreshButton:SetSize(150, 34)
frame.refreshButton:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -34, 30)
SkinActionButton(frame.refreshButton)
frame.refreshButton:SetText("Refresh")
frame.refreshButton:SetScript("OnClick", function()
  SendCommand("REFRESH", "")
end)

local function CreateHotkeyBadge(parent, text)
  local badge = CreateFrame("Frame", nil, parent)
  badge:SetSize(24, 18)
  badge:SetPoint("LEFT", parent, "LEFT", 10, 0)

  badge.bg = badge:CreateTexture(nil, "BACKGROUND")
  badge.bg:SetAllPoints(badge)
  badge.bg:SetTexture(
    "Interface\\AddOns\\HeroStoneUI\\Art\\HotkeyBackground.tga"
  )

  badge.text = CreateText(
    badge,
    "OVERLAY",
    "GameFontHighlightSmall",
    10,
    0.92,
    0.92,
    0.88,
    "CENTER"
  )
  badge.text:SetPoint("CENTER", badge, "CENTER", 0, 0)
  badge.text:SetText(text)
  return badge
end

frame.closeKey = CreateHotkeyBadge(frame.closeButton, "Esc")
frame.refreshKey = CreateHotkeyBadge(frame.refreshButton, "R")
frame.closeButton.hotkeyBadge = frame.closeKey
frame.refreshButton.hotkeyBadge = frame.refreshKey

frame.closeButton:GetFontString():SetPoint("LEFT", frame.closeKey, "RIGHT", 8, 0)
frame.refreshButton:GetFontString():SetPoint(
  "LEFT",
  frame.refreshKey,
  "RIGHT",
  8,
  0
)

local function ResetState()
  frame.state.title = "Hero Stone"
  frame.state.subtitle = ""
  frame.state.body = ""
  frame.state.icon = "Interface\\AddOns\\HeroStoneUI\\Art\\INV_Misc_Rune_100.tga"
  frame.state.npcDisplayId = 0
  frame.state.theme = activePrefix == "TELEPORT_MASTER_UI" and "brown" or "dark"
  frame.state.section = "Options"
  frame.state.closeText = "Close"
  frame.state.refreshText = "Refresh"
  frame.state.items = {}
end

local function ApplyTheme()
  local isBrown = frame.state.theme == "brown"

  frame.bg:SetShown(not isBrown)
  frame.shadow:SetShown(not isBrown)
  frame.brownTop:SetShown(isBrown)
  frame.brownMid:SetShown(isBrown)
  frame.brownBottom:SetShown(isBrown)

  frame.iconFront:SetTexture(
    isBrown
      and "Interface\\AddOns\\HeroStoneUI\\Art\\BrownPortraitFrame.tga"
      or "Interface\\AddOns\\HeroStoneUI\\Art\\ParchmentPortraitFrame.tga"
  )

  frame.divider:SetTexture(
    isBrown
      and "Interface\\AddOns\\HeroStoneUI\\Art\\BrownDivider.tga"
      or "Interface\\AddOns\\HeroStoneUI\\Art\\ParchmentDivider.tga"
  )

  frame.title:ClearAllPoints()
  frame.subtitle:ClearAllPoints()
  frame.body:ClearAllPoints()
  frame.section:ClearAllPoints()
  frame.options:ClearAllPoints()
  frame.footerDivider:ClearAllPoints()
  frame.closeButton:ClearAllPoints()
  frame.refreshButton:ClearAllPoints()

  if isBrown then
    frame.title:SetFont(STANDARD_TEXT_FONT, 21, "")
    frame.title:SetTextColor(0.31, 0.22, 0.15)
    frame.subtitle:SetFont(STANDARD_TEXT_FONT, 13, "")
    frame.subtitle:SetTextColor(0.45, 0.33, 0.24)
    frame.body:SetTextColor(0.34, 0.25, 0.18)
    frame.section:SetTextColor(0.39, 0.29, 0.20)
    frame.body:SetWidth(352)
    frame.body:SetSpacing(5)

    frame.subtitle:SetPoint("TOPLEFT", frame.iconBorder, "TOPRIGHT", 16, 2)
    frame.title:SetPoint("TOPLEFT", frame.subtitle, "BOTTOMLEFT", 0, -4)
    frame.title:SetPoint("RIGHT", frame, "RIGHT", -50, 0)

    frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 40, -150)
    frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -26)
    frame.options:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", -2, -12)
    frame.options:SetSize(360, 290)

    frame.sectionBg:Hide()
    frame.footerDivider:Hide()
    frame.footerGlow:Hide()

    frame.closeButton:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 42, 30)
    frame.refreshButton:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -42, 30)
  else
    frame.title:SetFont(STANDARD_TEXT_FONT, 23, "")
    frame.title:SetTextColor(0.95, 0.95, 0.92)
    frame.subtitle:SetFont(STANDARD_TEXT_FONT, 13, "")
    frame.subtitle:SetTextColor(0.72, 0.72, 0.68)
    frame.body:SetTextColor(0.88, 0.88, 0.84)
    frame.section:SetTextColor(0.96, 0.94, 0.86)
    frame.body:SetWidth(366)
    frame.body:SetSpacing(6)

    frame.title:SetPoint("TOPLEFT", frame.iconBorder, "TOPRIGHT", 16, -2)
    frame.title:SetPoint("RIGHT", frame, "RIGHT", -48, 0)
    frame.subtitle:SetPoint("TOPLEFT", frame.title, "BOTTOMLEFT", 0, -8)
    frame.subtitle:SetPoint("RIGHT", frame, "RIGHT", -48, 0)

    frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 36, -126)
    frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -24)
    frame.options:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", 0, -10)
    frame.options:SetSize(370, 290)

    frame.sectionBg:Show()
    frame.footerDivider:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 34, 82)
    frame.footerDivider:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -34, 82)
    frame.footerDivider:Show()
    frame.footerGlow:Show()

    frame.closeButton:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 34, 30)
    frame.refreshButton:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -34, 30)
  end

  SetButtonTheme(frame.closeButton, frame.state.theme, false)
  SetButtonTheme(frame.refreshButton, frame.state.theme, true)

  for _, button in ipairs(frame.optionButtons) do
    SetButtonTheme(button, frame.state.theme, false)
    button.iconBg:SetTexture(
      isBrown
        and "Interface\\AddOns\\HeroStoneUI\\Art\\BrownItemButtonBackground.tga"
        or "Interface\\AddOns\\HeroStoneUI\\Art\\ItemButtonBackground.tga"
    )
    button.iconBorder:SetTexture(
      isBrown
        and "Interface\\AddOns\\HeroStoneUI\\Art\\BrownRewardChoiceItemBorder.tga"
        or "Interface\\AddOns\\HeroStoneUI\\Art\\ItemBorder.tga"
    )
    button.desc:SetTextColor(
      isBrown and 0.47 or 0.68,
      isBrown and 0.35 or 0.68,
      isBrown and 0.25 or 0.66
    )
    button.label:SetTextColor(
      isBrown and 0.30 or 0.96,
      isBrown and 0.22 or 0.96,
      isBrown and 0.16 or 0.93
    )
  end
end

local function Refresh()
  ApplyTheme()
  UpdateHeaderIcon()
  frame.title:SetText(frame.state.title or "Hero Stone")
  frame.subtitle:SetText(frame.state.subtitle or "")
  frame.body:SetText(frame.state.body or "")
  frame.section:SetText(frame.state.section or "Options")
  frame.closeButton:SetText(frame.state.closeText or "Close")
  frame.refreshButton:SetText(frame.state.refreshText or "Refresh")

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
      for knownPrefix in pairs(VALID_PREFIXES) do
        RegisterAddonMessagePrefix(knownPrefix)
      end
    end
    return
  end

  if not VALID_PREFIXES[prefix] or type(message) ~= "string" then
    return
  end

  activePrefix = prefix

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
    frame.state.icon = parts[4] or frame.state.icon
    frame.state.npcDisplayId = tonumber(parts[5]) or 0
    frame.state.theme = activePrefix == "TELEPORT_MASTER_UI" and "brown" or "dark"
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
