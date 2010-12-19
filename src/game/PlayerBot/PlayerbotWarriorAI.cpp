/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "PlayerbotWarriorAI.h"
#include "PlayerbotMgr.h"

class PlayerbotAI;

PlayerbotWarriorAI::PlayerbotWarriorAI(Player* const bot, PlayerbotAI* const ai): PlayerbotClassAI(bot, ai)
{
    InitSpells(ai);
}

void PlayerbotWarriorAI::InitSpells(PlayerbotAI* const ai)
{
    BATTLE_STANCE           = ai->initSpell(BATTLE_STANCE_1); //ARMS
    CHARGE                  = ai->initSpell(CHARGE_1); //ARMS
    OVERPOWER               = ai->initSpell(OVERPOWER_1); // ARMS
    HEROIC_STRIKE           = ai->initSpell(HEROIC_STRIKE_1); //ARMS
    REND                    = ai->initSpell(REND_1); //ARMS
    THUNDER_CLAP            = ai->initSpell(THUNDER_CLAP_1);  //ARMS
    HAMSTRING               = ai->initSpell(HAMSTRING_1);  //ARMS
    MOCKING_BLOW            = ai->initSpell(MOCKING_BLOW_1);  //ARMS
    RETALIATION             = ai->initSpell(RETALIATION_1);  //ARMS
    SWEEPING_STRIKES        = ai->initSpell(SWEEPING_STRIKES_1); //ARMS
    MORTAL_STRIKE           = ai->initSpell(MORTAL_STRIKE_1);  //ARMS
    BLADESTORM              = ai->initSpell(BLADESTORM_1);  //ARMS
    HEROIC_THROW            = ai->initSpell(HEROIC_THROW_1);  //ARMS
    SHATTERING_THROW        = ai->initSpell(SHATTERING_THROW_1);  //ARMS
    BLOODRAGE               = ai->initSpell(BLOODRAGE_1); //PROTECTION
    DEFENSIVE_STANCE        = ai->initSpell(DEFENSIVE_STANCE_1); //PROTECTION
    DEVASTATE               = ai->initSpell(DEVASTATE_1); //PROTECTION
    SUNDER_ARMOR            = ai->initSpell(SUNDER_ARMOR_1); //PROTECTION
    TAUNT                   = ai->initSpell(TAUNT_1); //PROTECTION
    SHIELD_BASH             = ai->initSpell(SHIELD_BASH_1); //PROTECTION
    REVENGE                 = ai->initSpell(REVENGE_1); //PROTECTION
    SHIELD_BLOCK            = ai->initSpell(SHIELD_BLOCK_1); //PROTECTION
    DISARM                  = ai->initSpell(DISARM_1); //PROTECTION
    SHIELD_WALL             = ai->initSpell(SHIELD_WALL_1); //PROTECTION
    SHIELD_SLAM             = ai->initSpell(SHIELD_SLAM_1); //PROTECTION
    VIGILANCE               = ai->initSpell(VIGILANCE_1); //PROTECTION
    DEVASTATE               = ai->initSpell(DEVASTATE_1); //PROTECTION
    SHOCKWAVE               = ai->initSpell(SHOCKWAVE_1); //PROTECTION
    CONCUSSION_BLOW         = ai->initSpell(CONCUSSION_BLOW_1); //PROTECTION
    SPELL_REFLECTION        = ai->initSpell(SPELL_REFLECTION_1); //PROTECTION
    LAST_STAND              = ai->initSpell(LAST_STAND_1); //PROTECTION
    BATTLE_SHOUT            = ai->initSpell(BATTLE_SHOUT_1); //FURY
    DEMORALIZING_SHOUT      = ai->initSpell(DEMORALIZING_SHOUT_1); //FURY
    CLEAVE                  = ai->initSpell(CLEAVE_1); //FURY
    INTIMIDATING_SHOUT      = ai->initSpell(INTIMIDATING_SHOUT_1); //FURY
    EXECUTE                 = ai->initSpell(EXECUTE_1); //FURY
    CHALLENGING_SHOUT       = ai->initSpell(CHALLENGING_SHOUT_1); //FURY
    SLAM                    = ai->initSpell(SLAM_1); //FURY
    BERSERKER_STANCE        = ai->initSpell(BERSERKER_STANCE_1); //FURY
    INTERCEPT               = ai->initSpell(INTERCEPT_1); //FURY
    DEATH_WISH              = ai->initSpell(DEATH_WISH_1); //FURY
    BERSERKER_RAGE          = ai->initSpell(BERSERKER_RAGE_1); //FURY
    WHIRLWIND               = ai->initSpell(WHIRLWIND_1); //FURY
    PUMMEL                  = ai->initSpell(PUMMEL_1); //FURY
    BLOODTHIRST             = ai->initSpell(BLOODTHIRST_1); //FURY
    RECKLESSNESS            = ai->initSpell(RECKLESSNESS_1); //FURY
    RAMPAGE                 = 0; // passive
    HEROIC_FURY             = ai->initSpell(HEROIC_FURY_1); //FURY
    COMMANDING_SHOUT        = ai->initSpell(COMMANDING_SHOUT_1); //FURY
    ENRAGED_REGENERATION    = ai->initSpell(ENRAGED_REGENERATION_1); //FURY
    PIERCING_HOWL           = ai->initSpell(PIERCING_HOWL_1); //FURY

    RECENTLY_BANDAGED       = 11196; // first aid check

    // racial
    GIFT_OF_THE_NAARU       = ai->initSpell(GIFT_OF_THE_NAARU_WARRIOR); // draenei
    STONEFORM               = ai->initSpell(STONEFORM_ALL); // dwarf
    ESCAPE_ARTIST           = ai->initSpell(ESCAPE_ARTIST_ALL); // gnome
    EVERY_MAN_FOR_HIMSELF   = ai->initSpell(EVERY_MAN_FOR_HIMSELF_ALL); // human
    SHADOWMELD              = ai->initSpell(SHADOWMELD_ALL); // night elf
    BLOOD_FURY              = ai->initSpell(BLOOD_FURY_MELEE_CLASSES); // orc
    WAR_STOMP               = ai->initSpell(WAR_STOMP_ALL); // tauren
    BERSERKING              = ai->initSpell(BERSERKING_ALL); // troll
    WILL_OF_THE_FORSAKEN    = ai->initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead
}

PlayerbotWarriorAI::~PlayerbotWarriorAI() {}

bool PlayerbotWarriorAI::DoFirstCombatManeuver(Unit* pTarget)
{
    PlayerbotAI* ai = GetAI();
    if (!ai)
        return false;

    Player* m_bot = GetPlayerBot();
    if (!m_bot)
        return false;

    PlayerbotAI::CombatOrderType co = ai->GetCombatOrder();
    float fTargetDist = m_bot->GetDistance(pTarget);

    if ((co & PlayerbotAI::ORDERS_TANK) && !ai->HasAura(DEFENSIVE_STANCE, m_bot) && ai->CastSpell(DEFENSIVE_STANCE))
        return true;
    else if ((co & PlayerbotAI::ORDERS_TANK) && ai->HasAura(DEFENSIVE_STANCE, m_bot) && ai->CastSpell(TAUNT, pTarget))
        return false;
    else if (!ai->HasAura(BATTLE_STANCE, m_bot) && ai->CastSpell(BATTLE_STANCE))
        return true;
    else if (ai->HasAura(BATTLE_STANCE, m_bot))
    {
        if (fTargetDist < 8.0f)
            return false;
        else if (fTargetDist > 25.0f)
            return true;
        else if (ai->CastSpell(CHARGE, pTarget))
        {
            float x, y, z;
            pTarget->GetContactPoint(m_bot, x, y, z, 3.666666f);
            m_bot->Relocate(x, y, z);
            return false;
        }
    }

    return false;
}

void PlayerbotWarriorAI::DoNextCombatManeuver(Unit* pTarget)
{
    PlayerbotAI* ai = GetAI();
    if (!ai)
        return;

    Player* m_bot = GetPlayerBot();
    if (!m_bot)
        return;

    /*switch (ai->GetScenarioType())
    {
        //SCENARIO_PVEEASY,
        //SCENARIO_PVEHARD,
        //SCENARIO_DUEL,
        //SCENARIO_PVPEASY,
        //SCENARIO_PVPHARD
        //return;
    }*/

    //ai->SetMovementOrder( PlayerbotAI::MOVEMENT_FOLLOW, m_master ); // dont want to melee mob

    // Damage Attacks

    ai->SetInFront(pTarget);
    Unit* pVictim = pTarget->getVictim();
    float fTargetDist = m_bot->GetDistance(pTarget);
    PlayerbotAI::CombatOrderType co = ai->GetCombatOrder();

    // decide what stance to use
    if ((co & PlayerbotAI::ORDERS_TANK) && !ai->HasAura(DEFENSIVE_STANCE, m_bot) && ai->CastSpell(DEFENSIVE_STANCE))
        return;
    else if (!(co & PlayerbotAI::ORDERS_TANK) && !ai->HasAura(BATTLE_STANCE, m_bot) && ai->CastSpell(BATTLE_STANCE))
        return;

    // get spell sequence
    if (pTarget->IsNonMeleeSpellCasted(true))
        SpellSequence = WarriorSpellPreventing;
    else if (ai->HasAura(BATTLE_STANCE, m_bot))
        SpellSequence = WarriorBattle;
    else if (ai->HasAura(DEFENSIVE_STANCE, m_bot))
        SpellSequence = WarriorDefensive;
    else if (ai->HasAura(BERSERKER_STANCE, m_bot))
        SpellSequence = WarriorBerserker;

    // do shouts, berserker rage, etc...
    if (!ai->HasAura(BERSERKER_RAGE, m_bot) && ai->CastSpell(BERSERKER_RAGE))
        return;
    else if (ai->GetRageAmount() >= 10 && !ai->HasAura(DEMORALIZING_SHOUT, pTarget) && ai->CastSpell(DEMORALIZING_SHOUT))
        return;
    else if (ai->GetRageAmount() >= 10 && !ai->HasAura(BATTLE_SHOUT, m_bot) && ai->CastSpell(BATTLE_SHOUT))
        return;

    switch (SpellSequence)
    {
        case WarriorSpellPreventing:
            if (ai->GetRageAmount() >= 10 && ai->CastSpell(SHIELD_BASH, pTarget))
                return;
            else if (ai->GetRageAmount() >= 10 && ai->CastSpell(PUMMEL, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && !ai->HasAura(SPELL_REFLECTION, m_bot) && ai->CastSpell(SPELL_REFLECTION, m_bot))
                return;

            return;

        case WarriorBattle:
            if (ai->GetRageAmount() >= 15 && pTarget->GetHealth() < pTarget->GetMaxHealth() * 0.2 && ai->CastSpell(EXECUTE, pTarget))
                return;
            else if (!ai->HasAura(LAST_STAND, m_bot) && m_bot->GetHealth() < m_bot->GetMaxHealth() * 0.5 && ai->CastSpell(LAST_STAND, m_bot))
                return;
            else if (ai->GetRageAmount() < 50 && !ai->HasAura(BLOODRAGE, m_bot) && ai->CastSpell(BLOODRAGE, m_bot))
                return;
            else if (ai->GetRageAmount() >= 10 && !ai->HasAura(DEATH_WISH, m_bot) && ai->CastSpell(DEATH_WISH, m_bot))
                return;
            else if (pVictim == m_bot && ai->GetAttackerCount() >= 2 && !ai->HasAura(RETALIATION, m_bot) && ai->CastSpell(RETALIATION, m_bot))
                return;
            else if (ai->GetRageAmount() >= 10 && !ai->HasAura(DEMORALIZING_SHOUT, pTarget) && ai->CastSpell(DEMORALIZING_SHOUT, pTarget))
                return;
            else if (ai->GetRageAmount() >= 30 && ai->GetAttackerCount() >= 2 && !ai->HasAura(SWEEPING_STRIKES, m_bot) && ai->CastSpell(SWEEPING_STRIKES, m_bot))
                return;
            else if (ai->GetRageAmount() >= 25 && pVictim == m_bot && !ai->HasAura(BLADESTORM, m_bot) && ai->GetAttackerCount() >= 3 && ai->CastSpell(BLADESTORM, pTarget))
                return;
            else if (ai->GetRageAmount() >= 30 && !ai->HasAura(MORTAL_STRIKE, pTarget) && ai->CastSpell(MORTAL_STRIKE, pTarget))
                return;
            else if (ai->GetRageAmount() >= 25 && ai->GetAttackerCount() > 5 && ai->CastSpell(INTIMIDATING_SHOUT, pTarget))
                return;
            else if (ai->GetRageAmount() >= 20 && pVictim == m_bot && !ai->HasAura(THUNDER_CLAP, pTarget) && ai->CastSpell(THUNDER_CLAP, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && !ai->HasAura(BERSERKER_RAGE, m_bot) && !ai->HasAura(ENRAGED_REGENERATION, m_bot) && m_bot->GetHealth() < m_bot->GetMaxHealth() * 0.5 && ai->CastSpell(ENRAGED_REGENERATION, m_bot))
                return;
            else if (ai->GetRageAmount() >= 15 && pVictim == m_bot && !ai->HasAura(WAR_STOMP, pTarget) && !ai->HasAura(PIERCING_HOWL, pTarget) && !ai->HasAura(SHOCKWAVE, pTarget) && !ai->HasAura(CONCUSSION_BLOW, pTarget) && ai->CastSpell(SHOCKWAVE, pTarget))
                return;
            else if (ai->GetRageAmount() >= 10 && !ai->HasAura(REND, pTarget) && ai->CastSpell(REND, pTarget))
                return;
            else if (ai->GetRageAmount() >= 10 && !ai->HasAura(HAMSTRING, pTarget) && ai->CastSpell(HAMSTRING, pTarget))
                return;
            else if (ai->GetRageAmount() >= 5 && pVictim != m_bot && ai->GetHealthPercent() > 25 && !ai->HasAura(MOCKING_BLOW, pTarget) && !ai->HasAura(CHALLENGING_SHOUT, pTarget) && ai->CastSpell(CHALLENGING_SHOUT, pTarget))
                return;
            else if (ai->GetRageAmount() >= 20 && !ai->HasAura(BLOODTHIRST, m_bot) && m_bot->GetHealth() < m_bot->GetMaxHealth() * 0.7 && ai->CastSpell(BLOODTHIRST, pTarget))
                return;
            else if (ai->GetRageAmount() >= 20 && ai->CastSpell(CLEAVE, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && ai->CastSpell(HEROIC_STRIKE, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && !ai->HasAura(WAR_STOMP, pTarget) && !ai->HasAura(PIERCING_HOWL, pTarget) && !ai->HasAura(SHOCKWAVE, pTarget) && !ai->HasAura(CONCUSSION_BLOW, pTarget) && ai->CastSpell(CONCUSSION_BLOW, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && ai->CastSpell(SLAM, pTarget))
                return;
            else if (ai->GetRageAmount() >= 10 && ai->GetAttackerCount() >= 3 && !ai->HasAura(WAR_STOMP, pTarget) && !ai->HasAura(PIERCING_HOWL, pTarget) && !ai->HasAura(SHOCKWAVE, pTarget) && !ai->HasAura(CONCUSSION_BLOW, pTarget) && ai->CastSpell(PIERCING_HOWL, pTarget))
                return;
            else if (ai->GetRageAmount() >= 10 && pVictim != m_bot && ai->GetHealthPercent() > 25 && !ai->HasAura(MOCKING_BLOW, pTarget) && !ai->HasAura(CHALLENGING_SHOUT, pTarget) && ai->CastSpell(MOCKING_BLOW, pTarget))
                return;
            else if (ai->GetRageAmount() >= 5 && ai->CastSpell(OVERPOWER, pTarget))
                return;
            else if (ai->CastSpell(SUNDER_ARMOR, pTarget))
                return;
            else if (!ai->HasAura(SHATTERING_THROW, pTarget) && ai->CastSpell(SHATTERING_THROW, pTarget))
                return;
            else if (ai->CastSpell(HEROIC_THROW, pTarget))
                return;
            else if (m_bot->getRace() == RACE_TAUREN && !ai->HasAura(WAR_STOMP, pTarget) && !ai->HasAura(PIERCING_HOWL, pTarget) && !ai->HasAura(SHOCKWAVE, pTarget) && !ai->HasAura(CONCUSSION_BLOW, pTarget) && ai->CastSpell(WAR_STOMP, pTarget))
                return;
            else if (m_bot->getRace() == RACE_HUMAN && m_bot->hasUnitState(UNIT_STAT_STUNNED) || m_bot->HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) || m_bot->HasAuraType(SPELL_AURA_MOD_CHARM) && ai->CastSpell(EVERY_MAN_FOR_HIMSELF, m_bot))
                return;
            else if (m_bot->getRace() == RACE_UNDEAD_PLAYER && m_bot->HasAuraType(SPELL_AURA_MOD_FEAR) || m_bot->HasAuraType(SPELL_AURA_MOD_CHARM) && ai->CastSpell(WILL_OF_THE_FORSAKEN, m_bot))
                return;
            else if (m_bot->getRace() == RACE_DWARF && m_bot->HasAuraState(AURA_STATE_DEADLY_POISON) && ai->CastSpell(STONEFORM, m_bot))
                return;
            else if (m_bot->getRace() == RACE_GNOME && m_bot->hasUnitState(UNIT_STAT_STUNNED) || m_bot->HasAuraType(SPELL_AURA_MOD_DECREASE_SPEED) && ai->CastSpell(ESCAPE_ARTIST, m_bot))
                return;
            else if (m_bot->getRace() == RACE_NIGHTELF && pVictim == m_bot && ai->GetHealthPercent() < 25 && !ai->HasAura(SHADOWMELD, m_bot) && ai->CastSpell(SHADOWMELD, m_bot))
                return;
            else if (m_bot->getRace() == RACE_ORC && !ai->HasAura(BLOOD_FURY, m_bot) && ai->CastSpell(BLOOD_FURY, m_bot))
                return;
            else if (m_bot->getRace() == RACE_TROLL && !ai->HasAura(BERSERKING, m_bot) && ai->CastSpell(BERSERKING, m_bot))
                return;
            else if (m_bot->getRace() == RACE_DRAENEI && ai->GetHealthPercent() < 25 && !ai->HasAura(GIFT_OF_THE_NAARU, m_bot) && ai->CastSpell(GIFT_OF_THE_NAARU, m_bot))
                return;

            return;

        case WarriorDefensive:
            if (ai->GetRageAmount() >= 15 && !ai->HasAura(DISARM, pTarget) && ai->CastSpell(DISARM, pTarget))
                return;
            else if (ai->GetRageAmount() >= 15 && ai->CastSpell(SUNDER_ARMOR, pTarget))
                return;
            else if (ai->GetRageAmount() >= 5 && ai->CastSpell(REVENGE, pTarget))
                return;
            else if (!ai->HasAura(SHIELD_BLOCK, m_bot) && ai->CastSpell(SHIELD_BLOCK, m_bot))
                return;
            else if (!ai->HasAura(SHIELD_WALL, m_bot) && ai->CastSpell(SHIELD_WALL, m_bot))
                return;

            return;

        case WarriorBerserker:
            if (ai->GetRageAmount() >= 25 && ai->CastSpell(WHIRLWIND, pTarget))
                return;

            return;
    }
}

void PlayerbotWarriorAI::DoNonCombatActions()
{
    PlayerbotAI* ai = GetAI();
    if (!ai)
        return;

    Player* m_bot = GetPlayerBot();
    if (!m_bot)
        return;

    Player* m_master = ai->GetMaster();
    if (!m_master)
        return;

    // TODO (by Runsttren): check if shout aura bot has is casted by this bot,
    // otherwise cast other useful shout
    // If the bot is protect talented, she/he needs stamina not attack power.
    // With stance change can the shout change to.
    // Inserted line to battle shout ai->HasAura( COMMANDING_SHOUT, m_bot)
    // Natsukawa

    // we do have a useful shout, no rage coming but can cast bloodrage... do it
    if ((!ai->HasAura(COMMANDING_SHOUT, m_bot) || !ai->HasAura(BATTLE_SHOUT, m_bot)) && ai->GetRageAmount() < 10 && !ai->HasAura(BLOODRAGE, m_bot) && ai->CastSpell(BLOODRAGE, m_bot))
        return;
    // use commanding shout now
    else if (!ai->HasAura(COMMANDING_SHOUT, m_bot) && ai->CastSpell(COMMANDING_SHOUT, m_bot))
        return;
    // use battle shout
    else if (!ai->HasAura(BATTLE_SHOUT, m_bot) && !ai->HasAura(COMMANDING_SHOUT, m_bot) && ai->CastSpell(BATTLE_SHOUT, m_bot))
        return;
    // buff master with VIGILANCE
    else if (!ai->HasAura(VIGILANCE, m_master) && ai->CastSpell(VIGILANCE, m_master))
        return;

    if (m_bot->getStandState() != UNIT_STAND_STATE_STAND)
        m_bot->SetStandState(UNIT_STAND_STATE_STAND);

    Item* pItemFood = ai->FindFood();
    Item* pItemBandage = ai->FindBandage();

    // hp check
    if (pItemFood && ai->GetHealthPercent() < 30)
    {
        ai->UseItem(pItemFood);
        ai->SetIgnoreUpdateTime(30);
        return;
    }
    else if (!pItemFood && pItemBandage && !ai->HasAura(RECENTLY_BANDAGED, m_bot) && ai->GetHealthPercent() < 70)
    {
        ai->UseItem(pItemBandage);
        ai->SetIgnoreUpdateTime(8);
        return;
    }
    else if (!pItemFood && !pItemBandage && m_bot->getRace() == RACE_DRAENEI && !ai->HasAura(GIFT_OF_THE_NAARU, m_bot) && ai->GetHealthPercent() < 70 && ai->CastSpell(GIFT_OF_THE_NAARU, m_bot))
        return;
}
