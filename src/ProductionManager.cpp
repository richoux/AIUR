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

#include <ProductionManager.h>

ProductionManager::ProductionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer)
{
	this->arbitrator = arbitrator;
	this->placer = placer;
	for(std::set<BWAPI::UnitType>::const_iterator i=BWAPI::UnitTypes::allUnitTypes().begin();i!=BWAPI::UnitTypes::allUnitTypes().end(); ++i)
	{
		plannedCount[*i]=0;
		startedCount[*i]=0;
	}
}
bool ProductionManager::canMake(BWAPI::Unit* builder, BWAPI::UnitType type)
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
		if (BWAPI::Broodwar->self()->completedUnitCount((i->first)) >= i->second)
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

void ProductionManager::onOffer(std::set<BWAPI::Unit*> units)
{
	//go through all the units that are being offered to us
	for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end(); ++i)
	{
		//we will loop through the unit types in the production queue for this type of unit
		std::map<BWAPI::UnitType,std::list<ProductionUnitType> >::iterator q=productionQueues.find((*i)->getType());
		bool used=false;
		if (q!=productionQueues.end() && !q->second.empty()) //well, only if the queue actually exists
		{
			for(std::list<ProductionUnitType>::iterator t=q->second.begin();t!=q->second.end(); ++t) //loop through the queue
			{
				BWAPI::UnitType ut=t->type;
				bool canMakeUnit=canMake(*i,ut);
				if (t->forceNoAddon==true && (*i)->getAddon()!=NULL)
					canMakeUnit=false;
				if (canMakeUnit) //only accept if the given unit can make the type of unit we want to make
				{
					producingUnits[*i].type=*t;
					producingUnits[*i].unit=NULL;
					producingUnits[*i].lastAttemptFrame=-100;
					producingUnits[*i].started=false;
					q->second.erase(t);
					//tell the arbitrator we accept the unit, and raise the bid to hopefully prevent other managers from using it
					arbitrator->accept(this,*i);
					arbitrator->setBid(this,*i,100.0);
					used=true;
					break;
				}
			}
		}
		if (!used) //decline this unit if we didnt use it
		{
			arbitrator->decline(this,*i,0);
			arbitrator->removeBid(this,*i);
		}
	}
}

void ProductionManager::onRevoke(BWAPI::Unit* unit, double bid)
{
	onRemoveUnit(unit);
}

void ProductionManager::update()
{
	std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
	for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); ++u)
	{
		std::map<BWAPI::UnitType,std::list<ProductionUnitType> >::iterator p=productionQueues.find((*u)->getType());
		if (p!=productionQueues.end() && !p->second.empty() && (*u)->isCompleted() && producingUnits.find(*u)==producingUnits.end())
			arbitrator->setBid(this, *u, 50);
	}

	std::map<BWAPI::Unit*,Unit>::iterator i_next;
	//go through all the factories that are producing units
	for(std::map<BWAPI::Unit*,Unit>::iterator i=producingUnits.begin();i!=producingUnits.end();i=i_next)
	{
		i_next=i;
		i_next++;
		if (i->first->isLifted()) //if the factory is lifted, tell it to land
		{
			if (i->first->isIdle())
				i->first->land(placer->getBuildLocationNear(i->first->getTilePosition()+BWAPI::TilePosition(0,1),i->first->getType()));
		}
		else //otherwise we are not lifted
		{
			//if we are training and can see the build unit, save the build unit
			if (i->first->isTraining() && i->first->getBuildUnit()!=NULL)
				i->second.unit=i->first->getBuildUnit();

			if (i->second.unit==NULL) //if the build unit doesnt exist, train it
			{
				if (BWAPI::Broodwar->getFrameCount()>i->second.lastAttemptFrame+BWAPI::Broodwar->getLatency()*2)
					if (BWAPI::Broodwar->canMake(i->first,i->second.type.type))
					{
						i->first->train(i->second.type.type);
						i->second.lastAttemptFrame=BWAPI::Broodwar->getFrameCount();
					}
			}
			else //we can see the build unit
			{
				//so we have started production
				if (!i->second.started)
				{
					startedCount[i->second.type.type]++;
					i->second.started=true;
				}

				//if the build unit is completed
				if (i->second.unit->isCompleted())
				{
					//and if the build unit is the right type
					if (i->second.unit->getType()==i->second.type.type)
					{
						//we are done!
						arbitrator->removeBid(this, i->first);
						startedCount[i->second.type.type]--;
						plannedCount[i->second.type.type]--;
						producingUnits.erase(i);
					}
					else //if its not the right type, we don't care about the build unit
						i->second.unit=NULL;
				}
				else //if the build unit is not complete
				{
					//if the build unit is not the right type, stop training it
					if (i->second.unit->exists() && i->second.unit->getType()!=i->second.type.type)
						i->first->cancelTrain();

					//if the factory is not training, set the build unit does not exist
					if (!i->first->isTraining())
						i->second.unit=NULL;
				}
			}
		}
	}
}

std::string ProductionManager::getName() const
{
	return "Production Manager";
}

std::string ProductionManager::getShortName() const
{
	return "Prod";
}

void ProductionManager::onRemoveUnit(BWAPI::Unit* unit)
{
	//remove this unit from the producingUnits map
	if (producingUnits.find(unit)!=producingUnits.end())
	{
		productionQueues[unit->getType()].push_front(producingUnits[unit].type); //add the type of unit it was making to the front of the queue
		producingUnits.erase(unit);
	}
}

bool ProductionManager::train(BWAPI::UnitType type, bool forceNoAddon)
{
	//production order starts here
	if (!type.whatBuilds().first.canProduce() || type.isBuilding()) //we only accept things that can be produced
		return false;
	//we don't care about zerg unless the unit can be produced in an infested command center
	if (type.getRace()==BWAPI::Races::Zerg)
		if (type.whatBuilds().first!=BWAPI::UnitTypes::Zerg_Infested_Command_Center)
			return false;

	//add this unit type to the end of the queue
	ProductionUnitType newType;
	newType.type=type;
	newType.forceNoAddon=forceNoAddon;
	productionQueues[type.whatBuilds().first].push_back(newType);

	plannedCount[type]++;
	return true;
}

int ProductionManager::getPlannedCount(BWAPI::UnitType type) const
{
	std::map<BWAPI::UnitType,int>::const_iterator i=plannedCount.find(type);
	if (i!=plannedCount.end())
		return i->second;
	return 0;
}

int ProductionManager::getStartedCount(BWAPI::UnitType type) const
{
	std::map<BWAPI::UnitType,int>::const_iterator i=startedCount.find(type);
	if (i!=startedCount.end())
		return i->second;
	return 0;
}

BWAPI::UnitType ProductionManager::getBuildType(BWAPI::Unit* unit) const
{
	if (producingUnits.find(unit)==producingUnits.end())
		return BWAPI::UnitTypes::None;
	return producingUnits.find(unit)->second.type.type;
}