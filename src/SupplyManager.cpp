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

#include <SupplyManager.h>
#include <UnitGroupManager.h>
SupplyManager::SupplyManager()
{
	this->buildManager      = NULL;
	this->buildOrderManager = NULL;
	this->lastFrameCheck    = 0;
	this->seedPosition      = BWAPI::TilePositions::None;
}

void SupplyManager::setBuildManager(BuildManager* buildManager)
{
	this->buildManager = buildManager;
}
void SupplyManager::setBuildOrderManager(BuildOrderManager* buildOrderManager)
{
	this->buildOrderManager = buildOrderManager;
}
void SupplyManager::update()
{
	if (BWAPI::Broodwar->getFrameCount()>lastFrameCheck+25)
	{
		int productionCapacity       = 0;
		lastFrameCheck               = BWAPI::Broodwar->getFrameCount();
		std::set<BWAPI::Unit*> units = BWAPI::Broodwar->self()->getUnits();
		int supplyBuildTime = BWAPI::Broodwar->self()->getRace().getSupplyProvider().buildTime();
		int time = BWAPI::Broodwar->getFrameCount() + supplyBuildTime*2;
		for(std::set<BuildOrderManager::MetaUnit*>::iterator i = this->buildOrderManager->MetaUnitPointers.begin(); i != this->buildOrderManager->MetaUnitPointers.end(); ++i)
		{
			std::set<BWAPI::UnitType> m=this->buildOrderManager->unitsCanMake(*i,time);
			int max=0;
			for(std::set<BWAPI::UnitType>::iterator j=m.begin();j!=m.end(); ++j)
			{
				int s=j->supplyRequired();
				if (j->isTwoUnitsInOneEgg())
					s*=2;
				if (j->buildTime()<supplyBuildTime && (*i)->getType().getRace()!=BWAPI::Races::Zerg)
					s*=2;
				if (s > max)
					max=s;
			}
			productionCapacity += max;
		}
		if (getPlannedSupply() <= BWAPI::Broodwar->self()->supplyUsed() + productionCapacity)
		{
			// first pylon in a good place
			if (BWAPI::Broodwar->self()->getRace() == BWAPI::Races::Protoss && BWAPI::Broodwar->self()->visibleUnitCount(BWAPI::UnitTypes::Protoss_Pylon) == 0)
			{
				BWAPI::TilePosition start		= BWAPI::Broodwar->self()->getStartLocation();
				BWTA::BaseLocation *bl			= BWTA::getNearestBaseLocation(start);
				UnitGroup minerals					= UnitGroup::getUnitGroup(bl->getMinerals());
				UnitGroup geysers						= UnitGroup::getUnitGroup(bl->getGeysers());
				BWAPI::Position posMineral	= minerals.getCenter();
				BWAPI::Position posGeyser		= geysers.getCenter();
				BWAPI::TilePosition tileMineral(posMineral);
				BWAPI::TilePosition tileGeyser(posGeyser);
				bool up = true, right = true, down = true, left = true;

				// minerals or geysers to the left
				if (tileMineral.x() <= start.x() || tileGeyser.x() <= start.x())
				{
					left = false;
				}

				// minerals or geysers to the right
				if (tileMineral.x() >= start.x() + 4 || tileGeyser.x() >= start.x() + 4)
				{
					right = false;
				}

				// minerals or geysers to the top
				if (tileMineral.y() <= start.y() || tileGeyser.y() <= start.y())
				{
					up = false;
				}

				// minerals or geysers to the bottom
				if (tileMineral.y() >= start.y() + 3 || tileGeyser.y() >= start.y() + 3)
				{
					down = false;
				}

				// if we are surrounded by minerals and geysers, choose to build in north.
				if (!left && !right && !down && !up)
					up = true;

				BWAPI::TilePosition *tilePylon;
				if (up)
				{
					tilePylon = new BWAPI::TilePosition(start.x() + 4, start.y() - 3);
					buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(start.x() + 1, start.y() - 3), 2, 2);
				}
				else if (right)
				{
					tilePylon = new BWAPI::TilePosition(start.x() + 5, start.y() + 4);
					buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(start.x() + 5, start.y() + 1), 2, 2);
				}
				else if (down)
				{
					tilePylon = new BWAPI::TilePosition(start.x() - 2, start.y() + 4);
					buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(start.x() + 1, start.y() + 4), 2, 2);
				}
				else if (left)
				{
					tilePylon = new BWAPI::TilePosition(start.x() - 3, start.y() - 2);
					buildManager->getBuildingPlacer()->reserveTiles(BWAPI::TilePosition(start.x() - 3, start.y() + 1), 2, 2);
				}
				this->buildOrderManager->buildAdditional(1, BWAPI::UnitTypes::Protoss_Pylon, 1000, *tilePylon);
				delete tilePylon;
			}
			else
				this->buildOrderManager->buildAdditional(1,BWAPI::Broodwar->self()->getRace().getSupplyProvider(),1000,seedPosition);
		}
	}
}

std::string SupplyManager::getName() const
{
	return "Supply Manager";
}

int SupplyManager::getPlannedSupply() const
{
	int plannedSupply=0;
	//planned supply depends on the the amount of planned supply providers times the amount of supply they provide.
	plannedSupply+=buildOrderManager->getPlannedCount(BWAPI::UnitTypes::Terran_Supply_Depot)*BWAPI::UnitTypes::Terran_Supply_Depot.supplyProvided();
	plannedSupply+=buildOrderManager->getPlannedCount(BWAPI::UnitTypes::Protoss_Pylon)*BWAPI::UnitTypes::Protoss_Pylon.supplyProvided();
	plannedSupply+=buildOrderManager->getPlannedCount(BWAPI::UnitTypes::Zerg_Overlord)*BWAPI::UnitTypes::Zerg_Overlord.supplyProvided();

	plannedSupply+=SelectAll(BWAPI::UnitTypes::Terran_Command_Center).size()*BWAPI::UnitTypes::Terran_Command_Center.supplyProvided();
	plannedSupply+=SelectAll(BWAPI::UnitTypes::Protoss_Nexus).size()*BWAPI::UnitTypes::Protoss_Nexus.supplyProvided();
	plannedSupply+=SelectAll(BWAPI::UnitTypes::Zerg_Hatchery).size()*BWAPI::UnitTypes::Zerg_Hatchery.supplyProvided();
	plannedSupply+=SelectAll(BWAPI::UnitTypes::Zerg_Lair).size()*BWAPI::UnitTypes::Zerg_Lair.supplyProvided();
	plannedSupply+=SelectAll(BWAPI::UnitTypes::Zerg_Hive).size()*BWAPI::UnitTypes::Zerg_Hive.supplyProvided();
	return plannedSupply;
}
int SupplyManager::getSupplyTime(int supplyCount) const
{
	if (getPlannedSupply()<supplyCount)
		return -1; //not planning to make this much supply

	if (BWAPI::Broodwar->self()->supplyTotal()>=supplyCount)
		return BWAPI::Broodwar->getFrameCount(); //already have this much supply

	int supply=BWAPI::Broodwar->self()->supplyTotal();
	int time = BWAPI::Broodwar->getFrameCount();
	std::set<BWAPI::Unit*> units = SelectAll()(-isCompleted);
	std::map<int, int> supplyAdditions;
	for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); ++i)
	{
		if ((*i)->getType().supplyProvided()>0)
		{
			supplyAdditions[time+(*i)->getRemainingBuildTime()]+=(*i)->getType().supplyProvided();
		}
	}
	for(std::map<int,int>::iterator i=supplyAdditions.begin();i!=supplyAdditions.end(); ++i)
	{
		time=i->second;
		supply+=i->first;
		if (supply>=supplyCount)
			return time;
	}
	return -1;
}

void SupplyManager::setSeedPosition(BWAPI::TilePosition p)
{
	this->seedPosition = p;
}