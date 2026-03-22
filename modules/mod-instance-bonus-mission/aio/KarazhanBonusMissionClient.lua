local AIO = AIO or require("AIO")

if AIO.AddAddon() then
    return
end

local BonusMissionHandlers = AIO.AddHandlers("KarazhanBonusMission", {})
local frame = CreateFrame("Frame", "KarazhanBonusMissionFrame", UIParent, "UIPanelDialogTemplate")
frame:SetSize(360, 240)
frame:SetPoint("CENTER", UIParent, "CENTER", 0, 140)
frame:Hide()
frame:SetMovable(true)
frame:EnableMouse(true)
frame:RegisterForDrag("LeftButton")
frame:SetScript("OnDragStart", frame.StartMoving)
frame:SetScript("OnDragStop", frame.StopMovingOrSizing)
AIO.SavePosition(frame, true)

local title = frame:CreateFontString(nil, "OVERLAY", "GameFontNormalLarge")
title:SetPoint("TOP", frame, "TOP", 0, -14)
title:SetText("추가 임무")

local function CreateLabel(anchor, offsetY, text)
    local label = frame:CreateFontString(nil, "OVERLAY", "GameFontNormal")
    label:SetPoint("TOPLEFT", frame, "TOPLEFT", 18, offsetY)
    label:SetJustifyH("LEFT")
    label:SetText(text)
    return label
end

local function CreateValue(anchor, offsetY)
    local value = frame:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
    value:SetPoint("TOPLEFT", frame, "TOPLEFT", 96, offsetY)
    value:SetWidth(240)
    value:SetJustifyH("LEFT")
    value:SetJustifyV("TOP")
    return value
end

local labels = {
    theme = CreateLabel(frame, -40, "테마:"),
    mission = CreateLabel(frame, -62, "임무:"),
    progress = CreateLabel(frame, -84, "진행도:"),
    timer = CreateLabel(frame, -106, "남은 시간:"),
    status = CreateLabel(frame, -128, "상태:"),
    briefing = CreateLabel(frame, -150, "브리핑:"),
}

local values = {
    theme = CreateValue(frame, -40),
    mission = CreateValue(frame, -62),
    progress = CreateValue(frame, -84),
    timer = CreateValue(frame, -106),
    status = CreateValue(frame, -128),
    briefing = CreateValue(frame, -150),
}
values.briefing:SetWidth(250)

local closeButton = CreateFrame("Button", nil, frame, "UIPanelButtonTemplate")
closeButton:SetSize(64, 22)
closeButton:SetPoint("BOTTOM", frame, "BOTTOM", 0, 16)
closeButton:SetText("닫기")
closeButton:SetScript("OnClick", function()
    frame:Hide()
end)

local function FormatTime(seconds)
    if not seconds or seconds <= 0 then
        return "-"
    end

    local minutes = math.floor(seconds / 60)
    local remain = math.floor(math.mod(seconds, 60))
    return string.format("%02d:%02d", minutes, remain)
end

local function RequestState()
    local inInstance = IsInInstance()
    if not inInstance then
        frame:Hide()
        return
    end

    AIO.Handle("KarazhanBonusMission", "RequestState")
end

function BonusMissionHandlers.ReceiveState(state)
    if not state or not state.active then
        frame:Hide()
        return
    end

    values.theme:SetText(state.theme_name or "-")
    values.mission:SetText(state.title or "-")
    values.progress:SetText(state.progress_text or "-")
    values.timer:SetText(FormatTime(state.remaining_sec))
    values.status:SetText(state.status_text or "진행 중")
    values.briefing:SetText(state.briefing or state.announcement or "-")
    frame:Show()
end

function BonusMissionHandlers.HideFrame()
    frame:Hide()
end

SLASH_KARAZHANBONUSMISSION1 = "/kbm"
SlashCmdList["KARAZHANBONUSMISSION"] = function()
    if frame:IsShown() then
        frame:Hide()
        return
    end

    RequestState()
end

local ticker = CreateFrame("Frame")
local elapsedSincePoll = 0

ticker:SetScript("OnUpdate", function(_, elapsed)
    elapsedSincePoll = elapsedSincePoll + elapsed
    if elapsedSincePoll < 2 then
        return
    end

    elapsedSincePoll = 0
    RequestState()
end)