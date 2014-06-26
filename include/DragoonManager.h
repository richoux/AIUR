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
#include <UnitGroupManager.h>
#include <InformationManager.h>
#include <AttackUnitPlanner.h>
#include <BWAPI.h>
#include <UnitStates.h>

using namespace BWAPI;
using namespace std;

class DragoonManager
{
public:
	class DragoonData
	{
	public:
		inline DragoonData() : lastFrameDodge(0), lastFrameAttack(0), lastTarget(NULL){}
		int lastFrameDodge;
		int lastFrameAttack;
		Unit* lastTarget;
	};

	DragoonManager();
	DragoonManager( UnitStates* );

	bool					availableToAttack			( Unit* );	
	void					setMyDragoons				( map<Unit*, DragoonData> );
	map<Unit*, DragoonData>	getMyDragoons				();
	void					addDragoon					( Unit* );
	void					deleteDragoon				( Unit* );
	void					update						();
	void					setInformationManager		( InformationManager* );
	void					setAttackUnitPlanner		( AttackUnitPlanner* );

private:
	map<Unit*, DragoonData>	myDragoons;
	int						lastFrameCheck;
	InformationManager		*informationManager;
	AttackUnitPlanner		*attackUnitPlanner;
	UnitStates				*unitStates;
};