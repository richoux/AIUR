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

#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
class BuildingPlacer;
class ConstructionManager;
class ProductionManager;
class MorphManager;
class BuildManager
{
public:
	BuildManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	~BuildManager();
	void update();
	std::string getName() const;
	BuildingPlacer* getBuildingPlacer() const;
	void onRemoveUnit(BWAPI::Unit* unit);
	bool build(BWAPI::UnitType type);
	bool build(BWAPI::UnitType type, bool forceNoAddon);
	bool build(BWAPI::UnitType type, BWAPI::TilePosition goalPosition);
	bool build(BWAPI::UnitType type, BWAPI::TilePosition goalPosition, bool forceNoAddon, bool forcePosition = false);
	int getPlannedCount(BWAPI::UnitType type) const;
	int getStartedCount(BWAPI::UnitType type) const;
	int getCompletedCount(BWAPI::UnitType type) const;
	void setBuildDistance(int distance);
	BWAPI::UnitType getBuildType(BWAPI::Unit* unit) const;
	void setDebugMode(bool debugMode);
	int cancelAllBuildingsOfType(BWAPI::UnitType);

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
	BuildingPlacer* buildingPlacer;
	ConstructionManager* constructionManager;
	ProductionManager* productionManager;
	MorphManager* morphManager;
	bool debugMode;
};