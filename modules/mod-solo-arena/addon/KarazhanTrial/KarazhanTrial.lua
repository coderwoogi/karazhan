local addonName = ...
local ARENA_INSTANCE_TYPE = "arena"

local Trial = CreateFrame("Frame", "KarazhanTrialFrame", UIParent)
Trial:SetSize(860, 540)
Trial:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
Trial:SetClampedToScreen(true)
Trial:EnableMouse(true)
Trial:SetMovable(true)
Trial:RegisterForDrag("LeftButton")
Trial:SetScript("OnDragStart", Trial.StartMoving)
Trial:SetScript("OnDragStop", Trial.StopMovingOrSizing)
Trial:Hide()

Trial:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
  edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
  tile = true,
  tileSize = 32,
  edgeSize = 32,
  insets = { left = 11, right = 12, top = 12, bottom = 11 },
})
Trial:SetBackdropColor(0.04, 0.04, 0.04, 0.96)

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

local function CreateLabel(parent, template, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", template)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local function NewState()
  return {
    highestCleared = 0,
    stages = {},
    selected = 1,
    inProgress = false,
    pendingArena = false,
    startedAt = nil,
    endedAt = nil,
  }
end

Trial.state = NewState()
Trial.buttons = {}
local SendCommand

local title = CreateLabel(Trial, "GameFontHighlightLarge", 20, 0.96, 0.84, 0.30)
title:SetPoint("TOP", Trial, "TOP", 0, -18)
title:SetText("시련")

local subtitle = CreateLabel(Trial, "GameFontNormal", 12, 0.72, 0.72, 0.72)
subtitle:SetPoint("TOP", title, "BOTTOM", 0, -4)
subtitle:SetText("단계를 선택하고 당신의 그림자와 결투를 시작하세요.")

local close = CreateFrame("Button", nil, Trial, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", Trial, "TOPRIGHT", -10, -10)

Trial.exitButton = CreateFrame("Button", "KarazhanTrialExitButton", UIParent,
  "UIPanelButtonTemplate")
Trial.exitButton:SetSize(120, 28)
Trial.exitButton:SetPoint("BOTTOM", UIParent, "BOTTOM", 0, 18)
Trial.exitButton:SetText("시련 종료")
Trial.exitButton:Hide()

Trial.statusBox = CreateFrame("Frame", nil, UIParent)
Trial.statusBox:SetSize(240, 112)
Trial.statusBox:SetPoint("BOTTOM", UIParent, "BOTTOM", 0, 54)
Trial.statusBox:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
  edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
  tile = true,
  tileSize = 32,
  edgeSize = 16,
  insets = { left = 5, right = 5, top = 5, bottom = 5 },
})
Trial.statusBox:SetBackdropColor(0.04, 0.04, 0.04, 0.94)
Trial.statusBox:Hide()

Trial.statusTitle = CreateLabel(
  Trial.statusBox,
  "GameFontHighlight",
  13,
  1.0,
  0.84,
  0.25
)
Trial.statusTitle:SetPoint("TOPLEFT", Trial.statusBox, "TOPLEFT", 14, -12)
Trial.statusTitle:SetText("시련 진행 정보")

Trial.currentTimeText = CreateLabel(
  Trial.statusBox,
  "GameFontNormal",
  12,
  0.92,
  0.92,
  0.92
)
Trial.currentTimeText:SetPoint("TOPLEFT", Trial.statusTitle, "BOTTOMLEFT", 0, -10)

Trial.startTimeText = CreateLabel(
  Trial.statusBox,
  "GameFontNormal",
  12,
  0.92,
  0.92,
  0.92
)
Trial.startTimeText:SetPoint("TOPLEFT", Trial.currentTimeText, "BOTTOMLEFT", 0, -6)

Trial.endTimeText = CreateLabel(
  Trial.statusBox,
  "GameFontNormal",
  12,
  0.92,
  0.92,
  0.92
)
Trial.endTimeText:SetPoint("TOPLEFT", Trial.startTimeText, "BOTTOMLEFT", 0, -6)

Trial.leftPane = CreateFrame("Frame", nil, Trial)
Trial.leftPane:SetPoint("TOPLEFT", Trial, "TOPLEFT", 24, -54)
Trial.leftPane:SetSize(268, 452)

Trial.leftHeader = CreateLabel(
  Trial.leftPane,
  "GameFontHighlight",
  14,
  1.0,
  0.84,
  0.25
)
Trial.leftHeader:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 6, 0)
Trial.leftHeader:SetText("단계 선택")

Trial.leftSub = CreateLabel(
  Trial.leftPane,
  "GameFontNormal",
  11,
  0.68,
  0.68,
  0.68
)
Trial.leftSub:SetPoint("TOPRIGHT", Trial.leftPane, "TOPRIGHT", -6, 0)
Trial.leftSub:SetText("")

Trial.leftDivider = Trial.leftPane:CreateTexture(nil, "ARTWORK")
Trial.leftDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
Trial.leftDivider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
Trial.leftDivider:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 0, -22)
Trial.leftDivider:SetPoint("TOPRIGHT", Trial.leftPane, "TOPRIGHT", 0, -22)
Trial.leftDivider:SetHeight(8)

Trial.leftBg = Trial.leftPane:CreateTexture(nil, "BACKGROUND")
Trial.leftBg:SetTexture("Interface\\Buttons\\WHITE8x8")
Trial.leftBg:SetVertexColor(0.02, 0.02, 0.02, 0.55)
Trial.leftBg:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 0, -30)
Trial.leftBg:SetPoint("BOTTOMRIGHT", Trial.leftPane, "BOTTOMRIGHT", 0, 0)

Trial.rightPane = CreateFrame("Frame", nil, Trial)
Trial.rightPane:SetPoint("TOPRIGHT", Trial, "TOPRIGHT", -24, -54)
Trial.rightPane:SetSize(534, 452)

Trial.rightBg = Trial.rightPane:CreateTexture(nil, "BACKGROUND")
Trial.rightBg:SetTexture("Interface\\Buttons\\WHITE8x8")
Trial.rightBg:SetVertexColor(0.08, 0.03, 0.03, 0.62)
Trial.rightBg:SetAllPoints(Trial.rightPane)

Trial.rightBorder = CreateFrame("Frame", nil, Trial.rightPane)
Trial.rightBorder:SetAllPoints(Trial.rightPane)
Trial.rightBorder:SetBackdrop({
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 12,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
Trial.rightBorder:SetBackdropBorderColor(0.45, 0.30, 0.10, 0.80)

Trial.stageBadge = CreateFrame("Frame", nil, Trial.rightPane)
Trial.stageBadge:SetSize(52, 52)
Trial.stageBadge:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 18, -16)
Trial.stageBadge:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  edgeSize = 12,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
Trial.stageBadge:SetBackdropColor(0.12, 0.08, 0.02, 0.95)
Trial.stageBadge:SetBackdropBorderColor(0.88, 0.70, 0.22, 0.90)

Trial.stageBadgeText = CreateLabel(
  Trial.stageBadge,
  "GameFontHighlightLarge",
  18,
  1.0,
  0.84,
  0.25,
  "CENTER"
)
Trial.stageBadgeText:SetPoint("CENTER", Trial.stageBadge, "CENTER", 0, 0)

Trial.stageTitle = CreateLabel(
  Trial.rightPane,
  "GameFontHighlightLarge",
  22,
  0.96,
  0.92,
  0.86
)
Trial.stageTitle:SetPoint("TOPLEFT", Trial.stageBadge, "TOPRIGHT", 14, -2)
Trial.stageTitle:SetWidth(420)

Trial.stageMeta = CreateLabel(
  Trial.rightPane,
  "GameFontNormal",
  12,
  0.86,
  0.76,
  0.34
)
Trial.stageMeta:SetPoint("TOPLEFT", Trial.stageTitle, "BOTTOMLEFT", 0, -6)
Trial.stageMeta:SetWidth(420)

Trial.stageDivider = Trial.rightPane:CreateTexture(nil, "ARTWORK")
Trial.stageDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
Trial.stageDivider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
Trial.stageDivider:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 16, -84)
Trial.stageDivider:SetPoint("TOPRIGHT", Trial.rightPane, "TOPRIGHT", -16, -84)
Trial.stageDivider:SetHeight(8)

Trial.model = CreateFrame("DressUpModel", nil, Trial.rightPane)
Trial.model:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 20, -104)
Trial.model:SetPoint("BOTTOMRIGHT", Trial.rightPane, "BOTTOMRIGHT", -20, 126)
Trial.model:SetFacing(0.45)
Trial.model:SetModelScale(1.0)

Trial.modelBg = Trial.rightPane:CreateTexture(nil, "BORDER")
Trial.modelBg:SetTexture("Interface\\PaperDollInfoFrame\\UI-Character-General-TopLeft")
Trial.modelBg:SetTexCoord(0.12, 0.88, 0.08, 0.92)
Trial.modelBg:SetVertexColor(0.65, 0.20, 0.14, 0.22)
Trial.modelBg:SetPoint("TOPLEFT", Trial.model, "TOPLEFT", 0, 0)
Trial.modelBg:SetPoint("BOTTOMRIGHT", Trial.model, "BOTTOMRIGHT", 0, 0)

Trial.stageDesc = CreateLabel(
  Trial.rightPane,
  "GameFontNormal",
  13,
  0.95,
  0.82,
  0.24
)
Trial.stageDesc:SetPoint("TOPLEFT", Trial.model, "BOTTOMLEFT", 0, -14)
Trial.stageDesc:SetPoint("TOPRIGHT", Trial.model, "BOTTOMRIGHT", 0, -14)
Trial.stageDesc:SetJustifyH("LEFT")

Trial.start = CreateFrame("Button", nil, Trial.rightPane, "UIPanelButtonTemplate")
Trial.start:SetSize(160, 28)
Trial.start:SetPoint("BOTTOMRIGHT", Trial.rightPane, "BOTTOMRIGHT", -18, 16)
Trial.start:SetText("도전 시작")

Trial.cancel = CreateFrame("Button", nil, Trial.rightPane, "UIPanelButtonTemplate")
Trial.cancel:SetSize(120, 28)
Trial.cancel:SetPoint("RIGHT", Trial.start, "LEFT", -10, 0)
Trial.cancel:SetText("닫기")
Trial.cancel:SetScript("OnClick", function()
  Trial:Hide()
end)

Trial.abandon = CreateFrame("Button", nil, Trial.rightPane, "UIPanelButtonTemplate")
Trial.abandon:SetSize(120, 28)
Trial.abandon:SetPoint("RIGHT", Trial.cancel, "LEFT", -10, 0)
Trial.abandon:SetText("시련 포기")
Trial.abandon:Hide()
Trial.abandon:SetScript("OnClick", function()
  SendCommand("ABANDON")
  Trial.state.inProgress = false
  Trial.state.pendingArena = false
  Trial.state.endedAt = time()
  Trial.statusBox:Hide()
  Trial.exitButton:Hide()
  Trial:Hide()
end)

Trial.exitButton:SetScript("OnClick", function()
  SendCommand("ABANDON")
  Trial.state.inProgress = false
  Trial.state.pendingArena = false
  Trial.state.endedAt = time()
  Trial.statusBox:Hide()
  Trial.exitButton:Hide()
end)

SendCommand = function(payload)
  SendAddonMessage("TRIAL_CMD", payload, "WHISPER", UnitName("player"))
end

local function IsInArenaInstance()
  local _, instanceType = GetInstanceInfo()
  return instanceType == ARENA_INSTANCE_TYPE
end

local function FormatClock(value)
  if not value then
    return "진행 중"
  end

  return date("%H:%M:%S", value)
end

local function RefreshStatusTimes()
  Trial.currentTimeText:SetText("현재 시간: " .. date("%H:%M:%S"))
  Trial.startTimeText:SetText("시작 시간: " .. FormatClock(Trial.state.startedAt))
  Trial.endTimeText:SetText("종료 시간: " .. FormatClock(Trial.state.endedAt))
end

local function RefreshExitButton()
  if (Trial.state.inProgress or Trial.state.pendingArena)
    and IsInArenaInstance()
    and not Trial:IsShown() then
    RefreshStatusTimes()
    Trial.statusBox:Show()
    Trial.exitButton:Show()
  else
    Trial.statusBox:Hide()
    Trial.exitButton:Hide()
  end
end

local function GetStageDescription(stage)
  if not stage then
    return ""
  end

  local lines = {
    "언더시티 투기장에서 당신의 그림자와 1:1 결투를 치릅니다.",
    string.format("체력 배율 %.2f / 공격 배율 %.2f", stage.health, stage.damage),
    string.format("스킬 주기 %dms / 이동 속도 %.2f", stage.spellInterval, stage.moveSpeed),
  }

  return table.concat(lines, "\n")
end

local function RefreshSelection()
  local stage = Trial.state.stages[Trial.state.selected]
  if not stage then
    Trial.stageBadgeText:SetText("-")
    Trial.stageTitle:SetText("선택 가능한 시련이 없습니다")
    Trial.stageMeta:SetText("이전 단계를 먼저 클리어해야 다음 시련이 나타납니다.")
    Trial.stageDesc:SetText("")
    Trial.start:Disable()
    Trial.abandon:Hide()
    return
  end

  Trial.stageBadgeText:SetText(stage.stageId)
  Trial.stageTitle:SetText(stage.name)
  Trial.stageMeta:SetText(
    string.format("해금 상태: %d단계 클리어", Trial.state.highestCleared)
  )
  Trial.stageDesc:SetText(GetStageDescription(stage))
  if Trial.state.inProgress then
    Trial.start:Disable()
    Trial.abandon:Show()
  else
    Trial.start:Enable()
    Trial.abandon:Hide()
  end
end

local function SelectStage(index)
  Trial.state.selected = index
  for i, button in ipairs(Trial.buttons) do
    if i == index then
      button:SetBackdropColor(0.24, 0.17, 0.05, 0.95)
      button:SetBackdropBorderColor(0.95, 0.78, 0.22, 1)
    else
      button:SetBackdropColor(0.05, 0.05, 0.05, 0.82)
      button:SetBackdropBorderColor(0.18, 0.18, 0.18, 0.90)
    end
  end
  RefreshSelection()
end

local function CreateStageButton(index)
  local button = CreateFrame("Button", nil, Trial.leftPane)
  button:SetSize(252, 52)
  button:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 8, -38 - ((index - 1) * 56))
  button:SetBackdrop({
    bgFile = "Interface\\Buttons\\WHITE8x8",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    edgeSize = 10,
    insets = { left = 2, right = 2, top = 2, bottom = 2 },
  })
  button:SetBackdropColor(0.05, 0.05, 0.05, 0.82)
  button:SetBackdropBorderColor(0.18, 0.18, 0.18, 0.90)
  button:Hide()

  button.icon = button:CreateTexture(nil, "ARTWORK")
  button.icon:SetTexture("Interface\\Icons\\Achievement_Arena_2v2_7")
  button.icon:SetSize(26, 26)
  button.icon:SetPoint("LEFT", button, "LEFT", 10, 0)

  button.name = CreateLabel(button, "GameFontHighlight", 14, 1.0, 0.84, 0.25)
  button.name:SetPoint("TOPLEFT", button.icon, "TOPRIGHT", 10, -2)
  button.name:SetWidth(190)

  button.meta = CreateLabel(button, "GameFontNormalSmall", 11, 0.72, 0.72, 0.72)
  button.meta:SetPoint("TOPLEFT", button.name, "BOTTOMLEFT", 0, -5)
  button.meta:SetWidth(190)

  button:SetScript("OnClick", function(self)
    SelectStage(self.index)
  end)

  button.index = index
  return button
end

for i = 1, 8 do
  Trial.buttons[i] = CreateStageButton(i)
end

local function RefreshList()
  Trial.leftSub:SetText("해금 " .. tostring(#Trial.state.stages) .. "개")

  for i, button in ipairs(Trial.buttons) do
    local stage = Trial.state.stages[i]
    if stage then
      button.name:SetText(stage.name)
      button.meta:SetText(string.format("단계 %d", stage.stageId))
      button:Show()
    else
      button:Hide()
    end
  end

  if #Trial.state.stages > 0 then
    if Trial.state.selected < 1 or Trial.state.selected > #Trial.state.stages then
      Trial.state.selected = 1
    end
    SelectStage(Trial.state.selected)
  else
    Trial.state.selected = 0
    RefreshSelection()
  end

  RefreshExitButton()
end

local function ApplyOpen(parts)
  local previousStartedAt = Trial.state.startedAt
  Trial.state = NewState()
  Trial.state.highestCleared = tonumber(parts[2]) or 0
  Trial.state.inProgress = tonumber(parts[4]) == 1
  Trial.state.pendingArena = Trial.state.inProgress
  if Trial.state.inProgress then
    Trial.state.startedAt = previousStartedAt or time()
  end

  local encoded = parts[3] or ""
  if encoded ~= "" then
    local items = Split(encoded, "|")
    for _, item in ipairs(items) do
      local fields = Split(item, "~")
      table.insert(Trial.state.stages, {
        stageId = tonumber(fields[1]) or 0,
        name = fields[2] or "시련",
        health = tonumber(fields[3]) or 1,
        damage = tonumber(fields[4]) or 1,
        spellInterval = tonumber(fields[5]) or 0,
        moveSpeed = tonumber(fields[6]) or 1,
      })
    end
  end

  RefreshList()
  Trial.model:SetUnit("player")
  Trial:Show()
  RefreshExitButton()
end

Trial.start:SetScript("OnClick", function()
  local stage = Trial.state.stages[Trial.state.selected]
  if not stage then
    return
  end
  Trial.state.pendingArena = true
  Trial.state.startedAt = time()
  Trial.state.endedAt = nil
  SendCommand("START\t" .. tostring(stage.stageId))
  Trial:Hide()
  RefreshExitButton()
end)

Trial:RegisterEvent("PLAYER_LOGIN")
Trial:RegisterEvent("CHAT_MSG_ADDON")
Trial:RegisterEvent("PLAYER_ENTERING_WORLD")
Trial:RegisterEvent("ZONE_CHANGED_NEW_AREA")
Trial:RegisterEvent("PLAYER_LEAVING_WORLD")
Trial:SetScript("OnEvent", function(self, event, prefix, message)
  if event == "PLAYER_LOGIN" then
    if RegisterAddonMessagePrefix then
      RegisterAddonMessagePrefix("TRIAL_UI")
      RegisterAddonMessagePrefix("TRIAL_CMD")
    end

    SLASH_KARAZHANTRIAL1 = "/trial"
    SlashCmdList.KARAZHANTRIAL = function()
      SendCommand("OPEN")
    end
    RefreshExitButton()
    return
  end

  if event == "PLAYER_ENTERING_WORLD" or event == "ZONE_CHANGED_NEW_AREA" then
    if not IsInArenaInstance() then
      Trial.state.pendingArena = false
      if not Trial:IsShown() then
        Trial.state.inProgress = false
        if Trial.state.startedAt and not Trial.state.endedAt then
          Trial.state.endedAt = time()
        end
      end
    end
    RefreshExitButton()
    return
  end

  if event == "PLAYER_LEAVING_WORLD" then
    RefreshStatusTimes()
    return
  end

  if prefix ~= "TRIAL_UI" or type(message) ~= "string" then
    return
  end

  local parts = Split(message, "\t")
  if parts[1] == "OPEN" then
    ApplyOpen(parts)
  end
end)

Trial:SetScript("OnUpdate", function(_, elapsed)
  Trial._clockElapsed = (Trial._clockElapsed or 0) + elapsed
  if Trial._clockElapsed < 1 then
    return
  end

  Trial._clockElapsed = 0
  if Trial.statusBox:IsShown() then
    RefreshStatusTimes()
  end
end)
