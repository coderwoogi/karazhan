local addonName = ...

local KBM = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent)
KBM:SetSize(344, 246)
KBM:SetPoint("TOPRIGHT", UIParent, "TOPRIGHT", -96, -168)
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
  inactive = {0.60, 0.60, 0.60},
  active = {0.22, 0.82, 1.00},
  complete = {0.22, 0.95, 0.42},
  failed = {1.00, 0.24, 0.24},
}

local themeColors = {
  slaughter = {0.86, 0.24, 0.24},
  clean_run = {0.22, 0.78, 0.36},
  speed_run = {0.95, 0.72, 0.20},
  boss_focus = {0.70, 0.42, 0.95},
}

local function CreateFont(parent, size, r, g, b, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", "GameFontNormal")
  fs:SetFont(STANDARD_TEXT_FONT, size, "OUTLINE")
  fs:SetTextColor(r or 1, g or 1, b or 1)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetJustifyV("TOP")
  return fs
end

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
    return "무사고형"
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

KBM:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background-Dark",
  edgeFile = "Interface\\AchievementFrame\\UI-Achievement-WoodBorder",
  tile = true,
  tileSize = 16,
  edgeSize = 16,
  insets = { left = 6, right = 6, top = 6, bottom = 6 },
})
KBM:SetBackdropColor(0.03, 0.03, 0.05, 0.95)

KBM.header = CreateFrame("Frame", nil, KBM)
KBM.header:SetPoint("TOPLEFT", KBM, "TOPLEFT", 14, -14)
KBM.header:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -14, -14)
KBM.header:SetHeight(36)
KBM.header:SetBackdrop({
  bgFile = "Interface\\AchievementFrame\\UI-Achievement-Parchment-Horizontal",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 32,
  edgeSize = 10,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.header:SetBackdropColor(0.20, 0.15, 0.07, 0.94)
KBM.header:SetBackdropBorderColor(0.70, 0.58, 0.22, 0.85)

KBM.headerTitle = CreateFont(KBM.header, 14, 1.0, 0.86, 0.22)
KBM.headerTitle:SetPoint("TOPLEFT", KBM.header, "TOPLEFT", 12, -8)
KBM.headerTitle:SetText("추가 임무")

KBM.headerSub = CreateFont(KBM.header, 9, 0.78, 0.74, 0.64)
KBM.headerSub:SetPoint("TOPLEFT", KBM.headerTitle, "BOTTOMLEFT", 0, -3)
KBM.headerSub:SetText("던전 추가 목표")

KBM.themeBadge = CreateFrame("Frame", nil, KBM.header)
KBM.themeBadge:SetSize(104, 20)
KBM.themeBadge:SetPoint("RIGHT", KBM.header, "RIGHT", -28, 0)
KBM.themeBadge:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})

KBM.themeBadgeText = CreateFont(KBM.themeBadge, 10, 1, 1, 1, "CENTER")
KBM.themeBadgeText:SetPoint("CENTER", KBM.themeBadge, "CENTER", 0, 0)
KBM.themeBadgeText:SetText("대기")

local close = CreateFrame("Button", nil, KBM, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -4, -4)
close:SetScript("OnClick", function()
  KBM:Hide()
  KBM.reopen:Show()
end)

KBM.reopen = CreateFrame("Button", "KarazhanBonusMissionReopenButton", UIParent, "UIPanelButtonTemplate")
KBM.reopen:SetSize(92, 22)
KBM.reopen:SetPoint("TOPRIGHT", UIParent, "TOPRIGHT", -98, -138)
KBM.reopen:SetText("임무 보기")
KBM.reopen:SetScript("OnClick", function()
  KBM:Show()
  KBM.reopen:Hide()
end)
KBM.reopen:Hide()

KBM.body = CreateFrame("Frame", nil, KBM)
KBM.body:SetPoint("TOPLEFT", KBM.header, "BOTTOMLEFT", 0, -10)
KBM.body:SetPoint("TOPRIGHT", KBM.header, "BOTTOMRIGHT", 0, -10)
KBM.body:SetHeight(154)
KBM.body:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.body:SetBackdropColor(0.05, 0.05, 0.07, 0.92)
KBM.body:SetBackdropBorderColor(0.32, 0.28, 0.20, 0.75)

KBM.currentLabel = CreateFont(KBM.body, 10, 1.0, 0.82, 0.15)
KBM.currentLabel:SetPoint("TOPLEFT", KBM.body, "TOPLEFT", 12, -10)
KBM.currentLabel:SetText("현재 임무")

KBM.status = CreateFont(KBM.body, 10, 0.60, 0.60, 0.60, "RIGHT")
KBM.status:SetPoint("TOPRIGHT", KBM.body, "TOPRIGHT", -12, -10)
KBM.status:SetText("상태: 대기")

KBM.mission = CreateFont(KBM.body, 13, 1.0, 1.0, 1.0)
KBM.mission:SetPoint("TOPLEFT", KBM.currentLabel, "BOTTOMLEFT", 0, -6)
KBM.mission:SetWidth(230)
KBM.mission:SetText("-")

KBM.timerBox = CreateFrame("Frame", nil, KBM.body)
KBM.timerBox:SetSize(124, 24)
KBM.timerBox:SetPoint("TOPLEFT", KBM.mission, "BOTTOMLEFT", 0, -10)
KBM.timerBox:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.timerBox:SetBackdropColor(0.10, 0.12, 0.18, 0.95)
KBM.timerBox:SetBackdropBorderColor(0.25, 0.45, 0.86, 0.86)

KBM.timer = CreateFont(KBM.timerBox, 11, 0.88, 0.94, 1.0, "CENTER")
KBM.timer:SetPoint("CENTER", KBM.timerBox, "CENTER", 0, 0)
KBM.timer:SetText("남은 시간 00:00")

KBM.progressLabel = CreateFont(KBM.body, 10, 1.0, 0.82, 0.15)
KBM.progressLabel:SetPoint("TOPLEFT", KBM.timerBox, "BOTTOMLEFT", 0, -12)
KBM.progressLabel:SetText("진행도")

KBM.progressText = CreateFont(KBM.body, 10, 0.94, 0.94, 0.94, "RIGHT")
KBM.progressText:SetPoint("RIGHT", KBM.body, "RIGHT", -12, -78)
KBM.progressText:SetText("0 / 0")

KBM.progressBarBG = CreateFrame("Frame", nil, KBM.body)
KBM.progressBarBG:SetPoint("TOPLEFT", KBM.progressLabel, "BOTTOMLEFT", 0, -6)
KBM.progressBarBG:SetSize(300, 16)
KBM.progressBarBG:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.progressBarBG:SetBackdropColor(0.03, 0.03, 0.03, 0.96)
KBM.progressBarBG:SetBackdropBorderColor(0.34, 0.34, 0.34, 0.76)

KBM.progressBar = KBM.progressBarBG:CreateTexture(nil, "ARTWORK")
KBM.progressBar:SetTexture("Interface\\TARGETINGFRAME\\UI-StatusBar")
KBM.progressBar:SetVertexColor(0.16, 0.76, 1.00, 0.96)
KBM.progressBar:SetPoint("TOPLEFT", KBM.progressBarBG, "TOPLEFT", 2, -2)
KBM.progressBar:SetPoint("BOTTOMLEFT", KBM.progressBarBG, "BOTTOMLEFT", 2, 2)
KBM.progressBar:SetWidth(1)

KBM.objective = CreateFont(KBM.body, 11, 0.92, 0.92, 0.92)
KBM.objective:SetPoint("TOPLEFT", KBM.progressBarBG, "BOTTOMLEFT", 0, -10)
KBM.objective:SetWidth(300)
KBM.objective:SetText("목표: -")

KBM.briefingTitle = CreateFont(KBM.body, 10, 1.0, 0.82, 0.15)
KBM.briefingTitle:SetPoint("TOPLEFT", KBM.objective, "BOTTOMLEFT", 0, -10)
KBM.briefingTitle:SetText("작전 브리핑")

KBM.briefing = CreateFont(KBM.body, 11, 0.86, 0.88, 0.94)
KBM.briefing:SetPoint("TOPLEFT", KBM.briefingTitle, "BOTTOMLEFT", 0, -4)
KBM.briefing:SetWidth(300)
KBM.briefing:SetText("추가 임무가 아직 배정되지 않았습니다.")

KBM.noticePanel = CreateFrame("Frame", nil, KBM)
KBM.noticePanel:SetPoint("TOPLEFT", KBM.body, "BOTTOMLEFT", 0, -10)
KBM.noticePanel:SetPoint("TOPRIGHT", KBM.body, "BOTTOMRIGHT", 0, -10)
KBM.noticePanel:SetHeight(32)
KBM.noticePanel:SetBackdrop({
  bgFile = "Interface\\Buttons\\WHITE8x8",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 8,
  edgeSize = 8,
  insets = { left = 2, right = 2, top = 2, bottom = 2 },
})
KBM.noticePanel:SetBackdropColor(0.09, 0.08, 0.04, 0.94)
KBM.noticePanel:SetBackdropBorderColor(0.42, 0.34, 0.16, 0.76)

KBM.notice = CreateFont(KBM.noticePanel, 10, 1.0, 0.86, 0.20)
KBM.notice:SetPoint("TOPLEFT", KBM.noticePanel, "TOPLEFT", 10, -8)
KBM.notice:SetWidth(292)
KBM.notice:SetText("")

local function UpdateThemeVisuals()
  local color = themeColors[KBM.state.themeKey] or {0.45, 0.45, 0.45}
  local text = GetThemeText(KBM.state.themeKey, KBM.state.themeName)
  KBM.themeBadge:SetBackdropColor(color[1] * 0.24, color[2] * 0.24, color[3] * 0.24, 0.96)
  KBM.themeBadge:SetBackdropBorderColor(color[1], color[2], color[3], 0.96)
  KBM.themeBadgeText:SetText(text)
  KBM.themeBadgeText:SetTextColor(color[1], color[2], color[3])
end

local function Refresh()
  UpdateThemeVisuals()
  KBM.mission:SetText(KBM.state.title or "-")
  KBM.objective:SetText(string.format("목표: %s", KBM.state.targetLabel or "-"))
  KBM.progressText:SetText(string.format("%d / %d", KBM.state.currentCount or 0, KBM.state.targetCount or 0))
  KBM.timer:SetText("남은 시간 " .. FormatRemaining(KBM.state.remaining or 0))
  KBM.status:SetText("상태: " .. GetStatusText(KBM.state.status))
  KBM.briefing:SetText(KBM.state.briefing or "")

  local statusColor = statusColors[KBM.state.status] or statusColors.inactive
  KBM.status:SetTextColor(statusColor[1], statusColor[2], statusColor[3])

  local maxCount = KBM.state.targetCount or 0
  local current = KBM.state.currentCount or 0
  local width = 1
  if maxCount > 0 then
    width = math.floor(296 * math.min(current / maxCount, 1))
    if width < 1 then
      width = 1
    end
  end
  KBM.progressBar:SetWidth(width)

  if KBM.state.status == "complete" then
    KBM.progressBar:SetVertexColor(0.20, 0.92, 0.42, 0.96)
  elseif KBM.state.status == "failed" then
    KBM.progressBar:SetVertexColor(0.95, 0.22, 0.24, 0.96)
  else
    KBM.progressBar:SetVertexColor(0.16, 0.76, 1.00, 0.96)
  end
end

local function ResetState()
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
  KBM.notice:SetText("")
  Refresh()
end

local function ApplyState(parts)
  KBM.state.themeKey = parts[4] or ""
  KBM.state.themeName = parts[5] or "-"
  KBM.state.title = parts[6] or "-"
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

  if self.state.status ~= "active" then
    return
  end

  if self.state.expiresAt then
    local remaining = math.max(0, math.floor(self.state.expiresAt - GetTime()))
    self.state.remaining = remaining
    self.timer:SetText("남은 시간 " .. FormatRemaining(remaining))
  end
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

  local parts = Split(message, "	")
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
    self.briefing:SetText(self.state.briefing)
    return
  end

  if kind == "ANNOUNCEMENT" then
    self.state.announcement = parts[2] or ""
    self.notice:SetText(self.state.announcement)
    self:Show()
    self.reopen:Hide()
    return
  end

  if kind == "ALERT" then
    local alertMessage = parts[2] or ""
    self.notice:SetText(alertMessage)
    self:Show()
    self.reopen:Hide()
    ShowRaidAlert(alertMessage)
    return
  end
end)

Refresh()
