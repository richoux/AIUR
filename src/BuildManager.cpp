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

#include <BuildManager.h>
#include <BuildingPlacer.h>
#include <ConstructionManager.h>
#include <ProductionManager.h>
#include <MorphManager.h>
BuildManager::BuildManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator=arbitrator;
	this->buildingPlacer=new BuildingPlacer();
	this->constructionManager=new ConstructionManager(this->arbitrator,this->buildingPlacer);
	this->productionManager=new ProductionManager(this->arbitrator,this->buildingPlacer);
	this->morphManager=new MorphManager(this->arbitrator);
	this->debugMode = false;
}

BuildManager::~BuildManager()
{
	delete this->buildingPlacer;
	delete this->constructionManager;
	delete this->productionManager;
	delete this->morphManager;
}

void BuildManager::update()
{
	this->constructionManager->update();
	this->productionManager->update();
	this->morphManager->update();
	if (this->debugMode)
	{
		for( int x = 0; x < BWAPI::Broodwar->mapWidth(); ++x )
			for( int y = 0; y < BWAPI::Broodwar->mapHeight(); ++y )
				if( this->buildingPlacer->isReserved( x, y ) )
					BWAPI::Broodwar->drawBoxMap( x*32, y*32, x*32+32, y*32+32, BWAPI::Colors::Red );
	}
}

std::string BuildManager::getName() const
{
	return "Build Manager";
}

int BuildManager::cancelAllBuildingsOfType(BWAPI::UnitType type)
{
	if (type.isBuilding())
		return this->constructionManager->cancelAllBuildingsOfType(type);
	return 0;
}

BuildingPlacer* BuildManager::getBuildingPlacer() const
{
	return this->buildingPlacer;
}
void BuildManager::onRemoveUnit(BWAPI::Unit* unit)
{
	this->constructionManager->onRemoveUnit(unit);
	this->productionManager->onRemoveUnit(unit);
	this->morphManager->onRemoveUnit(unit);
}

bool BuildManager::build(BWAPI::UnitType type)
{
	return build(type, BWAPI::Broodwar->self()->getStartLocation(),false);
}

bool BuildManager::build(BWAPI::UnitType type, bool forceNoAddon)
{
	return build(type, BWAPI::Broodwar->self()->getStartLocation(),forceNoAddon);
}
bool BuildManager::build(BWAPI::UnitType type, BWAPI::TilePosition goalPosition)
{
	return build(type, goalPosition,false);
}

bool BuildManager::build(BWAPI::UnitType type, BWAPI::TilePosition goalPosition, bool forceNoAddon, bool forcePosition)
{
	if (type==BWAPI::UnitTypes::None || type==BWAPI::UnitTypes::Unknown) return false;

	//send this order off to the right sub-manager
	if (type.getRace()==BWAPI::Races::Zerg && type.isBuilding()==type.whatBuilds().first.isBuilding())
		return this->morphManager->morph(type);
	else
	{
		if (type.isBuilding())
			return this->constructionManager->build(type, goalPosition, forcePosition);
		else
			return this->productionManager->train(type,forceNoAddon);
	}
	return false;
}

int BuildManager::getPlannedCount(BWAPI::UnitType type) const
{
	if (type.getRace()==BWAPI::Races::Zerg && type.isBuilding()==type.whatBuilds().first.isBuilding())
		return BWAPI::Broodwar->self()->completedUnitCount(type)+this->morphManager->getPlannedCount(type);
	else
	{
		if (type.isBuilding())
			return BWAPI::Broodwar->self()->completedUnitCount(type)+this->constructionManager->getPlannedCount(type);
		else
			return BWAPI::Broodwar->self()->completedUnitCount(type)+this->productionManager->getPlannedCount(type);
	}
}

int BuildManager::getStartedCount(BWAPI::UnitType type) const
{
	if (type.getRace()==BWAPI::Races::Zerg && type.isBuilding()==type.whatBuilds().first.isBuilding())
		return BWAPI::Broodwar->self()->completedUnitCount(type)+this->morphManager->getStartedCount(type);
	else
	{
		if (type.isBuilding())
			return BWAPI::Broodwar->self()->completedUnitCount(type)+this->constructionManager->getStartedCount(type);
		else
			return BWAPI::Broodwar->self()->completedUnitCount(type)+this->productionManager->getStartedCount(type);
	}
}

int BuildManager::getCompletedCount(BWAPI::UnitType type) const
{
	return BWAPI::Broodwar->self()->completedUnitCount(type);
}

void BuildManager::setBuildDistance(int distance)
{
	this->buildingPlacer->setBuildDistance(distance);
}
BWAPI::UnitType BuildManager::getBuildType(BWAPI::Unit* unit) const
{
	BWAPI::UnitType t=this->productionManager->getBuildType(unit);
	if (t==BWAPI::UnitTypes::None)
		t=this->morphManager->getBuildType(unit);
	return t;
}
void BuildManager::setDebugMode(bool debugMode)
{
	this->debugMode=debugMode;
}