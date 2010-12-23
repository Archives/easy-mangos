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

#ifndef _SCRIPTMGR_H
#define _SCRIPTMGR_H

#include "Common.h"
#include "Policies/Singleton.h"

enum eScriptCommand
{
    SCRIPT_COMMAND_TALK                     = 0,            // source = WorldObject, target = any/none, datalong (see enum ChatType for supported CHAT_TYPE_'s)
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius, datalong4 = language
                                                            // data_flags = flag_target_player_as_source    = 0x01
                                                            //              flag_original_source_as_target  = 0x02
                                                            //              flag_buddy_as_target            = 0x04
                                                            // dataint = text entry from db_script_string -table. dataint2-4 optional for random selected text.
    SCRIPT_COMMAND_EMOTE                    = 1,            // source = unit, datalong = emote_id
    SCRIPT_COMMAND_FIELD_SET                = 2,            // source = any, datalong = field_id, datalong2 = value
    SCRIPT_COMMAND_MOVE_TO                  = 3,            // source = Creature, datalong2 = time, x/y/z
    SCRIPT_COMMAND_FLAG_SET                 = 4,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_FLAG_REMOVE              = 5,            // source = any, datalong = field_id, datalong2 = bitmask
    SCRIPT_COMMAND_TELEPORT_TO              = 6,            // source or target with Player, datalong = map_id, x/y/z
    SCRIPT_COMMAND_QUEST_EXPLORED           = 7,            // one from source or target must be Player, another GO/Creature, datalong=quest_id, datalong2=distance or 0
    SCRIPT_COMMAND_KILL_CREDIT              = 8,            // source or target with Player, datalong = creature entry, datalong2 = bool (0=personal credit, 1=group credit)
    SCRIPT_COMMAND_RESPAWN_GAMEOBJECT       = 9,            // source = any (summoner), datalong=db_guid, datalong2=despawn_delay
    SCRIPT_COMMAND_TEMP_SUMMON_CREATURE     = 10,           // source = any (summoner), datalong=creature entry, datalong2=despawn_delay
    SCRIPT_COMMAND_OPEN_DOOR                = 11,           // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_CLOSE_DOOR               = 12,           // source = unit, datalong=db_guid, datalong2=reset_delay
    SCRIPT_COMMAND_ACTIVATE_OBJECT          = 13,           // source = unit, target=GO
    SCRIPT_COMMAND_REMOVE_AURA              = 14,           // source (datalong2!=0) or target (datalong==0) unit, datalong = spell_id
    SCRIPT_COMMAND_CAST_SPELL               = 15,           // source/target cast spell at target/source (script->datalong2: 0: s->t 1: s->s 2: t->t 3: t->s
    SCRIPT_COMMAND_PLAY_SOUND               = 16,           // source = any object, target=any/player, datalong (sound_id), datalong2 (bitmask: 0/1=anyone/target, 0/2=with distance dependent, so 1|2 = 3 is target with distance dependent)
    SCRIPT_COMMAND_CREATE_ITEM              = 17,           // source or target must be player, datalong = item entry, datalong2 = amount
    SCRIPT_COMMAND_DESPAWN_SELF             = 18,           // source or target must be creature, datalong = despawn delay
    SCRIPT_COMMAND_PLAY_MOVIE               = 19,           // target can only be a player, datalog = movie id
    SCRIPT_COMMAND_MOVEMENT                 = 20,           // source or target must be creature. datalong = MovementType (0:idle, 1:random or 2:waypoint)
                                                            // datalong2 = creature entry (searching for a buddy, closest to source), datalong3 = creature search radius
    SCRIPT_COMMAND_SET_ACTIVEOBJECT         = 21,           // source=any, target=creature
                                                            // datalong=bool 0=off, 1=on
                                                            // datalong2=creature entry, datalong3=search radius
    SCRIPT_COMMAND_SET_FACTION              = 22,           // source=any, target=creature
                                                            // datalong=factionId,
                                                            // datalong2=creature entry, datalong3=search radius
    SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL  = 23,           // source=any, target=creature
                                                            // datalong=creature entry/modelid (depend on data_flags)
                                                            // datalong2=creature entry, datalong3=search radius
                                                            // dataflags= 0x01 to use datalong value as modelid explicit
    SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL  = 24,           // source=any, target=creature
                                                            // datalong=creature entry/modelid (depend on data_flags)
                                                            // datalong2=creature entry, datalong3=search radius
                                                            // dataflags= 0x01 to use datalong value as modelid explicit
    SCRIPT_COMMAND_SET_RUN                  = 25,           // source=any, target=creature
                                                            // datalong= bool 0=off, 1=on
                                                            // datalong2=creature entry, datalong3=search radius
};

#define MAX_TEXT_ID 4                                       // used for SCRIPT_COMMAND_TALK

struct ScriptInfo
{
    uint32 id;
    uint32 delay;
    uint32 command;

    union
    {
        struct                                              // SCRIPT_COMMAND_TALK (0)
        {
            uint32 chatType;                                // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 language;                                // datalong4
            uint32 flags;                                   // data_flags
            int32  textId[MAX_TEXT_ID];                     // dataint to dataint4
        } talk;

        struct                                              // SCRIPT_COMMAND_EMOTE (1)
        {
            uint32 emoteId;                                 // datalong
        } emote;

        struct                                              // SCRIPT_COMMAND_FIELD_SET (2)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setField;

        struct                                              // SCRIPT_COMMAND_MOVE_TO (3)
        {
            uint32 unused1;                                 // datalong
            uint32 travelTime;                              // datalong2
        } moveTo;

        struct                                              // SCRIPT_COMMAND_FLAG_SET (4)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } setFlag;

        struct                                              // SCRIPT_COMMAND_FLAG_REMOVE (5)
        {
            uint32 fieldId;                                 // datalong
            uint32 fieldValue;                              // datalong2
        } removeFlag;

        struct                                              // SCRIPT_COMMAND_TELEPORT_TO (6)
        {
            uint32 mapId;                                   // datalong
        } teleportTo;

        struct                                              // SCRIPT_COMMAND_QUEST_EXPLORED (7)
        {
            uint32 questId;                                 // datalong
            uint32 distance;                                // datalong2
        } questExplored;

        struct                                              // SCRIPT_COMMAND_KILL_CREDIT (8)
        {
            uint32 creatureEntry;                           // datalong
            uint32 isGroupCredit;                           // datalong2
        } killCredit;

        struct                                              // SCRIPT_COMMAND_RESPAWN_GAMEOBJECT (9)
        {
            uint32 goGuid;                                  // datalong
            int32 despawnDelay;                             // datalong2
        } respawnGo;

        struct                                              // SCRIPT_COMMAND_TEMP_SUMMON_CREATURE (10)
        {
            uint32 creatureEntry;                           // datalong
            uint32 despawnDelay;                            // datalong2
            uint32 unused1;                                 // datalong3
            uint32 unused2;                                 // datalong4
            uint32 flags;                                   // data_flags
        } summonCreature;

        struct                                              // SCRIPT_COMMAND_OPEN_DOOR (11)
        {
            uint32 goGuid;                                  // datalong
            int32 resetDelay;                               // datalong2
        } openDoor;

        struct                                              // SCRIPT_COMMAND_CLOSE_DOOR (12)
        {
            uint32 goGuid;                                  // datalong
            int32 resetDelay;                               // datalong2
        } closeDoor;

                                                            // SCRIPT_COMMAND_ACTIVATE_OBJECT (13)

        struct                                              // SCRIPT_COMMAND_REMOVE_AURA (14)
        {
            uint32 spellId;                                 // datalong
            uint32 isSourceTarget;                          // datalong2
        } removeAura;

        struct                                              // SCRIPT_COMMAND_CAST_SPELL (15)
        {
            uint32 spellId;                                 // datalong
            uint32 flags;                                   // datalong2
        } castSpell;

        struct                                              // SCRIPT_COMMAND_PLAY_SOUND (16)
        {
            uint32 soundId;                                 // datalong
            uint32 flags;                                   // datalong2
        } playSound;

        struct                                              // SCRIPT_COMMAND_CREATE_ITEM (17)
        {
            uint32 itemEntry;                               // datalong
            uint32 amount;                                  // datalong2
        } createItem;

        struct                                              // SCRIPT_COMMAND_DESPAWN_SELF (18)
        {
            uint32 despawnDelay;                            // datalong
        } despawn;

        struct                                              // SCRIPT_COMMAND_PLAY_MOVIE (19)
        {
            uint32 movieId;                                 // datalong
        } playMovie;

        struct                                              // SCRIPT_COMMAND_MOVEMENT (20)
        {
            uint32 movementType;                            // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } movement;

        struct                                              // SCRIPT_COMMAND_SET_ACTIVEOBJECT (21)
        {
            uint32 activate;                                // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } activeObject;

        struct                                              // SCRIPT_COMMAND_SET_FACTION (22)
        {
            uint32 factionId;                               // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } faction;

        struct                                              // SCRIPT_COMMAND_MORPH_TO_ENTRY_OR_MODEL (23)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty1;                                  // datalong4
            uint32 flags;                                   // data_flags
        } morph;

        struct                                              // SCRIPT_COMMAND_MOUNT_TO_ENTRY_OR_MODEL (24)
        {
            uint32 creatureOrModelEntry;                    // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
            uint32 empty1;                                  // datalong4
            uint32 flags;                                   // data_flags
        } mount;

        struct                                              // SCRIPT_COMMAND_SET_RUN (25)
        {
            uint32 run;                                     // datalong
            uint32 creatureEntry;                           // datalong2
            uint32 searchRadius;                            // datalong3
        } run;

        struct
        {
            uint32 data[9];
        } raw;
    };

    float x;
    float y;
    float z;
    float o;

    // helpers
    uint32 GetGOGuid() const
    {
        switch(command)
        {
            case SCRIPT_COMMAND_RESPAWN_GAMEOBJECT: return respawnGo.goGuid;
            case SCRIPT_COMMAND_OPEN_DOOR: return openDoor.goGuid;
            case SCRIPT_COMMAND_CLOSE_DOOR: return closeDoor.goGuid;
            default: return 0;
        }
    }
};

typedef std::multimap<uint32, ScriptInfo> ScriptMap;
typedef std::map<uint32, ScriptMap > ScriptMapMap;
extern ScriptMapMap sQuestEndScripts;
extern ScriptMapMap sQuestStartScripts;
extern ScriptMapMap sSpellScripts;
extern ScriptMapMap sGameObjectScripts;
extern ScriptMapMap sEventScripts;
extern ScriptMapMap sGossipScripts;
extern ScriptMapMap sCreatureMovementScripts;

class ScriptMgr
{
public:
    ScriptMgr();
    ~ScriptMgr();

    typedef std::vector<std::string> ScriptNameMap;

    void LoadGameObjectScripts();
    void LoadQuestEndScripts();
    void LoadQuestStartScripts();
    void LoadEventScripts();
    void LoadSpellScripts();
    void LoadGossipScripts();
    void LoadCreatureMovementScripts();

    void LoadDbScriptStrings();

    void LoadScriptNames();
    void LoadAreaTriggerScripts();
    void LoadEventIdScripts();

    uint32 GetAreaTriggerScriptId(uint32 triggerId) const;
    uint32 GetEventIdScriptId(uint32 eventId) const;

    ScriptNameMap const& GetScriptNames() { return m_scriptNames; }
    const char* GetScriptName(uint32 id) const { return id < m_scriptNames.size() ? m_scriptNames[id].c_str() : ""; }
    uint32 GetScriptId(const char *name) const;
private:
    void LoadScripts(ScriptMapMap& scripts, const char* tablename);
    void CheckScriptTexts(ScriptMapMap const& scripts, std::set<int32>& ids);

    typedef UNORDERED_MAP<uint32, uint32> AreaTriggerScriptMap;
    typedef UNORDERED_MAP<uint32, uint32> EventIdScriptMap;

    AreaTriggerScriptMap    m_AreaTriggerScripts;
    EventIdScriptMap        m_EventIdScripts;

    ScriptNameMap           m_scriptNames;
};

#define sScriptMgr MaNGOS::Singleton<ScriptMgr>::Instance()

MANGOS_DLL_SPEC uint32 GetAreaTriggerScriptId(uint32 triggerId);
MANGOS_DLL_SPEC uint32 GetEventIdScriptId(uint32 eventId);
MANGOS_DLL_SPEC uint32 GetScriptId(const char *name);
MANGOS_DLL_SPEC ScriptMgr::ScriptNameMap const& GetScriptNames();

#endif
