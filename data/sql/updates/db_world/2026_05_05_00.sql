-- DB update 2026_05_04_01 -> 2026_05_05_00

-- Allow Warchief's Blessing and Spirit of Zandalar above level 63.
DELETE FROM `conditions`
WHERE `SourceTypeOrReferenceId` = 13
  AND `SourceEntry` IN (16609, 24425)
  AND `ConditionTypeOrReference` = 27
  AND `ConditionValue1` = 63
  AND `ConditionValue2` = 4;

-- Allow Rallying Cry of the Dragonslayer to keep its normal duration above level 63.
DELETE FROM `spell_script_names`
WHERE `spell_id` = 22888
  AND `ScriptName` = 'spell_gen_disabled_above_63';
