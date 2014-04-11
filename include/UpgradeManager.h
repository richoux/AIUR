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
#include <BuildingPlacer.h>
class UpgradeManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class Upgrade
	{
	public:
		BWAPI::UpgradeType type;
		int level;
	};
	UpgradeManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	void setBuildingPlacer(BuildingPlacer* placer);
	virtual void onOffer(std::set<BWAPI::Unit*> units);
	virtual void onRevoke(BWAPI::Unit* unit, double bid);
	virtual void update();
	virtual std::string getName() const;
	void onRemoveUnit(BWAPI::Unit* unit);
	bool upgrade(BWAPI::UpgradeType type, int level = -1);
	int getPlannedLevel(BWAPI::UpgradeType type) const;
	int getStartedLevel(BWAPI::UpgradeType type) const;
	int getCompletedLevel(BWAPI::UpgradeType type) const;

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
	BuildingPlacer* placer;
	std::map<BWAPI::UnitType,std::list< Upgrade > > upgradeQueues;
	std::map<BWAPI::Unit*,Upgrade> upgradingUnits;
	std::map<BWAPI::UpgradeType, int> plannedLevel;
	std::map<BWAPI::UpgradeType, int> startedLevel;
	int lastFrameCheck;
};