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

#include <BaseManager.h>
#include <BuildOrderManager.h>
#include <BorderManager.h>
#include <BuildingPlacer.h>

#define TAKEGAS	5000

BaseManager::BaseManager()
{
	this->builder = NULL;
	this->refineryNeeded  = 1;
	this->refineryBuildPriority = 0;
	lastFrameCheck = 0;
	lastTakenBaseLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
	myRegions.insert(lastTakenBaseLocation->getRegion());
}

void BaseManager::setBuildOrderManager(BuildOrderManager* builder)
{
	this->builder = builder;
}

void BaseManager::setBuildManager(BuildManager* buildManager)
{
	this->buildManager = buildManager;
}

void BaseManager::setMoodManager(MoodManager* moodManager)
{
	this->moodManager = moodManager;
}

void BaseManager::setBorderManager(BorderManager* borderManager)
{
	this->borderManager = borderManager;
}

void BaseManager::update()
{
	if ((lastFrameCheck == 0) || (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 60))
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		for(std::set<Base*>::iterator b = this->allBases.begin(); b != this->allBases.end(); ++b)
		{
			if (!(*b)->isActive())
			{
				if ((*b)->getResourceDepot() == NULL)
				{
					BWAPI::TilePosition tile = (*b)->getBaseLocation()->getTilePosition();
					std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(tile.x(),tile.y());
					for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); ++u)
						if ((*u)->getPlayer() == BWAPI::Broodwar->self() && (*u)->getType().isResourceDepot())
						{
							(*b)->setResourceDepot(*u);
							break;
						}
				}
				if ((*b)->getResourceDepot()!=NULL)
				{
					if (!(*b)->getResourceDepot()->exists())
						(*b)->setResourceDepot(NULL);
					else
					{
						if ((*b)->getResourceDepot()->isCompleted() || (*b)->getResourceDepot()->getRemainingBuildTime() < 250)
							(*b)->setActive(true);
					}
				}
			}

			//Set Refinerys
			if (!(*b)->isActiveGas() && (*b)->hasGas())
			{
				if ((*b)->getRefinery() == NULL)
				{
					std::set<BWAPI::Unit*> baseGeysers = (*b)->getBaseLocation()->getGeysers();

					BWAPI::TilePosition geyserLocation;

					//cycle through geysers & get tile location
					for(std::set<BWAPI::Unit*>::iterator bg = baseGeysers.begin(); bg != baseGeysers.end(); ++bg)
					{
						geyserLocation = (*bg)->getTilePosition();
					}

					//check for refinery already on geyser
					std::set<BWAPI::Unit*> unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserLocation.x(),geyserLocation.y());

					for(std::set<BWAPI::Unit*>::iterator u = unitsOnGeyser.begin(); u != unitsOnGeyser.end(); ++u)
					{
						if ((*u)->getPlayer() == BWAPI::Broodwar->self() && (*u)->getType().isRefinery())
						{
							(*b)->setRefinery(*u);
							break;
						}
					}
				}

				if ((*b)->getRefinery() != NULL)
				{
					if ((*b)->getRefinery()->exists()==false)
						(*b)->setResourceDepot(NULL);
					else
					{
						if ((*b)->getRefinery()->isCompleted() || (*b)->getRefinery()->getRemainingBuildTime() < 250)
							(*b)->setActiveGas(true);
					}
				}
			}
		}

		// add our starting base
		if (lastFrameCheck == 0)
		{
			for(std::set<BWTA::BaseLocation*>::const_iterator bl = BWTA::getBaseLocations().begin(); bl != BWTA::getBaseLocations().end(); ++bl)
			{
				if (location2base.find(*bl) == location2base.end())
				{
					BWAPI::TilePosition tile = (*bl)->getTilePosition();
					std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(tile.x(), tile.y());
					for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); ++u)
						if ((*u)->getPlayer() == BWAPI::Broodwar->self() && (*u)->getType().isResourceDepot() && ((*u)->exists() || (*u)->isBeingConstructed()))
							addBase(*bl);
				}
			}
		}
	
		if(BWAPI::Broodwar->getFrameCount() >= 3000 && refineryBuildPriority > 0)
			updateRefineries();
	}
}

void BaseManager::resetBases()
{
	allBases.clear();
	location2base.clear();
	borderManager->reset();


	for(std::set<BWTA::BaseLocation*>::const_iterator bl = BWTA::getBaseLocations().begin(); bl != BWTA::getBaseLocations().end(); ++bl)
	{
		if (location2base.find(*bl) == location2base.end())
		{
			BWAPI::TilePosition tile = (*bl)->getTilePosition();
			std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(tile.x(), tile.y());
			for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); ++u)
				if ((*u)->getPlayer() == BWAPI::Broodwar->self() && (*u)->getType().isResourceDepot() && ((*u)->exists() || (*u)->isBeingConstructed()))
					addBase(*bl);
		}
	}
}

void BaseManager::updateRefineries()
{
	//if refinerys needed, build ONE refinery.
	if (this->isRefineryNeeded())
	{
		std::set<Base*> gasBases = this->getAllBasesWithGas();

		for(std::set<Base*>::iterator b = gasBases.begin(); b != gasBases.end(); ++b)
		{
			BWTA::BaseLocation* location = (*b)->getBaseLocation();
			if (!this->hasRefinery(location))
			{
				this->builder->buildAdditional(1,BWAPI::Broodwar->self()->getRace().getRefinery(),refineryBuildPriority,(*b)->getBaseLocation()->getTilePosition());
				break;
			}
		}
	}
}

bool BaseManager::isRefineryNeeded()
{
	return (BWAPI::Broodwar->getFrameCount() >= TAKEGAS && this->refineryNeeded > this->builder->getPlannedCount(BWAPI::Broodwar->self()->getRace().getRefinery()));
}

void BaseManager::setRefineryBuildPriority(int priority)
{
	if( refineryBuildPriority == 0 && BWAPI::Broodwar->getFrameCount() > TAKEGAS )
		refineryNeeded = allBases.size();

	refineryBuildPriority = priority;
}

int BaseManager::getRefineryBuildPriority()
{
	return this->refineryBuildPriority;
}

void BaseManager::addBase(BWTA::BaseLocation* location)
{
	Base* newBase = new Base(location);
	allBases.insert(newBase);
	this->location2base[location] = newBase;
	this->borderManager->addMyBase(location);
	myRegions.insert(location->getRegion());
}
void BaseManager::removeBase(BWTA::BaseLocation* location)
{
	std::map<BWTA::BaseLocation*,Base*>::iterator removeBase;

	removeBase = this->location2base.find(location);
	this->borderManager->removeMyBase(location);
	allBases.erase(removeBase->second);
	this->location2base.erase(removeBase);
	if (location == lastTakenBaseLocation)
		lastTakenBaseLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
}

Base* BaseManager::getBase(BWTA::BaseLocation* location)
{
	std::map<BWTA::BaseLocation*,Base*>::iterator i=location2base.find(location);
	if (i==location2base.end())
		return NULL;
	return i->second;
}

BWTA::BaseLocation* BaseManager::getLastTakenBaseLocation()
{
	return lastTakenBaseLocation;
}


BWTA::BaseLocation* BaseManager::expand(int priority)
{
	BWTA::BaseLocation* location=NULL;
	double minDist=-1;
	BWTA::BaseLocation* home=BWTA::getStartLocation(BWAPI::Broodwar->self());
	for(std::set<BWTA::BaseLocation*>::const_iterator i=BWTA::getBaseLocations().begin();i!=BWTA::getBaseLocations().end(); ++i)
	{
		double dist=home->getGroundDistance(*i);
		if (dist>0 && getBase(*i)==NULL)
		{
			if (minDist == -1 || dist<minDist)
			{
				minDist=dist;
				location=*i;
			}
		}
	}
	return expand(location,priority);
}
BWTA::BaseLocation* BaseManager::expand(BWTA::BaseLocation* location, int priority)
{
	if (location==NULL)
		return NULL;

	//this->builder->buildAdditional(1, BWAPI::UnitTypes::Protoss_Photon_Cannon, 90, 
	//		this->builder->getBuildManager()->getBuildingPlacer()->getBuildLocationNear(location->getTilePosition(), BWAPI::UnitTypes::Protoss_Photon_Cannon, 0));

	lastTakenBaseLocation = location;
	addBase(location);
	this->builder->buildAdditional(1, BWAPI::Broodwar->self()->getRace().getCenter(), priority, location->getTilePosition());
	//this->buildManager->build(BWAPI::Broodwar->self()->getRace().getCenter(), location->getTilePosition(), 0, 1);

	// also build a pylon when frames > 7000
	if ( BWAPI::Broodwar->getFrameCount() > 7000 )
	{
		UnitGroup minerals					= UnitGroup::getUnitGroup(location->getMinerals());
		UnitGroup geysers						= UnitGroup::getUnitGroup(location->getGeysers());
		BWAPI::Position posMineral	= minerals.getCenter();
		BWAPI::Position posGeyser		= geysers.getCenter();
		BWAPI::TilePosition tileMineral(posMineral);
		BWAPI::TilePosition tileGeyser(posGeyser);
		bool up = true, right = true, down = true, left = true;

		// minerals or geysers to the left
		if (tileMineral.x() <= location->getTilePosition().x() || tileGeyser.x() <= location->getTilePosition().x())
		{
			left = false;
		}

		// minerals or geysers to the right
		if (tileMineral.x() >= location->getTilePosition().x() + 4 || tileGeyser.x() >= location->getTilePosition().x() + 4)
		{
			right = false;
		}

		// minerals or geysers to the top
		if (tileMineral.y() <= location->getTilePosition().y() || tileGeyser.y() <= location->getTilePosition().y())
		{
			up = false;
		}

		// minerals or geysers to the bottom
		if (tileMineral.y() >= location->getTilePosition().y() + 3 || tileGeyser.y() >= location->getTilePosition().y() + 3)
		{
			down = false;
		}

		// if we are surrounded by minerals and geysers, choose to build in north.
		if (!left && !right && !down && !up)
			up = true;

		BWAPI::TilePosition *tilePylon;
		if (up)
		{
			tilePylon = new BWAPI::TilePosition(location->getTilePosition().x() + 4, location->getTilePosition().y() - 3);
			buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(location->getTilePosition().x() + 1, location->getTilePosition().y() - 3), 2, 2);
		}
		else if (right)
		{
			tilePylon = new BWAPI::TilePosition(location->getTilePosition().x() + 5, location->getTilePosition().y() + 4);
			buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(location->getTilePosition().x() + 5, location->getTilePosition().y() + 1), 2, 2);
		}
		else if (down)
		{
			tilePylon = new BWAPI::TilePosition(location->getTilePosition().x() - 2, location->getTilePosition().y() + 4);
			buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(location->getTilePosition().x() + 1, location->getTilePosition().y() + 4), 2, 2);
		}
		else if (left)
		{
			tilePylon = new BWAPI::TilePosition(location->getTilePosition().x() - 3, location->getTilePosition().y() - 2);
			buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(location->getTilePosition().x() - 3, location->getTilePosition().y() + 1), 2, 2);
		}
		this->builder->buildAdditional(1, BWAPI::UnitTypes::Protoss_Pylon, priority, *tilePylon);
		delete tilePylon;
	}

	if(!(location->isMineralOnly()))  
	{
		this->refineryNeeded++;
		const std::set<BWAPI::Unit*> closestGeyser = location->getGeysers();

		if (BWAPI::Broodwar->getFrameCount() >= TAKEGAS && !(this->hasRefinery(location)) && this->refineryBuildPriority != 0)
			this->builder->buildAdditional(1,BWAPI::Broodwar->self()->getRace().getRefinery(),priority, (*closestGeyser.begin())->getTilePosition());
	}
	return location;
}


std::set<Base*> BaseManager::getActiveBases() const
{
	std::set<Base*> activeBases;
	for(std::set<Base*>::const_iterator b = this->allBases.begin(); b != this->allBases.end(); ++b)
		if ((*b)->isActive())
			activeBases.insert(*b);
	return activeBases;
}

const std::set<BWTA::Region*>& BaseManager::getMyRegions() const
{
	return this->myRegions;
}

std::set<Base*> BaseManager::getAllBases() const
{
	return allBases;
}

std::set<Base*> BaseManager::getAllBasesWithGas()
{
	std::set<Base*> allBasesWithGas;

	for(std::set<Base*>::iterator b = this->allBases.begin(); b != this->allBases.end(); ++b)
		if ((*b)->hasGas())
		{
			allBasesWithGas.insert(*b);
		}
		return allBasesWithGas;
}

std::string BaseManager::getName()
{
	return "Base Manager";
}
void BaseManager::onRemoveUnit(BWAPI::Unit* unit)
{
	if( unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType().isResourceDepot() )
	{
		std::set<Base*> basesToErase;
		basesToErase.clear();

		//for(std::set<Base*>::iterator b = this->allBases.begin(); b != this->allBases.end(); ++b)
		for each(Base *b in allBases)
		{
			if(b->getResourceDepot() == unit)
			{
				if (unit->isCompleted() || unit->isBeingConstructed())
				{
					this->borderManager->removeMyBase(b->getBaseLocation());

					BWTA::BaseLocation* blocation  = b->getBaseLocation();
					removeBase(blocation);
					basesToErase.insert(b);
				}
				else
					b->setResourceDepot(NULL);
				break;
			}
			else if(b->getRefinery() == unit)
			{
				b->setRefinery(NULL);
				b->setActiveGas(false);
				break;
			}
		}

		for each (Base *b in basesToErase)
			allBases.erase(b);
	}
}

bool BaseManager::hasRefinery(BWTA::BaseLocation* location)
{
	bool refinery = false;

	//if base has gas
	if(!(location->isMineralOnly()))
	{
		std::set<BWAPI::Unit*> basegeysers = location->getGeysers();

		BWAPI::TilePosition geyserlocation;

		//cycle through geysers & get tile location
		for(std::set<BWAPI::Unit*>::iterator bg = basegeysers.begin(); bg != basegeysers.end(); ++bg)
		{
			geyserlocation = (*bg)->getInitialTilePosition();
		}

		//check for refinery already on geyser

		//get units on geyser
		std::set<BWAPI::Unit*> unitsOnGeyser = BWAPI::Broodwar->getUnitsOnTile(geyserlocation.x(),geyserlocation.y());

		//cycle through units on geyser
		for(std::set<BWAPI::Unit*>::iterator u = unitsOnGeyser.begin(); u != unitsOnGeyser.end(); ++u)
		{
			//if unit is a refinery
			if ((*u)->getType().isRefinery())
			{
				refinery = true;
			}
		}
	}

	return refinery;
}
