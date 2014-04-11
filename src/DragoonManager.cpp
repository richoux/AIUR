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

#include <DragoonManager.h>
#include <ArmyManager.h>

using namespace BWAPI;
using namespace std;

DragoonManager::DragoonManager()
{
	lastFrameCheck = 0;
}

DragoonManager::DragoonManager(UnitStates *unitStates) : unitStates(unitStates)
{
	lastFrameCheck = 0;
}

bool DragoonManager::availableToAttack( Unit *dragoon )
{
        map<Unit*, DragoonData>::iterator dragoonIt = myDragoons.find( dragoon );
        if( dragoonIt == myDragoons.end() || !dragoonIt->first || !dragoonIt->first->exists() )
                return false;
        return (Broodwar->getFrameCount() > dragoonIt->second.lastFrameAttack + 9);
}

void DragoonManager::setMyDragoons(map<Unit*, DragoonData> dragoons)
{
	myDragoons = dragoons;
}

map<Unit*, DragoonManager::DragoonData> DragoonManager::getMyDragoons()
{
	return myDragoons;
}

void DragoonManager::addDragoon(Unit *dragoon)
{
	myDragoons.insert( make_pair( dragoon, DragoonData() ) );
}

void DragoonManager::deleteDragoon(Unit *dragoon)
{
	myDragoons.erase(dragoon);
}

void DragoonManager::setInformationManager(InformationManager *informationManager)
{
        this->informationManager = informationManager;
}

void DragoonManager::setAttackUnitPlanner(AttackUnitPlanner *attackUnitPlanner)
{
        this->attackUnitPlanner = attackUnitPlanner;
}

void DragoonManager::update()
{
	if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Dragoon) != 0)
	{
		for (map<Unit*, DragoonData>::iterator dragoon = myDragoons.begin(); dragoon != myDragoons.end(); ++dragoon)
		{
			if( dragoon->first->isAttackFrame() || dragoon->first->isAttacking() || dragoon->first->isUnderAttack() )
			{
				if (dragoon->first->isAttackFrame() && Broodwar->getFrameCount() - dragoon->second.lastFrameAttack > 9)
					dragoon->second.lastFrameAttack = Broodwar->getFrameCount();

				// No damage if one stops a dragoon before waiting at least 5 attack frames.
				if (Broodwar->getFrameCount() - dragoon->second.lastFrameAttack > 4)
				{
					if( unitStates->isState( dragoon->first, UnitStates::Dodging ) )
					{
						// if we are under attack and we cannot reply (attack cooldown)
						if( dragoon->first->isUnderAttack() && dragoon->first->getGroundWeaponCooldown() > 0 )
						{					
							// we dodge regarding attackers' center or the nearest attacker?
							//UnitGroup attackers = UnitGroup::getUnitGroup(ArmyManager::whoIsAttacking(dragoon->first));
							//Position posAttackers = attackers.getCenter();
							set<Unit*> attackers = ArmyManager::whoIsAttacking( dragoon->first );
							Position posAttackers;
							if( !attackers.empty() )
							{
								posAttackers = ArmyManager::nearestUnit( dragoon->first, attackers )->getPosition();
								// direction toward we when to "hit 'n run". *100 to be sure it is a point far enough (we just want the direction, remember)
								Position posToGo(3*dragoon->first->getPosition().x() - 2*posAttackers.x(), 3*dragoon->first->getPosition().y() - 2*posAttackers.y());
								posToGo.makeValid();

								dragoon->first->move( posToGo );
								unitStates->setState( dragoon->first, UnitStates::Dodging );
							}
						}
					}
					else
					{
						if( dragoon->first->getGroundWeaponCooldown() == 0 )
						{
							//if( !attackUnitPlanner->attackNextTarget( dragoon->first ) )
								dragoon->first->attack(ArmyManager::nearestUnit(dragoon->first, ArmyManager::whoIsAttacking(dragoon->first)));
							unitStates->setState( dragoon->first, UnitStates::Attacking );
						}
					}
				}
			}
		}
	}
}