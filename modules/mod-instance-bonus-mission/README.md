# mod-instance-bonus-mission

This module adds random bonus mission content for dungeon and raid
instances.

## Direction

The current runtime keeps the mission rules server-side and selects one
eligible mission at random for the current map and difficulty.

The local LLM is not used as a free-form game rules engine.
It is only used as an optional selector/narrator inside a
server-controlled framework.

## High-level flow

1. A party enters an instance.
2. The server collects party context.
   - party size
   - class composition
   - role composition
   - average item level
   - dungeon and difficulty
3. The server filters the mission pool by map and difficulty.
4. One mission is selected from the eligible candidates.
5. The optional local LLM can choose one mission from the same candidate list.
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
10. The server grants the configured reward.

## Responsibility split

### LLM
- optionally choose one mission from server-provided candidates
- optionally refine the mission announcement text

### Server
- validate all candidates
- track progress and time limits
- track deaths, wipes, and completion state
- compute score and grade
- decide the final reward tier and grant rewards

## Why this structure matters

The runtime now favors predictability and easier content management.
Mission authors can pre-build missions in the database, and the server
simply chooses one that matches the current map and difficulty.

## Planned database structure

### instance_bonus_mission_pool
Defines the individual mission objectives.

### instance_bonus_reward_tier
Reserved for reward-tier expansion.

## V2 schema for web and audit

The current runtime still uses the prototype tables below:
- `instance_bonus_mission_pool`
- `instance_bonus_theme_pool` (legacy-compatible, not used by runtime)
- `instance_bonus_theme_mission` (legacy-compatible, not used by runtime)
- `instance_bonus_reward_tier`
- `instance_bonus_mission_live`

To avoid breaking current gameplay, the web/CMS and audit schema is added
side by side as a v2 data model:
- `instance_bonus_map_config`
- `instance_bonus_mission`
- `instance_bonus_theme`
- `instance_bonus_theme_mission_link`
- `instance_bonus_reward_profile`
- `instance_bonus_reward_profile_item`
- `instance_bonus_run_live`
- `instance_bonus_run_history`
- `instance_bonus_run_member`
- `instance_bonus_vote_log`
- `instance_bonus_reward_log`
- `instance_bonus_event_log`
- `instance_bonus_llm_log`

This lets the current module keep running while web tooling and future runtime
migration can move to the richer schema without data loss.

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

1. Expand the mission catalog per map and difficulty.
2. Add richer completion scoring if needed.
3. Add optional LLM completion summaries.
4. Add more audit and web tooling on top of the v2 schema.

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
- mission title
- progress
- remaining time
- mission status
- briefing or announcement text

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
