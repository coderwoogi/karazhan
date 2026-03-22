local addonName = ...

local KBM = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent)
KBM:SetSize(456, 560)
KBM:SetPoint("CENTER", UIParent, "CENTER", 160, 0)
KBM:SetClampedToScreen(true)
KBM:EnableMouse(true)
KBM:SetMovable(true)
KBM:RegisterForDrag("LeftButton")
KBM:SetScript("OnDragStart", KBM.StartMoving)
KBM:SetScript("OnDragStop", KBM.StopMovingOrSizing)
KBM:Hide()

KBM.state = {
  themeKey = "",
  themeName = "-",
  title = "-",
  targetLabel = "-",
  currentCount = 0,
  targetCount = 0,
  remaining = 0,
  timeLimit = 0,
  status = "inactive",
  missionType = 0,
  briefing = "",
  announcement = "",
  expiresAt = nil,
}

local statusColors = {
  inactive = {0.48, 0.48, 0.48},
  active = {0.20, 0.58, 0.82},
  complete = {0.18, 0.64, 0.32},
  failed = {0.84, 0.22, 0.22},
}

local themeIcons = {
  slaughter = "Interface\\Icons\\Ability_Warrior_Cleave",
  clean_run = "Interface\\Icons\\Spell_Holy_GuardianSpirit",
  speed_run = "Interface\\Icons\\Ability_Rogue_Sprint",
  boss_focus = "Interface\\Icons\\Ability_Warrior_BattleShout",
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

local function FormatRemaining(seconds)
  if not seconds or seconds <= 0 then
    return "00:00"
  end

  local minutes = math.floor(seconds / 60)
  local remain = math.floor(seconds % 60)
  return string.format("%02d:%02d", minutes, remain)
end

local function GetStatusText(status)
  if status == "complete" then
    return "완료"
  elseif status == "failed" then
    return "실패"
  elseif status == "active" then
    return "진행 중"
  end

  return "대기"
end

local function GetThemeText(themeKey, themeName)
  if themeName and themeName ~= "" and themeName ~= "-" then
    return themeName
  end

  if themeKey == "slaughter" then
    return "학살형"
  elseif themeKey == "clean_run" then
    return "무사고 클리어형"
  elseif themeKey == "speed_run" then
    return "속전속결형"
  elseif themeKey == "boss_focus" then
    return "보스 집중형"
  end

  return "미지정"
end

local function ShowRaidAlert(message)
  if not message or message == "" then
    return
  end

  if RaidNotice_AddMessage and RaidWarningFrame and ChatTypeInfo["RAID_WARNING"] then
    RaidNotice_AddMessage(RaidWarningFrame, message, ChatTypeInfo["RAID_WARNING"])
  end

  if UIErrorsFrame then
    UIErrorsFrame:AddMessage(message, 1.0, 0.82, 0.18, 1.0)
  end
end

local function CreateQuestText(parent, template, size)
  local fs = parent:CreateFontString(nil, "OVERLAY", template or "QuestFont")
  fs:SetFont(STANDARD_TEXT_FONT, size or 16, "")
  fs:SetTextColor(0.21, 0.16, 0.10)
  fs:SetJustifyH("LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

KBM.bg = KBM:CreateTexture(nil, "BACKGROUND")
KBM.bg:SetTexture("Interface\\QuestFrame\\QuestBG")
KBM.bg:SetAllPoints(KBM)
KBM.bg:SetTexCoord(0, 1, 0, 1)

KBM.topBorder = KBM:CreateTexture(nil, "BORDER")
KBM.topBorder:SetTexture("Interface\\QuestFrame\\UI-QuestLog-TopLeft")
KBM.topBorder:SetSize(512, 128)
KBM.topBorder:SetPoint("TOPLEFT", KBM, "TOPLEFT", -28, 20)
KBM.topBorder:SetAlpha(0.92)

KBM.bottomBorder = KBM:CreateTexture(nil, "BORDER")
KBM.bottomBorder:SetTexture("Interface\\QuestFrame\\UI-QuestLog-BotLeft")
KBM.bottomBorder:SetSize(512, 128)
KBM.bottomBorder:SetPoint("BOTTOMLEFT", KBM, "BOTTOMLEFT", -28, -18)
KBM.bottomBorder:SetAlpha(0.92)

local close = CreateFrame("Button", nil, KBM, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -8, -8)
close:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.reopen = CreateFrame(
  "Button",
  "KarazhanBonusMissionReopenButton",
  UIParent,
  "UIPanelButtonTemplate")
KBM.reopen:SetSize(92, 24)
KBM.reopen:SetPoint("CENTER", UIParent, "CENTER", 390, -250)
KBM.reopen:SetText("임무 보기")
KBM.reopen:SetScript("OnClick", function()
  KBM:Show()
  KBM.reopen:Hide()
end)
KBM.reopen:Hide()

KBM.iconRing = CreateFrame("Frame", nil, KBM)
KBM.iconRing:SetSize(52, 52)
KBM.iconRing:SetPoint("TOPLEFT", KBM, "TOPLEFT", 28, -30)

KBM.iconBorder = KBM.iconRing:CreateTexture(nil, "ARTWORK")
KBM.iconBorder:SetTexture("Interface\\Minimap\\MiniMap-TrackingBorder")
KBM.iconBorder:SetAllPoints(KBM.iconRing)

KBM.icon = KBM.iconRing:CreateTexture(nil, "BACKGROUND")
KBM.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
KBM.icon:SetSize(34, 34)
KBM.icon:SetPoint("CENTER", KBM.iconRing, "CENTER", 0, 0)

KBM.title = CreateQuestText(KBM, "QuestTitleFont", 28)
KBM.title:SetPoint("TOPLEFT", KBM, "TOPLEFT", 88, -36)
KBM.title:SetWidth(290)
KBM.title:SetText("추가 임무")

KBM.titleLine = KBM:CreateTexture(nil, "ARTWORK")
KBM.titleLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.titleLine:SetPoint("TOPLEFT", KBM.title, "BOTTOMLEFT", 0, -10)
KBM.titleLine:SetPoint("RIGHT", KBM, "RIGHT", -46, 0)
KBM.titleLine:SetHeight(8)
KBM.titleLine:SetTexCoord(0, 1, 0, 0.5)

KBM.theme = CreateQuestText(KBM, "QuestTitleFontBlackShadow", 15)
KBM.theme:SetPoint("TOPLEFT", KBM.titleLine, "BOTTOMLEFT", 0, -10)
KBM.theme:SetWidth(300)
KBM.theme:SetText("테마: 미지정")

KBM.status = CreateQuestText(KBM, "QuestTitleFontBlackShadow", 15)
KBM.status:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -46, -92)
KBM.status:SetWidth(120)
KBM.status:SetJustifyH("RIGHT")
KBM.status:SetText("상태: 대기")

KBM.briefing = CreateQuestText(KBM, "QuestFont", 17)
KBM.briefing:SetPoint("TOPLEFT", KBM.theme, "BOTTOMLEFT", 0, -18)
KBM.briefing:SetWidth(344)
KBM.briefing:SetSpacing(8)
KBM.briefing:SetText("추가 임무가 아직 배정되지 않았습니다.")

KBM.objectivesTitle = CreateQuestText(KBM, "QuestTitleFontBlackShadow", 18)
KBM.objectivesTitle:SetPoint("TOPLEFT", KBM.briefing, "BOTTOMLEFT", 0, -18)
KBM.objectivesTitle:SetText("목표")

KBM.objectives = CreateQuestText(KBM, "QuestFont", 17)
KBM.objectives:SetPoint("TOPLEFT", KBM.objectivesTitle, "BOTTOMLEFT", 0, -10)
KBM.objectives:SetWidth(344)
KBM.objectives:SetSpacing(6)
KBM.objectives:SetText("아직 목표가 없습니다.")

KBM.timer = CreateQuestText(KBM, "QuestFontHighlight", 15)
KBM.timer:SetPoint("TOPLEFT", KBM.objectives, "BOTTOMLEFT", 0, -10)
KBM.timer:SetWidth(160)
KBM.timer:SetText("남은 시간: 00:00")

KBM.progress = CreateQuestText(KBM, "QuestFontHighlight", 15)
KBM.progress:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -46, -328)
KBM.progress:SetWidth(160)
KBM.progress:SetJustifyH("RIGHT")
KBM.progress:SetText("진행도: 0 / 0")

KBM.progressBarBG = CreateFrame("Frame", nil, KBM)
KBM.progressBarBG:SetPoint("TOPLEFT", KBM.timer, "BOTTOMLEFT", 0, -8)
KBM.progressBarBG:SetSize(344, 18)
KBM.progressBarBG:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.progressBarBG:SetBackdropColor(0.10, 0.08, 0.05, 0.95)
KBM.progressBarBG:SetBackdropBorderColor(0.46, 0.36, 0.24, 0.90)

KBM.progressBar = KBM.progressBarBG:CreateTexture(nil, "ARTWORK")
KBM.progressBar:SetTexture("Interface\\TARGETINGFRAME\\UI-StatusBar")
KBM.progressBar:SetVertexColor(0.59, 0.33, 0.19, 0.95)
KBM.progressBar:SetPoint("TOPLEFT", KBM.progressBarBG, "TOPLEFT", 2, -2)
KBM.progressBar:SetPoint("BOTTOMLEFT", KBM.progressBarBG, "BOTTOMLEFT", 2, 2)
KBM.progressBar:SetWidth(1)

KBM.rewardsTitle = CreateQuestText(KBM, "QuestTitleFontBlackShadow", 18)
KBM.rewardsTitle:SetPoint("TOPLEFT", KBM.progressBarBG, "BOTTOMLEFT", 0, -18)
KBM.rewardsTitle:SetText("보상")

KBM.rewardLeftIcon = KBM:CreateTexture(nil, "ARTWORK")
KBM.rewardLeftIcon:SetTexture("Interface\\Icons\\INV_Misc_Coin_01")
KBM.rewardLeftIcon:SetSize(36, 36)
KBM.rewardLeftIcon:SetPoint("TOPLEFT", KBM.rewardsTitle, "BOTTOMLEFT", 0, -12)

KBM.rewardLeftText = CreateQuestText(KBM, "QuestFontHighlight", 15)
KBM.rewardLeftText:SetPoint("LEFT", KBM.rewardLeftIcon, "RIGHT", 10, 0)
KBM.rewardLeftText:SetWidth(120)
KBM.rewardLeftText:SetText("추가 보상")

KBM.rewardRightIcon = KBM:CreateTexture(nil, "ARTWORK")
KBM.rewardRightIcon:SetTexture("Interface\\Icons\\INV_Chest_Cloth_17")
KBM.rewardRightIcon:SetSize(36, 36)
KBM.rewardRightIcon:SetPoint("TOPLEFT", KBM.rewardLeftIcon, "TOPLEFT", 188, 0)

KBM.rewardRightText = CreateQuestText(KBM, "QuestFontHighlight", 15)
KBM.rewardRightText:SetPoint("LEFT", KBM.rewardRightIcon, "RIGHT", 10, 0)
KBM.rewardRightText:SetWidth(120)
KBM.rewardRightText:SetText("임무 상자")

KBM.footerLine = KBM:CreateTexture(nil, "ARTWORK")
KBM.footerLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.footerLine:SetPoint("TOPLEFT", KBM.rewardLeftIcon, "BOTTOMLEFT", -2, -18)
KBM.footerLine:SetPoint("RIGHT", KBM, "RIGHT", -50, 0)
KBM.footerLine:SetHeight(8)
KBM.footerLine:SetTexCoord(0, 1, 0, 0.5)

KBM.accept = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.accept:SetSize(160, 34)
KBM.accept:SetPoint("BOTTOMLEFT", KBM, "BOTTOMLEFT", 38, 24)
KBM.accept:SetText("확인")
KBM.accept:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.decline = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.decline:SetSize(160, 34)
KBM.decline:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -42, 24)
KBM.decline:SetText("접기")
KBM.decline:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

local function UpdateThemeVisuals()
  local themeText = GetThemeText(KBM.state.themeKey, KBM.state.themeName)
  KBM.theme:SetText("테마: " .. themeText)
  KBM.icon:SetTexture(themeIcons[KBM.state.themeKey] or "Interface\\Icons\\INV_Misc_QuestionMark")
end

local function UpdateRewards()
  if KBM.state.status == "complete" then
    KBM.rewardLeftText:SetText("추가 보상 획득")
    KBM.rewardRightText:SetText("완료 보상 지급")
  elseif KBM.state.status == "failed" then
    KBM.rewardLeftText:SetText("기본 보상만 유지")
    KBM.rewardRightText:SetText("추가 보상 없음")
  else
    KBM.rewardLeftText:SetText("추가 보상")
    KBM.rewardRightText:SetText("임무 상자")
  end
end

local function Refresh()
  UpdateThemeVisuals()
  UpdateRewards()

  KBM.title:SetText(KBM.state.title or "추가 임무")
  KBM.status:SetText("상태: " .. GetStatusText(KBM.state.status))

  local statusColor = statusColors[KBM.state.status] or statusColors.inactive
  KBM.status:SetTextColor(statusColor[1], statusColor[2], statusColor[3])

  KBM.briefing:SetText(KBM.state.briefing ~= "" and KBM.state.briefing or "작전 브리핑이 아직 없습니다.")
  KBM.objectives:SetText(string.format("%s %d / %d", KBM.state.targetLabel or "-", KBM.state.currentCount or 0, KBM.state.targetCount or 0))
  KBM.timer:SetText("남은 시간: " .. FormatRemaining(KBM.state.remaining or 0))
  KBM.progress:SetText(string.format("진행도: %d / %d", KBM.state.currentCount or 0, KBM.state.targetCount or 0))

  local maxCount = KBM.state.targetCount or 0
  local current = KBM.state.currentCount or 0
  local width = 1
  if maxCount > 0 then
    width = math.floor(340 * math.min(current / maxCount, 1))
    if width < 1 then
      width = 1
    end
  end
  KBM.progressBar:SetWidth(width)

  if KBM.state.status == "complete" then
    KBM.progressBar:SetVertexColor(0.26, 0.68, 0.28, 0.95)
  elseif KBM.state.status == "failed" then
    KBM.progressBar:SetVertexColor(0.72, 0.20, 0.20, 0.95)
  else
    KBM.progressBar:SetVertexColor(0.59, 0.33, 0.19, 0.95)
  end
end

local function ResetState()
  KBM.state = {
    themeKey = "",
    themeName = "-",
    title = "추가 임무",
    targetLabel = "-",
    currentCount = 0,
    targetCount = 0,
    remaining = 0,
    timeLimit = 0,
    status = "inactive",
    missionType = 0,
    briefing = "",
    announcement = "",
    expiresAt = nil,
  }
  Refresh()
end

local function ApplyState(parts)
  KBM.state.themeKey = parts[4] or ""
  KBM.state.themeName = parts[5] or "-"
  KBM.state.title = parts[6] or "추가 임무"
  KBM.state.targetLabel = parts[7] or "-"
  KBM.state.currentCount = tonumber(parts[8]) or 0
  KBM.state.targetCount = tonumber(parts[9]) or 0
  KBM.state.remaining = tonumber(parts[10]) or 0
  KBM.state.timeLimit = tonumber(parts[11]) or 0
  KBM.state.status = parts[12] or "inactive"
  KBM.state.missionType = tonumber(parts[13]) or 0
  KBM.state.expiresAt = GetTime() + (KBM.state.remaining or 0)
  Refresh()
  KBM:Show()
  KBM.reopen:Hide()
end

KBM:SetScript("OnUpdate", function(self, elapsed)
  self._acc = (self._acc or 0) + elapsed
  if self._acc < 0.2 then
    return
  end
  self._acc = 0

  if self.state.status ~= "active" or not self.state.expiresAt then
    return
  end

  local remaining = math.max(0, math.floor(self.state.expiresAt - GetTime()))
  self.state.remaining = remaining
  self.timer:SetText("남은 시간: " .. FormatRemaining(remaining))
end)

KBM:RegisterEvent("PLAYER_LOGIN")
KBM:RegisterEvent("CHAT_MSG_ADDON")
KBM:SetScript("OnEvent", function(self, event, prefix, message)
  if event == "PLAYER_LOGIN" then
    if RegisterAddonMessagePrefix then
      RegisterAddonMessagePrefix("KBM_UI")
    end

    SLASH_KARAZHANBONUSMISSION1 = "/kbm"
    SlashCmdList.KARAZHANBONUSMISSION = function()
      if self:IsShown() then
        self:Hide()
        self.reopen:Show()
      else
        self:Show()
        self.reopen:Hide()
      end
    end

    ResetState()
    return
  end

  if prefix ~= "KBM_UI" or type(message) ~= "string" then
    return
  end

  local parts = Split(message, "\t")
  local kind = parts[1]

  if kind == "CLEAR" then
    ResetState()
    self:Hide()
    self.reopen:Hide()
    return
  end

  if kind == "STATE" then
    ApplyState(parts)
    return
  end

  if kind == "BRIEFING" then
    self.state.briefing = parts[2] or ""
    Refresh()
    return
  end

  if kind == "ANNOUNCEMENT" then
    self.state.announcement = parts[2] or ""
    self.briefing:SetText(self.state.announcement)
    self:Show()
    self.reopen:Hide()
    return
  end

  if kind == "ALERT" then
    local alertMessage = parts[2] or ""
    self.state.announcement = alertMessage
    self.briefing:SetText(alertMessage)
    self:Show()
    self.reopen:Hide()
    ShowRaidAlert(alertMessage)
    return
  end
end)

Refresh()
