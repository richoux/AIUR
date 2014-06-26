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
#include <BWAPI.h>
#include <BWTA.h>
#include <set>
class Base
{
public:
	Base(BWTA::BaseLocation* location);
	BWTA::BaseLocation* getBaseLocation() const;
	BWAPI::Unit* getResourceDepot() const;
	BWAPI::Unit* getRefinery() const;
	std::set<BWAPI::Unit*> getMinerals() const;
	std::set<BWAPI::Unit*> getGeysers() const;
	bool isActive() const;
	bool isActiveGas() const;
	bool isBeingConstructed() const;

	bool hasGas() const;
	bool hasPhoton;

	void setResourceDepot(BWAPI::Unit* unit);
	void setRefinery(BWAPI::Unit* unit);
	void setActive(bool active);
	void setActiveGas(bool active);

private:
	BWTA::BaseLocation* baseLocation;
	BWAPI::Unit* resourceDepot;
	BWAPI::Unit* refinery;
	std::set<BWAPI::Unit*> minerals;
	bool active;
	bool activeGas;
	bool beingConstructed;
};
