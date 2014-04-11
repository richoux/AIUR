/*
* The Artificial Intelligence Using Randomness (AIUR) is an AI for StarCraft: Broodwar, 
* aiming to be unpredictable thanks to some stochastic behaviors. 
* Please visit http://code.google.com/p/aiurproject/ for further information.
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

#include <AttackTracker.h>
#include <UnitGroupManager.h>

UnitGroup allAttacks;
std::map<BWAPI::Unit*, UnitGroup> sourcesOf; /* sourcesOf[t] is the attackers of a target t*/
std::map<BWAPI::Unit*, BWAPI::Unit*> targetOf; /* targetOf[s] is the target of an attacker s*/
AttackTracker* TheAttackTracker = NULL;

AttackTracker* AttackTracker::create()
{
	if (TheAttackTracker) return TheAttackTracker;
	return new AttackTracker();
}
void AttackTracker::destroy()
{
	if (TheAttackTracker)
		delete TheAttackTracker;
}
AttackTracker::AttackTracker()
{
	TheAttackTracker = this;
}
AttackTracker::~AttackTracker()
{
	TheAttackTracker = NULL;
}

void AttackTracker::update()
{
	UnitGroup currentAttacks = UnitGroup::getUnitGroup( BWAPI::Broodwar->getAllUnits() );
	currentAttacks = currentAttacks(exists)(isAttacking);
	if( !allAttacks.empty() )
	{
		// Find all units who don't attack anymore
		UnitGroup formerSources = allAttacks - currentAttacks;
		for each( BWAPI::Unit *s in formerSources )
		{
			// Get the former target of these units
			BWAPI::Unit *t = targetOf[s];
			
			// If the target died, delete it from the map
			// otherwise, just delete the former attacker from its sources
			if( t != NULL )
				if( !t->exists() )
					sourcesOf.erase(t);
				else
					sourcesOf[t].erase(s);

			// Since the source s stopped to attack, 
			// delete it from the map.
			targetOf.erase(s);
		}
	}

	UnitGroup newAttacks = currentAttacks - allAttacks;
	for each( BWAPI::Unit *s in newAttacks )
	{
		BWAPI::Unit *t = s->getOrderTarget();
		if( t != NULL && t->exists() )
		{
			targetOf[s] = t;
			sourcesOf[t].insert(s);
		}
	}

	allAttacks = currentAttacks;
}

UnitGroup AllAttacks()
{
	return allAttacks;
}

UnitGroup SelectAllAttacks()
{
	return allAttacks( BWAPI::Broodwar->self() );
}

UnitGroup SelectAllEnemyAttacks()
{
	return allAttacks( BWAPI::Broodwar->enemy() );
}

UnitGroup SelectAllAttacks(BWAPI::Player* player)
{
	return allAttacks( player );
}

BWAPI::Unit* SelectAttacksFrom(BWAPI::Unit *source)
{
	return targetOf[source];
}

UnitGroup SelectAttacksTo(BWAPI::Unit *target)
{
	return sourcesOf[target];
}

UnitGroup SelectAttacksFromGroup(UnitGroup sources)
{
	UnitGroup targets;

	for each( BWAPI::Unit *s in sources )
		targets.insert( targetOf[s] );

	return targets;
}

UnitGroup SelectAttacksToGroup(UnitGroup targets)
{
	UnitGroup sources;

	for each( BWAPI::Unit *t in targets)
		sources += sourcesOf[t];

	return sources;
}
