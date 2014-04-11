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

#include <DefenseManager.h>
#include <BorderManager.h>
#include <ArmyManager.h>

using namespace BWAPI;

int numberSteps(BWTA::Region *source, BWTA::Region *target)
{
	int nbSteps = 0;
	std::map<BWTA::Region*, int> allRegions;
	std::set<BWTA::Region*> regions = BWTA::getRegions();
	for each (BWTA::Region *r in regions)
		allRegions.insert(std::make_pair(r, 0));

	std::list<BWTA::Region*> toExplore;
	toExplore.push_front(source);
	allRegions[source] = 1;
	
	while(!toExplore.empty())
	{
		BWTA::Region *pop = toExplore.back();
		toExplore.pop_back();
		if (pop == target)
			return allRegions[pop];
		for each (BWTA::Region *r in pop->getReachableRegions())
			if (allRegions[r] == 0)
			{
				toExplore.push_front(r);
				allRegions[r] = allRegions[pop]+1;
			}
	}

	return -1;
}

DefenseManager::DefenseManager(Arbitrator::Arbitrator<Unit*,double>* arbitrator)
{
	lastFrameCheck = 0;
	this->arbitrator = arbitrator;
	hasObserver = false;
}
void DefenseManager::setBorderManager(BorderManager* borderManager)
{
	this->borderManager=borderManager;
}

void DefenseManager::setSpyManager(SpyManager* spyManager)
{
	this->spyManager=spyManager;
}

void DefenseManager::setDragoonManager(DragoonManager* dragoonManager)
{
	this->dragoonManager = dragoonManager;
}

void DefenseManager::setBuildOrderManager(BuildOrderManager* buildOrderManager)
{
	this->buildOrderManager=buildOrderManager;
}

void DefenseManager::setBaseManager(BaseManager *baseManager)
{
	this->baseManager = baseManager;
}

void DefenseManager::setInformationManager(InformationManager *informationManager)
{
	this->informationManager = informationManager;
}

void DefenseManager::setAttackUnitPlanner(AttackUnitPlanner *attackUnitPlanner)
{
	this->attackUnitPlanner = attackUnitPlanner;
}

std::set<Unit*>& DefenseManager::getIdleDefenders()
{
	idleDefenders.clear();

	for (std::map<Unit*, DefenseData>::iterator it = defenders.begin(); it != defenders.end(); ++it)
	{
		if ((it->second.mode == DefenseData::Idle) || (it->second.mode == DefenseData::Moving))
			idleDefenders.insert(it->first);
	}

	return idleDefenders;
}

std::set<Unit*>& DefenseManager::getDefenders()
{
	allDefenders.clear();

	for (std::map<Unit*, DefenseData>::iterator it = defenders.begin(); it != defenders.end(); ++it)
		allDefenders.insert(it->first);

	return allDefenders;
}

bool DefenseManager::isDefending()
{
	for (std::map<Unit*, DefenseData>::iterator it = defenders.begin(); it != defenders.end(); ++it)
		if (it->second.mode == DefenseData::Defending)
			return true;
	return false;
}

void DefenseManager::onOffer(std::set<Unit*> units)
{
	for(std::set<Unit*>::iterator u = units.begin(); u != units.end(); ++u)
	{
		if (defenders.find(*u) == defenders.end())
		{
			if ((*u)->getType() == UnitTypes::Protoss_Observer)
			{
				if (hasObserver)
				{
					arbitrator->decline(this, *u, 0);
					continue;
				}
				else
				{
					spyManager->removeBid(*u);
					hasObserver = true;
				}
			}
			arbitrator->accept(this, *u);
			DefenseData temp;
			temp.hasReachRegion = false;

			defenders.insert(std::make_pair(*u, temp));
		}
	}
}

void DefenseManager::onRevoke(Unit* unit, double bid)
{
	defenders.erase(unit);
}

void DefenseManager::onRemoveUnit(Unit* unit)
{
	if( defenders.find( unit ) != defenders.end() )
	{
		if ( unit->getType() == UnitTypes::Protoss_Observer )
			hasObserver = false;
		defenders.erase(unit);
	}
}

void DefenseManager::update()
{
	if ((lastFrameCheck == 0) || (Broodwar->getFrameCount() > lastFrameCheck + 10))
	{
		lastFrameCheck = Broodwar->getFrameCount();
		std::map<Unit*,DefenseData>::iterator shouldRemove = defenders.end();

		// if we are leaving our defending region, the enemy is probably fleeing so stop purchasing it!
		for (std::map<Unit*,DefenseData>::iterator it = defenders.begin(); it != defenders.end(); ++it)
		{
			if (it->second.mode == DefenseData::Defending && 
				!it->second.hasReachRegion && it->second.regionOrigin == BWTA::getRegion(it->first->getTilePosition()))
			{
				it->second.hasReachRegion = true;
			}

			if (it->second.mode == DefenseData::Defending && 
				it->second.hasReachRegion && it->second.regionOrigin != BWTA::getRegion(it->first->getTilePosition()))
			{
				it->second.mode = DefenseData::Idle;
			}

			if (hasWorker && it->first->getType().isWorker() && Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Zealot) > 1)
			{
				hasWorker = false;
				arbitrator->removeBid(this, it->first);
				shouldRemove = it;
			}
		}

		if (shouldRemove != defenders.end())
			defenders.erase(shouldRemove);

		// Bid on all completed military units
		//std::set<Unit*> myPlayerUnits=::Broodwar->self()->getUnits();
		UnitGroup myPlayerUnits = SelectAll();
		for (std::set<Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); ++u)
		{
			if ((*u)->isCompleted() && 
				!(*u)->getType().isWorker() && 
				!(*u)->getType().isBuilding() &&
				(*u)->getType().canAttack() &&
				(*u)->getType() != UnitTypes::Zerg_Egg &&
				(*u)->getType() != UnitTypes::Zerg_Larva)
			{
				arbitrator->setBid(this, *u, 20);
			}

			if ((*u)->isCompleted() && (*u)->getType() == UnitTypes::Protoss_Observer)
			{
				arbitrator->setBid(this, *u, 50);
			}

			// add a worker as a defenser against Zerg or Random when we just have one zealot
			if ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Zealot) == 1 
				 && 
				 ( Broodwar->enemy()->getRace() == Races::Zerg || Broodwar->enemy()->getRace() == Races::Random )
				 && 
				 !hasWorker 
				 && 
				 (*u)->getType().isWorker() )
			{
				hasWorker = true;
				arbitrator->setBid(this, *u, 20);
			}
		}

		// Dirty! To move away! This should not be done by the DefenseManager.
		if (!hasObserver && 
				(//Broodwar->getFrameCount() > 14000 || 
				informationManager->enemyHasBuilt(UnitTypes::Zerg_Lurker) ||
				informationManager->enemyHasBuilt(UnitTypes::Zerg_Lurker_Egg) ||
				informationManager->enemyHasBuilt(UnitTypes::Terran_Wraith) ||
				informationManager->enemyHasBuilt(UnitTypes::Terran_Ghost) ||
				informationManager->enemyHasBuilt(UnitTypes::Protoss_Dark_Templar)))
		{
			//if (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Robotics_Facility) == 0)
			//	buildOrderManager->build(1, UnitTypes::Protoss_Robotics_Facility, 100);
			//if (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Observatory) == 0 && 
			//		buildOrderManager->getBuildManager()->getPlannedCount(UnitTypes::Protoss_Observatory) == 0)
			//	buildOrderManager->build(1, UnitTypes::Protoss_Observatory, 100);
			//buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Observer, 100);

			spyManager->buildObserver(true);
		}

		bool borderUpdated=false;
		if (myBorder!=borderManager->getMyBorder())
		{
			myBorder=borderManager->getMyBorder();
			borderUpdated=true;
		}

		//search among our bases which ones are in a border region
		std::vector<Base*> borderBases;
		for each (Base *base in baseManager->getAllBases())
		{
			bool isBorderBase = false;

			for each (BWTA::Chokepoint *c in base->getBaseLocation()->getRegion()->getChokepoints())
			{
				if (myBorder.find(c) != myBorder.end())
				{
					isBorderBase = true;
					break;
				}
			}

			if (isBorderBase)
				borderBases.push_back(base);
		}

		std::vector<BWTA::BaseLocation*> enemyBasesLocation = informationManager->getEnemyBases();
		myBorderVector.clear();

		for(int i=0; i < (int)borderBases.size(); i++)
		{
			//search the nearest enemy base from current borderBases[i]
			BWTA::BaseLocation *nearestEnemyBaseLocation = NULL;
			double shortestDistance = std::numeric_limits<double>::max();

			for each (BWTA::BaseLocation *bl in enemyBasesLocation)
			{
				double distance = borderBases[i]->getBaseLocation()->getGroundDistance(bl);
				if (distance < shortestDistance && distance > 0)
				{
					shortestDistance = distance;
					nearestEnemyBaseLocation = bl;
				}
			}

			if (nearestEnemyBaseLocation == NULL)
				nearestEnemyBaseLocation = informationManager->getEnemyStartLocation();

			//now search the nearest choke of borderBases[i] from the enemy base
			BWTA::Chokepoint *bestChoke;
			shortestDistance = std::numeric_limits<double>::max();
			int bestPath = std::numeric_limits<int>::max();

			for each(BWTA::Chokepoint *c in borderBases[i]->getBaseLocation()->getRegion()->getChokepoints())
			{
				std::pair<BWTA::Region*, BWTA::Region*> sides = c->getRegions();
				BWTA::Region *otherSide = sides.first == borderBases[i]->getBaseLocation()->getRegion() ? sides.second : sides.first;
				int path = numberSteps(otherSide, nearestEnemyBaseLocation->getRegion());
				double distance = nearestEnemyBaseLocation->getPosition().getApproxDistance(c->getCenter());
				if (distance < shortestDistance)
					shortestDistance = distance;

				if (path == bestPath && distance == shortestDistance)
					bestChoke = c;
				if (path != -1 && path < bestPath)
				{
					bestPath = path;
					bestChoke = c;
				}				
			}
			myBorderVector.push_back(bestChoke);
		}

		//Order all units to choke
		round = (int)myBorderVector.size() - 1;
		if (!myBorder.empty())
		{
			for (std::map<Unit*,DefenseData>::iterator u = defenders.begin(); u != defenders.end(); ++u)
			{
				if (u->first->isIdle())
				{
					u->second.mode = DefenseData::Idle;
				}

				if( u->second.mode == DefenseData::Idle || 
					borderUpdated ||
					( u->second.mode == DefenseData::Defending && 		
					  myBorder.size() > 1 && 		
					  baseManager->getAllBases().size() > 2 ) )
				{
					Position chokePosition=myBorderVector[round]->getCenter();
					round--;
					if (round < 0)
						round = (int)myBorderVector.size() - 1;

					//wait to the 3/4 between your current position and the center of the choke to protect
					int borderBasesSize = (int)borderBases.size();
					Position lastBase = borderBases[borderBasesSize - round - 1]->getBaseLocation()->getPosition();					
					int x_wait = chokePosition.x() - lastBase.x();
					int y_wait = chokePosition.y() - lastBase.y();
					x_wait = (x_wait / 4) * 3;
					y_wait = (y_wait / 4) * 3;
					x_wait += lastBase.x();
					y_wait += lastBase.y();

					Position waitPosition(x_wait, y_wait); 

					// For dragoons micro
					if( u->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(u->first) )
					{
						//if( !attackUnitPlanner->attackNextTarget( u->first ) )
							(*u).first->attack(waitPosition);
						(*u).second.mode = DefenseData::Moving;
					}
				}

				if (u->first->isUnderAttack() && u->first->getType() != UnitTypes::Protoss_Observer)
				{
					std::set<Unit *> backup = this->getIdleDefenders();
					std::set<Unit*> attackers = ArmyManager::whoIsAttacking(u->first);

					// first, check if there are invisible enemies and if we have an observer. If not, build it!
					if( !hasObserver )
					{
						UnitGroup invisiblesInRegion = SelectAllEnemy()(isCloaked).inRegion( BWTA::getRegion( u->first->getTilePosition() ) );
						if( !invisiblesInRegion.empty() )
						{
							if( spyManager->getNumberSpies() > 0 )
								spyManager->removeBid();
							else
								spyManager->buildObserver(true);
						}
					}

					for each (Unit *bck in backup)
					{
						// For dragoons micro
						if( bck->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack( bck ) )
						{
							if( attackers.empty() )
								bck->attack(u->first->getPosition());
							else
							{
								UnitGroup airAttackers = UnitGroup::getUnitGroup( attackers )( isFlyer );
								UnitGroup groundAttackers = UnitGroup::getUnitGroup( attackers ).not( isFlyer );

								if( ( bck->getType().groundWeapon() != WeaponTypes::None && !groundAttackers.empty() )
									||
									( bck->getType().airWeapon() != WeaponTypes::None && !airAttackers.empty() ) )
								{
									//if( !attackUnitPlanner->attackNextTarget( bck ) )
										bck->attack(ArmyManager::whoIsTheWeakest(attackers)->getPosition());
								}
							}
							defenders[bck].mode = DefenseData::Defending;
							defenders[bck].regionOrigin = BWTA::getRegion(u->first->getTilePosition());
						}
					}
				}
			}
		}
	}
}

std::string DefenseManager::getName() const
{
	return "Defense Manager";
}

std::string DefenseManager::getShortName() const
{
	return "Def";
}