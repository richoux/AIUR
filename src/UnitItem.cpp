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

#include "BuildOrderManager/UnitItem.h"
#include "BuildManager.h"
BuildManager*& UnitItem::getBuildManager()
{
	static BuildManager* buildManager;
	return buildManager;
}
UnitItem::UnitItem()
{
	this->type=BWAPI::UnitTypes::None;
	this->nonadditional=0;
	nonadditionalPosition=BWAPI::TilePositions::None;
}
UnitItem::UnitItem(BWAPI::UnitType type)
{
	this->type=type;
	this->nonadditional=0;
	nonadditionalPosition=BWAPI::TilePositions::None;
}

void UnitItem::nonAdditionalReset()
{
	this->nonadditional = 0;
}

int UnitItem::getRemainingCount(int currentPlannedCount)
{
	//initialize counter
	int count=0;

	//if non-additional is not zero
	if (this->nonadditional!=0)
	{
		if (currentPlannedCount==-1)
			currentPlannedCount=UnitItem::getBuildManager()->getPlannedCount(this->type);

		count = this->nonadditional-currentPlannedCount;

		//make sure count is no less than 0
		if (count<0) count=0;
	}

	//loop through all additional unit items, and increment count as needed
	for(std::map<BWAPI::TilePosition, int>::iterator k=this->additional.begin();k!=this->additional.end(); ++k)
	{
		count+=k->second;
	}

	//return the total count
	return count;
}

void UnitItem::addAdditional(int count, BWAPI::TilePosition position)
{
	this->additional[position]+=count;
}

void UnitItem::setNonAdditional(int count, BWAPI::TilePosition position)
{
	this->nonadditional=count;
	this->nonadditionalPosition=position;
}

BWAPI::TilePosition UnitItem::decrementAdditional(BWAPI::TilePosition position)
{
	if (this->additional.empty()) return this->nonadditionalPosition;
	if (position==BWAPI::TilePositions::None)
	{
		(*this->additional.begin()).second--;
		position=(*this->additional.begin()).first;
		if ((*this->additional.begin()).second==0)
			this->additional.erase(this->additional.begin());
	}
	else
	{
		if (this->additional.find(position)==this->additional.end())
			return position;
		this->additional[position]--;
		if (this->additional[position]<=0)
			this->additional.erase(position);
	}
	return position;
}