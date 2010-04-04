/*
 * Copyright (C) 2005-2008 MaNGOS <http://getmangos.com/>
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

#ifndef OUTDOOR_PVP_MGR_H_
#define OUTDOOR_PVP_MGR_H_

#include "OutdoorPvP.h"
#include "Policies/Singleton.h"

class Player;
class GameObject;
class Creature;
struct GossipOption;

// class to handle player enter / leave / areatrigger / GO use events
class OutdoorPvPMgr
{
    public:
        OutdoorPvPMgr();
        ~OutdoorPvPMgr();

        void CreateOutdoorPvP(uint32 typeId, uint32 mapId);
        // create outdoor pvp events
        void InitOutdoorPvP();
        // handle custom (non-exist in dbc) spell if registered
        bool HandleCustomSpell(Player* plr, uint32 spellId, GameObject* go);

        void AddZone(uint32 zoneid, OutdoorPvP * handle);
        OutdoorPvP* GetOutdoorPvPToZoneId(uint32 zoneid);

        // will be called from constructor/destructor of opvp class
        void AddOPvP(OutdoorPvP* opvp) { m_OutdoorPvPSet.insert(opvp); }
        void DelOPvP(OutdoorPvP* opvp) { m_OutdoorPvPSet.erase(opvp); }

        typedef std::set<OutdoorPvP*> OutdoorPvPSet;
        typedef std::map<uint32 /* zoneid */, OutdoorPvP*> OutdoorPvPMap;
    private:
        // contains all initiated outdoor pvp events
        // used when initing / cleaning up
        OutdoorPvPSet  m_OutdoorPvPSet;
        // maps the zone ids to an outdoor pvp event
        // used in player event handling
        OutdoorPvPMap   m_OutdoorPvPMap;
};

#define sOutdoorPvPMgr MaNGOS::Singleton<OutdoorPvPMgr>::Instance()

#endif /*OUTDOOR_PVP_MGR_H_*/
