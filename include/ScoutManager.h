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
#include <InformationManager.h>
#include <BuildManager.h>
#include <ArmyManager.h>
#include <BWAPI.h>
#include <Random.h>
#include <HashMap.h>
#include <BWTA.h>
#include <UnitStates.h>

class ScoutManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>*, InformationManager*, BuildManager*);
	ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>*, InformationManager*, BuildManager*, UnitStates*);
	~ScoutManager();

	virtual void onOffer(std::set<BWAPI::Unit*>);
	virtual void onRevoke(BWAPI::Unit*, double);
	virtual void update();

	virtual std::string getName() const;
	virtual std::string getShortName() const;
	void onRemoveUnit(BWAPI::Unit* unit);

	void setArmyManager(ArmyManager*);

	// Non-Controller methods.
	bool isScouting() const;
	bool isScouting(BWAPI::Unit*);
	void setScoutCount(int count);
	void setDebugMode(bool debugMode);

	std::map<BWAPI::Unit*, BWTA::BaseLocation*> targetOf;
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;

	std::list<BWTA::BaseLocation*> baseLocationsToScout;
	std::set<BWTA::BaseLocation*> baseLocationsExplored;
	BWTA::BaseLocation *myStartLocation;

private:
	bool needMoreScouts			() const;
	void requestScout			(double bid);
	void updateScoutAssignments	();
	void drawAssignments		();

	InformationManager  *informationManager;
	BuildManager		*buildManager;
	ArmyManager			*armyManager;
	//Random				*randomPylon;
	UnitStates			*unitStates;
	HashMap				 hashMap;

	size_t desiredScoutCount;
	int scoutingStartFrame;
	bool debugMode;
	int lastFrameCheck;
	bool scoutDone;
	bool mustContinue;

	BWTA::Polygon			poly;
	BWTA::Polygon::iterator itPoly;
	BWAPI::Position			mineralsCenter;
};
