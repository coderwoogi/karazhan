local addonName = ...

local KBM = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent)
KBM:SetSize(438, 548)
KBM:SetPoint("CENTER", UIParent, "CENTER", 180, 0)
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

local statusColors = {
  inactive = {0.42, 0.38, 0.30},
  active = {0.45, 0.28, 0.12},
  complete = {0.17, 0.51, 0.21},
  failed = {0.72, 0.19, 0.17},
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

local function CreateText(parent, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", "GameFontNormal")
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

KBM.bg = CreateFrame("Frame", nil, KBM)
KBM.bg:SetAllPoints(KBM)
KBM.bg:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 16,
  edgeSize = 14,
  insets = { left = 4, right = 4, top = 4, bottom = 4 },
})
KBM.bg:SetBackdropColor(0.93, 0.82, 0.63, 0.98)
KBM.bg:SetBackdropBorderColor(0.53, 0.37, 0.20, 0.95)

KBM.inner = KBM:CreateTexture(nil, "BACKGROUND")
KBM.inner:SetTexture("Interface\\Buttons\\WHITE8x8")
KBM.inner:SetVertexColor(0.90, 0.79, 0.60, 0.96)
KBM.inner:SetPoint("TOPLEFT", KBM, "TOPLEFT", 14, -16)
KBM.inner:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -14, 18)

KBM.topShade = KBM:CreateTexture(nil, "BORDER")
KBM.topShade:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.topShade:SetPoint("TOPLEFT", KBM, "TOPLEFT", 18, -12)
KBM.topShade:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -18, -12)
KBM.topShade:SetHeight(28)
KBM.topShade:SetAlpha(0.55)

KBM.bottomShade = KBM:CreateTexture(nil, "BORDER")
KBM.bottomShade:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.bottomShade:SetPoint("BOTTOMLEFT", KBM, "BOTTOMLEFT", 18, 18)
KBM.bottomShade:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -18, 18)
KBM.bottomShade:SetHeight(18)
KBM.bottomShade:SetAlpha(0.40)

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
KBM.reopen:SetPoint("CENTER", UIParent, "CENTER", 390, -248)
KBM.reopen:SetText("임무 보기")
KBM.reopen:SetScript("OnClick", function()
  KBM:Show()
  KBM.reopen:Hide()
end)
KBM.reopen:Hide()

KBM.iconBorder = CreateFrame("Frame", nil, KBM)
KBM.iconBorder:SetSize(52, 52)
KBM.iconBorder:SetPoint("TOPLEFT", KBM, "TOPLEFT", 28, -34)
KBM.iconBorder:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 10,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.iconBorder:SetBackdropColor(0.28, 0.21, 0.12, 0.95)
KBM.iconBorder:SetBackdropBorderColor(0.61, 0.46, 0.24, 0.95)

KBM.icon = KBM.iconBorder:CreateTexture(nil, "ARTWORK")
KBM.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
KBM.icon:SetPoint("TOPLEFT", KBM.iconBorder, "TOPLEFT", 6, -6)
KBM.icon:SetPoint("BOTTOMRIGHT", KBM.iconBorder, "BOTTOMRIGHT", -6, 6)

KBM.title = CreateText(KBM, 26, 0.20, 0.14, 0.09)
KBM.title:SetPoint("TOPLEFT", KBM.iconBorder, "TOPRIGHT", 14, -2)
KBM.title:SetWidth(300)
KBM.title:SetText("추가 임무")

KBM.titleLine = KBM:CreateTexture(nil, "ARTWORK")
KBM.titleLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.titleLine:SetPoint("TOPLEFT", KBM.title, "BOTTOMLEFT", 0, -12)
KBM.titleLine:SetPoint("RIGHT", KBM, "RIGHT", -40, 0)
KBM.titleLine:SetHeight(8)
KBM.titleLine:SetTexCoord(0, 1, 0, 0.5)
KBM.titleLine:SetAlpha(0.80)

KBM.theme = CreateText(KBM, 15, 0.36, 0.24, 0.12)
KBM.theme:SetPoint("TOPLEFT", KBM.titleLine, "BOTTOMLEFT", 0, -10)
KBM.theme:SetWidth(240)
KBM.theme:SetText("테마: 미지정")

KBM.status = CreateText(KBM, 15, 0.36, 0.24, 0.12, "RIGHT")
KBM.status:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -40, -110)
KBM.status:SetWidth(120)
KBM.status:SetText("상태: 대기")

KBM.briefing = CreateText(KBM, 17, 0.22, 0.16, 0.10)
KBM.briefing:SetPoint("TOPLEFT", KBM.theme, "BOTTOMLEFT", 0, -22)
KBM.briefing:SetWidth(352)
KBM.briefing:SetSpacing(8)
KBM.briefing:SetText("추가 임무가 아직 배정되지 않았습니다.")

KBM.objectivesTitle = CreateText(KBM, 18, 0.28, 0.18, 0.08)
KBM.objectivesTitle:SetPoint("TOPLEFT", KBM.briefing, "BOTTOMLEFT", 0, -24)
KBM.objectivesTitle:SetText("목표")

KBM.objectives = CreateText(KBM, 17, 0.22, 0.16, 0.10)
KBM.objectives:SetPoint("TOPLEFT", KBM.objectivesTitle, "BOTTOMLEFT", 0, -10)
KBM.objectives:SetWidth(352)
KBM.objectives:SetSpacing(6)
KBM.objectives:SetText("아직 목표가 없습니다.")

KBM.timer = CreateText(KBM, 15, 0.36, 0.24, 0.12)
KBM.timer:SetPoint("TOPLEFT", KBM.objectives, "BOTTOMLEFT", 0, -14)
KBM.timer:SetWidth(160)
KBM.timer:SetText("남은 시간: 00:00")

KBM.progress = CreateText(KBM, 15, 0.36, 0.24, 0.12, "RIGHT")
KBM.progress:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -40, -340)
KBM.progress:SetWidth(160)
KBM.progress:SetText("진행도: 0 / 0")

KBM.progressBarBG = CreateFrame("Frame", nil, KBM)
KBM.progressBarBG:SetPoint("TOPLEFT", KBM.timer, "BOTTOMLEFT", 0, -10)
KBM.progressBarBG:SetSize(350, 18)
KBM.progressBarBG:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.progressBarBG:SetBackdropColor(0.18, 0.11, 0.06, 0.90)
KBM.progressBarBG:SetBackdropBorderColor(0.52, 0.36, 0.20, 0.90)

KBM.progressBar = KBM.progressBarBG:CreateTexture(nil, "ARTWORK")
KBM.progressBar:SetTexture("Interface\\TARGETINGFRAME\\UI-StatusBar")
KBM.progressBar:SetVertexColor(0.58, 0.31, 0.15, 0.96)
KBM.progressBar:SetPoint("TOPLEFT", KBM.progressBarBG, "TOPLEFT", 2, -2)
KBM.progressBar:SetPoint("BOTTOMLEFT", KBM.progressBarBG, "BOTTOMLEFT", 2, 2)
KBM.progressBar:SetWidth(1)

KBM.rewardsTitle = CreateText(KBM, 18, 0.28, 0.18, 0.08)
KBM.rewardsTitle:SetPoint("TOPLEFT", KBM.progressBarBG, "BOTTOMLEFT", 0, -22)
KBM.rewardsTitle:SetText("보상")

KBM.rewardLeftIcon = KBM:CreateTexture(nil, "ARTWORK")
KBM.rewardLeftIcon:SetTexture("Interface\\Icons\\INV_Misc_Coin_01")
KBM.rewardLeftIcon:SetSize(36, 36)
KBM.rewardLeftIcon:SetPoint("TOPLEFT", KBM.rewardsTitle, "BOTTOMLEFT", 0, -12)

KBM.rewardLeftText = CreateText(KBM, 15, 0.22, 0.16, 0.10)
KBM.rewardLeftText:SetPoint("LEFT", KBM.rewardLeftIcon, "RIGHT", 10, 0)
KBM.rewardLeftText:SetWidth(140)
KBM.rewardLeftText:SetJustifyV("MIDDLE")
KBM.rewardLeftText:SetText("추가 보상")

KBM.rewardRightIcon = KBM:CreateTexture(nil, "ARTWORK")
KBM.rewardRightIcon:SetTexture("Interface\\Icons\\INV_Chest_Cloth_17")
KBM.rewardRightIcon:SetSize(36, 36)
KBM.rewardRightIcon:SetPoint("TOPLEFT", KBM.rewardLeftIcon, "TOPLEFT", 186, 0)

KBM.rewardRightText = CreateText(KBM, 15, 0.22, 0.16, 0.10)
KBM.rewardRightText:SetPoint("LEFT", KBM.rewardRightIcon, "RIGHT", 10, 0)
KBM.rewardRightText:SetWidth(140)
KBM.rewardRightText:SetJustifyV("MIDDLE")
KBM.rewardRightText:SetText("임무 상자")

KBM.footerLine = KBM:CreateTexture(nil, "ARTWORK")
KBM.footerLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.footerLine:SetPoint("TOPLEFT", KBM.rewardLeftIcon, "BOTTOMLEFT", -2, -18)
KBM.footerLine:SetPoint("RIGHT", KBM, "RIGHT", -42, 0)
KBM.footerLine:SetHeight(8)
KBM.footerLine:SetTexCoord(0, 1, 0, 0.5)
KBM.footerLine:SetAlpha(0.80)

KBM.accept = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.accept:SetSize(160, 34)
KBM.accept:SetPoint("BOTTOMLEFT", KBM, "BOTTOMLEFT", 34, 22)
KBM.accept:SetText("확인")
KBM.accept:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.decline = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.decline:SetSize(160, 34)
KBM.decline:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -34, 22)
KBM.decline:SetText("접기")
KBM.decline:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

local function UpdateThemeVisuals()
  KBM.theme:SetText("테마: " .. GetThemeText(KBM.state.themeKey, KBM.state.themeName))
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
    width = math.floor(346 * math.min(current / maxCount, 1))
    if width < 1 then
      width = 1
    end
  end
  KBM.progressBar:SetWidth(width)

  if KBM.state.status == "complete" then
    KBM.progressBar:SetVertexColor(0.24, 0.62, 0.24, 0.96)
  elseif KBM.state.status == "failed" then
    KBM.progressBar:SetVertexColor(0.70, 0.18, 0.18, 0.96)
  else
    KBM.progressBar:SetVertexColor(0.58, 0.31, 0.15, 0.96)
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
