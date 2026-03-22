local addonName = ...

local KBM = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent)
KBM:SetSize(478, 610)
KBM:SetPoint("CENTER", UIParent, "CENTER", 170, -10)
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
  inactive = {0.46, 0.36, 0.22},
  active = {0.56, 0.27, 0.08},
  complete = {0.17, 0.53, 0.21},
  failed = {0.71, 0.18, 0.17},
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
end

local function CreateText(parent, layer, font, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, layer or "OVERLAY", font)
  fs:SetFont(STANDARD_TEXT_FONT, size, "")
  fs:SetTextColor(r, g, b)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

local function CreateSectionHeader(parent, text, anchor, offsetY)
  local texture = parent:CreateTexture(nil, "BORDER")
  texture:SetTexture("Interface\\Buttons\\WHITE8x8")
  texture:SetPoint("TOPLEFT", anchor, "BOTTOMLEFT", -4, offsetY)
  texture:SetSize(390, 24)
  texture:SetVertexColor(0.76, 0.66, 0.46, 0.28)

  local label = CreateText(parent, "ARTWORK", "GameFontNormal", 16, 0.26, 0.17, 0.08)
  label:SetPoint("LEFT", texture, "LEFT", 10, 0)
  label:SetText(text)

  return texture, label
end

KBM:SetBackdrop({
  bgFile = "Interface\\QuestFrame\\UI-QuestLogDualPane-Left",
  edgeFile = nil,
  tile = false,
  edgeSize = 0,
  insets = { left = 5, right = 5, top = 5, bottom = 5 },
})
KBM:SetBackdropColor(1, 1, 1, 1)

KBM.headerLine = KBM:CreateTexture(nil, "ARTWORK")
KBM.headerLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.headerLine:SetPoint("TOPLEFT", KBM, "TOPLEFT", 92, -82)
KBM.headerLine:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -38, -82)
KBM.headerLine:SetHeight(8)
KBM.headerLine:SetTexCoord(0, 1, 0, 0.5)
KBM.headerLine:SetAlpha(0.95)

local close = CreateFrame("Button", nil, KBM, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -10, -10)
close:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.reopen = CreateFrame("Button", "KarazhanBonusMissionReopenButton", UIParent, "UIPanelButtonTemplate")
KBM.reopen:SetSize(94, 24)
KBM.reopen:SetPoint("CENTER", UIParent, "CENTER", 388, -248)
KBM.reopen:SetText("임무 보기")
KBM.reopen:SetScript("OnClick", function()
  KBM:Show()
  KBM.reopen:Hide()
end)
KBM.reopen:Hide()

KBM.iconBorder = CreateFrame("Frame", nil, KBM)
KBM.iconBorder:SetSize(58, 58)
KBM.iconBorder:SetPoint("TOPLEFT", KBM, "TOPLEFT", 24, -48)
KBM.iconBorder:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Buttons\\UI-Quickslot2",
  tile = false,
  edgeSize = 12,
  insets = { left = 3, right = 3, top = 3, bottom = 3 },
})
KBM.iconBorder:SetBackdropColor(0.20, 0.14, 0.08, 0.92)
KBM.iconBorder:SetBackdropBorderColor(1, 1, 1, 1)

KBM.icon = KBM.iconBorder:CreateTexture(nil, "ARTWORK")
KBM.icon:SetTexture("Interface\\Icons\\INV_Misc_QuestionMark")
KBM.icon:SetPoint("TOPLEFT", KBM.iconBorder, "TOPLEFT", 7, -7)
KBM.icon:SetPoint("BOTTOMRIGHT", KBM.iconBorder, "BOTTOMRIGHT", -7, 7)

KBM.title = CreateText(KBM, "OVERLAY", "GameFontHighlightLarge", 28, 0.24, 0.18, 0.10)
KBM.title:SetPoint("TOPLEFT", KBM.iconBorder, "TOPRIGHT", 14, -2)
KBM.title:SetPoint("RIGHT", KBM, "RIGHT", -44, 0)
KBM.title:SetText("추가 임무")

KBM.themeTag = CreateFrame("Frame", nil, KBM)
KBM.themeTag:SetSize(132, 28)
KBM.themeTag:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -40, -106)
KBM.themeTag:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = false,
  edgeSize = 10,
  insets = { left = 3, right = 3, top = 3, bottom = 3 },
})
KBM.themeTag:SetBackdropColor(0.33, 0.21, 0.11, 0.80)
KBM.themeTag:SetBackdropBorderColor(0.55, 0.40, 0.22, 0.90)

KBM.theme = CreateText(KBM.themeTag, "OVERLAY", "GameFontNormal", 14, 0.97, 0.86, 0.64, "CENTER")
KBM.theme:SetPoint("CENTER", KBM.themeTag, "CENTER", 0, 0)
KBM.theme:SetText("미지정")

KBM.body = CreateFrame("Frame", nil, KBM)
KBM.body:SetPoint("TOPLEFT", KBM, "TOPLEFT", 26, -132)
KBM.body:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -26, 94)

KBM.status = CreateText(KBM.body, "OVERLAY", "GameFontHighlight", 15, 0.55, 0.20, 0.14, "RIGHT")
KBM.status:SetPoint("TOPRIGHT", KBM.body, "TOPRIGHT", -6, 0)
KBM.status:SetWidth(150)
KBM.status:SetText("상태: 대기")

KBM.briefing = CreateText(KBM.body, "OVERLAY", "GameFontNormal", 17, 0.22, 0.16, 0.10)
KBM.briefing:SetPoint("TOPLEFT", KBM.body, "TOPLEFT", 8, -8)
KBM.briefing:SetWidth(404)
KBM.briefing:SetSpacing(7)
KBM.briefing:SetText("추가 임무가 아직 배정되지 않았습니다.")

KBM.objectiveHeaderTexture, KBM.objectivesTitle = CreateSectionHeader(KBM.body, "목표", KBM.briefing, -84)

KBM.objectives = CreateText(KBM.body, "OVERLAY", "GameFontNormal", 16, 0.22, 0.16, 0.10)
KBM.objectives:SetPoint("TOPLEFT", KBM.objectiveHeaderTexture, "BOTTOMLEFT", 8, -10)
KBM.objectives:SetWidth(404)
KBM.objectives:SetText("아직 목표가 없습니다.")

KBM.timer = CreateText(KBM.body, "OVERLAY", "GameFontHighlight", 15, 0.32, 0.21, 0.10)
KBM.timer:SetPoint("TOPLEFT", KBM.objectives, "BOTTOMLEFT", 0, -18)
KBM.timer:SetText("남은 시간 00:00")

KBM.progress = CreateText(KBM.body, "OVERLAY", "GameFontHighlight", 15, 0.32, 0.21, 0.10, "RIGHT")
KBM.progress:SetPoint("TOPRIGHT", KBM.body, "TOPRIGHT", -6, -182)
KBM.progress:SetWidth(150)
KBM.progress:SetText("진행도 0 / 0")

KBM.progressBarBG = CreateFrame("Frame", nil, KBM.body)
KBM.progressBarBG:SetPoint("TOPLEFT", KBM.timer, "BOTTOMLEFT", 0, -10)
KBM.progressBarBG:SetSize(404, 20)
KBM.progressBarBG:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = false,
  edgeSize = 10,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.progressBarBG:SetBackdropColor(0.17, 0.11, 0.07, 0.80)
KBM.progressBarBG:SetBackdropBorderColor(0.43, 0.28, 0.12, 0.90)

KBM.progressBar = KBM.progressBarBG:CreateTexture(nil, "ARTWORK")
KBM.progressBar:SetTexture("Interface\\TargetingFrame\\UI-StatusBar")
KBM.progressBar:SetPoint("TOPLEFT", KBM.progressBarBG, "TOPLEFT", 2, -2)
KBM.progressBar:SetPoint("BOTTOMLEFT", KBM.progressBarBG, "BOTTOMLEFT", 2, 2)
KBM.progressBar:SetWidth(1)

KBM.rewardHeaderTexture, KBM.rewardsTitle = CreateSectionHeader(KBM.body, "보상", KBM.progressBarBG, -26)

KBM.rewardLeft = CreateFrame("Frame", nil, KBM.body)
KBM.rewardLeft:SetSize(176, 48)
KBM.rewardLeft:SetPoint("TOPLEFT", KBM.rewardHeaderTexture, "BOTTOMLEFT", 0, -12)

KBM.rewardLeftHighlight = KBM.rewardLeft:CreateTexture(nil, "BACKGROUND")
KBM.rewardLeftHighlight:SetTexture("Interface\\Buttons\\WHITE8x8")
KBM.rewardLeftHighlight:SetAllPoints(KBM.rewardLeft)
KBM.rewardLeftHighlight:SetVertexColor(0.85, 0.77, 0.58, 0.18)

KBM.rewardLeftBorder = KBM.rewardLeft:CreateTexture(nil, "BORDER")
KBM.rewardLeftBorder:SetTexture("Interface\\Buttons\\UI-Quickslot2")
KBM.rewardLeftBorder:SetPoint("TOPLEFT", KBM.rewardLeft, "TOPLEFT", 2, -2)
KBM.rewardLeftBorder:SetSize(42, 42)

KBM.rewardLeftIcon = KBM.rewardLeft:CreateTexture(nil, "ARTWORK")
KBM.rewardLeftIcon:SetTexture("Interface\\Icons\\INV_Misc_Coin_01")
KBM.rewardLeftIcon:SetPoint("TOPLEFT", KBM.rewardLeft, "TOPLEFT", 6, -6)
KBM.rewardLeftIcon:SetSize(34, 34)

KBM.rewardLeftText = CreateText(KBM.rewardLeft, "OVERLAY", "GameFontNormal", 15, 0.22, 0.16, 0.10)
KBM.rewardLeftText:SetPoint("LEFT", KBM.rewardLeftIcon, "RIGHT", 12, 0)
KBM.rewardLeftText:SetWidth(118)
KBM.rewardLeftText:SetJustifyV("MIDDLE")
KBM.rewardLeftText:SetText("기본 보상")

KBM.rewardRight = CreateFrame("Frame", nil, KBM.body)
KBM.rewardRight:SetSize(176, 48)
KBM.rewardRight:SetPoint("TOPRIGHT", KBM.body, "TOPRIGHT", -10, -316)

KBM.rewardRightHighlight = KBM.rewardRight:CreateTexture(nil, "BACKGROUND")
KBM.rewardRightHighlight:SetTexture("Interface\\Buttons\\WHITE8x8")
KBM.rewardRightHighlight:SetAllPoints(KBM.rewardRight)
KBM.rewardRightHighlight:SetVertexColor(0.85, 0.77, 0.58, 0.18)

KBM.rewardRightBorder = KBM.rewardRight:CreateTexture(nil, "BORDER")
KBM.rewardRightBorder:SetTexture("Interface\\Buttons\\UI-Quickslot2")
KBM.rewardRightBorder:SetPoint("TOPLEFT", KBM.rewardRight, "TOPLEFT", 2, -2)
KBM.rewardRightBorder:SetSize(42, 42)

KBM.rewardRightIcon = KBM.rewardRight:CreateTexture(nil, "ARTWORK")
KBM.rewardRightIcon:SetTexture("Interface\\Icons\\INV_Chest_Cloth_17")
KBM.rewardRightIcon:SetPoint("TOPLEFT", KBM.rewardRight, "TOPLEFT", 6, -6)
KBM.rewardRightIcon:SetSize(34, 34)

KBM.rewardRightText = CreateText(KBM.rewardRight, "OVERLAY", "GameFontNormal", 15, 0.22, 0.16, 0.10)
KBM.rewardRightText:SetPoint("LEFT", KBM.rewardRightIcon, "RIGHT", 12, 0)
KBM.rewardRightText:SetWidth(118)
KBM.rewardRightText:SetJustifyV("MIDDLE")
KBM.rewardRightText:SetText("추가 보상")

KBM.noticeLine = KBM.body:CreateTexture(nil, "ARTWORK")
KBM.noticeLine:SetTexture("Interface\\QuestFrame\\UI-QuestTitleHighlight")
KBM.noticeLine:SetPoint("TOPLEFT", KBM.rewardLeft, "BOTTOMLEFT", -4, -16)
KBM.noticeLine:SetPoint("TOPRIGHT", KBM.rewardRight, "BOTTOMRIGHT", 4, -16)
KBM.noticeLine:SetHeight(8)
KBM.noticeLine:SetTexCoord(0, 1, 0, 0.5)
KBM.noticeLine:SetAlpha(0.85)

KBM.notice = CreateText(KBM.body, "OVERLAY", "GameFontHighlight", 14, 0.36, 0.24, 0.12)
KBM.notice:SetPoint("TOPLEFT", KBM.noticeLine, "BOTTOMLEFT", 6, -8)
KBM.notice:SetWidth(404)
KBM.notice:SetSpacing(4)
KBM.notice:SetText("")

KBM.accept = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.accept:SetSize(172, 34)
KBM.accept:SetPoint("BOTTOMLEFT", KBM, "BOTTOMLEFT", 32, 46)
KBM.accept:SetText("확인")
KBM.accept:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.fold = CreateFrame("Button", nil, KBM, "UIPanelButtonTemplate")
KBM.fold:SetSize(172, 34)
KBM.fold:SetPoint("BOTTOMRIGHT", KBM, "BOTTOMRIGHT", -32, 46)
KBM.fold:SetText("접기")
KBM.fold:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

local function UpdateThemeVisuals()
  KBM.theme:SetText(GetThemeText(KBM.state.themeKey, KBM.state.themeName))
  KBM.icon:SetTexture(themeIcons[KBM.state.themeKey] or "Interface\\Icons\\INV_Misc_QuestionMark")
end

local function UpdateRewards()
  if KBM.state.status == "complete" then
    KBM.rewardLeftText:SetText("기본 보상 획득")
    KBM.rewardRightText:SetText("추가 보상 지급")
  elseif KBM.state.status == "failed" then
    KBM.rewardLeftText:SetText("기본 보상 유지")
    KBM.rewardRightText:SetText("추가 보상 없음")
  else
    KBM.rewardLeftText:SetText("기본 보상 대기")
    KBM.rewardRightText:SetText("추가 보상 도전")
  end
end

local function Refresh()
  UpdateThemeVisuals()
  UpdateRewards()

  KBM.title:SetText(KBM.state.title or "추가 임무")

  local statusText = "상태: " .. GetStatusText(KBM.state.status)
  KBM.status:SetText(statusText)

  local statusColor = statusColors[KBM.state.status] or statusColors.inactive
  KBM.status:SetTextColor(statusColor[1], statusColor[2], statusColor[3])

  if KBM.state.briefing ~= "" then
    KBM.briefing:SetText(KBM.state.briefing)
  else
    KBM.briefing:SetText("작전 브리핑이 아직 전달되지 않았습니다.")
  end

  KBM.objectives:SetText(string.format("%s %d / %d", KBM.state.targetLabel or "-", KBM.state.currentCount or 0, KBM.state.targetCount or 0))

  KBM.timer:SetText("남은 시간 " .. FormatRemaining(KBM.state.remaining or 0))
  KBM.progress:SetText(string.format("진행도 %d / %d", KBM.state.currentCount or 0, KBM.state.targetCount or 0))

  local maxCount = KBM.state.targetCount or 0
  local current = KBM.state.currentCount or 0
  local width = 1
  if maxCount > 0 then
    width = math.floor(400 * math.min(current / maxCount, 1))
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
    KBM.progressBar:SetVertexColor(0.64, 0.42, 0.10, 0.96)
  end

  KBM.notice:SetText(KBM.state.announcement or "")
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
  self.timer:SetText("남은 시간 " .. FormatRemaining(remaining))
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
    Refresh()
    self:Show()
    self.reopen:Hide()
    return
  end

  if kind == "ALERT" then
    local alertMessage = parts[2] or ""
    self.state.announcement = alertMessage
    Refresh()
    self:Show()
    self.reopen:Hide()
    ShowRaidAlert(alertMessage)
    return
  end
end)

Refresh()
