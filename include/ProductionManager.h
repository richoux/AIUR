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

#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BuildingPlacer.h>
class ProductionManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer);
	virtual void onOffer(std::set<BWAPI::Unit*> units);
	virtual void onRevoke(BWAPI::Unit* unit, double bid);
	virtual void update();
	virtual std::string getName() const;
	virtual std::string getShortName() const;

	void onRemoveUnit(BWAPI::Unit* unit);
	bool train(BWAPI::UnitType type, bool forceNoAddon=false);
	int getPlannedCount(BWAPI::UnitType type) const;
	int getStartedCount(BWAPI::UnitType type) const;
	BWAPI::UnitType getBuildType(BWAPI::Unit* unit) const;

private:
	class ProductionUnitType
	{
	public:
		BWAPI::UnitType type;
		bool forceNoAddon;
	};
	class Unit
	{
	public:
		ProductionUnitType type;
		int lastAttemptFrame;
		BWAPI::Unit* unit;
		bool started;
	};
	bool canMake(BWAPI::Unit* builder, BWAPI::UnitType type);
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
	std::map<BWAPI::UnitType,std::list<ProductionUnitType > > productionQueues;
	std::map<BWAPI::Unit*,Unit> producingUnits;
	BuildingPlacer* placer;
	std::map<BWAPI::UnitType, int> plannedCount;
	std::map<BWAPI::UnitType, int> startedCount;
};