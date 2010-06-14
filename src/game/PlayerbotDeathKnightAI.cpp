// a simple DK class by rrtn :)

#include "PlayerbotDeathKnightAI.h"
#include "PlayerbotMgr.h"

class PlayerbotAI;
PlayerbotDeathKnightAI::PlayerbotDeathKnightAI(Player* const bot, PlayerbotAI* const ai): PlayerbotClassAI(bot, ai)
{
    InitSpells(ai);
}

PlayerbotDeathKnightAI::~PlayerbotDeathKnightAI() {}

void PlayerbotDeathKnightAI::InitSpells(PlayerbotAI* const ai)
{
    PLAGUE_STRIKE     = ai->getSpellId("plague strike"); // Unholy
    DEATH_GRIP        = ai->getSpellId("death grip");
    DEATH_COIL        = ai->getSpellId("death coil");
    DEATH_STRIKE      = ai->getSpellId("death strike");
    UNHOLY_BLIGHT     = ai->getSpellId("unholy blight");
    SCOURGE_STRIKE    = ai->getSpellId("scourge strike");
    DEATH_AND_DECAY   = ai->getSpellId("death and decay");
    CORPSE_EXPLOSION  = ai->getSpellId("corpse explosion");
    BONE_SHIELD          = ai->getSpellId("shield"); // buffs
    ANTI_MAGIC_SHELL  = ai->getSpellId("anti-magic shell");
    ANTI_MAGIC_ZONE   = ai->getSpellId("anti-magic");
    GHOUL_FRENZY      = ai->getSpellId("ghoul frenzy");
    RAISE_DEAD        = ai->getSpellId("raise dead"); // pets
    SUMMON_GARGOYLE   = ai->getSpellId("summon gargoyle");
    ARMY_OF_THE_DEAD  = ai->getSpellId("army of the dead");
    ICY_TOUCH         = ai->getSpellId("icy touch"); // Frost
    OBLITERATE        = ai->getSpellId("obliterate");
    HOWLING_BLAST     = ai->getSpellId("howling blast");
    FROST_STRIKE      = ai->getSpellId("frost strike");
    CHAINS_OF_ICE     = ai->getSpellId("chains of ice");
    RUNE_STRIKE       = ai->getSpellId("rune strike");
    ICY_CLUTCH        = ai->getSpellId("icy clutch");
    MIND_FREEZE       = ai->getSpellId("mind freeze");
    HUNGERING_COLD    = ai->getSpellId("hungering cold");
    KILLING_MACHINE   = ai->getSpellId("killing machine"); // buffs
    DEATHCHILL        = ai->getSpellId("deathchill");
    HORN_OF_WINTER    = ai->getSpellId("horn of winter");
    ICEBOUND_FORTITUDE= ai->getSpellId("icebound fortitude");
    EMPOWER_WEAPON    = ai->getSpellId("empower rune weapon");
    UNBREAKABLE_ARMOR = ai->getSpellId("unbreakable armor");
    BLOOD_STRIKE      = ai->getSpellId("blood strike"); // Blood
    PESTILENCE        = ai->getSpellId("pestilence");
    STRANGULATE       = ai->getSpellId("strangulate");
    BLOOD_BOIL        = ai->getSpellId("blood boil");
    HEART_STRIKE      = ai->getSpellId("heart strike");
    DANCING_WEAPON    = ai->getSpellId("dancing rune weapon");
    DARK_COMMAND      = ai->getSpellId("dark command");
    MARK_OF_BLOOD     = ai->getSpellId("mark of blood"); // buffs
    RUNE_TAP          = ai->getSpellId("rune tap");
    VAMPIRIC_BLOOD    = ai->getSpellId("vampiric blood");
    DEATH_PACT        = ai->getSpellId("death pact");
    DEATH_RUNE_MASTERY= ai->getSpellId("death rune mastery");
    HYSTERIA          = ai->getSpellId("hysteria");
    UNHOLY_PRESENCE   = ai->getSpellId("unholy presence"); // presence (TODO: better spell == presence)
    FROST_PRESENCE    = ai->getSpellId("frost presence");
    BLOOD_PRESENCE    = ai->getSpellId("blood presence");

    RECENTLY_BANDAGED = 11196; // first aid check

    // racial
    ARCANE_TORRENT          = ai->getSpellId("arcane torrent"); // blood elf
    GIFT_OF_THE_NAARU       = ai->getSpellId("gift of the naaru"); // draenei
    STONEFORM               = ai->getSpellId("stoneform"); // dwarf
    ESCAPE_ARTIST           = ai->getSpellId("escape artist"); // gnome
    EVERY_MAN_FOR_HIMSELF   = ai->getSpellId("every man for himself"); // human
    SHADOWMELD              = ai->getSpellId("shadowmeld"); // night elf
    BLOOD_FURY              = ai->getSpellId("blood fury"); // orc
    WAR_STOMP               = ai->getSpellId("war stomp"); // tauren
    BERSERKING              = ai->getSpellId("berserking"); // troll
    WILL_OF_THE_FORSAKEN    = ai->getSpellId("will of the forsaken"); // undead
}

void PlayerbotDeathKnightAI::DoNextCombatManeuver(Unit *pTarget)
{
    PlayerbotAI* ai = GetAI();
    if (!ai)
        return;

    Player* pMaster = ai->GetMaster();
    if (!pMaster)
        return;

    switch (ai->GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_DUEL:
            ai->CastSpell(PLAGUE_STRIKE);
            return;
    }

    // ------- Non Duel combat ----------

    //ai->SetMovementOrder( PlayerbotAI::MOVEMENT_FOLLOW, pMaster ); // dont want to melee mob

    // DK Attacks: Unholy, Frost & Blood

    // damage spells
    ai->SetInFront( pTarget );//<---
    Player *m_bot = GetPlayerBot();
    Unit* pVictim = pTarget->getVictim();
    Pet *pet = m_bot->GetPet();
    float dist = m_bot->GetDistance( pTarget );

    switch (SpellSequence)
    {
        case SPELL_DK_UNHOLY:
             if (UNHOLY_PRESENCE > 0)
                (!m_bot->HasAura(UNHOLY_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(BLOOD_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(FROST_PRESENCE, EFFECT_INDEX_0) && ai->CastSpell (UNHOLY_PRESENCE, *m_bot));

            // check for BONE_SHIELD in combat
            if (BONE_SHIELD > 0)
                 (!m_bot->HasAura(BONE_SHIELD, EFFECT_INDEX_0) && !m_bot->HasAura(ARMY_OF_THE_DEAD, EFFECT_INDEX_0) && ai->CastSpell (BONE_SHIELD, *m_bot));

            if (ARMY_OF_THE_DEAD > 0 && ai->GetAttackerCount()>=5 && LastSpellUnholyDK < 1)
            {
                ai->CastSpell(ARMY_OF_THE_DEAD);
                if (ARMY_OF_THE_DEAD > 0 && m_bot->HasAura(ARMY_OF_THE_DEAD, EFFECT_INDEX_0))
                    ai->SetIgnoreUpdateTime(7);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (PLAGUE_STRIKE > 0 && !pTarget->HasAura(PLAGUE_STRIKE, EFFECT_INDEX_0) && LastSpellUnholyDK < 2)
            {
                ai->CastSpell(PLAGUE_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (DEATH_GRIP > 0 && !pTarget->HasAura(DEATH_GRIP, EFFECT_INDEX_0) && LastSpellUnholyDK < 3)
            {
                ai->CastSpell(DEATH_GRIP, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (DEATH_COIL > 0 && LastSpellUnholyDK < 4 && ai->GetRunicPower() >= 40)
            {
                ai->CastSpell(DEATH_COIL, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (DEATH_STRIKE > 0 && !pTarget->HasAura(DEATH_STRIKE, EFFECT_INDEX_0) && LastSpellUnholyDK < 5)
            {
                ai->CastSpell(DEATH_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (UNHOLY_BLIGHT > 0 && !pTarget->HasAura(UNHOLY_BLIGHT, EFFECT_INDEX_0) && LastSpellUnholyDK < 6)
            {
                ai->CastSpell(UNHOLY_BLIGHT);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (SCOURGE_STRIKE > 0 && LastSpellUnholyDK < 7)
            {
                ai->CastSpell(SCOURGE_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (DEATH_AND_DECAY > 0 && ai->GetAttackerCount()>=3 && dist <= ATTACK_DISTANCE && !pTarget->HasAura(DEATH_AND_DECAY, EFFECT_INDEX_0) && LastSpellUnholyDK < 8)
            {
                ai->CastSpell(DEATH_AND_DECAY);
                ai->SetIgnoreUpdateTime(1);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (SUMMON_GARGOYLE > 0 && !m_bot->HasAura(ARMY_OF_THE_DEAD, EFFECT_INDEX_0) && !pTarget->HasAura(SUMMON_GARGOYLE, EFFECT_INDEX_0) && LastSpellUnholyDK < 9 && ai->GetRunicPower() >= 60)
            {
                ai->CastSpell(SUMMON_GARGOYLE, *pTarget);
                ai->SetIgnoreUpdateTime(2);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (CORPSE_EXPLOSION > 0 && dist <= ATTACK_DISTANCE && LastSpellUnholyDK < 10)
            {
                ai->CastSpell(CORPSE_EXPLOSION, *pTarget);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (ANTI_MAGIC_SHELL > 0 && pTarget->IsNonMeleeSpellCasted(true) && !m_bot->HasAura(ANTI_MAGIC_SHELL, EFFECT_INDEX_0) && LastSpellUnholyDK < 11 && ai->GetRunicPower() >= 20)
            {
                ai->CastSpell(ANTI_MAGIC_SHELL, *m_bot);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (ANTI_MAGIC_ZONE > 0 && pTarget->IsNonMeleeSpellCasted(true) && !m_bot->HasAura(ANTI_MAGIC_SHELL, EFFECT_INDEX_0) && LastSpellUnholyDK < 12)
            {
                ai->CastSpell(ANTI_MAGIC_ZONE, *m_bot);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (( !pet )
                && (RAISE_DEAD > 0 && !m_bot->HasAura(ARMY_OF_THE_DEAD, EFFECT_INDEX_0) && LastSpellUnholyDK < 13))
            {
                ai->CastSpell(RAISE_DEAD);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (( pet )
                && (GHOUL_FRENZY > 0 && pVictim == pet && !pet->HasAura(GHOUL_FRENZY, EFFECT_INDEX_0) && LastSpellUnholyDK < 14))
            {
                ai->CastSpell(GHOUL_FRENZY, *pet);
                SpellSequence = SPELL_DK_FROST;
                LastSpellUnholyDK = LastSpellUnholyDK +1;
                break;
            }
            else if (LastSpellUnholyDK > 15)
            {
                LastSpellUnholyDK = 0;
                SpellSequence = SPELL_DK_FROST;
                break;
            }

            LastSpellUnholyDK = 0;

        case SPELL_DK_FROST:
            if (FROST_PRESENCE > 0)
                (!m_bot->HasAura(FROST_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(BLOOD_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(UNHOLY_PRESENCE, EFFECT_INDEX_0) && ai->CastSpell (FROST_PRESENCE, *m_bot));

            if (DEATHCHILL > 0)
                (!m_bot->HasAura(DEATHCHILL, EFFECT_INDEX_0) && !m_bot->HasAura(KILLING_MACHINE, EFFECT_INDEX_0) && ai->CastSpell (DEATHCHILL, *m_bot));
            else if (KILLING_MACHINE > 0)
                (!m_bot->HasAura(KILLING_MACHINE, EFFECT_INDEX_0) && !m_bot->HasAura(DEATHCHILL, EFFECT_INDEX_0) && ai->CastSpell (KILLING_MACHINE, *m_bot));

            if (ICY_TOUCH > 0 && !pTarget->HasAura(ICY_TOUCH, EFFECT_INDEX_0) && LastSpellFrostDK < 1)
            {
                ai->CastSpell(ICY_TOUCH, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (OBLITERATE > 0 && LastSpellFrostDK < 2)
            {
                ai->CastSpell(OBLITERATE, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (FROST_STRIKE > 0 && LastSpellFrostDK < 3 && ai->GetRunicPower() >= 40)
            {
                ai->CastSpell(FROST_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (HOWLING_BLAST > 0 && ai->GetAttackerCount()>=3 && LastSpellFrostDK < 4)
            {
                ai->CastSpell(HOWLING_BLAST, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (CHAINS_OF_ICE > 0 && !pTarget->HasAura(CHAINS_OF_ICE, EFFECT_INDEX_0) && LastSpellFrostDK < 5)
            {
                ai->CastSpell(CHAINS_OF_ICE, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (RUNE_STRIKE > 0 && LastSpellFrostDK < 6 && ai->GetRunicPower() >= 20)
            {
                ai->CastSpell(RUNE_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (ICY_CLUTCH > 0 && !pTarget->HasAura(ICY_CLUTCH, EFFECT_INDEX_0) && LastSpellFrostDK < 7)
            {
                ai->CastSpell(ICY_CLUTCH, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (ICEBOUND_FORTITUDE > 0 && ai->GetHealthPercent() < 50 && pVictim == m_bot && !m_bot->HasAura(ICEBOUND_FORTITUDE, EFFECT_INDEX_0) && LastSpellFrostDK < 8 && ai->GetRunicPower() >= 20)
            {
                ai->CastSpell(ICEBOUND_FORTITUDE, *m_bot);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (MIND_FREEZE > 0 && pTarget->IsNonMeleeSpellCasted(true) && dist <= ATTACK_DISTANCE && LastSpellFrostDK < 9 && ai->GetRunicPower() >= 20)
            {
                ai->CastSpell(MIND_FREEZE, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (HUNGERING_COLD > 0 && ai->GetAttackerCount()>=3 && dist <= ATTACK_DISTANCE && LastSpellFrostDK < 10 && ai->GetRunicPower() >= 40)
            {
                ai->CastSpell(HUNGERING_COLD, *pTarget);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (EMPOWER_WEAPON > 0 && ai->GetRunicPower() < 20 && LastSpellFrostDK < 11)
            {
                ai->CastSpell(EMPOWER_WEAPON, *m_bot);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (UNBREAKABLE_ARMOR > 0 && !m_bot->HasAura(UNBREAKABLE_ARMOR, EFFECT_INDEX_0) && ai->GetHealthPercent() < 70 && pVictim == m_bot && LastSpellFrostDK < 12)
            {
                ai->CastSpell(UNBREAKABLE_ARMOR, *m_bot);
                SpellSequence = SPELL_DK_BLOOD;
                LastSpellFrostDK = LastSpellFrostDK +1;
                break;
            }
            else if (LastSpellFrostDK > 13)
            {
                LastSpellFrostDK = 0;
                SpellSequence = SPELL_DK_BLOOD;
                break;
            }

            LastSpellFrostDK = 0;

        case SPELL_DK_BLOOD:
            if (BLOOD_PRESENCE > 0)
                (!m_bot->HasAura(BLOOD_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(UNHOLY_PRESENCE, EFFECT_INDEX_0) && !m_bot->HasAura(FROST_PRESENCE, EFFECT_INDEX_0) && ai->CastSpell (BLOOD_PRESENCE, *m_bot));

            if (MARK_OF_BLOOD > 0 && !pTarget->HasAura(MARK_OF_BLOOD, EFFECT_INDEX_0) && LastSpellBloodDK < 1)
            {
                ai->CastSpell(MARK_OF_BLOOD, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (BLOOD_STRIKE > 0 && LastSpellBloodDK < 2)
            {
                ai->CastSpell(BLOOD_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK =  LastSpellBloodDK +1;
                break;
            }
            else if (PESTILENCE > 0 && dist <= ATTACK_DISTANCE && ai->GetAttackerCount()>=3 && LastSpellBloodDK < 3)
            {
                ai->CastSpell(PESTILENCE, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (STRANGULATE > 0 && !pTarget->HasAura(STRANGULATE, EFFECT_INDEX_0) && LastSpellBloodDK < 4)
            {
                ai->CastSpell(STRANGULATE, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (BLOOD_BOIL > 0 && ai->GetAttackerCount()>=5 && dist <= ATTACK_DISTANCE && LastSpellBloodDK < 5)
            {
                ai->CastSpell(BLOOD_BOIL, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (HEART_STRIKE > 0 && LastSpellBloodDK < 6)
            {
                ai->CastSpell(HEART_STRIKE, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (VAMPIRIC_BLOOD > 0 && ai->GetHealthPercent() < 70 && !m_bot->HasAura(VAMPIRIC_BLOOD, EFFECT_INDEX_0) && LastSpellBloodDK < 7)
            {
                ai->CastSpell(VAMPIRIC_BLOOD, *m_bot);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (RUNE_TAP > 0 && ai->GetHealthPercent() < 70 && !m_bot->HasAura(VAMPIRIC_BLOOD, EFFECT_INDEX_0) && LastSpellBloodDK < 8)
            {
                ai->CastSpell(RUNE_TAP, *m_bot);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (HYSTERIA > 0 && ai->GetHealthPercent() > 25 && !m_bot->HasAura(HYSTERIA, EFFECT_INDEX_0) && LastSpellBloodDK < 9)
            {
                ai->CastSpell(HYSTERIA, *m_bot);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (DANCING_WEAPON > 0 && !m_bot->HasAura(DANCING_WEAPON, EFFECT_INDEX_0) && ai->GetRunicPower() >= 60 && LastSpellBloodDK < 10)
            {
                ai->CastSpell(DANCING_WEAPON, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (DARK_COMMAND > 0 && ai->GetHealthPercent() > 50 && pVictim != m_bot && !pTarget->HasAura(DARK_COMMAND, EFFECT_INDEX_0) && dist <= ATTACK_DISTANCE && LastSpellBloodDK < 11)
            {
                ai->CastSpell(DARK_COMMAND, *pTarget);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (( pet )
                && (DEATH_PACT > 0 && ai->GetHealthPercent() < 50 && LastSpellBloodDK < 12 && ai->GetRunicPower() >= 40))
            {
                ai->CastSpell(DEATH_PACT, *pet);
                SpellSequence = SPELL_DK_UNHOLY;
                LastSpellBloodDK = LastSpellBloodDK +1;
                break;
            }
            else if (LastSpellBloodDK > 13)
            {
                LastSpellBloodDK = 0;
                SpellSequence = SPELL_DK_UNHOLY;
                break;
            }
            else
            {
                LastSpellBloodDK = 0;
                SpellSequence = SPELL_DK_UNHOLY;
            }
    }
} // end DoNextCombatManeuver

void PlayerbotDeathKnightAI::DoNonCombatActions()
{
    PlayerbotAI* ai = GetAI();
    if (!ai)
        return;

    Player* pMaster = ai->GetMaster();
    if (!pMaster)
        return;

    Player *m_bot = GetPlayerBot();
    if (!m_bot)
        return;

    SpellSequence = SPELL_DK_UNHOLY;

    // buff master with HORN_OF_WINTER
    if (HORN_OF_WINTER> 0)
        (!pMaster->HasAura(HORN_OF_WINTER,EFFECT_INDEX_0) && ai->CastSpell (HORN_OF_WINTER, *pMaster));

    // hp check
    if (m_bot->getStandState() != UNIT_STAND_STATE_STAND)
        m_bot->SetStandState(UNIT_STAND_STATE_STAND);

    Item* pItem = ai->FindFood();
    Item* fItem = ai->FindBandage();

    if (pItem != NULL && ai->GetHealthPercent() < 30)
    {
        ai->TellMaster("Je devrais manger un peu...");
        ai->UseItem(*pItem);
        ai->SetIgnoreUpdateTime(30);
        return;
    }
    else if (pItem == NULL && fItem != NULL && !m_bot->HasAura(RECENTLY_BANDAGED, EFFECT_INDEX_0) && ai->GetHealthPercent() < 70)
    {
        ai->TellMaster("J'ai besoin d'un bandage...");
        ai->UseItem(*fItem);
        ai->SetIgnoreUpdateTime(8);
        return;
    }
} // end DoNonCombatActions
