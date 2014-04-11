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

class UnitStates
{
public:
	enum State
	{
		Idle,
		Moving,
		Guarding,
		Waiting,
		InAttackAnimation,
		Attacking,
		Defending,
		Fleeing,
		Dodging,
		Roaming,
		Searching,
		Scouting,
		Harassing,
		Following,
		Spying,
		Cheesing,
		Dropping
	};

	inline UnitStates(){}
	inline ~UnitStates(){}
	inline State		getState( BWAPI::Unit *u )			{ return unitStatesMap[u]; }
	inline void			setState( BWAPI::Unit *u, State s )	{ unitStatesMap[u] = s; }
	inline bool			isState	( BWAPI::Unit *u, State s )	{ return unitStatesMap[u] == s; }
	inline void			remove	( BWAPI::Unit *u )			{ unitStatesMap.erase(u); }
	std::string			getName	( BWAPI::Unit *u )
	{ 
		switch( getState( u ) )
		{
		case Idle:
			return "Idle";
		case Moving:
			return "Moving";
		case Guarding:
			return "Guarding";
		case Waiting:
			return "Waiting";
		case InAttackAnimation:
			return "InAttackAnimation";
		case Attacking:
			return "Attacking";
		case Defending:
			return "Defending";
		case Fleeing:
			return "Fleeing";
		case Dodging:
			return "Dodging";
		case Roaming:
			return "Roaming";
		case Searching:
			return "Searching";
		case Scouting:
			return "Scouting";
		case Harassing:
			return "Harassing";
		case Following:
			return "Following";
		case Spying:
			return "Spying";
		case Cheesing:
			return "Cheesing";
		case Dropping:
			return "Dropping";
		default:
			;
		}
		return "Unknow";
	}

private:
	std::map<BWAPI::Unit*, State> unitStatesMap;
};