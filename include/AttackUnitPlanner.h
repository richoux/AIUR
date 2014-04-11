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
#include <BWAPI.h>
#include <UnitGroup.h>
#include <AttackTracker.h>

class AttackUnitPlanner
{
public:
	static AttackUnitPlanner* create();
	static void destroy();

	void update();
	void			setAttackTracker( AttackTracker* );
	BWAPI::Unit*	chooseNextTarget( BWAPI::Unit *attacker );
	bool			attackNextTarget( BWAPI::Unit *attacker );

private:
	AttackUnitPlanner();
	~AttackUnitPlanner();

	AttackTracker *attackTracker;

	double computeRatio(BWAPI::Unit *attacker, BWAPI::Unit *target);

	class AttackData
	{
	public:
		int HPLeft;
		BWAPI::Unit *attacker;
		int attackFrame;
	};

	// List (actually vector) of attacks from our units attaking or planned to attack a enemy unit (the key map).
	// This list is ordered by increasing attack frame, i.e., from the first unit to attack till the last one,
	// until the enemy unit dies.
	// Note that the same unit can be several times in a same list.
	std::map<BWAPI::Unit*, std::vector<AttackUnitPlanner::AttackData> > attackersList;
};

extern AttackUnitPlanner *TheAttackUnitPlanner;
