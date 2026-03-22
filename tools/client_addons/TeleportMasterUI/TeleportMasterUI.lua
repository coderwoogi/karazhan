local addonName = ...

local PREFIX = "TELEPORT_MASTER_UI"

local frame = CreateFrame("Frame", "TeleportMasterUIFrame", UIParent)
frame:SetSize(472, 660)
frame:SetPoint("CENTER", UIParent, "CENTER", 340, -8)
frame:SetScale(0.72)
frame:SetFrameStrata("DIALOG")
frame:SetToplevel(true)
frame:SetClampedToScreen(true)
frame:EnableMouse(true)
frame:SetMovable(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", function(self)
  self:StartMoving()
end)
frame:SetScript("OnDragStop", function(self)
  self:StopMovingOrSizing()
end)
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

local function CreateText(parent, layer, size, color, justify)
  local fs = parent:CreateFontString(nil, layer or "OVERLAY", nil)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(color[1], color[2], color[3])
  fs:SetShadowColor(0, 0, 0, 0.85)
  fs:SetShadowOffset(1, -1)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local function SkinActionButton(button, texturePath, fontSize)
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
  button.hl:SetAlpha(0.22)

  if not button.text then
    button.text = button:CreateFontString(nil, "OVERLAY")
    button.text:SetPoint("CENTER", button, "CENTER", 12, 0)
    button:SetFontString(button.text)
  end

  button.text:SetFont(STANDARD_TEXT_FONT, fontSize, "")
  button.text:SetTextColor(0.95, 0.92, 0.84)
  button.text:SetShadowColor(0, 0, 0, 0.9)
  button.text:SetShadowOffset(1, -1)
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
  button.bg:SetAlpha(0.96)

  button.hl = button:CreateTexture(nil, "HIGHLIGHT")
  button.hl:SetAllPoints(button)
  button.hl:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\RewardChoice-Highlight.tga"
  )
  button.hl:SetBlendMode("ADD")
  button.hl:SetAlpha(0.16)
end

frame.bgTop = frame:CreateTexture(nil, "BACKGROUND")
frame.bgTop:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownParchmentFromBG512.tga"
)
frame.bgTop:SetPoint("TOPLEFT", frame, "TOPLEFT", 0, 0)
frame.bgTop:SetPoint("TOPRIGHT", frame, "TOPRIGHT", 0, 0)
frame.bgTop:SetHeight(162)
frame.bgTop:SetTexCoord(0, 1, 0.0, 0.245)

frame.bgMid = frame:CreateTexture(nil, "BACKGROUND")
frame.bgMid:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownParchmentFromBG512.tga"
)
frame.bgMid:SetPoint("TOPLEFT", frame.bgTop, "BOTTOMLEFT", 0, 0)
frame.bgMid:SetPoint("TOPRIGHT", frame.bgTop, "BOTTOMRIGHT", 0, 0)
frame.bgMid:SetPoint("BOTTOM", frame, "BOTTOM", 0, 118)
frame.bgMid:SetTexCoord(0, 1, 0.245, 0.885)

frame.bgBottom = frame:CreateTexture(nil, "BACKGROUND")
frame.bgBottom:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownParchmentFromBG512.tga"
)
frame.bgBottom:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 0, 0)
frame.bgBottom:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", 0, 0)
frame.bgBottom:SetHeight(118)
frame.bgBottom:SetTexCoord(0, 1, 0.885, 1.0)

frame.close = CreateFrame("Button", nil, frame)
frame.close:SetSize(22, 22)
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
frame.iconBorder:SetSize(74, 74)
frame.iconBorder:SetPoint("TOPLEFT", frame, "TOPLEFT", 22, -26)

frame.icon = frame.iconBorder:CreateTexture(nil, "ARTWORK")
frame.icon:SetPoint("TOPLEFT", frame.iconBorder, "TOPLEFT", 12, -12)
frame.icon:SetPoint("BOTTOMRIGHT", frame.iconBorder, "BOTTOMRIGHT", -12, 12)
frame.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

frame.iconFront = frame.iconBorder:CreateTexture(nil, "OVERLAY")
frame.iconFront:SetAllPoints(frame.iconBorder)
frame.iconFront:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownPortraitFrame.tga"
)

frame.subtitle = CreateText(
  frame,
  "OVERLAY",
  18,
  {0.45, 0.35, 0.24},
  "LEFT"
)
frame.subtitle:SetPoint("TOPLEFT", frame.iconBorder, "TOPRIGHT", 16, -1)
frame.subtitle:SetWidth(320)

frame.title = CreateText(
  frame,
  "OVERLAY",
  34,
  {0.18, 0.13, 0.08},
  "LEFT"
)
frame.title:SetPoint("TOPLEFT", frame.subtitle, "BOTTOMLEFT", 0, -4)
frame.title:SetWidth(320)

frame.divider = frame:CreateTexture(nil, "ARTWORK")
frame.divider:SetTexture(
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownHeaderDividerWidePOT.tga"
)
frame.divider:SetPoint("TOPLEFT", frame, "TOPLEFT", 44, -112)
frame.divider:SetSize(384, 22)

frame.body = CreateText(
  frame,
  "OVERLAY",
  20,
  {0.21, 0.16, 0.11},
  "LEFT"
)
frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 40, -148)
frame.body:SetWidth(370)
frame.body:SetSpacing(9)

frame.section = CreateText(
  frame,
  "OVERLAY",
  22,
  {0.34, 0.25, 0.14},
  "LEFT"
)
frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -16)

frame.options = CreateFrame("Frame", nil, frame)
frame.options:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", 0, -12)
frame.options:SetSize(370, 260)

frame.optionButtons = {}
for index = 1, 5 do
  local button = CreateFrame("Button", nil, frame.options)
  button:SetSize(370, 46)
  if index == 1 then
    button:SetPoint("TOPLEFT", frame.options, "TOPLEFT", 0, 0)
  else
    button:SetPoint(
      "TOPLEFT",
      frame.optionButtons[index - 1],
      "BOTTOMLEFT",
      0,
      -7
    )
  end
  SkinListButton(button)

  button.iconBg = button:CreateTexture(nil, "ARTWORK")
  button.iconBg:SetSize(42, 42)
  button.iconBg:SetPoint("LEFT", button, "LEFT", 10, 0)
  button.iconBg:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownItemButtonBackground.tga"
  )

  button.icon = button:CreateTexture(nil, "BORDER")
  button.icon:SetSize(28, 28)
  button.icon:SetPoint("CENTER", button.iconBg, "CENTER", 0, 0)
  button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  button.iconBorder = button:CreateTexture(nil, "OVERLAY")
  button.iconBorder:SetSize(46, 46)
  button.iconBorder:SetPoint("CENTER", button.iconBg, "CENTER", 0, 0)
  button.iconBorder:SetTexture(
    "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownRewardChoiceItemBorder.tga"
  )

  button.label = CreateText(
    button,
    "OVERLAY",
    19,
    {0.20, 0.16, 0.10},
    "LEFT"
  )
  button.label:SetPoint("TOPLEFT", button.iconBg, "TOPRIGHT", 12, -4)
  button.label:SetWidth(292)

  button.desc = CreateText(
    button,
    "OVERLAY",
    17,
    {0.45, 0.35, 0.24},
    "LEFT"
  )
  button.desc:SetPoint("TOPLEFT", button.label, "BOTTOMLEFT", 0, -1)
  button.desc:SetWidth(292)

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
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownHeaderDividerWidePOT.tga"
)
frame.footerDivider:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 40, 108)
frame.footerDivider:SetSize(384, 18)

frame.closeButton = CreateFrame("Button", nil, frame)
frame.closeButton:SetSize(164, 40)
frame.closeButton:SetPoint("BOTTOMLEFT", frame, "BOTTOMLEFT", 42, 48)
SkinActionButton(
  frame.closeButton,
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownOptionBackgroundCommon.tga",
  16
)
frame.closeButton:SetText("닫기")
frame.closeButton:SetScript("OnClick", function()
  frame:Hide()
end)

frame.refreshButton = CreateFrame("Button", nil, frame)
frame.refreshButton:SetSize(164, 40)
frame.refreshButton:SetPoint("LEFT", frame.closeButton, "RIGHT", 18, 0)
SkinActionButton(
  frame.refreshButton,
  "Interface\\AddOns\\TeleportMasterUI\\Art\\BrownOptionBackgroundGrey.tga",
  16
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
    10,
    {0.95, 0.93, 0.86},
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
    return
  end

  frame.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
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
      button.icon:SetTexture(
        item.icon or "Interface\\Icons\\INV_Misc_QuestionMark"
      )
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
