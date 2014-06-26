/*
* The Artificial Intelligence Using Randomness (AIUR) is an AI for StarCraft: Broodwar, 
* aiming to be unpredictable thanks to some stochastic behaviors. 
* Please visit https://github.com/AIUR-group/AIUR for further information.
* 
* Copyright (C) 2011 - 2014 Florian Richoux
*
* This file is part of AIUR.
* AIUR is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* AIUR is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with AIUR.  If not, see http://www.gnu.org/licenses/.
*/

#include <UnitGroupManager.h>

std::map<BWAPI::Unit*,BWAPI::Player*> unitOwner;
std::map<BWAPI::Unit*,BWAPI::UnitType> unitType;
std::map<BWAPI::Player*, std::map<BWAPI::UnitType,UnitGroup > > data;
std::map<BWAPI::Player*, UnitGroup> allOwnedUnits;
UnitGroup allUnits;
BWAPI::Player* neutral;
UnitGroupManager* TheUnitGroupManager = NULL;

UnitGroupManager* UnitGroupManager::create()
{
	if (TheUnitGroupManager) return TheUnitGroupManager;
	return new UnitGroupManager();
}
void UnitGroupManager::destroy()
{
	if (TheUnitGroupManager)
		delete TheUnitGroupManager;
}
UnitGroupManager::UnitGroupManager()
{
	TheUnitGroupManager = this;
	for(std::set<BWAPI::Unit*>::iterator i=BWAPI::Broodwar->getAllUnits().begin();i!=BWAPI::Broodwar->getAllUnits().end(); ++i)
	{
		onUnitDiscover(*i);
	}
	neutral=NULL;
	for(std::set<BWAPI::Player*>::iterator i=BWAPI::Broodwar->getPlayers().begin();i!=BWAPI::Broodwar->getPlayers().end(); ++i)
	{
		if ((*i)->isNeutral())
			neutral=*i;
	}
}
UnitGroupManager::~UnitGroupManager()
{
	TheUnitGroupManager = NULL;
}
void UnitGroupManager::onUnitDiscover(BWAPI::Unit* unit)
{
	unitOwner[unit]=unit->getPlayer();
	unitType[unit]=unit->getType();
	data[unit->getPlayer()][unit->getType()].insert(unit);
	allOwnedUnits[unit->getPlayer()].insert(unit);
	allUnits.insert(unit);
}
void UnitGroupManager::onUnitEvade(BWAPI::Unit* unit)
{
	unitOwner[unit]=unit->getPlayer();
	unitType[unit]=unit->getType();
	data[unit->getPlayer()][unit->getType()].erase(unit);
	allOwnedUnits[unit->getPlayer()].erase(unit);
	allUnits.erase(unit);
}
void UnitGroupManager::onUnitMorph(BWAPI::Unit* unit)
{
	data[unitOwner[unit]][unitType[unit]].erase(unit);
	unitType[unit]=unit->getType();
	unitOwner[unit]=unit->getPlayer();
	data[unit->getPlayer()][unit->getType()].insert(unit);
}
void UnitGroupManager::onUnitRenegade(BWAPI::Unit* unit)
{
	data[unitOwner[unit]][unitType[unit]].erase(unit);
	allOwnedUnits[unitOwner[unit]].erase(unit);
	unitType[unit]=unit->getType();
	unitOwner[unit]=unit->getPlayer();
	data[unit->getPlayer()][unit->getType()].insert(unit);
	allOwnedUnits[unit->getPlayer()].insert(unit);
}
UnitGroup AllUnits()
{
	return allUnits;
}
UnitGroup SelectAll()
{
	return allOwnedUnits[BWAPI::Broodwar->self()];
}
UnitGroup SelectAll(BWAPI::UnitType type)
{
	if (type.isNeutral() && neutral!=NULL)
		return data[neutral][type];
	return data[BWAPI::Broodwar->self()][type];
}
UnitGroup SelectAllEnemy()
{
	return allOwnedUnits[BWAPI::Broodwar->enemy()];
}
UnitGroup SelectAllEnemy(BWAPI::UnitType type)
{
	return data[BWAPI::Broodwar->enemy()][type];
}
UnitGroup SelectAll(BWAPI::Player* player, BWAPI::UnitType type)
{
	return data[player][type];
}