local addonName = ...

local VALID_PREFIXES = {
  HERO_STONE_UI = true,
}

local activePrefix = "HERO_STONE_UI"

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

local function CreateText(parent, template, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", template)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local frame = CreateFrame("Frame", "HeroStoneUIFrame", UIParent)
frame:SetSize(760, 520)
frame:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
frame:SetFrameStrata("DIALOG")
frame:SetToplevel(true)
frame:SetMovable(true)
frame:EnableMouse(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", frame.StartMoving)
frame:SetScript("OnDragStop", frame.StopMovingOrSizing)
frame:Hide()
table.insert(UISpecialFrames, frame:GetName())

frame.state = {
  title = "Hero Stone",
  subtitle = "",
  body = "",
  icon = "Interface\\Icons\\INV_Misc_Rune_01",
  npcDisplayId = 0,
  section = "Options",
  closeText = "Close",
  refreshText = "Refresh",
  items = {},
}

frame.bg = frame:CreateTexture(nil, "BACKGROUND")
frame.bg:SetTexture("Interface\\DialogFrame\\UI-DialogBox-Background")
frame.bg:SetPoint("TOPLEFT", 11, -12)
frame.bg:SetPoint("BOTTOMRIGHT", -12, 11)

for _, point in ipairs({
  {"TOPLEFT", 0, 1, 0, 1},
  {"TOPRIGHT", 1, 0, 0, 1},
  {"BOTTOMLEFT", 0, 1, 1, 0},
  {"BOTTOMRIGHT", 1, 0, 1, 0},
}) do
  local tex = frame:CreateTexture(nil, "BORDER")
  tex:SetTexture("Interface\\DialogFrame\\UI-DialogBox-Border")
  tex:SetPoint(point[1])
  tex:SetSize(256, 256)
  tex:SetTexCoord(point[2], point[3], point[4], point[5])
end

frame.title = CreateText(frame, "GameFontHighlightLarge", "CENTER")
frame.title:SetPoint("TOP", 0, -18)

frame.close = CreateFrame("Button", nil, frame, "UIPanelCloseButton")
frame.close:SetPoint("TOPRIGHT", -6, -6)

frame.headerIcon = CreateFrame("Frame", nil, frame)
frame.headerIcon:SetSize(40, 40)
frame.headerIcon:SetPoint("TOPLEFT", 22, -24)
frame.headerIcon.bg = frame.headerIcon:CreateTexture(nil, "BACKGROUND")
frame.headerIcon.bg:SetTexture("Interface\\Buttons\\UI-Quickslot2")
frame.headerIcon.bg:SetAllPoints()
frame.headerIcon.icon = frame.headerIcon:CreateTexture(nil, "ARTWORK")
frame.headerIcon.icon:SetPoint("TOPLEFT", 6, -6)
frame.headerIcon.icon:SetPoint("BOTTOMRIGHT", -6, 6)
frame.headerIcon.icon:SetTexture(frame.state.icon)

frame.subtitle = CreateText(frame, "GameFontNormal")
frame.subtitle:SetPoint("TOPLEFT", frame.headerIcon, "TOPRIGHT", 12, -2)
frame.subtitle:SetPoint("RIGHT", frame, "RIGHT", -48, 0)

frame.body = CreateText(frame, "GameFontHighlight")
frame.body:SetPoint("TOPLEFT", frame, "TOPLEFT", 24, -76)
frame.body:SetWidth(712)

frame.section = CreateText(frame, "GameFontNormalLarge")
frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -18)

frame.listInset = CreateFrame("Frame", nil, frame)
frame.listInset:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", -4, -10)
frame.listInset:SetSize(720, 260)

frame.listInset.bg = frame.listInset:CreateTexture(nil, "BACKGROUND")
frame.listInset.bg:SetTexture("Interface\\Buttons\\WHITE8x8")
frame.listInset.bg:SetAllPoints()
frame.listInset.bg:SetVertexColor(0, 0, 0, 0.35)

frame.listInset.border = CreateFrame("Frame", nil, frame.listInset)
frame.listInset.border:SetAllPoints()
frame.listInset.border:SetBackdrop({
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 14,
})
frame.listInset.border:SetBackdropBorderColor(0.6, 0.6, 0.6, 0.8)

frame.optionButtons = {}

for index = 1, 8 do
  local button = CreateFrame(
    "Button",
    "HeroStoneUIButton" .. index,
    frame.listInset,
    "UIPanelButtonTemplate"
  )
  button:SetSize(694, 28)
  if index == 1 then
    button:SetPoint("TOPLEFT", frame.listInset, "TOPLEFT", 14, -14)
  else
    button:SetPoint(
      "TOPLEFT",
      frame.optionButtons[index - 1],
      "BOTTOMLEFT",
      0,
      -6
    )
  end

  button.iconHolder = CreateFrame("Frame", nil, button)
  button.iconHolder:SetSize(24, 24)
  button.iconHolder:SetPoint("LEFT", button, "LEFT", 4, 0)
  button.iconHolder.bg = button.iconHolder:CreateTexture(nil, "BACKGROUND")
  button.iconHolder.bg:SetTexture("Interface\\Buttons\\UI-Quickslot2")
  button.iconHolder.bg:SetAllPoints()
  button.icon = button.iconHolder:CreateTexture(nil, "ARTWORK")
  button.icon:SetPoint("TOPLEFT", 4, -4)
  button.icon:SetPoint("BOTTOMRIGHT", -4, 4)
  button.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

  button.text = CreateText(button, "GameFontNormal")
  button.text:SetPoint("LEFT", button.iconHolder, "RIGHT", 8, 0)
  button.text:SetPoint("RIGHT", button, "RIGHT", -10, 0)
  button.text:SetJustifyV("MIDDLE")

  button:SetScript("OnClick", function(self)
    if self.actionId then
      SendCommand("ACT", tostring(self.actionId))
    end
  end)

  button:Hide()
  frame.optionButtons[index] = button
end

frame.closeButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
frame.closeButton:SetSize(130, 24)
frame.closeButton:SetPoint("BOTTOMRIGHT", frame, "BOTTOMRIGHT", -24, 24)
frame.closeButton:SetScript("OnClick", function()
  frame:Hide()
end)

frame.refreshButton = CreateFrame(
  "Button",
  nil,
  frame,
  "UIPanelButtonTemplate"
)
frame.refreshButton:SetSize(130, 24)
frame.refreshButton:SetPoint("RIGHT", frame.closeButton, "LEFT", -10, 0)
frame.refreshButton:SetScript("OnClick", function()
  SendCommand("REFRESH", "")
end)

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

local function UpdateHeaderIcon()
  if frame.state.icon == "PORTRAIT_NPC" then
    if SetSafePortraitTexture(frame.headerIcon.icon, "npc") then
      return
    end

    if SetSafePortraitTexture(frame.headerIcon.icon, "target") then
      return
    end

    if frame.state.npcDisplayId and frame.state.npcDisplayId > 0
        and SetPortraitTextureFromCreatureDisplayID then
      SetPortraitTextureFromCreatureDisplayID(
        frame.headerIcon.icon,
        frame.state.npcDisplayId
      )
      return
    end
  end

  frame.headerIcon.icon:SetTexture(
    frame.state.icon or "Interface\\Icons\\INV_Misc_QuestionMark"
  )
end

local function ResetState()
  frame.state.title = "Hero Stone"
  frame.state.subtitle = ""
  frame.state.body = ""
  frame.state.icon = "Interface\\Icons\\INV_Misc_Rune_01"
  frame.state.npcDisplayId = 0
  frame.state.section = "Options"
  frame.state.closeText = "Close"
  frame.state.refreshText = "Refresh"
  frame.state.items = {}
end

local function Refresh()
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
      button.icon:SetTexture(
        item.icon or "Interface\\Icons\\INV_Misc_QuestionMark"
      )
      button.text:SetText(item.label or "")
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
