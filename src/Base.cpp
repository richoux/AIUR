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

#include <BaseManager.h>
Base::Base(BWTA::BaseLocation* location)
{
	this->baseLocation			= location;
	this->resourceDepot			= NULL;
	this->refinery					= NULL;
	this->active						= false;
	this->activeGas					= false;
	this->beingConstructed	= false;
	this->hasPhoton					= false;
}

BWTA::BaseLocation* Base::getBaseLocation() const
{
	return this->baseLocation;
}

BWAPI::Unit* Base::getResourceDepot() const
{
	return this->resourceDepot;
}

BWAPI::Unit* Base::getRefinery() const
{
	return this->refinery;
}

std::set<BWAPI::Unit*> Base::getMinerals() const
{
	return this->baseLocation->getMinerals();
}

std::set<BWAPI::Unit*> Base::getGeysers() const
{
	return this->baseLocation->getGeysers();
}

bool Base::isActive() const
{
	return this->active;
}

bool Base::isActiveGas() const
{
	return this->activeGas;
}

bool Base::isBeingConstructed() const
{
	return this->beingConstructed;
}

bool Base::hasGas() const
{
	bool gas = false;

	//if Baselocation has a vespene geyser
	if(!(this->getBaseLocation()->isMineralOnly()))
	{
		gas = true;
	}
	return gas;
}

void Base::setResourceDepot(BWAPI::Unit* unit)
{
	this->resourceDepot = unit;
}

void Base::setRefinery(BWAPI::Unit* unit)
{
	this->refinery = unit;
}

void Base::setActive(bool active)
{
	this->active = active;
}

void Base::setActiveGas(bool active)
{
	this->activeGas = active;
}
