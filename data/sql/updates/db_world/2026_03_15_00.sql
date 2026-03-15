SET @TRAINER_ID := 190020;
SET @CREATURE_ID := 190020;

DELETE FROM `trainer_spell`
WHERE `TrainerId` = @TRAINER_ID;

DELETE FROM `creature_default_trainer`
WHERE `CreatureId` = @CREATURE_ID;

DELETE FROM `trainer`
WHERE `Id` = @TRAINER_ID;

INSERT INTO `trainer` (
    `Id`, `Type`, `Requirement`, `Greeting`, `VerifiedBuild`
) VALUES (
    @TRAINER_ID, 2, 0, 'Hello!  Ready for some training?', 0
);

INSERT INTO `trainer_spell` (
    `TrainerId`, `SpellId`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`,
    `ReqAbility1`, `ReqAbility2`, `ReqAbility3`, `ReqLevel`, `VerifiedBuild`
) VALUES
(@TRAINER_ID, 196, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 197, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 198, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 199, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 200, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 201, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 202, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 227, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 264, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 266, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 1180, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 2567, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 5011, 0, 0, 0, 0, 0, 0, 0, 0),
(@TRAINER_ID, 15590, 0, 0, 0, 0, 0, 0, 0, 0);

INSERT INTO `creature_default_trainer` (
    `CreatureId`, `TrainerId`
) VALUES (
    @CREATURE_ID, @TRAINER_ID
);
