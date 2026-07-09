-- Bind the granting item ("장인의 인장", entry 600035) to the module's ItemScript.
-- The item's on-use spell (51150) is a shared dummy used by many custom items, so we hook
-- the item by ScriptName instead of the spell.
UPDATE `item_template` SET `ScriptName` = 'item_artisan_seal' WHERE `entry` = 600035;

-- Make the item consumed on use (like every other consumable, e.g. potions and item 600020).
-- spellcharges_1 = -1 marks the on-use spell as "expendable" so the core destroys 1 on use.
-- NOTE: only newly-created copies pick up the charge; copies already sitting in a bag keep their
-- old stored charge (0) and must be re-issued to be consumable.
UPDATE `item_template` SET `spellcharges_1` = -1 WHERE `entry` = 600035;
