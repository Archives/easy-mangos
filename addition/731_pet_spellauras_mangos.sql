ALTER TABLE `spell_pet_auras` DROP PRIMARY KEY;
ALTER TABLE `spell_pet_auras` ADD PRIMARY KEY ( `spell` , `effectId` , `pet` , `aura` );

-- DK Ghoul
DELETE FROM `spell_pet_auras` WHERE `aura` = 54566;
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 26125;
INSERT INTO `spell_pet_auras` VALUES
(0,0,26125,51996),
(0,0,26125,54566),
(0,0,26125,61697);

-- Hunter pets
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 1;
INSERT INTO `spell_pet_auras` VALUES
(0,0,1,34902),
(0,0,1,34903),
(0,0,1,34904),
(0,0,1,61017);

-- Warlock demons
-- imp
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 416;
INSERT INTO `spell_pet_auras` VALUES
(0,0,416,34947),
(0,0,416,34956),
(0,0,416,34957),
(0,0,416,34958),
(0,0,416,61013);

-- felhunter
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 417;
INSERT INTO `spell_pet_auras` VALUES
(0,0,417,34947),
(0,0,417,34956),
(0,0,417,34957),
(0,0,417,34958),
(0,0,417,61013);

-- succubus
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 1863;
INSERT INTO `spell_pet_auras` VALUES
(0,0,1863,34947),
(0,0,1863,34956),
(0,0,1863,34957),
(0,0,1863,34958),
(0,0,1863,61013);

-- felguard
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 17252;
INSERT INTO `spell_pet_auras` VALUES
(0,0,17252,34947),
(0,0,17252,34956),
(0,0,17252,34957),
(0,0,17252,34958),
(0,0,17252,61013);

-- Shaman wolf
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 29264;
INSERT INTO `spell_pet_auras` VALUES
(0,0,29264,58877),
(0,0,29264,34902),
(0,0,29264,34903),
(0,0,29264,34904),
(0,0,29264,61783);

-- Shadowfiend
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 19668;
INSERT INTO `spell_pet_auras` VALUES
(0,0,19668,34947);

-- Mage water elemental
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 37994;
INSERT INTO `spell_pet_auras` VALUES
(0,0,37994,34947),
(0,0,37994,34956);

-- Mage mirror image
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 31216;
INSERT INTO `spell_pet_auras` VALUES
(0,0,31216,34947);

-- DK Bloodworms
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 28017;
INSERT INTO `spell_pet_auras` VALUES
(0,0,28017,50453);

-- DK Gargoyle
DELETE FROM `spell_pet_auras` WHERE `spell` = 0 AND `pet` = 27829;
INSERT INTO `spell_pet_auras` VALUES
(0,0,27829,54566);
