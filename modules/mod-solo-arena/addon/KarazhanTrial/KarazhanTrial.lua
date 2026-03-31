local addonName = ...

local ARENA_INSTANCE_TYPE = "arena"
local SESSION_PENDING_SPAWN = 0
local SESSION_WAITING_FOR_START = 1
local SESSION_ACTIVE = 2
local SESSION_PENDING_FINISH = 3
local SESSION_AWAITING_RETURN = 4

local function Split(input, sep)
  local parts = {}
  local text = input or ""
  local start = 1
  local index = string.find(text, sep, start, true)

  while index do
    table.insert(parts, string.sub(text, start, index - 1))
    start = index + string.len(sep)
    index = string.find(text, sep, start, true)
  end

  table.insert(parts, string.sub(text, start))
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

local function CreatePanel(parent, width, height)
  local frame = CreateFrame("Frame", nil, parent)
  frame:SetSize(width, height)
  frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    tile = true,
    tileSize = 32,
    edgeSize = 14,
    insets = { left = 3, right = 3, top = 3, bottom = 3 },
  })
  frame:SetBackdropColor(0.06, 0.04, 0.04, 0.88)
  frame:SetBackdropBorderColor(0.42, 0.28, 0.08, 0.90)
  return frame
end

local function FormatDuration(seconds)
  seconds = math.max(0, tonumber(seconds) or 0)
  if seconds <= 0 then
    return "0초"
  end

  local minutes = math.floor(seconds / 60)
  local remain = math.floor(seconds % 60)
  if minutes > 0 then
    return string.format("%d분 %d초", minutes, remain)
  end

  return string.format("%d초", remain)
end

local function FormatRemaining(targetTime)
  if not targetTime or targetTime <= 0 then
    return "-"
  end

  return string.format("%d초", math.max(0, targetTime - time()))
end

local function GetStageNameById(stageId)
  local names = {
    [1] = "그림자 시련 1단계",
    [2] = "그림자 시련 2단계",
    [3] = "그림자 시련 3단계",
    [4] = "그림자 시련 4단계",
    [5] = "그림자 시련 5단계",
    [6] = "그림자 시련 6단계",
    [7] = "그림자 시련 7단계",
    [8] = "그림자 시련 8단계",
    [9] = "그림자 시련 9단계",
    [10] = "그림자 시련 10단계",
  }

  return names[tonumber(stageId) or 0]
    or string.format("시련 %d단계", tonumber(stageId) or 0)
end

local function GetMechanicNameByStage(stageId)
  local names = {
    [1] = "시련의 숨결",
    [2] = "뒤틀린 파편",
    [3] = "균열의 제단",
  }

  return names[tonumber(stageId) or 0] or ""
end

local function IsInArenaInstance()
  local _, instanceType = GetInstanceInfo()
  return instanceType == ARENA_INSTANCE_TYPE
end

local function NewState()
  return {
    highestCleared = 0,
    stages = {},
    selected = 1,
    inProgress = false,
    pendingArena = false,
    sessionState = SESSION_PENDING_SPAWN,
    preparationEndsAt = nil,
    combatEndsAt = nil,
    resultShown = false,
    result = {
      stageId = 0,
      stageName = "",
      resultLabel = "",
      rankLabel = "-",
      durationSec = 0,
    },
  }
end

local function SendCommand(payload)
  SendAddonMessage("TRIAL_CMD", payload, "WHISPER", UnitName("player"))
end

StaticPopupDialogs["KARAZHAN_TRIAL_ABANDON_CONFIRM"] = {
  text = "시련 종료시 어떠한 보상을 받을 수 없고, 미션 또한 실패로 간주합니다.\n그래도 종료 하시겠습니까?",
  button1 = ACCEPT,
  button2 = CANCEL,
  OnAccept = function()
    SendCommand("ABANDON")
  end,
  timeout = 0,
  whileDead = true,
  hideOnEscape = true,
  preferredIndex = STATICPOPUP_NUMDIALOGS,
}

local Trial = CreateFrame("Frame", "KarazhanTrialFrame", UIParent)
Trial:SetSize(860, 540)
Trial:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
Trial:SetClampedToScreen(true)
Trial:EnableMouse(true)
Trial:SetFrameStrata("DIALOG")
Trial:SetMovable(true)
Trial:RegisterForDrag("LeftButton")
Trial:SetScript("OnDragStart", Trial.StartMoving)
Trial:SetScript("OnDragStop", Trial.StopMovingOrSizing)
Trial:Hide()
Trial.state = NewState()
Trial.buttons = {}

tinsert(UISpecialFrames, "KarazhanTrialFrame")

Trial:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
  edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
  tile = true,
  tileSize = 32,
  edgeSize = 32,
  insets = { left = 11, right = 12, top = 12, bottom = 11 },
})
Trial:SetBackdropColor(0.04, 0.04, 0.04, 0.96)

local title = CreateLabel(
  Trial, "GameFontHighlightLarge", 20, 0.96, 0.84, 0.30, "CENTER")
title:SetPoint("TOP", Trial, "TOP", 0, -18)
title:SetText("시련")

local subtitle = CreateLabel(
  Trial, "GameFontNormal", 12, 0.72, 0.72, 0.72, "CENTER")
subtitle:SetPoint("TOP", title, "BOTTOM", 0, -4)
subtitle:SetText("단계를 선택하고 당신의 그림자와 결투를 시작하세요.")

local close = CreateFrame("Button", nil, Trial, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", Trial, "TOPRIGHT", -10, -10)

Trial.leftPane = CreateFrame("Frame", nil, Trial)
Trial.leftPane:SetPoint("TOPLEFT", Trial, "TOPLEFT", 24, -54)
Trial.leftPane:SetSize(272, 452)

Trial.leftHeader = CreateLabel(
  Trial.leftPane, "GameFontHighlight", 14, 1.0, 0.84, 0.25)
Trial.leftHeader:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 6, 0)
Trial.leftHeader:SetText("단계 선택")

Trial.leftSub = CreateLabel(
  Trial.leftPane, "GameFontNormal", 11, 0.68, 0.68, 0.68, "RIGHT")
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

Trial.stageBadge = CreatePanel(Trial.rightPane, 52, 52)
Trial.stageBadge:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 18, -16)

Trial.stageBadgeText = CreateLabel(
  Trial.stageBadge, "GameFontHighlightLarge", 18, 1.0, 0.84, 0.25, "CENTER")
Trial.stageBadgeText:SetPoint("CENTER", Trial.stageBadge, "CENTER", 0, 0)

Trial.stageTitle = CreateLabel(
  Trial.rightPane, "GameFontHighlightLarge", 22, 0.96, 0.92, 0.86)
Trial.stageTitle:SetPoint("TOPLEFT", Trial.stageBadge, "TOPRIGHT", 14, -2)
Trial.stageTitle:SetWidth(420)

Trial.stageMeta = CreateLabel(
  Trial.rightPane, "GameFontNormal", 12, 0.86, 0.76, 0.34)
Trial.stageMeta:SetPoint("TOPLEFT", Trial.stageTitle, "BOTTOMLEFT", 0, -6)
Trial.stageMeta:SetWidth(420)

Trial.stageDivider = Trial.rightPane:CreateTexture(nil, "ARTWORK")
Trial.stageDivider:SetTexture("Interface\\QuestFrame\\UI-QuestLogTitleHighlight")
Trial.stageDivider:SetVertexColor(0.85, 0.72, 0.24, 0.85)
Trial.stageDivider:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 16, -84)
Trial.stageDivider:SetPoint("TOPRIGHT", Trial.rightPane, "TOPRIGHT", -16, -84)
Trial.stageDivider:SetHeight(8)

Trial.contentPane = CreateFrame("Frame", nil, Trial.rightPane)
Trial.contentPane:SetPoint("TOPLEFT", Trial.rightPane, "TOPLEFT", 20, -104)
Trial.contentPane:SetPoint("BOTTOMRIGHT", Trial.rightPane, "BOTTOMRIGHT", -20, 60)

Trial.modelPane = CreateFrame("Frame", nil, Trial.contentPane)
Trial.modelPane:SetPoint("TOPLEFT", Trial.contentPane, "TOPLEFT", 0, 0)
Trial.modelPane:SetPoint("BOTTOMLEFT", Trial.contentPane, "BOTTOMLEFT", 0, 0)
Trial.modelPane:SetWidth(245)

Trial.infoPane = CreateFrame("Frame", nil, Trial.contentPane)
Trial.infoPane:SetPoint("TOPLEFT", Trial.modelPane, "TOPRIGHT", 16, 0)
Trial.infoPane:SetPoint("BOTTOMRIGHT", Trial.contentPane, "BOTTOMRIGHT", 0, 0)

Trial.model = CreateFrame("DressUpModel", nil, Trial.rightPane)
Trial.model:SetParent(Trial.modelPane)
Trial.model:SetPoint("TOPLEFT", Trial.modelPane, "TOPLEFT", 0, 0)
Trial.model:SetPoint("BOTTOMRIGHT", Trial.modelPane, "BOTTOMRIGHT", 0, 0)
Trial.model:SetFacing(0.45)
Trial.model:SetModelScale(1.0)

Trial.modelBg = Trial.modelPane:CreateTexture(nil, "BORDER")
Trial.modelBg:SetTexture("Interface\\PaperDollInfoFrame\\UI-Character-General-TopLeft")
Trial.modelBg:SetTexCoord(0.12, 0.88, 0.08, 0.92)
Trial.modelBg:SetVertexColor(0.65, 0.20, 0.14, 0.22)
Trial.modelBg:SetAllPoints(Trial.model)

Trial.stageDesc = CreateLabel(
  Trial.infoPane, "GameFontNormal", 13, 0.95, 0.82, 0.24)
Trial.stageDesc:SetPoint("TOPLEFT", Trial.infoPane, "TOPLEFT", 0, -4)
Trial.stageDesc:SetPoint("TOPRIGHT", Trial.infoPane, "TOPRIGHT", 0, -4)
Trial.stageDesc:SetWidth(250)
Trial.stageDesc:SetHeight(108)
Trial.stageDesc:SetJustifyH("LEFT")
Trial.stageDesc:SetJustifyV("TOP")
if Trial.stageDesc.SetNonSpaceWrap then
  Trial.stageDesc:SetNonSpaceWrap(false)
end
if Trial.stageDesc.SetWordWrap then
  Trial.stageDesc:SetWordWrap(true)
end

Trial.rewardTitle = CreateLabel(
  Trial.infoPane, "GameFontHighlight", 13, 1.0, 0.84, 0.25)
Trial.rewardTitle:SetPoint("TOPLEFT", Trial.stageDesc, "BOTTOMLEFT", 0, -12)
Trial.rewardTitle:SetText("보상")

Trial.rewardIconBg = CreatePanel(Trial.infoPane, 40, 40)
Trial.rewardIconBg:SetPoint("TOPLEFT", Trial.rewardTitle, "BOTTOMLEFT", 0, -8)
Trial.rewardIconBg.itemEntry = nil

Trial.rewardIcon = Trial.rewardIconBg:CreateTexture(nil, "ARTWORK")
Trial.rewardIcon:SetPoint("TOPLEFT", Trial.rewardIconBg, "TOPLEFT", 4, -4)
Trial.rewardIcon:SetPoint("BOTTOMRIGHT", Trial.rewardIconBg, "BOTTOMRIGHT", -4, 4)
Trial.rewardIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")

Trial.rewardIconBg:EnableMouse(true)
Trial.rewardIconBg:SetScript("OnEnter", function(self)
  if not self.itemEntry or self.itemEntry <= 0 then
    return
  end

  GameTooltip:SetOwner(self, "ANCHOR_RIGHT")
  GameTooltip:SetHyperlink("item:" .. tostring(self.itemEntry))
  GameTooltip:Show()
end)
Trial.rewardIconBg:SetScript("OnLeave", function()
  GameTooltip:Hide()
end)

Trial.rewardText = CreateLabel(
  Trial.infoPane, "GameFontNormal", 12, 0.95, 0.82, 0.24)
Trial.rewardText:SetPoint("TOPLEFT", Trial.rewardIconBg, "TOPRIGHT", 12, -2)
Trial.rewardText:SetPoint("TOPRIGHT", Trial.infoPane, "TOPRIGHT", 0, -58)
Trial.rewardText:SetJustifyH("LEFT")
if Trial.rewardText.SetWordWrap then
  Trial.rewardText:SetWordWrap(true)
end

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

Trial.statusBox = CreatePanel(UIParent, 280, 118)
Trial.statusBox:SetPoint("BOTTOM", UIParent, "BOTTOM", 0, 54)
Trial.statusBox:SetClampedToScreen(true)
Trial.statusBox:EnableMouse(true)
Trial.statusBox:SetMovable(true)
Trial.statusBox:RegisterForDrag("LeftButton")
Trial.statusBox:SetScript("OnDragStart", Trial.statusBox.StartMoving)
Trial.statusBox:SetScript("OnDragStop", Trial.statusBox.StopMovingOrSizing)
Trial.statusBox:Hide()

Trial.statusTitle = CreateLabel(
  Trial.statusBox, "GameFontHighlight", 13, 1.0, 0.84, 0.25, "CENTER")
Trial.statusTitle:SetPoint("TOP", Trial.statusBox, "TOP", 0, -12)
Trial.statusTitle:SetText("시련 진행 정보")

Trial.currentTimeText = CreateLabel(
  Trial.statusBox, "GameFontNormal", 12, 0.92, 0.92, 0.92, "CENTER")
Trial.currentTimeText:SetPoint("TOP", Trial.statusTitle, "BOTTOM", 0, -10)

Trial.exitButton = CreateFrame(
  "Button", "KarazhanTrialExitButton", Trial.statusBox, "UIPanelButtonTemplate")
Trial.exitButton:SetSize(140, 28)
Trial.exitButton:SetPoint("BOTTOM", Trial.statusBox, "BOTTOM", 0, 14)
Trial.exitButton:SetText("시련 종료")
Trial.exitButton:SetScript("OnClick", function()
  StaticPopup_Show("KARAZHAN_TRIAL_ABANDON_CONFIRM")
end)

Trial.resultFrame = CreatePanel(UIParent, 360, 220)
Trial.resultFrame:SetPoint("CENTER", UIParent, "CENTER", 0, 0)
Trial.resultFrame:SetClampedToScreen(true)
Trial.resultFrame:Hide()

Trial.resultTitle = CreateLabel(
  Trial.resultFrame, "GameFontHighlightLarge", 18, 1.0, 0.84, 0.25, "CENTER")
Trial.resultTitle:SetPoint("TOP", Trial.resultFrame, "TOP", 0, -18)
Trial.resultTitle:SetText("시련 결과")

Trial.resultStage = CreateLabel(
  Trial.resultFrame, "GameFontHighlight", 15, 0.96, 0.92, 0.86, "CENTER")
Trial.resultStage:SetPoint("TOP", Trial.resultTitle, "BOTTOM", 0, -14)
Trial.resultStage:SetWidth(300)

Trial.resultSummary = CreateLabel(
  Trial.resultFrame, "GameFontNormalLarge", 16, 0.95, 0.82, 0.24, "CENTER")
Trial.resultSummary:SetPoint("TOP", Trial.resultStage, "BOTTOM", 0, -16)
Trial.resultSummary:SetWidth(280)

Trial.resultTime = CreateLabel(
  Trial.resultFrame, "GameFontNormal", 13, 0.90, 0.90, 0.90, "CENTER")
Trial.resultTime:SetPoint("TOP", Trial.resultSummary, "BOTTOM", 0, -16)
Trial.resultTime:SetWidth(280)

Trial.returnButton = CreateFrame(
  "Button", nil, Trial.resultFrame, "UIPanelButtonTemplate")
Trial.returnButton:SetSize(140, 28)
Trial.returnButton:SetPoint("BOTTOM", Trial.resultFrame, "BOTTOM", 0, 18)
Trial.returnButton:SetText("복귀")

local function GetStageDescription(stage)
  if not stage then
    return ""
  end

  local lines = {
    "언더시티 투기장에서 당신의 그림자와 1:1 결투를 치릅니다.",
    "입장 조건: 아이템 600022 입장권 1개 필요",
    "일일 입장 제한: 하루 최대 5회",
    string.format("체력 배율 %.2f / 공격 배율 %.2f", stage.health, stage.damage),
    string.format("스킬 주기 %dms / 이동 속도 %.2f", stage.spellInterval, stage.moveSpeed),
  }

  if stage.mechanicName and stage.mechanicName ~= "" then
    table.insert(lines, "주요 기믹: " .. stage.mechanicName)
  end

  return table.concat(lines, "\n")
end

local function GetBestRankText(stage)
  if not stage or not stage.bestRankLabel or stage.bestRankLabel == "-" then
    return "최고 랭크: 미기록"
  end

  return string.format(
    "최고 랭크: %s (%s)",
    stage.bestRankLabel,
    FormatDuration(stage.bestTimeSec)
  )
end

local function GetStageReward(stage)
  if not stage or not stage.rewards or #stage.rewards == 0 then
    return {
      icon = "Interface\\Icons\\INV_Misc_QuestionMark",
      text = "설정된 보상이 없습니다.",
      itemEntry = nil,
    }
  end

  local firstReward = stage.rewards[1]
  local icon = GetItemIcon(firstReward.itemEntry)
    or "Interface\\Icons\\INV_Misc_QuestionMark"
  local lines = {}

  for _, reward in ipairs(stage.rewards) do
    local itemName = GetItemInfo(reward.itemEntry)
      or ("아이템 " .. tostring(reward.itemEntry))
    local chanceText = ""
    if reward.chance and reward.chance > 0 and reward.chance < 100 then
      chanceText = string.format(" (%.1f%%)", reward.chance)
    end

    table.insert(lines, string.format(
      "%s x%d%s",
      itemName,
      reward.itemCount or 1,
      chanceText
    ))
  end

  return {
    icon = icon,
    text = table.concat(lines, "\n"),
    itemEntry = firstReward.itemEntry,
  }
end

local function RefreshStatusTimes()
  if Trial.state.sessionState == SESSION_ACTIVE and Trial.state.combatEndsAt then
    Trial.currentTimeText:SetText(
      "전투 종료까지: " .. FormatRemaining(Trial.state.combatEndsAt))
  else
    Trial.currentTimeText:SetText(
      "전투 시작까지: " .. FormatRemaining(Trial.state.preparationEndsAt))
  end
end

local function RefreshStatusBox()
  if Trial.state.resultShown then
    Trial.statusBox:Hide()
    return
  end

  if (Trial.state.inProgress or Trial.state.pendingArena)
    and IsInArenaInstance()
    and not Trial:IsShown() then
    RefreshStatusTimes()
    Trial.statusBox:Show()
  else
    Trial.statusBox:Hide()
  end
end

local function RefreshSelection()
  local stage = Trial.state.stages[Trial.state.selected]
  if not stage then
    Trial.stageBadgeText:SetText("-")
    Trial.stageTitle:SetText("선택 가능한 시련이 없습니다")
    Trial.stageMeta:SetText("이전 단계를 먼저 클리어해야 다음 단계가 열립니다.")
    Trial.stageDesc:SetText("")
    Trial.rewardIcon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
    Trial.rewardText:SetText("설정된 보상이 없습니다.")
    Trial.rewardIconBg.itemEntry = nil
    Trial.start:Disable()
    return
  end

  Trial.stageBadgeText:SetText(stage.stageId)
  Trial.stageTitle:SetText(stage.name)
  Trial.stageMeta:SetText(GetBestRankText(stage))
  Trial.stageDesc:SetText(GetStageDescription(stage))

  local reward = GetStageReward(stage)
  Trial.rewardIcon:SetTexture(reward.icon)
  Trial.rewardText:SetText(reward.text)
  Trial.rewardIconBg.itemEntry = reward.itemEntry

  if Trial.state.inProgress or Trial.state.pendingArena then
    Trial.start:Disable()
  else
    Trial.start:Enable()
  end
end

local function SelectStage(index)
  Trial.state.selected = index
  for i, button in ipairs(Trial.buttons) do
    if i == index then
      button.bg:SetVertexColor(0.28, 0.18, 0.04, 0.96)
      button.top:SetVertexColor(0.95, 0.84, 0.35, 1.00)
      button.bottom:SetVertexColor(0.95, 0.84, 0.35, 1.00)
      button.left:SetVertexColor(0.95, 0.84, 0.35, 1.00)
      button.right:SetVertexColor(0.95, 0.84, 0.35, 1.00)
      button.text:SetTextColor(1.0, 0.92, 0.40)
    else
      button.bg:SetVertexColor(0.07, 0.07, 0.07, 0.88)
      button.top:SetVertexColor(0.42, 0.28, 0.08, 0.88)
      button.bottom:SetVertexColor(0.42, 0.28, 0.08, 0.88)
      button.left:SetVertexColor(0.42, 0.28, 0.08, 0.88)
      button.right:SetVertexColor(0.42, 0.28, 0.08, 0.88)
      button.text:SetTextColor(1.0, 0.84, 0.25)
    end
  end
  RefreshSelection()
end

local function CreateStageButton(index)
  local button = CreateFrame("Button", nil, Trial.leftPane)
  button:SetSize(252, 52)
  button:SetPoint("TOPLEFT", Trial.leftPane, "TOPLEFT", 8, -38 - ((index - 1) * 56))
  button:SetFrameStrata("DIALOG")
  button:SetFrameLevel(Trial.leftPane:GetFrameLevel() + 10)
  button:EnableMouse(true)
  button:Hide()
  button.index = index

  button.bg = button:CreateTexture(nil, "BACKGROUND")
  button.bg:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.bg:SetAllPoints(button)
  button.bg:SetVertexColor(0.07, 0.07, 0.07, 0.88)

  button.top = button:CreateTexture(nil, "BORDER")
  button.top:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.top:SetPoint("TOPLEFT", button, "TOPLEFT", 0, 0)
  button.top:SetPoint("TOPRIGHT", button, "TOPRIGHT", 0, 0)
  button.top:SetHeight(1)
  button.top:SetVertexColor(0.42, 0.28, 0.08, 0.88)

  button.bottom = button:CreateTexture(nil, "BORDER")
  button.bottom:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.bottom:SetPoint("BOTTOMLEFT", button, "BOTTOMLEFT", 0, 0)
  button.bottom:SetPoint("BOTTOMRIGHT", button, "BOTTOMRIGHT", 0, 0)
  button.bottom:SetHeight(1)
  button.bottom:SetVertexColor(0.42, 0.28, 0.08, 0.88)

  button.left = button:CreateTexture(nil, "BORDER")
  button.left:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.left:SetPoint("TOPLEFT", button, "TOPLEFT", 0, 0)
  button.left:SetPoint("BOTTOMLEFT", button, "BOTTOMLEFT", 0, 0)
  button.left:SetWidth(1)
  button.left:SetVertexColor(0.42, 0.28, 0.08, 0.88)

  button.right = button:CreateTexture(nil, "BORDER")
  button.right:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.right:SetPoint("TOPRIGHT", button, "TOPRIGHT", 0, 0)
  button.right:SetPoint("BOTTOMRIGHT", button, "BOTTOMRIGHT", 0, 0)
  button.right:SetWidth(1)
  button.right:SetVertexColor(0.42, 0.28, 0.08, 0.88)

  button.highlight = button:CreateTexture(nil, "HIGHLIGHT")
  button.highlight:SetTexture("Interface\\Buttons\\WHITE8x8")
  button.highlight:SetAllPoints(button)
  button.highlight:SetVertexColor(1.0, 0.9, 0.4, 0.08)

  button.text = button:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
  button.text:SetFont(STANDARD_TEXT_FONT, 13, "")
  button.text:SetPoint("LEFT", button, "LEFT", 12, 0)
  button.text:SetPoint("RIGHT", button, "RIGHT", -12, 0)
  button.text:SetJustifyH("LEFT")
  button.text:SetJustifyV("MIDDLE")
  button.text:SetTextColor(1.0, 0.84, 0.25)
  button.text:SetText("단계 로딩")

  button:SetScript("OnClick", function(self)
    SelectStage(self.index)
  end)

  return button
end

for i = 1, 10 do
  Trial.buttons[i] = CreateStageButton(i)
end

local function RefreshList()
  Trial.leftSub:SetText("해금 " .. tostring(#Trial.state.stages) .. "개")

  for i, button in ipairs(Trial.buttons) do
    local stage = Trial.state.stages[i]
    if stage then
      if stage.bestRankLabel and stage.bestRankLabel ~= "-" then
        button.text:SetText(string.format(
          "%s  |  최고 랭크 %s",
          stage.name,
          stage.bestRankLabel
        ))
      else
        button.text:SetText(stage.name)
      end
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

  RefreshStatusBox()
end

local function ShowResult()
  Trial.resultTitle:SetText("시련 결과")
  Trial.resultStage:SetText(Trial.state.result.stageName or "")
  Trial.resultSummary:SetText(string.format(
    "결과: %s\n랭크: %s",
    Trial.state.result.resultLabel or "종료",
    Trial.state.result.rankLabel or "-"
  ))
  Trial.resultTime:SetText("소요시간: " .. FormatDuration(Trial.state.result.durationSec))
  Trial.resultFrame:Show()
end

local function ApplyOpenPayload(highestCleared, encoded, inProgress,
  preparationEndsAt, combatEndsAt, sessionState)
  Trial.state = NewState()
  Trial.state.highestCleared = tonumber(highestCleared) or 0
  Trial.state.inProgress = tonumber(inProgress) == 1
  Trial.state.pendingArena = Trial.state.inProgress
  Trial.state.preparationEndsAt = tonumber(preparationEndsAt) or nil
  Trial.state.combatEndsAt = tonumber(combatEndsAt) or nil
  Trial.state.sessionState = tonumber(sessionState) or SESSION_PENDING_SPAWN

  encoded = encoded or ""
  if encoded ~= "" then
    local items = Split(encoded, "|")
    for _, item in ipairs(items) do
      local fields = Split(item, "~")
      local rankParts = Split(fields[8] or "-^0", "^")
      local stageId = tonumber(fields[1]) or 0
      local stage = {
        stageId = stageId,
        name = GetStageNameById(stageId),
        health = tonumber(fields[3]) or 1,
        damage = tonumber(fields[4]) or 1,
        spellInterval = tonumber(fields[5]) or 0,
        moveSpeed = tonumber(fields[6]) or 1,
        rewards = {},
        bestRankLabel = rankParts[1] or "-",
        bestTimeSec = tonumber(rankParts[2]) or 0,
        mechanicName = GetMechanicNameByStage(stageId),
      }

      local rewardField = fields[7] or ""
      if rewardField ~= "" and rewardField ~= "0^0^0" then
        local rewardEntries = Split(rewardField, ",")
        for _, rewardEntry in ipairs(rewardEntries) do
          local rewardParts = Split(rewardEntry, "^")
          local itemEntry = tonumber(rewardParts[1]) or 0
          if itemEntry > 0 then
            table.insert(stage.rewards, {
              itemEntry = itemEntry,
              itemCount = tonumber(rewardParts[2]) or 1,
              chance = tonumber(rewardParts[3]) or 100,
            })
          end
        end
      end

      table.insert(Trial.state.stages, stage)
    end
  end

  Trial.model:SetUnit("player")
  Trial.state.resultShown = false
  Trial.resultFrame:Hide()
  Trial:Show()
  RefreshList()

  if #Trial.state.stages == 0 then
    Trial.stageBadgeText:SetText("-")
    Trial.stageTitle:SetText("표시할 단계 데이터가 없습니다")
    Trial.stageMeta:SetText("서버에서 시련 단계 목록을 받지 못했습니다.")
    Trial.stageDesc:SetText("서버 로그의 SoloArena SendUi 항목을 확인해 주세요.")
  end
end

Trial:SetScript("OnShow", function()
  RefreshList()
end)

local function ApplyOpen(parts)
  ApplyOpenPayload(parts[2], parts[3], parts[4], parts[5], parts[7], parts[9])
end

Trial.start:SetScript("OnClick", function()
  local stage = Trial.state.stages[Trial.state.selected]
  if not stage then
    return
  end

  Trial.state.pendingArena = true
  SendCommand("START\t" .. tostring(stage.stageId))
  Trial:Hide()
  RefreshStatusBox()
end)

Trial.returnButton:SetScript("OnClick", function()
  SendCommand("RETURN")
  Trial.state.resultShown = false
  Trial.state.inProgress = false
  Trial.state.pendingArena = false
  Trial.state.sessionState = SESSION_PENDING_SPAWN
  Trial.resultFrame:Hide()
  RefreshStatusBox()
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
    RefreshStatusBox()
    return
  end

  if event == "PLAYER_ENTERING_WORLD" or event == "ZONE_CHANGED_NEW_AREA" then
    if not IsInArenaInstance() and not Trial.state.resultShown then
      Trial.state.pendingArena = false
      Trial.state.inProgress = false
      Trial.state.sessionState = SESSION_PENDING_SPAWN
    end
    RefreshStatusBox()
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
    return
  end

  if tonumber(parts[1]) then
    ApplyOpenPayload(parts[1], parts[2], parts[3], parts[4], parts[6], parts[8])
    return
  end

  if parts[1] == "TIME" then
    Trial.state.preparationEndsAt = tonumber(parts[2]) or nil
    Trial.state.combatEndsAt = tonumber(parts[4]) or nil
    Trial.state.sessionState = tonumber(parts[6]) or SESSION_PENDING_SPAWN
    Trial.state.inProgress = Trial.state.sessionState == SESSION_ACTIVE
      or Trial.state.sessionState == SESSION_WAITING_FOR_START
    if Trial.state.sessionState == SESSION_ACTIVE then
      Trial.state.pendingArena = false
    end
    RefreshStatusBox()
    return
  end

  if parts[1] == "RESULT" then
    Trial.state.resultShown = true
    Trial.state.inProgress = false
    Trial.state.pendingArena = false
    Trial.state.sessionState = SESSION_AWAITING_RETURN
    Trial.state.result.stageId = tonumber(parts[2]) or 0
    Trial.state.result.stageName = GetStageNameById(tonumber(parts[2]) or 0)
    Trial.state.result.resultLabel = parts[4] or "종료"
    Trial.state.result.rankLabel = parts[5] or "-"
    Trial.state.result.durationSec = tonumber(parts[6]) or 0
    Trial:Hide()
    Trial.statusBox:Hide()
    ShowResult()
  end
end)

Trial.statusBox:SetScript("OnUpdate", function(_, elapsed)
  Trial._clockElapsed = (Trial._clockElapsed or 0) + elapsed
  if Trial._clockElapsed < 1 then
    return
  end

  Trial._clockElapsed = 0
  if Trial.statusBox:IsShown() then
    RefreshStatusTimes()
  end
end)
