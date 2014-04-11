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
#include <InformationManager.h>
#include <BuildOrderManager.h>
#include <UnitGroupManager.h>
#include <MoodManager.h>
#include <Random.h>
#include <BWAPI.h>
#include <UnitStates.h>
#include <stack>

class SpyManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class SpyData
	{
	public:
		inline SpyData():target(NULL),lastFrameDodging(0){}
		BWTA::BaseLocation *target;
		int lastFrameDodging;
	};

	SpyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	SpyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>*, UnitStates*);
	~SpyManager();

	virtual void onOffer	(std::set<BWAPI::Unit*> units);
	virtual void onRevoke	(BWAPI::Unit* unit, double bid);
	virtual void update		();

	virtual std::string getName		() const;
	virtual std::string getShortName() const;
	void				onRemoveUnit(BWAPI::Unit* unit);
	void				removeBid	(BWAPI::Unit* unit);
	void				removeBid	();

	// Non-Controller methods.
	void	buildObserver			(bool);
	void	setInformationManager	(InformationManager*);
	void	setBuildOrderManager	(BuildOrderManager*);
	void	setMoodManager			(MoodManager*);
	void	increaseSpyNumber		();
	void	setDebugMode			(bool);
	void	initialize				();
	bool	isInitialized			();
	int		getNumberToSpy			();
	int		getNumberSpies			();
	int		addSight				();

private:
	void drawAssignments();
	Arbitrator::Arbitrator<BWAPI::Unit*,double>	*arbitrator;
	InformationManager	*informationManager;
	BuildOrderManager	*buildOrderManager;
	MoodManager			*moodManager;
	UnitStates			*unitStates;

	Random				*randomDodge;

	std::map<BWAPI::Unit*, SpyData>		spies;
	std::set<BWAPI::Unit*>				obsDefenders;
	std::stack <BWTA::BaseLocation*>	baseLocationsToSpy;
	std::set<BWTA::BaseLocation*>		baseLocationsSpied;
	BWTA::BaseLocation					*myStartLocation;
	const BWTA::BaseLocation			*enemyStartLocation;

	int		numberBasesToSpy;
	int		numberAdditionalBasesToSpy;
	bool	debugMode;
	int		lastFrameCheck;
	int		lastIncreaseSpyNumber;
	bool	initialized;
};
