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
#include <map>
#include <set>
#include <BWAPI.h>
#include <Arbitrator.h>
#include <BWTA.h>
#include <BuildManager.h>
#include <MoodManager.h>
#include "Base.h"

class BuildOrderManager;
class BorderManager;
class BaseManager
{
public:
	BaseManager();
	void				setBuildOrderManager	(BuildOrderManager*);
	void				setBuildManager			(BuildManager*);
	void				setBorderManager		(BorderManager*);
	void				setMoodManager			(MoodManager*);
	void				update					();
	void				resetBases				();
	void				updateRefineries		();
	Base*				getBase					(BWTA::BaseLocation* location);
	BWTA::BaseLocation*	getLastTakenBaseLocation();
	BWTA::BaseLocation*	expand					(int priority = 100);
	BWTA::BaseLocation*	expand					(BWTA::BaseLocation* location, int priority = 100);
	std::set<Base*>		getActiveBases			() const;
	std::set<Base*>		getAllBases				() const;
	std::set<Base*>		getAllBasesWithGas		();

	const std::set<BWTA::Region*>& getMyRegions	() const;

	std::string	getName					();
	void		onRemoveUnit			(BWAPI::Unit* unit);
	bool		hasRefinery				(BWTA::BaseLocation* location);
	bool		isRefineryNeeded		();
	void		setRefineryBuildPriority(int priority);
	int			getRefineryBuildPriority();

private:
	void addBase	(BWTA::BaseLocation* location);
	void removeBase	(BWTA::BaseLocation* location);

	BuildOrderManager					*builder;
	BorderManager						*borderManager;
	BuildManager						*buildManager;
	MoodManager							*moodManager;
	std::map<BWTA::BaseLocation*,Base*> location2base;
	std::set<Base*>						allBases;
	std::set<BWTA::Region*>				myRegions;
	BWTA::BaseLocation					*lastTakenBaseLocation;
	int									refineryNeeded;
	int									refineryBuildPriority;
	int									lastFrameCheck;
};