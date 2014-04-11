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

#include <MorphManager.h>

MorphManager::MorphManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;
	for(std::set<BWAPI::UnitType>::const_iterator i=BWAPI::UnitTypes::allUnitTypes().begin();i!=BWAPI::UnitTypes::allUnitTypes().end(); ++i)
	{
		plannedCount[*i]=0;
		startedCount[*i]=0;
	}
}

bool MorphManager::canMake(BWAPI::Unit* builder, BWAPI::UnitType type)
{
	if (builder != NULL)
	{
		/* Check if the owner of the unit is you */
		if (builder->getPlayer() != BWAPI::Broodwar->self())
			return false;

		/* Check if this unit can actually build the unit type */
		if (builder->getType() != (type.whatBuilds().first))
			return false;

		/* Carrier space */
		if (builder->getType() == BWAPI::UnitTypes::Protoss_Carrier)
		{
			int max_amt = 4;
			if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Carrier_Capacity)>0)
				max_amt += 4;
			if (builder->getInterceptorCount() + (int)builder->getTrainingQueue().size() >= max_amt)
				return false;
		}
		/* Reaver Space */
		if (builder->getType() == BWAPI::UnitTypes::Protoss_Reaver)
		{
			int max_amt = 5;
			if (BWAPI::Broodwar->self()->getUpgradeLevel(BWAPI::UpgradeTypes::Reaver_Capacity) > 0)
				max_amt += 5;
			if (builder->getScarabCount() + (int)builder->getTrainingQueue().size() >= max_amt)
				return false;
		}
	}

	BWAPI::UnitType addon = BWAPI::UnitTypes::None;
	for(std::map<BWAPI::UnitType, int>::const_iterator i = type.requiredUnits().begin(); i != type.requiredUnits().end(); ++i)
		if (i->first.isAddon())
			addon=i->first;

	for(std::map<BWAPI::UnitType, int>::const_iterator i = type.requiredUnits().begin(); i != type.requiredUnits().end(); ++i)
	{
		bool pass = false;
		if (BWAPI::Broodwar->self()->completedUnitCount(i->first) >= i->second)
			pass = true;
		if (i->first == BWAPI::UnitTypes::Zerg_Hatchery)
		{
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Lair) >= i->second)
				pass = true;
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) >= i->second)
				pass = true;
		}
		if (i->first == BWAPI::UnitTypes::Zerg_Lair)
			if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Zerg_Hive) >= i->second)
				pass = true;
		if (pass == false)
			return false;
	}

	if (type.requiredTech() != BWAPI::TechTypes::None)
		if (!BWAPI::Broodwar->self()->hasResearched((type.requiredTech())))
			return false;

	if (builder != NULL)
		if (addon != BWAPI::UnitTypes::None && addon.whatBuilds().first==type.whatBuilds().first)
			if (builder->getAddon() == NULL || builder->getAddon()->getType() != addon)
				return false;
	return true;
}

void MorphManager::onOffer(std::set<BWAPI::Unit*> units)
{
	//go through all the units that are being offered to us
	for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end(); ++i)
	{
		//we will go through all the units in the morph queue
		std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator q=morphQueues.find((*i)->getType());
		bool used=false;
		if (q!=morphQueues.end() && !q->second.empty()) //well, only if the queue actually exists
		{
			for(std::list<BWAPI::UnitType>::iterator t=q->second.begin();t!=q->second.end(); ++t) //loop through the queue
			{
				if (canMake(*i,*t)) //only accept if the given unit can make the type of unit we want to make
				{
					Unit newUnit;
					newUnit.type=*t;
					newUnit.started=false;
					morphingUnits.insert(std::make_pair(*i,newUnit));
					q->second.erase(t);
					//tell the arbitrator we accept the unit, and raise the bid to hopefully prevent other managers from using it
					arbitrator->accept(this,*i);
					arbitrator->setBid(this,*i,100.0);
					used=true;
					break;
				}
				else
				{
					if (BWAPI::Broodwar->self()->minerals()<(*t).mineralPrice() || BWAPI::Broodwar->self()->gas()<(*t).gasPrice())
					{
						break;
					}
				}
			}
		}
		//if we didnt use this unit, tell the arbitrator we decline it
		if (!used)
		{
			arbitrator->decline(this,*i,0);
			arbitrator->removeBid(this,*i);
		}
	}
}

void MorphManager::onRevoke(BWAPI::Unit* unit, double bid)
{
	std::map<BWAPI::Unit*,Unit>::iterator m=morphingUnits.find(unit);
	if (m!=morphingUnits.end())
	{
		BWAPI::UnitType type=m->second.type;
		if (m->first->getType()!=type || m->first->isMorphing())
			morphQueues[type.whatBuilds().first].push_front(type);
		morphingUnits.erase(m);
	}
}

void MorphManager::update()
{
	std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
	for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); ++u)
	{
		std::map<BWAPI::UnitType,std::list<BWAPI::UnitType> >::iterator q=morphQueues.find((*u)->getType());
		if (q!=morphQueues.end() && !q->second.empty() && (*u)->isCompleted() && morphingUnits.find(*u)==morphingUnits.end())
			arbitrator->setBid(this, *u, 50);
	}
	std::map<BWAPI::Unit*,Unit>::iterator i_next;
	//iterate through all the morphing units
	for(std::map<BWAPI::Unit*,Unit>::iterator i=morphingUnits.begin();i!=morphingUnits.end();i=i_next)
	{
		i_next=i;
		i_next++;
		if (i->first->isCompleted())
		{
			//if the unit is completed and is the right type, we are done
			if (i->first->getType()==i->second.type)
			{
				arbitrator->removeBid(this, i->first);
				plannedCount[i->second.type]--;
				startedCount[i->second.type]--;
				morphingUnits.erase(i);
			}
			else //if the unit is not the right type, tell it to morph into the right type
				if (BWAPI::Broodwar->canMake(NULL,i->second.type))
					i->first->morph(i->second.type);
		}
		else if (i->first->isMorphing())
		{
			//if the unit is morphing, note that we have started it in the startedCount
			if (!i->second.started)
			{
				startedCount[i->second.type]++;
				i->second.started=true;
			}
		}
	}
}

std::string MorphManager::getName() const
{
	return "Morph Manager";
}

std::string MorphManager::getShortName() const
{
	return "Morph";
}

void MorphManager::onRemoveUnit(BWAPI::Unit* unit)
{
	std::map<BWAPI::Unit*,Unit>::iterator m=morphingUnits.find(unit);
	if (m!=morphingUnits.end())
	{
		BWAPI::UnitType type=m->second.type;
		morphQueues[type.whatBuilds().first].push_front(type);
		morphingUnits.erase(m);
	}
}

bool MorphManager::morph(BWAPI::UnitType type)
{
	//morph order starts here

	//we only accept buildings that are made from other buildings
	//and units that are made from other units

	//fix: we should check to make sure the type is not an addon

	if (type.isBuilding()!=type.whatBuilds().first.isBuilding()) 
		return false;
	morphQueues[type.whatBuilds().first].push_back(type);
	plannedCount[type]++;
	return true;
}

int MorphManager::getPlannedCount(BWAPI::UnitType type) const
{
	std::map<BWAPI::UnitType,int>::const_iterator i=plannedCount.find(type);
	if (i!=plannedCount.end())
		return i->second;
	return 0;
}

int MorphManager::getStartedCount(BWAPI::UnitType type) const
{
	std::map<BWAPI::UnitType,int>::const_iterator i=startedCount.find(type);
	if (i!=startedCount.end())
		return i->second;
	return 0;
}
BWAPI::UnitType MorphManager::getBuildType(BWAPI::Unit* unit) const
{
	if (morphingUnits.find(unit)==morphingUnits.end())
		return BWAPI::UnitTypes::None;
	return morphingUnits.find(unit)->second.type;
}