# AIO UI Plan

This document describes the first AIO UI integration for the instance bonus
mission system.

## Goal

Add a client-side panel that shows:
- current run theme
- mission title
- mission briefing
- current progress
- remaining time
- current mission status

## Runtime split

### C++ module
- owns mission selection and mission progress
- writes live mission state into `instance_bonus_mission_live`
- remains the source of truth

### Eluna + AIO
- queries the live state table
- pushes state into a lightweight UI panel
- polls for updates while the player is inside an instance

## Install targets

### Server
Place the AIO server files under `lua_scripts` and place both custom files in
that same directory.

Required files:
- AIO.lua and its server dependencies
- `KarazhanBonusMissionServer.lua`
- `KarazhanBonusMissionClient.lua`

### Client
Place `AIO_Client` under `Interface/AddOns`.

## First UI scope

The first UI version focuses on visibility, not advanced interaction.

Visible fields:
- 테마
- 임무
- 브리핑
- 진행도
- 남은 시간
- 상태

Slash command:
- `/kbm`

## Data source

The UI reads from `instance_bonus_mission_live`.

Columns used:
- `instance_id`
- `map_id`
- `theme_key`
- `theme_name`
- `mission_id`
- `mission_type`
- `title`
- `target_label`
- `target_count`
- `current_count`
- `time_limit_sec`
- `start_time`
- `expire_time`
- `briefing`
- `announcement`
- `completed`
- `failed`

## Expected follow-up work

1. Add score and grade fields to live state.
2. Add reward preview fields.
3. Add result panel for S/A/B/C/D grade output.
4. Push briefing automatically on mission creation.
5. Replace polling with event-triggered updates where practical.
## Build note

If you are moving to AIO for this feature, build with Eluna available and avoid
running the ALE-only path for the same UI scripts.

Recommended practical setup:
- keep the C++ bonus mission module enabled
- enable `mod-eluna`
- do not rely on `mod-ale` for the AIO UI path