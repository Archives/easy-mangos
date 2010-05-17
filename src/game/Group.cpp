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

#include "Common.h"
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Group.h"
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "BattleGround.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "MapInstanced.h"
#include "Util.h"
#include "LootMgr.h"
#include "PlayerbotMgr.h"

#define LOOT_ROLL_TIMEOUT  (1*MINUTE*IN_MILLISECONDS)

Group::Group() : m_Id(0), m_leaderGuid(0), m_mainTank(0), m_mainAssistant(0),  m_groupType(GROUPTYPE_NORMAL),
    m_dungeonDifficulty(REGULAR_DIFFICULTY), m_raidDifficulty(REGULAR_DIFFICULTY),
    m_bgGroup(NULL), m_lootMethod(FREE_FOR_ALL), m_looterGuid(0), m_lootThreshold(ITEM_QUALITY_UNCOMMON),
    m_subGroupsCounts(NULL)
{
    for (int i = 0; i < TARGETICONCOUNT; ++i)
        m_targetIcons[i] = 0;
}

Group::~Group()
{
    if(m_bgGroup)
    {
        DEBUG_LOG("Group::~Group: battleground group being deleted.");
        if(m_bgGroup->GetBgRaid(ALLIANCE) == this)
            m_bgGroup->SetBgRaid(ALLIANCE, NULL);
        else if(m_bgGroup->GetBgRaid(HORDE) == this)
            m_bgGroup->SetBgRaid(HORDE, NULL);
        else
            sLog.outError("Group::~Group: battleground group is not linked to the correct battleground.");
    }
    Rolls::iterator itr;
    while(!RollId.empty())
    {
        itr = RollId.begin();
        Roll *r = *itr;
        RollId.erase(itr);
        delete(r);
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some instance saves
    for(uint8 i = 0; i < MAX_DIFFICULTY; ++i)
        for(BoundInstancesMap::iterator itr2 = m_boundInstances[i].begin(); itr2 != m_boundInstances[i].end(); ++itr2)
            itr2->second.save->RemoveGroup(this);

    // Sub group counters clean up
    if (m_subGroupsCounts)
        delete[] m_subGroupsCounts;
}

bool Group::Create(const uint64 &guid, const char * name)
{
    m_leaderGuid = guid;
    m_leaderName = name;

    m_groupType  = isBGGroup() ? GROUPTYPE_BGRAID : GROUPTYPE_NORMAL;

    if (m_groupType & GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_looterGuid = guid;

    m_dungeonDifficulty = DUNGEON_DIFFICULTY_NORMAL;
    m_raidDifficulty = RAID_DIFFICULTY_10MAN_NORMAL;
    if(!isBGGroup())
    {
        m_Id = sObjectMgr.GenerateGroupId();

        Player *leader = sObjectMgr.GetPlayer(guid);
        if(leader)
        {
            m_dungeonDifficulty = leader->GetDungeonDifficulty();
            m_raidDifficulty = leader->GetRaidDifficulty();
        }

        Player::ConvertInstancesToGroup(leader, this, guid);

        // store group in database
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId ='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId ='%u'", m_Id);
        CharacterDatabase.PExecute("INSERT INTO groups (groupId,leaderGuid,mainTank,mainAssistant,lootMethod,looterGuid,lootThreshold,icon1,icon2,icon3,icon4,icon5,icon6,icon7,icon8,isRaid,difficulty,raiddifficulty) "
            "VALUES ('%u','%u','%u','%u','%u','%u','%u','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','%u','%u','%u')",
            m_Id, GUID_LOPART(m_leaderGuid), GUID_LOPART(m_mainTank), GUID_LOPART(m_mainAssistant), uint32(m_lootMethod),
            GUID_LOPART(m_looterGuid), uint32(m_lootThreshold), m_targetIcons[0], m_targetIcons[1], m_targetIcons[2], m_targetIcons[3], m_targetIcons[4], m_targetIcons[5], m_targetIcons[6], m_targetIcons[7], uint8(m_groupType), uint32(m_dungeonDifficulty), m_raidDifficulty);
    }

    if(!AddMember(guid, name))
        return false;

    if(!isBGGroup())
        CharacterDatabase.CommitTransaction();

    return true;
}

bool Group::LoadGroupFromDB(Field* fields)
{
    //                                          0         1              2           3           4              5      6      7      8      9      10     11     12     13      14          15              16          17
    // result = CharacterDatabase.Query("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, difficulty, raiddifficulty, leaderGuid, groupId FROM groups");

    m_Id = fields[17].GetUInt32();
    m_leaderGuid = MAKE_NEW_GUID(fields[16].GetUInt32(),0,HIGHGUID_PLAYER);

    // group leader not exist
    if(!sObjectMgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_groupType  = GroupType(fields[13].GetUInt8());

    if (m_groupType & GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    uint32 diff = fields[14].GetUInt8();
    if (diff >= MAX_DUNGEON_DIFFICULTY)
        diff = DUNGEON_DIFFICULTY_NORMAL;
    m_dungeonDifficulty = Difficulty(diff);

    uint32 r_diff = fields[15].GetUInt8();
    if (r_diff >= MAX_RAID_DIFFICULTY)
        r_diff = RAID_DIFFICULTY_10MAN_NORMAL;
    m_raidDifficulty = Difficulty(r_diff);

    m_mainTank = fields[0].GetUInt64();
    m_mainAssistant = fields[1].GetUInt64();
    m_lootMethod = (LootMethod)fields[2].GetUInt8();
    m_looterGuid = MAKE_NEW_GUID(fields[3].GetUInt32(), 0, HIGHGUID_PLAYER);
    m_lootThreshold = (ItemQualities)fields[4].GetUInt16();

    for(int i = 0; i < TARGETICONCOUNT; ++i)
        m_targetIcons[i] = fields[5+i].GetUInt64();

    return true;
}

bool Group::LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant)
{
    MemberSlot member;
    member.guid      = MAKE_NEW_GUID(guidLow, 0, HIGHGUID_PLAYER);

    // skip non-existed member
    if(!sObjectMgr.GetPlayerNameByGUID(member.guid, member.name))
        return false;

    member.group     = subgroup;
    member.assistant = assistant;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subgroup);

    return true;
}

void Group::ConvertToRaid()
{
    m_groupType = GroupType(m_groupType | GROUPTYPE_RAID);

    _initRaidSubGroupsCounter();

    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET isRaid = 1 WHERE groupId='%u'", m_Id);
    SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        if(Player* player = sObjectMgr.GetPlayer(citr->guid))
            player->UpdateForQuestWorldObjects();
}

bool Group::AddInvite(Player *player)
{
    if( !player || player->GetGroupInvite() )
        return false;
    Group* group = player->GetGroup();
    if( group && group->isBGGroup() )
        group = player->GetOriginalGroup();
    if( group )
        return false;

    RemoveInvite(player);

    m_invitees.insert(player);

    player->SetGroupInvite(this);

    return true;
}

bool Group::AddLeaderInvite(Player *player)
{
    if(!AddInvite(player))
        return false;

    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    return true;
}

uint32 Group::RemoveInvite(Player *player)
{
    m_invitees.erase(player);

    player->SetGroupInvite(NULL);
    return GetMembersCount();
}

void Group::RemoveAllInvites()
{
    for(InvitesList::iterator itr = m_invitees.begin(); itr!=m_invitees.end(); ++itr)
        (*itr)->SetGroupInvite(NULL);

    m_invitees.clear();
}

Player* Group::GetInvited(const uint64& guid) const
{
    for(InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if((*itr)->GetGUID() == guid)
            return (*itr);
    }
    return NULL;
}

Player* Group::GetInvited(const std::string& name) const
{
    for(InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if((*itr)->GetName() == name)
            return (*itr);
    }
    return NULL;
}

bool Group::AddMember(const uint64 &guid, const char* name)
{
    if(!_addMember(guid, name))
        return false;

    SendUpdate();

    Player *player = sObjectMgr.GetPlayer(guid);
    if(player)
    {
        if(!IsLeader(player->GetGUID()) && !isBGGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid/heroic instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN,false);
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN,true);

            if (player->getLevel() >= LEVELREQUIREMENT_HEROIC)
            {
                if (player->GetDungeonDifficulty() != GetDungeonDifficulty())
                {
                    player->SetDungeonDifficulty(GetDungeonDifficulty());
                    player->SendDungeonDifficulty(true);
                }
                if (player->GetRaidDifficulty() != GetRaidDifficulty())
                {
                    player->SetRaidDifficulty(GetRaidDifficulty());
                    player->SendRaidDifficulty(true);
                }
            }
			// Group Interfactions interactions (test)
			if(sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
			{
				Group *group = player->GetGroup();
				if(Player *leader = sObjectMgr.GetPlayer(group->GetLeaderGUID()))
				{
					player->setFactionForRace(leader->getRace());
					sLog.outDebug( "WORLD: Group Interfaction Interactions - Faction changed (AddMember)" );
				}
			}			
        }
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid membership
        if(isRaidGroup())
            player->UpdateForQuestWorldObjects();
    }

    return true;
}

uint32 Group::RemoveMember(const uint64 &guid, const uint8 &method)
{
    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if(GetMembersCount() > uint32(isBGGroup() ? 1 : 2))           // in BG group case allow 1 members group
    {
        bool leaderChanged = _removeMember(guid);

        if(Player *player = sObjectMgr.GetPlayer( guid ))
        {
            // quest related GO state dependent from raid membership
            if(isRaidGroup())
                player->UpdateForQuestWorldObjects();

            WorldPacket data;

            if(method == 1)
            {
                data.Initialize( SMSG_GROUP_UNINVITE, 0 );
                player->GetSession()->SendPacket( &data );
            }

            //we already removed player from group and in player->GetGroup() is his original group!
            if( Group* group = player->GetGroup() )
            {
                group->SendUpdate();
            }
            else
            {
                data.Initialize(SMSG_GROUP_LIST, 1+1+1+1+8+4+4+8);
                data << uint8(0x10) << uint8(0) << uint8(0) << uint8(0);
                data << uint64(0) << uint32(0) << uint32(0) << uint64(0);
                player->GetSession()->SendPacket(&data);
            }
			// Restore original faction if needed
			if(sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
			{
				player->setFactionForRace(player->getRace());
				sLog.outDebug( "WORLD: Group Interfaction Interactions - Restore original faction (RemoveMember)" );
			}
            _homebindIfInstance(player);
        }

        if(leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_memberSlots.front().name.size()+1));
            data << m_memberSlots.front().name;
            BroadcastPacket(&data, true);
        }

        SendUpdate();
    }
    // if group before remove <= 2 disband it
    else
        Disband(true);

    return m_memberSlots.size();
}

void Group::ChangeLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);

    if(slot == m_memberSlots.end())
        return;

    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, slot->name.size()+1);
    data << slot->name;
    BroadcastPacket(&data, true);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player *player;

    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if(!player)
            continue;

        //we cannot call _removeMember because it would invalidate member iterator
        //if we are removing player from battleground raid
        if( isBGGroup() )
            player->RemoveFromBattleGroundRaid();
        else
        {
            //we can remove player who is in battleground from his original group
            if( player->GetOriginalGroup() == this )
                player->SetOriginalGroup(NULL);
            else
                player->SetGroup(NULL);
			// Restore original faction if needed
			if(sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
			{
				player->setFactionForRace(player->getRace());
				sLog.outDebug( "WORLD: Group Interfaction Interactions - Restore original faction (Disband)" );
			}
        }

        // quest related GO state dependent from raid membership
        if(isRaidGroup())
            player->UpdateForQuestWorldObjects();

        if(!player->GetSession())
            continue;

        WorldPacket data;
        if(!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
        }

        //we already removed player from group and in player->GetGroup() is his original group, send update
        if( Group* group = player->GetGroup() )
        {
            group->SendUpdate();
        }
        else
        {
            data.Initialize(SMSG_GROUP_LIST, 1+1+1+1+8+4+4+8);
            data << uint8(0x10) << uint8(0) << uint8(0) << uint8(0);
            data << uint64(0) << uint32(0) << uint32(0) << uint64(0);
            player->GetSession()->SendPacket(&data);
        }

        _homebindIfInstance(player);
    }
    RollId.clear();
    m_memberSlots.clear();

    RemoveAllInvites();

    if(!isBGGroup())
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId='%u'", m_Id);
        CharacterDatabase.CommitTransaction();
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, false, NULL);
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, true, NULL);
    }

    m_leaderGuid = 0;
    m_leaderName = "";
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

void Group::SendLootStartRoll(uint32 CountDown, uint32 mapid, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8+4+4+4+4+4+4+1));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(mapid);                                  // 3.3.3 mapid
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // item random property ID
    data << uint32(r.itemCount);                            // items in stack
    data << uint32(CountDown);                              // the countdown time to choose "need" or "greed"
    data << uint8(ALL_ROLL_VOTE_MASK);                      // roll type mask, allowed choises

    for (Roll::PlayerVote::const_iterator itr = r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRoll(ObjectGuid const& targetGuid, uint8 rollNumber, uint8 rollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8+4+8+4+4+4+1+1+1));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // unknown, maybe amount of players, or item slot in loot
    data << targetGuid;
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint8(rollNumber);                              // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << uint8(rollType);                                // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll
    data << uint8(0);                                       // auto pass on loot

    for( Roll::PlayerVote::const_iterator itr = r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRollWon(ObjectGuid const& targetGuid, uint8 rollNumber, RollVote rollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8+4+4+4+4+8+1+1));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomSuffix);                     // randomSuffix
    data << uint32(r.itemRandomPropId);                     // Item random property
    data << targetGuid;                                     // guid of the player who won.
    data << uint8(rollNumber);                              // rollnumber related to SMSG_LOOT_ROLL
    data << uint8(rollType);                                // Rolltype related to SMSG_LOOT_ROLL

    for( Roll::PlayerVote::const_iterator itr = r.playerVote.begin(); itr != r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootAllPassed(Roll const& r)
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8+4+4+4+4));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // The itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint32(r.itemRandomSuffix);                     // Item random suffix ID

    for( Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr!=r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != ROLL_NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::GroupLoot(WorldObject* object, Loot *loot)
{
    for(uint8 itemSlot = 0; itemSlot < loot->items.size(); ++itemSlot)
    {
        LootItem& lootItem = loot->items[itemSlot];
        ItemPrototype const *itemProto = ObjectMgr::GetItemPrototype(lootItem.itemid);
        if (!itemProto)
        {
            DEBUG_LOG("Group::GroupLoot: missing item prototype for item with id: %d", lootItem.itemid);
            continue;
        }

        //roll for over-threshold item if it's one-player loot
        if (itemProto->Quality >= uint32(m_lootThreshold) && !lootItem.freeforall)
            StartLootRool(object,loot,itemSlot,false);
        else
            lootItem.is_underthreshold = 1;
    }
}

void Group::NeedBeforeGreed(WorldObject* object, Loot *loot)
{
    for(uint8 itemSlot = 0; itemSlot < loot->items.size(); ++itemSlot)
    {
        LootItem& lootItem = loot->items[itemSlot];
        ItemPrototype const *itemProto = ObjectMgr::GetItemPrototype(lootItem.itemid);
        if (!itemProto)
        {
            DEBUG_LOG("Group::NeedBeforeGreed: missing item prototype for item with id: %d", lootItem.itemid);
            continue;
        }

        //only roll for one-player items, not for ones everyone can get
        if (itemProto->Quality >= uint32(m_lootThreshold) && !lootItem.freeforall)
            StartLootRool(object, loot, itemSlot, true);
        else
            lootItem.is_underthreshold = 1;
    }
}

void Group::MasterLoot(WorldObject* object, Loot* /*loot*/)
{
    uint32 real_count = 0;

    WorldPacket data(SMSG_LOOT_MASTER_LIST, 330);
    data << uint8(GetMembersCount());

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (!looter->IsInWorld())
            continue;

        if (looter->IsWithinDist(object, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
        {
            data << uint64(looter->GetGUID());
            ++real_count;
        }
    }

    data.put<uint8>(0, real_count);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (looter->IsWithinDist(object, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            looter->GetSession()->SendPacket(&data);
    }
}

void Group::CountRollVote(ObjectGuid const& playerGUID, ObjectGuid const& lootedTarget, uint32 itemSlot, RollVote choise)
{
    Rolls::iterator rollI = RollId.begin();
    for (; rollI != RollId.end(); ++rollI)
        if ((*rollI)->isValid() && (*rollI)->lootedTargetGUID == lootedTarget && (*rollI)->itemSlot == itemSlot)
            break;

    if (rollI == RollId.end())
        return;

    CountRollVote(playerGUID, rollI, choise);
}

bool Group::CountRollVote(ObjectGuid const& playerGUID, Rolls::iterator& rollI, RollVote choise)
{
    Roll* roll = *rollI;

    Roll::PlayerVote::iterator itr = roll->playerVote.find(playerGUID.GetRawValue());
    // this condition means that player joins to the party after roll begins
    if (itr == roll->playerVote.end())
        return true;                                        // result used for need iterator ++, so avoid for end of list

    if (roll->getLoot())
        if (roll->getLoot()->items.empty())
            return false;

    switch (choise)
    {
        case ROLL_PASS:                                     // Player choose pass
        {
            SendLootRoll(playerGUID, 128, 128, *roll);
            ++roll->totalPass;
            itr->second = ROLL_PASS;
            break;
        }
        case ROLL_NEED:                                     // player choose Need
        {
            SendLootRoll(playerGUID, 0, 0, *roll);
            ++roll->totalNeed;
            itr->second = ROLL_NEED;
            break;
        }
        case ROLL_GREED:                                    // player choose Greed
        {
            SendLootRoll(playerGUID, 128, ROLL_GREED, *roll);
            ++roll->totalGreed;
            itr->second = ROLL_GREED;
            break;
        }
        case ROLL_DISENCHANT:                               // player choose Disenchant
        {
            SendLootRoll(playerGUID, 128, ROLL_DISENCHANT, *roll);
            ++roll->totalGreed;
            itr->second = ROLL_DISENCHANT;
            break;
        }
        default:                                            // Roll removed case
            break;
    }

    if (roll->totalPass + roll->totalNeed + roll->totalGreed >= roll->totalPlayersRolling)
    {
        CountTheRoll(rollI);
        return true;
    }

    return false;
}

void Group::StartLootRool(WorldObject* lootTarget, Loot* loot, uint8 itemSlot, bool skipIfCanNotUse)
{
    if (itemSlot >= loot->items.size())
        return;

    LootItem const& lootItem =  loot->items[itemSlot];

    ItemPrototype const* item = ObjectMgr::GetItemPrototype(lootItem.itemid);

    Roll* r = new Roll(lootTarget->GetGUID(), lootItem);

    //a vector is filled with only near party members
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *playerToRoll = itr->getSource();
        if(!playerToRoll || !playerToRoll->GetSession())
            continue;

        if ((!skipIfCanNotUse || playerToRoll->CanUseItem(item)) && lootItem.AllowedForPlayer(playerToRoll) )
        {
            if (playerToRoll->IsWithinDist(lootTarget, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            {
                r->playerVote[playerToRoll->GetGUID()] = ROLL_NOT_EMITED_YET;
                ++r->totalPlayersRolling;
            }
        }
    }

    if (r->totalPlayersRolling > 0)                 // has looters
    {
        r->setLoot(loot);
        r->itemSlot = itemSlot;

        if (r->totalPlayersRolling == 1)            // single looter
            r->playerVote.begin()->second = ROLL_NEED;
        else
        {
            SendLootStartRoll(LOOT_ROLL_TIMEOUT, lootTarget->GetMapId(), *r);
            loot->items[itemSlot].is_blocked = true;
            if(lootTarget->GetTypeId() == TYPEID_UNIT)
                ((Creature*)lootTarget)->StartGroupLoot(this,LOOT_ROLL_TIMEOUT);
            else if(lootTarget->GetTypeId() == TYPEID_GAMEOBJECT)
            {
                ((GameObject*)lootTarget)->m_groupLootTimer = 60000;
                ((GameObject*)lootTarget)->m_groupLootId = GetId();  
            }
        }

        RollId.push_back(r);
    }
    else                                            // no looters??
        delete r;
}

// called when roll timer expires
void Group::EndRoll()
{
    while(!RollId.empty())
    {
        //need more testing here, if rolls disappear
        Rolls::iterator itr = RollId.begin();
        CountTheRoll(itr);                                  //i don't have to edit player votes, who didn't vote ... he will pass
    }
}

void Group::CountTheRoll(Rolls::iterator& rollI)
{
    Roll* roll = *rollI;
    if(!roll->isValid())                                    // is loot already deleted ?
    {
        rollI = RollId.erase(rollI);
        delete roll;
        return;
    }

    //end of the roll
    if (roll->totalNeed > 0)
    {
        if(!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            ObjectGuid maxguid  = (*roll->playerVote.begin()).first;
            Player *player;

            for( Roll::PlayerVote::const_iterator itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != ROLL_NEED)
                    continue;

                uint8 randomN = urand(1, 99);
                SendLootRoll(itr->first, randomN, ROLL_NEED, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(maxguid, maxresul, ROLL_NEED, *roll);
            player = sObjectMgr.GetPlayer(maxguid);

            if(player && player->GetSession())
            {
                player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_ROLL_NEED_ON_LOOT, roll->itemid, maxresul);

                ItemPosCountVec dest;
                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
                uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count );
                if ( msg == EQUIP_ERR_OK )
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;
                    player->StoreNewItem( dest, roll->itemid, true, item->randomPropertyId);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError( msg, NULL, NULL, roll->itemid );
                }
            }
        }
    }
    else if (roll->totalGreed > 0)
    {
        if(!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            uint64 maxguid = (*roll->playerVote.begin()).first;
            Player *player;
            RollVote rollvote = ROLL_PASS;                  //Fixed: Using uninitialized memory 'rollvote'

            Roll::PlayerVote::iterator itr;
            for (itr = roll->playerVote.begin(); itr != roll->playerVote.end(); ++itr)
            {
                if (itr->second != ROLL_GREED && itr->second != ROLL_DISENCHANT)
                    continue;

                uint8 randomN = urand(1, 99);
                SendLootRoll(itr->first, randomN, itr->second, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                    rollvote = itr->second;
                }
            }
            SendLootRollWon(maxguid, maxresul, rollvote, *roll);
            player = sObjectMgr.GetPlayer(maxguid);

            if(player && player->GetSession())
            {
                player->GetAchievementMgr().UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_ROLL_GREED_ON_LOOT, roll->itemid, maxresul);

                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);

                if(rollvote == ROLL_GREED)
                {
                    ItemPosCountVec dest;
                    uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count );
                    if ( msg == EQUIP_ERR_OK )
                    {
                        item->is_looted = true;
                        roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                        --roll->getLoot()->unlootedCount;
                        player->StoreNewItem( dest, roll->itemid, true, item->randomPropertyId);
                    }
                    else
                    {
                        item->is_blocked = false;
                        player->SendEquipError( msg, NULL, NULL, roll->itemid );
                    }
                }
                else if(rollvote == ROLL_DISENCHANT)
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;

                    ItemPrototype const *pProto = ObjectMgr::GetItemPrototype(roll->itemid);
                    player->AutoStoreLoot(pProto->DisenchantID, LootTemplates_Disenchant, true);
                }
            }
        }
    }
    else
    {
        SendLootAllPassed(*roll);
        LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
        if(item) item->is_blocked = false;
    }
    rollI = RollId.erase(rollI);
    delete roll;
}

void Group::SetTargetIcon(uint8 id, uint64 whoGuid, uint64 targetGuid)
{
    if(id >= TARGETICONCOUNT)
        return;

    // clean other icons
    if( targetGuid != 0 )
        for(int i = 0; i < TARGETICONCOUNT; ++i)
            if( m_targetIcons[i] == targetGuid )
                SetTargetIcon(i, 0, 0);

    m_targetIcons[id] = targetGuid;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1+8+1+8));
    data << uint8(0);                                       // set targets
    data << uint64(whoGuid);
    data << uint8(id);
    data << uint64(targetGuid);
    BroadcastPacket(&data, true);
}

void Group::GetDataForXPAtKill(Unit const* victim, uint32& count,uint32& sum_level, Player* & member_with_max_level, Player* & not_gray_member_with_max_level)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if(!member || !member->isAlive())                   // only for alive
            continue;

        if(!member->IsAtGroupRewardDistance(victim))        // at req. distance
            continue;

        ++count;
        sum_level += member->getLevel();
        if(!member_with_max_level || member_with_max_level->getLevel() < member->getLevel())
            member_with_max_level = member;

        uint32 gray_level = MaNGOS::XP::GetGrayLevel(member->getLevel());
        if( victim->getLevel() > gray_level && (!not_gray_member_with_max_level
           || not_gray_member_with_max_level->getLevel() < member->getLevel()))
            not_gray_member_with_max_level = member;
    }
}

void Group::SendTargetIconList(WorldSession *session)
{
    if(!session)
        return;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1+TARGETICONCOUNT*9));
    data << uint8(1);                                       // list targets

    for(int i = 0; i < TARGETICONCOUNT; ++i)
    {
        if(m_targetIcons[i] == 0)
            continue;

        data << uint8(i);
        data << uint64(m_targetIcons[i]);
    }

    session->SendPacket(&data);
}

void Group::SendUpdate()
{
    Player *player;

    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if(!player || !player->GetSession() || player->GetGroup() != this )
            continue;
                                                            // guess size
        WorldPacket data(SMSG_GROUP_LIST, (1+1+1+1+8+4+GetMembersCount()*20));
        data << uint8(m_groupType);                         // group type (flags in 3.3)
        data << uint8(citr->group);                         // groupid
        data << uint8(citr->assistant ? 0x01 : 0x00);       // 0x2 main assist, 0x4 main tank
        data << uint8(isBGGroup() ? 1 : 0);                 // 2.0.x, isBattleGroundGroup?
        if(m_groupType & GROUPTYPE_LFD)
        {
            data << uint8(0);
            data << uint32(0);
        }
        data << uint64(0x50000000FFFFFFFELL);               // related to voice chat?
        data << uint32(0);                                  // 3.3, this value increments every time SMSG_GROUP_LIST is sent
        data << uint32(GetMembersCount()-1);
        for(member_citerator citr2 = m_memberSlots.begin(); citr2 != m_memberSlots.end(); ++citr2)
        {
            if(citr->guid == citr2->guid)
                continue;
            Player* member = sObjectMgr.GetPlayer(citr2->guid);
            uint8 onlineState = (member) ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
            onlineState = onlineState | ((isBGGroup()) ? MEMBER_STATUS_PVP : 0);

            data << citr2->name;
            data << uint64(citr2->guid);
                                                            // online-state
            data << uint8(onlineState);
            data << uint8(citr2->group);                    // groupid
            data << uint8(citr2->assistant?0x01:0);         // 0x2 main assist, 0x4 main tank
            data << uint8(0);                               // 3.3, role?
        }

        data << uint64(m_leaderGuid);                       // leader guid
        if(GetMembersCount()-1)
        {
            data << uint8(m_lootMethod);                    // loot method
            data << uint64(m_looterGuid);                   // looter guid
            data << uint8(m_lootThreshold);                 // loot threshold
            data << uint8(m_dungeonDifficulty);             // Dungeon Difficulty
            data << uint8(m_raidDifficulty);                // Raid Difficulty
            data << uint8(0);                               // 3.3, dynamic difficulty?
        }
        player->GetSession()->SendPacket( &data );
    }
}

void Group::UpdatePlayerOutOfRange(Player* pPlayer)
{
    if(!pPlayer || !pPlayer->IsInWorld())
        return;

    Player *player;
    WorldPacket data;
    pPlayer->GetSession()->BuildPartyMemberStatsChangedPacket(pPlayer, &data);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        player = itr->getSource();
        if (player && player != pPlayer && !player->HaveAtClient(pPlayer))
            player->GetSession()->SendPacket(&data);
    }
}

void Group::BroadcastPacket(WorldPacket *packet, bool ignorePlayersInBGRaid, int group, uint64 ignore)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if(!pl || (ignore != 0 && pl->GetGUID() == ignore) || (ignorePlayersInBGRaid && pl->GetGroup() != this) )
            continue;

        if (pl->GetSession() && (group == -1 || itr->getSubGroup() == group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket *packet)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if(pl && pl->GetSession())
            if(IsLeader(pl->GetGUID()) || IsAssistant(pl->GetGUID()))
                pl->GetSession()->SendPacket(packet);
    }
}

void Group::OfflineReadyCheck()
{
    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player *pl = sObjectMgr.GetPlayer(citr->guid);
        if (!pl || !pl->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
            data << uint64(citr->guid);
            data << uint8(0);
            BroadcastReadyCheck(&data);
        }
    }
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; groupid < MAXRAIDSIZE / MAXGROUPSIZE; ++groupid)
        {
            if (m_subGroupsCounts[groupid] < MAXGROUPSIZE)
            {
                groupFound = true;
                break;
            }
        }
        // We are raid group and no one slot is free
        if (!groupFound)
            return false;
    }

    return _addMember(guid, name, isAssistant, groupid);
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group)
{
    if(IsFull())
        return false;

    if(!guid)
        return false;

    Player *player = sObjectMgr.GetPlayer(guid);

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(group);

    if(player)
    {
        player->SetGroupInvite(NULL);
        //if player is in group and he is being added to BG raid group, then call SetBattleGroundRaid()
        if( player->GetGroup() && isBGGroup() )
            player->SetBattleGroundRaid(this, group);
        //if player is in bg raid and we are adding him to normal group, then call SetOriginalGroup()
        else if ( player->GetGroup() )
            player->SetOriginalGroup(this, group);
        //if player is not in group, then call set group
        else
            player->SetGroup(this, group);
        // if the same group invites the player back, cancel the homebind timer
        InstanceGroupBind *bind = GetBoundInstance(player);
        if(bind && bind->save->GetInstanceId() == player->GetInstanceId())
            player->m_InstanceValid = true;
    }

    if(!isRaidGroup())                                      // reset targetIcons for non-raid-groups
    {
        for(int i = 0; i < TARGETICONCOUNT; ++i)
            m_targetIcons[i] = 0;
    }

    if(!isBGGroup())
    {
        // insert into group table
        CharacterDatabase.PExecute("INSERT INTO group_member(groupId,memberGuid,assistant,subgroup) VALUES('%u','%u','%u','%u')", m_Id, GUID_LOPART(member.guid), ((member.assistant==1)?1:0), member.group);
    }

    return true;
}

bool Group::_removeMember(const uint64 &guid)
{
    Player *player = sObjectMgr.GetPlayer(guid);
    if (player)
    {
        //if we are removing player from battleground raid
        if( isBGGroup() )
            player->RemoveFromBattleGroundRaid();
        else
        {
            //we can remove player who is in battleground from his original group
            if( player->GetOriginalGroup() == this )
                player->SetOriginalGroup(NULL);
            else
                player->SetGroup(NULL);
        }
    }

    _removeRolls(guid);

    member_witerator slot = _getMemberWSlot(guid);
    if (slot != m_memberSlots.end())
    {
        SubGroupCounterDecrease(slot->group);

        m_memberSlots.erase(slot);
    }

    if(!isBGGroup())
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid='%u'", GUID_LOPART(guid));

    if(m_leaderGuid == guid)                                // leader was removed
    {
        if(GetMembersCount() > 0)
            _setLeader(m_memberSlots.front().guid);
        return true;
    }

    return false;
}

void Group::_setLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot == m_memberSlots.end())
        return;

    if(!isBGGroup())
    {
        // TODO: set a time limit to have this function run rarely cause it can be slow
        CharacterDatabase.BeginTransaction();

        // update the group's bound instances when changing leaders

        // remove all permanent binds from the group
        // in the DB also remove solo binds that will be replaced with permbinds
        // from the new leader
        CharacterDatabase.PExecute(
            "DELETE FROM group_instance WHERE leaderguid='%u' AND (permanent = 1 OR "
            "instance IN (SELECT instance FROM character_instance WHERE guid = '%u')"
            ")", GUID_LOPART(m_leaderGuid), GUID_LOPART(slot->guid)
        );

        Player *player = sObjectMgr.GetPlayer(slot->guid);
        if(player)
        {
            for(uint8 i = 0; i < MAX_DIFFICULTY; ++i)
            {
                for(BoundInstancesMap::iterator itr = m_boundInstances[i].begin(); itr != m_boundInstances[i].end();)
                {
                    if(itr->second.perm)
                    {
                        itr->second.save->RemoveGroup(this);
                        m_boundInstances[i].erase(itr++);
                    }
                    else
                        ++itr;
                }
            }
        }

        // update the group's solo binds to the new leader
        CharacterDatabase.PExecute("UPDATE group_instance SET leaderGuid='%u' WHERE leaderGuid = '%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));

        // copy the permanent binds from the new leader to the group
        // overwriting the solo binds with permanent ones if necessary
        // in the DB those have been deleted already
        Player::ConvertInstancesToGroup(player, this, slot->guid);

        // update the group leader
        CharacterDatabase.PExecute("UPDATE groups SET leaderGuid='%u' WHERE groupId='%u'", GUID_LOPART(slot->guid), m_Id);
        CharacterDatabase.CommitTransaction();
    }

    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
}

void Group::_removeRolls(const uint64 &guid)
{
    for (Rolls::iterator it = RollId.begin(); it != RollId.end(); )
    {
        Roll* roll = *it;
        Roll::PlayerVote::iterator itr2 = roll->playerVote.find(guid);
        if(itr2 == roll->playerVote.end())
        {
            ++it;
            continue;
        }

        if (itr2->second == ROLL_GREED || itr2->second == ROLL_DISENCHANT)
            --roll->totalGreed;
        if (itr2->second == ROLL_NEED)
            --roll->totalNeed;
        if (itr2->second == ROLL_PASS)
            --roll->totalPass;
        if (itr2->second != ROLL_NOT_VALID)
            --roll->totalPlayersRolling;

        roll->playerVote.erase(itr2);

        if (!CountRollVote(guid, it, ROLL_NOT_EMITED_YET))
            ++it;
    }
}

bool Group::_setMembersGroup(const uint64 &guid, const uint8 &group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot == m_memberSlots.end())
        return false;

    slot->group = group;

    SubGroupCounterIncrease(group);

    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET subgroup='%u' WHERE memberGuid='%u'", group, GUID_LOPART(guid));

    return true;
}

bool Group::_setAssistantFlag(const uint64 &guid, const bool &state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot == m_memberSlots.end())
        return false;

    slot->assistant = state;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET assistant='%u' WHERE memberGuid='%u'", (state==true)?1:0, GUID_LOPART(guid));
    return true;
}

bool Group::_setMainTank(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot == m_memberSlots.end())
        return false;

    if(m_mainAssistant == guid)
        _setMainAssistant(0);
    m_mainTank = guid;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainTank='%u' WHERE groupId='%u'", GUID_LOPART(m_mainTank), m_Id);
    return true;
}

bool Group::_setMainAssistant(const uint64 &guid)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot == m_memberSlots.end())
        return false;

    if(m_mainTank == guid)
        _setMainTank(0);
    m_mainAssistant = guid;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainAssistant='%u' WHERE groupId='%u'", GUID_LOPART(m_mainAssistant), m_Id);
    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if(!member1 || !member2)
        return false;
    if (member1->GetGroup() != this || member2->GetGroup() != this)
        return false;
    else
        return member1->GetSubGroup() == member2->GetSubGroup();
}

// allows setting subgroup for offline members
void Group::ChangeMembersGroup(const uint64 &guid, const uint8 &group)
{
    if(!isRaidGroup())
        return;
    Player *player = sObjectMgr.GetPlayer(guid);

    if (!player)
    {
        uint8 prevSubGroup;
        prevSubGroup = GetMemberGroup(guid);

        SubGroupCounterDecrease(prevSubGroup);

        if(_setMembersGroup(guid, group))
            SendUpdate();
    }
    else
        // This methods handles itself groupcounter decrease
        ChangeMembersGroup(player, group);
}

// only for online members
void Group::ChangeMembersGroup(Player *player, const uint8 &group)
{
    if(!player || !isRaidGroup())
        return;
    if(_setMembersGroup(player->GetGUID(), group))
    {
        uint8 prevSubGroup = player->GetSubGroup();
        if( player->GetGroup() == this )
            player->GetGroupRef().setSubGroup(group);
        //if player is in BG raid, it is possible that he is also in normal raid - and that normal raid is stored in m_originalGroup reference
        else
        {
            prevSubGroup = player->GetOriginalSubGroup();
            player->GetOriginalGroupRef().setSubGroup(group);
        }
        SubGroupCounterDecrease(prevSubGroup);

        SendUpdate();
    }
}

void Group::UpdateLooterGuid( WorldObject* object, bool ifneed )
{
    switch (GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
            return;
        default:
            // round robin style looting applies for all low
            // quality items in each loot method except free for all and master loot
            break;
    }

    member_citerator guid_itr = _getMemberCSlot(GetLooterGuid());
    if(guid_itr != m_memberSlots.end())
    {
        if(ifneed)
        {
            // not update if only update if need and ok
            Player* looter = ObjectAccessor::FindPlayer(guid_itr->guid);
            if(looter && looter->IsWithinDist(object, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                return;
        }
        ++guid_itr;
    }

    // search next after current
    if(guid_itr != m_memberSlots.end())
    {
        for(member_citerator itr = guid_itr; itr != m_memberSlots.end(); ++itr)
        {
            if(Player* pl = ObjectAccessor::FindPlayer(itr->guid))
            {
                if (pl->IsWithinDist(object, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                {
                    bool refresh = pl->GetLootGUID() == object->GetGUID();

                    //if(refresh)                             // update loot for new looter
                    //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                    SetLooterGuid(pl->GetGUID());
                    SendUpdate();
                    if(refresh)                             // update loot for new looter
                        pl->SendLoot(object->GetGUID(), LOOT_CORPSE);
                    return;
                }
            }
        }
    }

    // search from start
    for(member_citerator itr = m_memberSlots.begin(); itr != guid_itr; ++itr)
    {
        if(Player* pl = ObjectAccessor::FindPlayer(itr->guid))
        {
            if (pl->IsWithinDist(object, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            {
                bool refresh = pl->GetLootGUID()==object->GetGUID();

                //if(refresh)                               // update loot for new looter
                //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                SetLooterGuid(pl->GetGUID());
                SendUpdate();
                if(refresh)                                 // update loot for new looter
                    pl->SendLoot(object->GetGUID(), LOOT_CORPSE);
                return;
            }
        }
    }

    SetLooterGuid(0);
    SendUpdate();
}

GroupJoinBattlegroundResult Group::CanJoinBattleGroundQueue(BattleGround const* bgOrTemplate, BattleGroundQueueTypeId bgQueueTypeId, uint32 MinPlayerCount, uint32 MaxPlayerCount, bool isRated, uint32 arenaSlot)
{
    BattlemasterListEntry const* bgEntry = sBattlemasterListStore.LookupEntry(bgOrTemplate->GetTypeID());
    if(!bgEntry)
        return ERR_GROUP_JOIN_BATTLEGROUND_FAIL;            // shouldn't happen

    // check for min / max count
    uint32 memberscount = GetMembersCount();

    // only check for MinPlayerCount since MinPlayerCount == MaxPlayerCount for arenas...
    if(bgOrTemplate->isArena() && memberscount != MinPlayerCount)
        return ERR_ARENA_TEAM_PARTY_SIZE;

    if(memberscount > bgEntry->maxGroupSize)                // no MinPlayerCount for battlegrounds
        return ERR_BATTLEGROUND_NONE;                       // ERR_GROUP_JOIN_BATTLEGROUND_TOO_MANY handled on client side

    // get a player as reference, to compare other players' stats to (arena team id, queue id based on level, etc.)
    Player * reference = GetFirstMember()->getSource();
    // no reference found, can't join this way
    if(!reference)
        return ERR_BATTLEGROUND_JOIN_FAILED;

    PvPDifficultyEntry const* bracketEntry = GetBattlegroundBracketByLevel(bgOrTemplate->GetMapId(), reference->getLevel());
    if(!bracketEntry)
        return ERR_BATTLEGROUND_JOIN_FAILED;

    uint32 arenaTeamId = reference->GetArenaTeamId(arenaSlot);
    uint32 team = reference->GetTeam();

    // check every member of the group to be able to join
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *member = itr->getSource();
        // offline member? don't let join
        if(!member)
            return ERR_BATTLEGROUND_JOIN_FAILED;
        // don't allow cross-faction join as group
        if(member->GetTeam() != team)
            return ERR_BATTLEGROUND_JOIN_TIMED_OUT;
        // not in the same battleground level bracket, don't let join
        PvPDifficultyEntry const* memberBracketEntry = GetBattlegroundBracketByLevel(bracketEntry->mapId, member->getLevel());
        if(memberBracketEntry != bracketEntry)
            return ERR_BATTLEGROUND_JOIN_RANGE_INDEX;
        // don't let join rated matches if the arena team id doesn't match
        if(isRated && member->GetArenaTeamId(arenaSlot) != arenaTeamId)
            return ERR_BATTLEGROUND_JOIN_FAILED;
        // don't let join if someone from the group is already in that bg queue
        if(member->InBattleGroundQueueForBattleGroundQueueType(bgQueueTypeId))
            return ERR_BATTLEGROUND_JOIN_FAILED;            // not blizz-like
        // check for deserter debuff in case not arena queue
        if(bgOrTemplate->GetTypeID() != BATTLEGROUND_AA && !member->CanJoinToBattleground())
            return ERR_GROUP_JOIN_BATTLEGROUND_DESERTERS;
        // check if member can join any more battleground queues
        if(!member->HasFreeBattleGroundQueueId())
            return ERR_BATTLEGROUND_TOO_MANY_QUEUES;        // not blizz-like
    }
    return GroupJoinBattlegroundResult(bgOrTemplate->GetTypeID());
}

//===================================================
//============== Roll ===============================
//===================================================

void Roll::targetObjectBuildLink()
{
    // called from link()
    getTarget()->addLootValidatorRef(this);
}

void Group::SetDungeonDifficulty(Difficulty difficulty)
{
    m_dungeonDifficulty = difficulty;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET difficulty = %u WHERE groupId='%u'", m_dungeonDifficulty, m_Id);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *player = itr->getSource();
        if(!player->GetSession() || player->getLevel() < LEVELREQUIREMENT_HEROIC)
            continue;
        player->SetDungeonDifficulty(difficulty);
        player->SendDungeonDifficulty(true);
    }
}

void Group::SetRaidDifficulty(Difficulty difficulty)
{
    m_raidDifficulty = difficulty;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET raiddifficulty = %u WHERE groupId='%u'", m_raidDifficulty, m_Id);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *player = itr->getSource();
        if(!player->GetSession() || player->getLevel() < LEVELREQUIREMENT_HEROIC)
            continue;
        player->SetRaidDifficulty(difficulty);
        player->SendRaidDifficulty(true);
    }
}

bool Group::InCombatToInstance(uint32 instanceId, bool bossOnly)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pPlayer = itr->getSource();
        if(!pPlayer->getAttackers().empty() && pPlayer->GetInstanceId() == instanceId)
        {
            if(!bossOnly)
                return true;

            for(std::set<Unit*>::const_iterator itr = pPlayer->getAttackers().begin(); itr != pPlayer->getAttackers().end(); itr++)
            {
                if((*itr)->GetTypeId() != TYPEID_PLAYER && ((Creature*)(*itr))->isWorldBoss())
                    return true;
            }
        }
    }
    return false;
}

void Group::ResetInstances(uint8 method, bool isRaid, Player* SendMsgTo)
{
    if(isBGGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_CHANGE_DIFFICULTY, INSTANCE_RESET_GROUP_DISBAND

    // we assume that when the difficulty changes, all instances that can be reset will be
    Difficulty diff = GetDifficulty(isRaid);

    for(BoundInstancesMap::iterator itr = m_boundInstances[diff].begin(); itr != m_boundInstances[diff].end();)
    {
        InstanceSave *p = itr->second.save;
        const MapEntry *entry = sMapStore.LookupEntry(itr->first);
        if (!entry || entry->IsRaid() != isRaid || (!p->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND))
        {
            ++itr;
            continue;
        }

        if(method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->map_type == MAP_RAID || diff == DUNGEON_DIFFICULTY_HEROIC)
            {
                ++itr;
                continue;
            }
        }

        bool isEmpty = true;
        // if the map is loaded, reset it
        Map *map = sMapMgr.FindMap(p->GetMapId(), p->GetInstanceId());
        if(map && map->IsDungeon() && !(method == INSTANCE_RESET_GROUP_DISBAND && !p->CanReset()))
            isEmpty = ((InstanceMap*)map)->Reset(method);

        if(SendMsgTo)
        {
            if(isEmpty)
                SendMsgTo->SendResetInstanceSuccess(p->GetMapId());
            else
                SendMsgTo->SendResetInstanceFailed(0, p->GetMapId());
        }

        if(isEmpty || method == INSTANCE_RESET_GROUP_DISBAND || method == INSTANCE_RESET_CHANGE_DIFFICULTY)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if(p->CanReset())
                p->DeleteFromDB();
            else
                CharacterDatabase.PExecute("DELETE FROM group_instance WHERE instance = '%u'", p->GetInstanceId());
            // i don't know for sure if hash_map iterators
            m_boundInstances[diff].erase(itr);
            itr = m_boundInstances[diff].begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            p->RemoveGroup(this);
        }
        else
            ++itr;
    }
}

InstanceGroupBind* Group::GetBoundInstance(Player* player)
{
    uint32 mapid = player->GetMapId();
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if(!mapEntry)
        return NULL;

    Difficulty difficulty = player->GetDifficulty(mapEntry->IsRaid());

    // some instances only have one difficulty
    MapDifficulty const* mapDiff = GetMapDifficultyData(mapid,difficulty);
    if(!mapDiff)
        difficulty = DUNGEON_DIFFICULTY_NORMAL;

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if(itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return NULL;
}

InstanceGroupBind* Group::GetBoundInstance(Map* aMap, Difficulty difficulty)
{
    // some instances only have one difficulty
    MapDifficulty const* mapDiff = GetMapDifficultyData(aMap->GetId(),difficulty);
    if(!mapDiff)
        return NULL;

    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(aMap->GetId());
    if(itr != m_boundInstances[difficulty].end())
        return &itr->second;
    else
        return NULL;
}

InstanceGroupBind* Group::BindToInstance(InstanceSave *save, bool permanent, bool load)
{
    if(save && !isBGGroup())
    {
        InstanceGroupBind& bind = m_boundInstances[save->GetDifficulty()][save->GetMapId()];
        if(bind.save)
        {
            // when a boss is killed or when copying the players's binds to the group
            if(permanent != bind.perm || save != bind.save)
                if(!load)
                    CharacterDatabase.PExecute("UPDATE group_instance SET instance = '%u', permanent = '%u' WHERE leaderGuid = '%u' AND instance = '%u'", save->GetInstanceId(), permanent, GUID_LOPART(GetLeaderGUID()), bind.save->GetInstanceId());
        }
        else if(!load)
                CharacterDatabase.PExecute("INSERT INTO group_instance (leaderGuid, instance, permanent) VALUES ('%u', '%u', '%u')", GUID_LOPART(GetLeaderGUID()), save->GetInstanceId(), permanent);

        if(bind.save != save)
        {
            if(bind.save)
                bind.save->RemoveGroup(this);
            save->AddGroup(this);
        }

        bind.save = save;
        bind.perm = permanent;
        if(!load)
            DEBUG_LOG("Group::BindToInstance: %d is now bound to map %d, instance %d, difficulty %d", GUID_LOPART(GetLeaderGUID()), save->GetMapId(), save->GetInstanceId(), save->GetDifficulty());
        return &bind;
    }
    else
        return NULL;
}

void Group::UnbindInstance(uint32 mapid, uint8 difficulty, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances[difficulty].find(mapid);
    if(itr != m_boundInstances[difficulty].end())
    {
        if(!unload)
            CharacterDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u' AND instance = '%u'", GUID_LOPART(GetLeaderGUID()), itr->second.save->GetInstanceId());
        itr->second.save->RemoveGroup(this);                // save can become invalid
        m_boundInstances[difficulty].erase(itr);
    }
}

void Group::_homebindIfInstance(Player *player)
{
    if(player && !player->isGameMaster() && sMapStore.LookupEntry(player->GetMapId())->IsDungeon())
    {
        // leaving the group in an instance, the homebind timer is started
        // unless the player is permanently saved to the instance
        InstanceSave *save = sInstanceSaveMgr.GetInstanceSave(player->GetInstanceId());
        InstancePlayerBind *playerBind = save ? player->GetBoundInstance(save->GetMapId(), save->GetDifficulty()) : NULL;
        if(!playerBind || !playerBind->perm)
            player->m_InstanceValid = false;
    }
}

void Group::RewardGroupAtKill(Unit* pVictim)
{
    // for creature case use tapped group (for avoid use group if not set and use if player switch group after tap
    //Group* pGroup = pVictim->GetTypeId() == TYPEID_UNIT ? ((Creature*)pVictim)->GetGroupLootRecipient() : GetGroup();

    bool PvP = pVictim->isCharmedOwnedByPlayerOrPlayer();

    // prepare data for near group iteration (PvP and !PvP cases)
    uint32 xp = 0;

    uint32 count = 0;
    uint32 sum_level = 0;
    Player* member_with_max_level = NULL;
    Player* not_gray_member_with_max_level = NULL;

    GetDataForXPAtKill(pVictim,count,sum_level,member_with_max_level,not_gray_member_with_max_level);

    if(member_with_max_level)
    {
        /// not get Xp in PvP or no not gray players in group
        xp = (PvP || !not_gray_member_with_max_level) ? 0 : MaNGOS::XP::Gain(not_gray_member_with_max_level, pVictim);

        /// skip in check PvP case (for speed, not used)
        bool is_raid = PvP ? false : sMapStore.LookupEntry(pVictim->GetMapId())->IsRaid() && isRaidGroup();
        bool is_dungeon = PvP ? false : sMapStore.LookupEntry(pVictim->GetMapId())->IsDungeon();
        float group_rate = MaNGOS::XP::xp_in_group_rate(count,is_raid);

        for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();
            if(!pGroupGuy)
                continue;

            if(!pGroupGuy->IsAtGroupRewardDistance(pVictim))
                continue;                               // member (alive or dead) or his corpse at req. distance

            // honor can be in PvP and !PvP (racial leader) cases (for alive)
            if (pGroupGuy->isAlive())
                pGroupGuy->RewardHonor(pVictim,count);

            // xp and reputation only in !PvP case
            if(!PvP)
            {
                float rate = group_rate * float(pGroupGuy->getLevel()) / sum_level;

                // if is in dungeon then all receive full reputation at kill
                // rewarded any alive/dead/near_corpse group member
                pGroupGuy->RewardReputation(pVictim,is_dungeon ? 1.0f : rate);

                // XP updated only for alive group member
                if(pGroupGuy->isAlive() && not_gray_member_with_max_level &&
                    pGroupGuy->getLevel() <= not_gray_member_with_max_level->getLevel())
                {
                    uint32 itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? uint32(xp*rate) : uint32((xp*rate/2)+1);

                    pGroupGuy->GiveXP(itr_xp, pVictim);
                    if(Pet* pet = pGroupGuy->GetPet())
                        pet->GivePetXP(itr_xp/2);
                }

                // quest objectives updated only for alive group member or dead but with not released body
                if(pGroupGuy->isAlive()|| !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                {
                    // normal creature (not pet/etc) can be only in !PvP case
                    if(pVictim->GetTypeId()==TYPEID_UNIT)
                        pGroupGuy->KilledMonster(((Creature*)pVictim)->GetCreatureInfo(), pVictim->GetObjectGuid());
                }
            }
        }
    }
}
