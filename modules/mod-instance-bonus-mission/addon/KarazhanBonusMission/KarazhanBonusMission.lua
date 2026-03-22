local KBM = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent)
KBM:Hide()
KBM:SetSize(320, 210)
KBM:SetPoint("CENTER", UIParent, "CENTER", 0, 120)
KBM:SetBackdrop({
  bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
  edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
  tile = true,
  tileSize = 16,
  edgeSize = 16,
  insets = { left = 4, right = 4, top = 4, bottom = 4 },
})
KBM:SetBackdropColor(0, 0, 0, 0.85)
KBM:EnableMouse(true)
KBM:SetMovable(true)
KBM:RegisterForDrag("LeftButton")
KBM:SetScript("OnDragStart", KBM.StartMoving)
KBM:SetScript("OnDragStop", KBM.StopMovingOrSizing)

KBM.state = {
  themeName = "-",
  title = "-",
  targetLabel = "-",
  currentCount = 0,
  targetCount = 0,
  remaining = 0,
  timeLimit = 0,
  status = "inactive",
  briefing = "",
  announcement = "",
}

local function CreateText(parent, size, anchor, rel, x, y, justify)
  local fs = parent:CreateFontString(nil, "OVERLAY", "GameFontNormal")
  fs:SetFont(STANDARD_TEXT_FONT, size, "OUTLINE")
  fs:SetPoint(anchor, rel, anchor, x, y)
  fs:SetJustifyH(justify or "LEFT")
  fs:SetWidth(280)
  return fs
end

KBM.title = CreateText(KBM, 14, "TOPLEFT", KBM, 16, -16, "LEFT")
KBM.title:SetText("Bonus Mission")
KBM.theme = CreateText(KBM, 12, "TOPLEFT", KBM.title, 0, -24, "LEFT")
KBM.mission = CreateText(KBM, 12, "TOPLEFT", KBM.theme, 0, -20, "LEFT")
KBM.progress = CreateText(KBM, 12, "TOPLEFT", KBM.mission, 0, -20, "LEFT")
KBM.timer = CreateText(KBM, 12, "TOPLEFT", KBM.progress, 0, -20, "LEFT")
KBM.status = CreateText(KBM, 12, "TOPLEFT", KBM.timer, 0, -20, "LEFT")
KBM.briefing = CreateText(KBM, 11, "TOPLEFT", KBM.status, 0, -26, "LEFT")
KBM.briefing:SetJustifyV("TOP")
KBM.notice = CreateText(KBM, 11, "BOTTOMLEFT", KBM, 16, 18, "LEFT")
KBM.notice:SetTextColor(1, 0.82, 0)

local close = CreateFrame("Button", nil, KBM, "UIPanelCloseButton")
close:SetPoint("TOPRIGHT", KBM, "TOPRIGHT", -6, -6)

local function Split(input, sep)
  local parts = {}
  local pattern = string.format("([^%s]+)", sep)
  for part in string.gmatch(input, pattern) do
    table.insert(parts, part)
  end
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
    return "Complete"
  end
  if status == "failed" then
    return "Failed"
  end
  if status == "active" then
    return "Active"
  end
  return "Idle"
end

local function Refresh()
  KBM.theme:SetText("Theme: " .. (KBM.state.themeName or "-"))
  KBM.mission:SetText("Mission: " .. (KBM.state.title or "-"))
  KBM.progress:SetText(string.format(
    "Progress: %d / %d",
    KBM.state.currentCount or 0,
    KBM.state.targetCount or 0
  ))
  KBM.timer:SetText("Time Left: " .. FormatRemaining(KBM.state.remaining or 0))
  KBM.status:SetText("Status: " .. GetStatusText(KBM.state.status))
  KBM.briefing:SetText(KBM.state.briefing or "")
end

local function ApplyState(parts)
  KBM.state.themeName = parts[5] or "-"
  KBM.state.title = parts[6] or "-"
  KBM.state.targetLabel = parts[7] or "-"
  KBM.state.currentCount = tonumber(parts[8]) or 0
  KBM.state.targetCount = tonumber(parts[9]) or 0
  KBM.state.remaining = tonumber(parts[10]) or 0
  KBM.state.timeLimit = tonumber(parts[11]) or 0
  KBM.state.status = parts[12] or "inactive"
  KBM.state.expiresAt = GetTime() + (KBM.state.remaining or 0)
  Refresh()
  KBM:Show()
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
    self.timer:SetText("Time Left: " .. FormatRemaining(remaining))
  end
end)

KBM:RegisterEvent("PLAYER_LOGIN")
KBM:RegisterEvent("CHAT_MSG_ADDON")
KBM:SetScript("OnEvent", function(self, event, prefix, message)
  if event == "PLAYER_LOGIN" then
    SLASH_KARAZHANBONUSMISSION1 = "/kbm"
    SlashCmdList.KARAZHANBONUSMISSION = function()
      if self:IsShown() then
        self:Hide()
      else
        self:Show()
      end
    end
    Refresh()
    return
  end

  if prefix ~= "KBM_UI" or type(message) ~= "string" then
    return
  end

  local parts = Split(message, "\t")
  local kind = parts[1]

  if kind == "CLEAR" then
    self.state = {
      themeName = "-",
      title = "-",
      targetLabel = "-",
      currentCount = 0,
      targetCount = 0,
      remaining = 0,
      timeLimit = 0,
      status = "inactive",
      briefing = "",
      announcement = "",
    }
    self.notice:SetText("")
    Refresh()
    self:Hide()
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
    return
  end
end)