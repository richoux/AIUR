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

#include <BWTA.h>
#include <SpyManager.h>
#include <UnitGroupManager.h>
BWTA::BaseLocation* findFarestBaseLocation(std::set<BWTA::BaseLocation *> allBaseLocations, const BWTA::BaseLocation *enemyStartLocation)
{
	BWTA::BaseLocation *toReturn;
	double maxDistance = -1;
	double distance;
	
	for each (BWTA::BaseLocation *bl in allBaseLocations)
	{
		distance = enemyStartLocation->getAirDistance(bl);
		if (distance > maxDistance)
		{
			maxDistance = distance;
			toReturn = bl;
		}
	}
	return toReturn;
}

SpyManager::SpyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator)
{
	this->arbitrator = arbitrator;
	this->debugMode=false;
	lastFrameCheck = 0;
	randomDodge = new Random(41);
	initialized = false;
}

SpyManager::SpyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator, UnitStates *unitStates)
: arbitrator(arbitrator), unitStates(unitStates)
{
	this->debugMode=false;
	lastFrameCheck = 0;
	randomDodge = new Random(41);
	initialized = false;
}

SpyManager::~SpyManager()
{
	delete randomDodge;
}

void SpyManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for each (BWAPI::Unit *unit in units)
	{
		if (spies.find(unit) == spies.end() && obsDefenders.find(unit) == obsDefenders.end())
		{
			arbitrator->accept(this, unit);
			SpyData temp;
			spies.insert(std::make_pair(unit,temp));
		}
		else
			arbitrator->decline(this, unit, 0);
	}
}

void SpyManager::onRevoke(BWAPI::Unit *unit, double bid)
{
	onRemoveUnit(unit);
}

void SpyManager::buildObserver(bool force)
{
	//if we don't have any observatories nor robotic bays, first build one of each - and quickly!
	if ( BWAPI::Broodwar->getFrameCount() > 8000 || force )
	{
		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Observatory) == 0)
			buildOrderManager->build(1, BWAPI::UnitTypes::Protoss_Observatory, 70);

		if (BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Robotics_Facility) == 0)
			buildOrderManager->build(1, BWAPI::UnitTypes::Protoss_Robotics_Facility, 70);

		//build an observer
		int observerCount = BWAPI::Broodwar->self()->allUnitCount(BWAPI::UnitTypes::Protoss_Observer);
		buildOrderManager->build(observerCount+1, BWAPI::UnitTypes::Protoss_Observer, 70);
	}
}

int SpyManager::getNumberToSpy()
{
	return numberBasesToSpy;
}

int SpyManager::getNumberSpies()
{
	return spies.size();
}

int SpyManager::addSight()
{
	if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Sensor_Array) == 0)
		return 0;
	else
		return 64;
}

void SpyManager::update()
{
	if (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 6)
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		std::set<BWAPI::Unit*> w = SelectAll()(isCompleted)(Protoss_Observer);
		for each(BWAPI::Unit *u in w)
			if (obsDefenders.find(u) == obsDefenders.end())
				arbitrator->setBid(this, u, 40);

		// if we didn't spy a new base after 10000 frames, do it! (starting from 15000 frames)
		if (BWAPI::Broodwar->getFrameCount() > lastIncreaseSpyNumber + 10000 && BWAPI::Broodwar->getFrameCount() > 15000)
		{
			numberAdditionalBasesToSpy++;
		}

		if (numberBasesToSpy < (signed)BWTA::getBaseLocations().size() && numberBasesToSpy < 2 + informationManager->getNumberBaseCenters() + numberAdditionalBasesToSpy)
		{
			numberBasesToSpy = 2 + informationManager->getNumberBaseCenters() + numberAdditionalBasesToSpy;
			lastIncreaseSpyNumber = BWAPI::Broodwar->getFrameCount();
		}

		if ((signed)baseLocationsSpied.size() < numberBasesToSpy && BWAPI::Broodwar->self()->incompleteUnitCount(BWAPI::UnitTypes::Protoss_Observer) == 0)
		{
			buildObserver(false);
		}

		std::map<BWAPI::Unit*, SpyData>::iterator it = spies.begin();
		for ( ; it != spies.end(); it++)
		{

			if( unitStates->isState( it->first, UnitStates::Idle ) && !baseLocationsToSpy.empty() )
			{
				it->second.target = baseLocationsToSpy.top();
				unitStates->setState( it->first, UnitStates::Spying );
				it->second.lastFrameDodging = 0;
				baseLocationsSpied.insert(baseLocationsToSpy.top());
				baseLocationsToSpy.pop();
				break;
			}

			if( unitStates->isState( it->first, UnitStates::Spying ) )
			{
				if ((it->second.lastFrameDodging == 0) || (BWAPI::Broodwar->getFrameCount() > it->second.lastFrameDodging + 300))
				{
					it->second.lastFrameDodging = BWAPI::Broodwar->getFrameCount();
					it->first->move(it->second.target->getPosition());
				}
				// Look if there are detectors around us. If yes, dodge it!
				//std::set<BWAPI::Unit*> surroundingUnits = it->first->getUnitsInRadius(it->first->getType().sightRange() + addSight());
				//UnitGroup surroundingUnitGroup = UnitGroup::getUnitGroup(surroundingUnits);
				//surroundingUnitGroup = surroundingUnitGroup(BWAPI::Broodwar->enemy())(isDetector);
				UnitGroup surroundingUnitGroup = SelectAllEnemy()(isDetector).inRadius(it->first->getType().sightRange() + addSight(), it->first->getPosition());
				
				if (!surroundingUnitGroup.empty())
				{
					int x = it->first->getPosition().x();
					int	y = it->first->getPosition().y();
					for each (BWAPI::Unit *u in surroundingUnitGroup)
					{
						x -= (u->getPosition().x() - it->first->getPosition().x())/2;
						y -= (u->getPosition().y() - it->first->getPosition().y())/2;						
					}

					x += (randomDodge->nextAnotherInt() - 20);
					y += (randomDodge->nextAnotherInt() - 20);

					BWAPI::Position pos(x,y);
					it->first->move(pos.makeValid());
				}
			}
		}

		if (debugMode)
		{
			drawAssignments();
		}
	}
}

void SpyManager::setInformationManager(InformationManager* infoManager)
{
	this->informationManager = infoManager;
}

void SpyManager::setBuildOrderManager(BuildOrderManager *boManager)
{
	this->buildOrderManager = boManager;
}

void SpyManager::setMoodManager(MoodManager *moodManager)
{
	this->moodManager = moodManager;
}

std::string SpyManager::getName() const
{
	return "Spy Manager";
}

std::string SpyManager::getShortName() const
{
	return "Spy";
}

void SpyManager::removeBid()
{
	removeBid( spies.rbegin()->first );
}

void SpyManager::removeBid(BWAPI::Unit* unit)
{
	std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bidders = arbitrator->getAllBidders(unit);
	for(std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j = bidders.begin(); j != bidders.end(); ++j)
	{
		if (j->first == this)
		{
			unit->stop();
			arbitrator->removeBid(this, unit);
		}
	}
	obsDefenders.insert(unit);
	onRemoveUnit(unit);
	// if baseLocationsSpied was empty
	if (spies.find(unit) != spies.end())
		spies.erase(unit);
}

void SpyManager::onRemoveUnit(BWAPI::Unit* unit)
{
	if (!baseLocationsSpied.empty() && spies.find(unit) != spies.end())
	{
		BWTA::BaseLocation* lostTarget = spies[unit].target;
		baseLocationsSpied.erase(lostTarget);

		if (baseLocationsSpied.find(lostTarget) == baseLocationsSpied.end())
		{
			baseLocationsToSpy.push(lostTarget);
			if (debugMode && lostTarget != NULL)
			{
				BWAPI::Broodwar->printf("Reassigning (%d,%d)", lostTarget->getPosition().x(), lostTarget->getPosition().y());
			}
		}
		spies.erase(unit);
	}
}

void SpyManager::increaseSpyNumber()
{
	this->numberBasesToSpy++;
	lastIncreaseSpyNumber = BWAPI::Broodwar->getFrameCount();
}

void SpyManager::setDebugMode(bool debugMode)
{
	this->debugMode = debugMode;
}

void SpyManager::initialize()
{
	//We should always spy at least 2 empty base locations.
	numberBasesToSpy = 2;
	numberAdditionalBasesToSpy = 0;
	lastIncreaseSpyNumber = BWAPI::Broodwar->getFrameCount();

	myStartLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
	//At this point, we should have only one base in infoManager::enemyLocations (if it was not ripped off by a cheese ^^)
	enemyStartLocation = informationManager->getEnemyStartLocation();
	std::set<BWTA::BaseLocation *> allBaseLocations = BWTA::getBaseLocations();
	allBaseLocations.erase(myStartLocation);

	while ((int)baseLocationsToSpy.size() < BWTA::getBaseLocations().size() - 1)
	{
		BWTA::BaseLocation *farBase = findFarestBaseLocation(allBaseLocations, enemyStartLocation);
		allBaseLocations.erase(farBase);
		baseLocationsToSpy.push(farBase);
	}

	initialized = true;
}

bool SpyManager::isInitialized()
{
  return initialized;
}

void SpyManager::drawAssignments()
{
	//draw target vector for each Spy
	for (std::map<BWAPI::Unit*,SpyData>::iterator s = spies.begin(); s != spies.end(); ++s)
	{
		if( !unitStates->isState( s->first, UnitStates::Idle ) )
		{
			BWAPI::Position SpyPos = (*s).first->getPosition();
			BWAPI::Position targetPos = (*s).second.target->getPosition();
			BWAPI::Broodwar->drawLineMap(SpyPos.x(), SpyPos.y(), targetPos.x(), targetPos.y(), BWAPI::Colors::Yellow);
			BWAPI::Broodwar->drawCircleMap(SpyPos.x(), SpyPos.y(), 6, BWAPI::Colors::Yellow);
			BWAPI::Broodwar->drawCircleMap(targetPos.x(), targetPos.y(), (*s).first->getType().sightRange() + addSight(), BWAPI::Colors::Yellow);
		}
	}
}
