# mod-karazhan-helper

Local LLM powered Karazhan helper for AzerothCore.

## Purpose

Players can ask natural-language item questions in chat and receive
server-aware answers via whisper.

Example:

- `정의의 휘장 어디서 드랍해요?`
- `그라추 아이템 어디서 구매 가능해?`

The helper:

1. detects a likely item question in chat
2. builds fuzzy item candidates from `item_template`
3. looks up sales and drop facts from `npc_vendor`,
   `creature_loot_template`, and `reference_loot_template`
4. asks the local helper bridge to resolve the best item and write the
   final answer
5. whispers the result back to the player

## Notes

- DB facts remain the source of truth
- the local LLM is used for fuzzy resolution and natural-language answers
- current first version focuses on item purchase/drop questions
