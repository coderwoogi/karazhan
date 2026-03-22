# mod-instance-bonus-mission

This module adds theme-driven bonus mission content for dungeon and raid
instances.

## Direction

The first prototype used a single random mission per instance.
The next iteration turns the system into a themed run evaluator.

The local LLM is not used as a free-form game rules engine.
It is used as a selector and narrator inside a server-controlled framework.

## High-level flow

1. A party enters an instance.
2. The server collects party context.
   - party size
   - class composition
   - role composition
   - average item level
   - dungeon and difficulty
3. The LLM selects a run theme.
   - slaughter
   - clean_run
   - speed_run
   - boss_focus
4. The server narrows the mission pool to the selected theme.
5. The LLM selects one mission bundle and writes the briefing text.
6. The server tracks gameplay metrics.
   - clear time
   - deaths
   - wipes
   - kill counts
   - boss kills
   - bonus conditions
7. On completion, the server computes a numeric score.
8. The server assigns a grade.
   - S / A / B / C / D
9. The LLM writes a result summary.
10. The server grants the reward tier bound to the grade and theme.

## Responsibility split

### LLM
- choose the run theme from server-provided candidates
- choose one mission bundle from the server-provided candidates
- generate the entry briefing text
- generate the completion summary text

### Server
- validate all candidates
- track progress and time limits
- track deaths, wipes, and completion state
- compute score and grade
- decide the final reward tier and grant rewards

## Why this structure matters

A purely random mission picker is easy to replace with code only.
The LLM becomes valuable only when it changes the feel of each run by
selecting the theme that best fits the current party and dungeon.

This means the same dungeon can feel different from run to run.

Examples:
- slaughter: heavy kill-count pressure
- clean_run: no-death or no-wipe emphasis
- speed_run: strict timer and faster routing
- boss_focus: short path to key bosses and high-value targets

## Planned database structure

### instance_bonus_mission_pool
Defines the individual mission objectives.

### instance_bonus_theme_pool
Defines the run themes per instance.

### instance_bonus_theme_mission
Maps themes to valid missions.

### instance_bonus_reward_tier
Defines reward tiers per theme and grade.

## Planned scoring model

The server computes score from real gameplay metrics.

Suggested weighting:
- bonus objective success: +30
- clear within time target: +30
- no deaths: +20
- no wipes: +10
- extra optional objective: +10
- death penalty: negative per death
- wipe penalty: heavy negative
- time overrun: negative by bracket

Suggested grades:
- S: 90 to 100
- A: 75 to 89
- B: 60 to 74
- C: 40 to 59
- D: 0 to 39

## Next implementation targets

1. Add theme tables and reward-tier tables.
2. Add party-context collection in the module.
3. Send theme-selection requests to the local bridge.
4. Support mission bundles instead of one mission only.
5. Add death-safe and no-wipe conditions.
6. Add end-of-run scoring and grade handling.
7. Add LLM completion summaries.

## AIO UI

The first AIO UI prototype is stored under `aio/`.

Files:
- `aio/KarazhanBonusMissionServer.lua`
- `aio/KarazhanBonusMissionClient.lua`
- `AIO_UI_PLAN.md`

The UI reads from the `instance_bonus_mission_live` table.
The C++ module writes live mission state to this table and the AIO layer polls
it for display.

The current panel shows:
- current theme
- mission title
- progress
- remaining time
- mission status
- briefing text

Client slash command:
- `/kbm`
## Custom Addon UI

The current UI path no longer depends on Eluna or AIO for runtime.
The server sends mission state through addon chat messages with the
`KBM_UI` prefix, and a lightweight client addon renders the panel.

Client addon files:
- `addon/KarazhanBonusMission/KarazhanBonusMission.toc`
- `addon/KarazhanBonusMission/KarazhanBonusMission.lua`

Client install path:
- `Interface/AddOns/KarazhanBonusMission`

Current addon features:
- `/kbm` toggle command
- auto-show when a mission starts
- theme, mission, progress, timer, and status display
- briefing and current announcement display

The older `aio/` prototype is kept only as an archive reference.
Do not use it for the active build path.