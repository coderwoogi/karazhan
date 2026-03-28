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

local function CreateText(parent, template, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", template)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local frame = CreateFrame("Frame", "TeleportMasterUIFrame", UIParent)
frame:SetSize(820, 540)
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
  title = "Teleport Master",
  subtitle = "The Karazhan",
  body = "",
  npcDisplayId = 0,
  section = "Destinations",
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

frame.model = CreateFrame("PlayerModel", nil, frame)
frame.model:SetSize(220, 300)
frame.model:SetPoint("TOPLEFT", 24, -54)
frame.model:SetCamDistanceScale(1)
frame.model:SetPosition(0, 0, 0)

frame.subtitle = CreateText(frame, "GameFontNormal")
frame.subtitle:SetPoint("TOPLEFT", frame.model, "TOPRIGHT", 18, -2)
frame.subtitle:SetPoint("RIGHT", frame, "RIGHT", -24, 0)

frame.body = CreateText(frame, "GameFontHighlight")
frame.body:SetPoint("TOPLEFT", frame.subtitle, "BOTTOMLEFT", 0, -12)
frame.body:SetWidth(530)

frame.section = CreateText(frame, "GameFontNormalLarge")
frame.section:SetPoint("TOPLEFT", frame.body, "BOTTOMLEFT", 0, -20)

frame.listInset = CreateFrame("Frame", nil, frame)
frame.listInset:SetPoint("TOPLEFT", frame.section, "BOTTOMLEFT", -4, -10)
frame.listInset:SetSize(548, 248)

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

frame.optionScroll = CreateFrame(
  "ScrollFrame",
  "TeleportMasterUIOptionScroll",
  frame.listInset,
  "UIPanelScrollFrameTemplate"
)
frame.optionScroll:SetPoint("TOPLEFT", 8, -8)
frame.optionScroll:SetPoint("BOTTOMRIGHT", -28, 8)

frame.optionContent = CreateFrame("Frame", nil, frame.optionScroll)
frame.optionContent:SetSize(520, 1)
frame.optionScroll:SetScrollChild(frame.optionContent)

frame.optionButtons = {}

local function CreateOptionButton(index)
  local button = CreateFrame(
    "Button",
    "TeleportMasterUIButton" .. index,
    frame.optionContent,
    "UIPanelButtonTemplate"
  )
  button:SetSize(500, 24)
  if index == 1 then
    button:SetPoint("TOPLEFT", frame.optionContent, "TOPLEFT", 6, 0)
  else
    button:SetPoint(
      "TOPLEFT",
      frame.optionButtons[index - 1],
      "BOTTOMLEFT",
      0,
      -6
    )
  end

  button.text = CreateText(button, "GameFontNormal")
  button.text:SetPoint("LEFT", button, "LEFT", 10, 0)
  button.text:SetPoint("RIGHT", button, "RIGHT", -10, 0)
  button.text:SetJustifyV("MIDDLE")

  button:SetScript("OnClick", function(self)
    if self.actionId then
      SendCommand("ACT", tostring(self.actionId))
    end
  end)

  button:Hide()
  frame.optionButtons[index] = button
  return button
end

for index = 1, 14 do
  CreateOptionButton(index)
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

local function UpdateNpcModel()
  if UnitExists("npc") then
    frame.model:SetUnit("npc")
    return
  end

  if UnitExists("target") then
    frame.model:SetUnit("target")
    return
  end

  if frame.state.npcDisplayId and frame.state.npcDisplayId > 0
      and frame.model.SetDisplayInfo then
    frame.model:SetDisplayInfo(frame.state.npcDisplayId)
    return
  end

  frame.model:ClearModel()
end

local function ResetState()
  frame.state.title = "Teleport Master"
  frame.state.subtitle = "The Karazhan"
  frame.state.body = ""
  frame.state.npcDisplayId = 0
  frame.state.section = "Destinations"
  frame.state.closeText = "Close"
  frame.state.refreshText = "Refresh"
  frame.state.items = {}
end

local function Refresh()
  frame.title:SetText(frame.state.title or "Teleport Master")
  frame.subtitle:SetText(frame.state.subtitle or "")
  frame.body:SetText(frame.state.body or "")
  frame.section:SetText(frame.state.section or "Destinations")
  frame.closeButton:SetText(frame.state.closeText or "Close")
  frame.refreshButton:SetText(frame.state.refreshText or "Refresh")
  UpdateNpcModel()

  while #frame.optionButtons < #frame.state.items do
    CreateOptionButton(#frame.optionButtons + 1)
  end

  for index, button in ipairs(frame.optionButtons) do
    local item = frame.state.items[index]
    if item then
      button.actionId = item.id
      button.text:SetText(item.label or "")
      button:Show()
    else
      button.actionId = nil
      button:Hide()
    end
  end

  local visibleCount = #frame.state.items
  if visibleCount < 1 then
    visibleCount = 1
  end
  frame.optionContent:SetHeight((visibleCount * 30) - 6)
  frame.optionScroll:SetVerticalScroll(0)
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
