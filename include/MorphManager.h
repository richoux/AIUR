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
class MorphManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class Unit
	{
	public:
		BWAPI::UnitType type;
		bool started;
	};
	MorphManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	virtual void onOffer(std::set<BWAPI::Unit*> units);
	virtual void onRevoke(BWAPI::Unit* unit, double bid);
	virtual void update();
	virtual std::string getName() const;
	virtual std::string getShortName() const;

	void onRemoveUnit(BWAPI::Unit* unit);
	bool morph(BWAPI::UnitType type);
	int getPlannedCount(BWAPI::UnitType type) const;
	int getStartedCount(BWAPI::UnitType type) const;
	BWAPI::UnitType getBuildType(BWAPI::Unit* unit) const;

private:
	bool canMake(BWAPI::Unit* builder, BWAPI::UnitType type);
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
	std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> > morphQueues;
	std::map<BWAPI::Unit*,Unit> morphingUnits;
	std::map<BWAPI::UnitType, int> plannedCount;
	std::map<BWAPI::UnitType, int> startedCount;
};