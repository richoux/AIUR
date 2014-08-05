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

#include <BuildOrderManager.h>
#include <BuildManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <WorkerManager.h>
#include <SupplyManager.h>
#include <algorithm>
#include <stdarg.h>
#include <UnitGroupManager.h>

using namespace std;
using namespace BWAPI;

map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem*>> *globalUnitSet;
int y;
int currentPriority;
map<const BWAPI::Unit*,int> nextFreeTimeData;
map<BWAPI::UnitType, set<BWAPI::UnitType>> makes;
map<BWAPI::UnitType, set<BWAPI::TechType>> researches;
map<BWAPI::UnitType, set<BWAPI::UpgradeType>> upgrades;
BuildOrderManager *buildOrderManager;

BuildOrderManager::BuildOrderManager(BuildManager* buildManager, TechManager* techManager, UpgradeManager* upgradeManager,
									 WorkerManager* workerManager, SupplyManager* supplyManager, BaseManager *baseManager)
{
	buildOrderManager			= this;
	this->buildManager			= buildManager;
	this->techManager			= techManager;
	this->upgradeManager		= upgradeManager;
	this->workerManager			= workerManager;
	this->supplyManager			= supplyManager;
	this->baseManager			= baseManager;
	this->usedMinerals		    = 0;
	this->usedGas				= 0;
	this->dependencyResolver	= false;
	this->debugMode				= false;
	UnitItem::getBuildManager() = buildManager;
	for(set<BWAPI::UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end(); ++i)
	{
		makes[(*i).whatBuilds().first].insert(*i);
	}
	for(set<BWAPI::TechType>::const_iterator i=TechTypes::allTechTypes().begin();i!=TechTypes::allTechTypes().end(); ++i)
	{
		researches[i->whatResearches()].insert(*i);
	}
	for(set<BWAPI::UpgradeType>::const_iterator i=UpgradeTypes::allUpgradeTypes().begin();i!=UpgradeTypes::allUpgradeTypes().end(); ++i)
	{
		upgrades[i->whatUpgrades()].insert(*i);
	}
}

void BuildOrderManager::setTakeGas( int takeGas )
{
	this->takeGas = takeGas;
}

//returns the next frame that the given unit type will be ready to produce units or research tech or upgrades
int BuildOrderManager::nextFreeTime(const MetaUnit* unit)
{
	int ctime=Broodwar->getFrameCount();
	if (!unit->isCompleted() && unit->unit!=NULL)
	{
		if (unit->getType().getRace()==Races::Protoss && unit->getType().isBuilding() && unit->getRemainingBuildTime()>0 && nextFreeTimeData[unit->unit]<ctime+24*3)
			nextFreeTimeData[unit->unit]=ctime+24*3;
		if (!unit->isBeingConstructed() && !unit->getType().isAddon())
			return -1;
	}
	int natime=ctime;
	natime=max(ctime,ctime+unit->getRemainingBuildTime());
	if (unit->getType().getRace()==Races::Protoss && unit->getType().isBuilding() && unit->getRemainingBuildTime()>0)
		natime=max(ctime,ctime+unit->getRemainingBuildTime()+24*3);
	natime=max(natime,ctime+unit->getRemainingTrainTime());
	natime=max(natime,ctime+unit->getRemainingResearchTime());
	natime=max(natime,ctime+unit->getRemainingUpgradeTime());
	if (unit->unit!=NULL)
		natime=max(natime,nextFreeTimeData[unit->unit]);
	if (this->buildManager->getBuildType((Unit*)(unit->unit))!=UnitTypes::None && !unit->hasBuildUnit() && !unit->isTraining() && !unit->isMorphing())
		natime=max(natime,ctime+this->buildManager->getBuildType((Unit*)(unit->unit)).buildTime());
	return natime;
}

//returns the next available time that at least one unit of the given type (buildings only right now) will be completed 
int BuildOrderManager::nextFreeTime(UnitType t)
{
	//if one unit of the given type is already completed, return the given frame count
	if (Broodwar->self()->completedUnitCount(t)>0)
		return Broodwar->getFrameCount();

	//if no units of the given type are being constructed, return -1
	if (t!=UnitTypes::Zerg_Larva)
		if (Broodwar->self()->incompleteUnitCount(t)==0)
			return -1;

	int time;
	bool setflag=false;
	for(set<MetaUnit*>::iterator i=this->MetaUnitPointers.begin();i!=this->MetaUnitPointers.end(); ++i)
	{
		if ((*i)->getType()!=t) continue;
		int ntime=nextFreeTime(*i);
		if (ntime>-1)
		{
			//set time to the earliest available time
			if (!setflag || ntime<time)
			{
				time=ntime;
				setflag=true;
			}
		}
	}
	if (t.supplyRequired()>0)
	{
		if (Broodwar->self()->supplyUsed()+t.supplyRequired()>Broodwar->self()->supplyTotal())
		{
			time=this->supplyManager->getSupplyTime(Broodwar->self()->supplyUsed()+t.supplyRequired());
		}
	}
	if (setflag)
		return time;

	//we can get here if construction has been halted by an SCV
	return -1;
}

//returns the next available time that the given unit will be able to train the given unit type
//takes into account required units
//todo: take into account required tech and supply
int BuildOrderManager::nextFreeTime(const MetaUnit* unit, UnitType t)
{
	int time=nextFreeTime(unit);
	for(map<UnitType,int>::const_iterator i=t.requiredUnits().begin();i!=t.requiredUnits().end(); ++i)
	{
		int ntime=nextFreeTime(i->first);
		if (ntime==-1)
			return -1;
		if (ntime>time)
			time=ntime;
		if (i->first.isAddon() && i->first.whatBuilds().first==unit->getType() && !unit->hasAddon())
			return -1;
	}
	return time;
}

int BuildOrderManager::nextFreeTime(const MetaUnit* unit, TechType t)
{
	int time=nextFreeTime(unit);
	//if something else is already researching it, this unit will never be able to research it
	if (Broodwar->self()->isResearching(t))
		return -1;
	return time;
}

int BuildOrderManager::nextFreeTime(const MetaUnit* unit, UpgradeType t)
{
	int time=nextFreeTime(unit);
	if (!Broodwar->self()->isUpgrading(t))
		return time;
	for(std::set<MetaUnit*>::iterator i=this->MetaUnitPointers.begin();i!=this->MetaUnitPointers.end(); ++i)
	{
		if ((*i)->getType()!=t.whatUpgrades()) continue;
		if ((*i)->isUpgrading() && (*i)->getUpgrade()==t)
		{
			time=max(time,nextFreeTime(*i));
		}
	}
	return time;
}

BuildManager* BuildOrderManager::getBuildManager()
{
	return this->buildManager;
}

bool BuildOrderManager::isResourceLimited()
{
	return this->isMineralLimited && this->isGasLimited;
}
//returns the set of unit types the given unit will be able to make at the given time
set<BWAPI::UnitType> BuildOrderManager::unitsCanMake(MetaUnit* builder, int time)
{
	set<BWAPI::UnitType> result;
	for(set<BWAPI::UnitType>::iterator i=makes[builder->getType()].begin();i!=makes[builder->getType()].end(); ++i)
	{
		int t=nextFreeTime(builder,*i);
		if (t>-1 && t<=time)
			result.insert(*i);
	}
	return result;
}

set<BWAPI::TechType> BuildOrderManager::techsCanResearch(MetaUnit* techUnit, int time)
{
	set<BWAPI::TechType> result;
	for(set<BWAPI::TechType>::iterator i=researches[techUnit->getType()].begin();i!=researches[techUnit->getType()].end(); ++i)
	{
		int t=nextFreeTime(techUnit,*i);
		if (t>-1 && t<=time)
			result.insert(*i);
	}
	return result;
}

set<BWAPI::UpgradeType> BuildOrderManager::upgradesCanResearch(MetaUnit* techUnit, int time)
{
	set<BWAPI::UpgradeType> result;
	for(set<BWAPI::UpgradeType>::iterator i=upgrades[techUnit->getType()].begin();i!=upgrades[techUnit->getType()].end(); ++i)
	{
		int t=nextFreeTime(techUnit,*i);
		if (t>-1 && t<=time)
			result.insert(*i);
	}
	return result;
}

//prefer unit types that have larger remaining unit counts
//if we need a tie-breaker, we prefer cheaper units
bool unitTypeOrderCompare(const pair<BWAPI::UnitType, int >& a, const pair<BWAPI::UnitType, int >& b)
{
	int rA=a.second;
	int rB=b.second;
	int pA=a.first.mineralPrice()+a.first.gasPrice();
	int pB=b.first.mineralPrice()+b.first.gasPrice();
	return rA>rB || (rA == rB && pA<pB);
}

UnitType getUnitType(set<UnitType>& validUnitTypes,vector<pair<BWAPI::UnitType, int > >& unitCounts)
{
	UnitType answer=UnitTypes::None;
	//sort unit counts in descending order of size
	sort(unitCounts.begin(),unitCounts.end(),unitTypeOrderCompare);
	for(vector<pair<BWAPI::UnitType, int > >::iterator i=unitCounts.begin();i!=unitCounts.end(); ++i)
	{
		//use the first valid unit type we find
		if (validUnitTypes.find(i->first)!=validUnitTypes.end())
		{
			answer=i->first;
			i->second--;
			if (i->second<=0)
			{
				unitCounts.erase(i);
			}
			break;
		}
	}
	return answer;
}

pair<TechType,UpgradeType> getTechOrUpgradeType(set<TechType>& validTechTypes, set<UpgradeType>& validUpgradeTypes, list<TechItem> &remainingTech)
{
	pair<TechType,UpgradeType> answer(TechTypes::None,UpgradeTypes::None);
	for(list<TechItem>::iterator i=remainingTech.begin();i!=remainingTech.end(); ++i)
	{
		//use the first valid unit type we find
		TechType t=(*i).techType;
		UpgradeType u=(*i).upgradeType;
		if (t!=TechTypes::None)
		{
			if (validTechTypes.find(t)!=validTechTypes.end())
			{
				answer.first=t;
				remainingTech.erase(i);
				break;
			}
		}
		else if (u!=UpgradeTypes::None)
		{
			if (validUpgradeTypes.find(u)!=validUpgradeTypes.end())
			{
				answer.second=u;
				remainingTech.erase(i);
				break;
			}
		}
	}
	return answer;
}
bool factoryCompare(const BuildOrderManager::MetaUnit* a, const BuildOrderManager::MetaUnit* b)
{
	return buildOrderManager->nextFreeTime(a)>buildOrderManager->nextFreeTime(b);
}
bool BuildOrderManager::updateUnits()
{
	set<MetaUnit*> allUnits = this->MetaUnitPointers;

	//sanity check the data (not sure if we need to, but just in case)
	map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i2;
	for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end();i=i2)
	{
		i2=i;
		i2++;
		map<BWAPI::UnitType, UnitItem* >::iterator j2;
		for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end();j=j2)
		{
			j2=j;
			j2++;
			if (j->second==NULL || j->second->getRemainingCount()==0)
			{
				i->second.erase(j);
			}
		}
		if (i->second.empty())
			globalUnitSet->erase(i);
	}

	if (globalUnitSet->empty())
	{
		//false = not resource limited
		return false;
	}

	//get the set of factory Units
	list<MetaUnit*> factories;
	for(set<MetaUnit*>::iterator i=allUnits.begin();i!=allUnits.end(); ++i)
	{
		MetaUnit* u=*i;
		UnitType type=u->getType();
		//only add the factory if it hasn't been reserved and if its a builder type that we need
		if (globalUnitSet->find(type)!=globalUnitSet->end() && this->reservedUnits.find(u)==this->reservedUnits.end())
			factories.push_back(u);
	}
	factories.sort(factoryCompare);

	//find the sequence of interesting points in time in the future
	set<int> times;
	//iterate through each type of builder
	for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end(); ++i)
	{
		UnitType unitType=i->first;//builder type

		//iterate through all our factory Units
		for(list<MetaUnit*>::iterator f=factories.begin();f!=factories.end(); ++f)
		{
			MetaUnit* u=*f;
			UnitType type=u->getType();
			if (type==i->first)//only look at units of this builder type
			{
				//iterate over all the types of units we want to make with this builder type
				for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end(); ++j)
				{
					//add the time to the sequence if it is in the future (and not -1)
					int time=nextFreeTime(*f,j->first);
					if (time>-1)
						times.insert(time);
				}
			}
		}
	}

	//get the remaining unit counts for each type of unit we want to make
	vector<pair<BWAPI::UnitType, int > > remainingUnitCounts;
	for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > >::iterator i=globalUnitSet->begin();i!=globalUnitSet->end(); ++i)
	{
		for(map<BWAPI::UnitType, UnitItem* >::iterator j=i->second.begin();j!=i->second.end(); ++j)
		{
			remainingUnitCounts.push_back(make_pair(j->first,j->second->getRemainingCount(currentlyPlannedCount[j->first])));
		}
	}

	//reserve units and resources for later

	//iterate through time (this gives earlier events higher priority)
	times.insert(Broodwar->getFrameCount());
	for(set<int>::iterator t=times.begin();t!=times.end(); ++t)
	{
		int ctime=*t;

		//remove all factories that have been reserved
		list<MetaUnit*>::iterator f2;
		for(list<MetaUnit*>::iterator f=factories.begin();f!=factories.end();f=f2)
		{
			f2=f;
			f2++;
			if (this->reservedUnits.find(*f)!=this->reservedUnits.end())
				factories.erase(f);
		}
		//iterate through all factories that haven't been reserved yet
		for(list<MetaUnit*>::iterator f=factories.begin();f!=factories.end(); ++f)
		{
			MetaUnit* factory=*f;
			//get a unit type, taking into account the remaining unit counts and the set of units this factory can make at time ctime
			UnitType t=getUnitType(unitsCanMake(*f,ctime),remainingUnitCounts);
			if (t==UnitTypes::None)
				continue;
			int btime=ctime;
			if (factory->getType().isWorker())
				btime=ctime+24*4;
			currentlyPlannedCount[t]++;
			this->reserveResources(factory,t);
			this->reservedUnits.insert(factory);
			this->savedPlan.push_back(Type(t,factory,currentPriority,ctime));
			if (this->isResourceLimited())
				return true;
		}
	}
	//false = not resource limited
	return false;
}
void BuildOrderManager::update()
{
	int time=Broodwar->getFrameCount();
	if (time>=this->nextUpdateFrame)
	{
		updatePlan();
		this->nextUpdateFrame=time+24*3;
	}
	y=0;
	this->reservedResources.clear();
	this->reservedUnits.clear();
	this->isGasLimited=false;
	this->isMineralLimited=false;
	debug("time=%d",time);
	list<Type>::iterator i2;
	for(list<Type>::iterator i = savedPlan.begin(); i != savedPlan.end(); i = i2)
	{
		i2 = i;
		i2++;
		UnitType unit=(*i).unitType;
		TechType tech=(*i).techType;
		UpgradeType upgrade=(*i).upgradeType;
		MetaUnit* factory=(*i).unit;
		int priority=(*i).priority;
		int ctime=(*i).time;
		if (ctime<Broodwar->getFrameCount())
			ctime=Broodwar->getFrameCount();
		int btime=ctime;
		if (factory->getType().isWorker())
			btime=ctime+24*4;
		if (unit!=UnitTypes::None)
		{
			if ((*i).time>=time)
				debug("%s at %d",unit.getName().c_str(),(*i).time);
			else
				debug("%s as soon as possible",unit.getName().c_str());
			if (ctime<=time && hasResources(unit,btime))
			{
				TilePosition tp=this->items[priority].units[factory->getType()][unit].decrementAdditional();
				if (!factory->hasAddon())
					this->buildManager->build(unit,tp,true);
				else
				{
					if (unit.isResourceDepot())
						this->buildManager->build(unit,tp, 0, 1);
					else
						this->buildManager->build(unit,tp);
				}
				nextFreeTimeData[factory->unit]=time+24*6;
				if (this->debugMode) Broodwar->printf("Build %s",unit.getName().c_str());
				this->spendResources(unit);
				savedPlan.erase(i);
			}
			else
				this->reserveResources(factory,unit);
		}
		else if (tech!=TechTypes::None)
		{
			if ((*i).time>=time)
				debug("%s at %d",tech.getName().c_str(),(*i).time);
			else
				debug("%s as soon as possible",tech.getName().c_str());
			if (ctime<=time && hasResources(tech,btime))
			{
				if (this->debugMode) Broodwar->printf("Research %s",tech.getName().c_str());
				this->techManager->research(tech);
				nextFreeTimeData[factory->unit]=time+24*6;
				this->spendResources(tech);
				savedPlan.erase(i);
			}
			else
				this->reserveResources(factory,tech);
		}
		else if (upgrade!=UpgradeTypes::None)
		{
			if ((*i).time>=time)
				debug("%s at %d",upgrade.getName().c_str(),(*i).time);
			else
				debug("%s as soon as possible",upgrade.getName().c_str());
			if (ctime<=time && hasResources(upgrade,btime))
			{
				if (this->debugMode) Broodwar->printf("Upgrade %s",upgrade.getName().c_str());
				this->upgradeManager->upgrade(upgrade);
				nextFreeTimeData[factory->unit]=time+24*6;
				this->spendResources(upgrade);
				savedPlan.erase(i);
			}
			else
				this->reserveResources(factory,upgrade);
		}
	}
}
void BuildOrderManager::updatePlan()
{
	this->savedPlan.clear();

	this->MetaUnits.clear();
	this->MetaUnitPointers.clear();
	for(set<Unit*>::const_iterator i=BWAPI::Broodwar->self()->getUnits().begin();i!=BWAPI::Broodwar->self()->getUnits().end(); ++i)
	{
		MetaUnits.push_back(MetaUnit(*i));
		if ((*i)->getType()==UnitTypes::Zerg_Hatchery || (*i)->getType()==UnitTypes::Zerg_Lair || (*i)->getType()==UnitTypes::Zerg_Hive)
		{
			int larva=(*i)->getLarva().size();
			if ((*i)->getRemainingTrainTime()>0)
				MetaUnits.push_back(MetaUnit(Broodwar->getFrameCount()+(*i)->getRemainingTrainTime()));
			for(int j=1;j<4;j++)
				MetaUnits.push_back(MetaUnit(Broodwar->getFrameCount()+(*i)->getRemainingTrainTime()+334*j));
		}
	}
	for(list<MetaUnit>::iterator i=MetaUnits.begin();i!=MetaUnits.end(); ++i)
	{
		this->MetaUnitPointers.insert(&(*i));
	}
	for(set<UnitType>::const_iterator i=UnitTypes::allUnitTypes().begin();i!=UnitTypes::allUnitTypes().end(); ++i)
	{
		currentlyPlannedCount[*i]=this->buildManager->getPlannedCount(*i);
	}
	map< int, PriorityLevel >::iterator l2;
	for(map< int, PriorityLevel >::iterator l=items.begin();l!=items.end();l=l2)
	{
		l2=l;
		l2++;
		if (l->second.techs.empty() && l->second.units.empty())
			items.erase(l);
	}
	if (items.empty()) return;
	map< int, PriorityLevel >::iterator l=items.end();
	l--;
	this->reservedResources.clear();
	this->reservedUnits.clear();
	this->isGasLimited=false;
	this->isMineralLimited=false;
	y=5;

	//Iterate through priority levels in decreasing order
	//---------------------------------------------------------------------------------------------------------
	for(;l!=items.end();l--)
	{
		currentPriority=l->first;

		//First consider all techs and upgrades for this priority level
		set<UnitType> techUnitTypes;
		for(list<TechItem>::iterator i=l->second.techs.begin();i!=l->second.techs.end(); ++i)
		{
			if (i->techType!=TechTypes::None)
				techUnitTypes.insert(i->techType.whatResearches());
			if (i->upgradeType!=UpgradeTypes::None)
				techUnitTypes.insert(i->upgradeType.whatUpgrades());
		}
		set<MetaUnit*> allUnits=this->MetaUnitPointers;
		//get the set of tech Units
		set<MetaUnit*> techUnits;
		for(set<MetaUnit*>::iterator i=allUnits.begin();i!=allUnits.end(); ++i)
		{
			MetaUnit* u=*i;
			UnitType type=u->getType();
			//only add the factory if it hasn't been reserved and if its a builder type that we need
			if (techUnitTypes.find(type)!=techUnitTypes.end() && this->reservedUnits.find(u)==this->reservedUnits.end())
				techUnits.insert(u);
		}

		//find the sequence of interesting points in time in the future
		set<int> times;
		for(set<MetaUnit*>::iterator i=techUnits.begin();i!=techUnits.end(); ++i)
		{
			//add the time to the sequence if it is in the future (and not -1)
			int time=nextFreeTime(*i);
			if (time>-1)
				times.insert(time);
		}

		//get the remaining tech
		list<TechItem > remainingTech=l->second.techs;

		if (this->dependencyResolver)
		{
			//check dependencies
			for(list<TechItem>::iterator i=remainingTech.begin();i!=remainingTech.end(); ++i)
			{
				TechType t=i->techType;
				UpgradeType u=i->upgradeType;
				if (t!=TechTypes::None)
				{
					if (this->getPlannedCount(t.whatResearches())==0)
					{
						this->build(1,t.whatResearches(),l->first);
					}
					//also check to see if we have enough gas, or a refinery planned
					if (t.gasPrice()>BWAPI::Broodwar->self()->gatheredGas()-this->usedGas)
					{
						UnitType refinery=Broodwar->self()->getRace().getRefinery();
						if (this->getPlannedCount(refinery)==0 && BWAPI::Broodwar->getFrameCount() >= takeGas)
							this->build(1,refinery,l->first);
					}
				}
				else if (u!=UpgradeTypes::None)
				{
					if (this->getPlannedCount(u.whatUpgrades())==0)
						this->build(1,u.whatUpgrades(),l->first);
					//also check to see if we have enough gas, or a refinery planned
					if (u.gasPrice(1)+u.gasPriceFactor()*(BWAPI::Broodwar->self()->getUpgradeLevel(u)-1)>BWAPI::Broodwar->self()->gatheredGas()-this->usedGas)
					{
						UnitType refinery=Broodwar->self()->getRace().getRefinery();
						if (this->getPlannedCount(refinery)==0 && BWAPI::Broodwar->getFrameCount() >= takeGas)
							this->build(1,refinery,l->first);
					}
					if (i->level>1)
					{
						if (u.getRace()==Races::Terran)
						{
							if (this->getPlannedCount(UnitTypes::Terran_Science_Facility)==0)
								this->build(1,UnitTypes::Terran_Science_Facility,l->first);
						}
						else if (u.getRace()==Races::Protoss)
						{
							if (this->getPlannedCount(UnitTypes::Protoss_Templar_Archives)==0)
								this->build(1,UnitTypes::Protoss_Templar_Archives,l->first);
						}
						else if (u.getRace()==Races::Zerg)
						{
							if (this->getPlannedCount(UnitTypes::Zerg_Lair)==0)
								this->build(1,UnitTypes::Zerg_Lair,l->first);
						}
					}
				}
			}
		}

		//reserve units and resources for later

		//iterate through time (this gives earlier events higher priority)
		for(set<int>::iterator t=times.begin();t!=times.end(); ++t)
		{
			int ctime=*t;

			//remove all tech units that have been reserved
			set<MetaUnit*>::iterator i2;
			for(set<MetaUnit*>::iterator i=techUnits.begin();i!=techUnits.end();i=i2)
			{
				i2=i;
				i2++;
				if (this->reservedUnits.find(*i)!=this->reservedUnits.end())
					techUnits.erase(i);
			}
			//iterate through all tech units that haven't been reserved yet
			for(set<MetaUnit*>::iterator i=techUnits.begin();i!=techUnits.end(); ++i)
			{
				MetaUnit* techUnit=*i;
				//get a unit type, taking into account the remaining unit counts and the set of units this factory can make right now
				pair< TechType,UpgradeType > p=getTechOrUpgradeType(techsCanResearch(*i,ctime),upgradesCanResearch(*i,ctime),remainingTech);
				TechType t=p.first;
				UpgradeType u=p.second;
				if (t==TechTypes::None && u==UpgradeTypes::None)
					continue;
				this->reservedUnits.insert(techUnit);
				if (t!=TechTypes::None)
				{
					this->reserveResources(techUnit,t);
					this->savedPlan.push_back(Type(t,techUnit,currentPriority,ctime));
				}
				else if (u!=UpgradeTypes::None)
				{
					this->reserveResources(techUnit,u);
					this->savedPlan.push_back(Type(u,techUnit,currentPriority,ctime));
				}
				if (this->isResourceLimited())
					return;
			}
		}
		//-------------------------------------------------------------------------------------------------------
		//Next consider all buildings and units for this priority level
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > buildings;
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > unitsA;//units that require addons
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem* > > units;
		for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=l->second.units.begin();i!=l->second.units.end(); ++i)
		{
			for(map<BWAPI::UnitType, UnitItem >::iterator j=i->second.begin();j!=i->second.end(); ++j)
			{
				if (j->first.isBuilding())
					buildings[i->first][j->first]=&(j->second);
				else
				{
					//see if the unit type requires an addon
					if (j->first==UnitTypes::Terran_Siege_Tank_Tank_Mode ||
						j->first==UnitTypes::Terran_Siege_Tank_Siege_Mode ||
						j->first==UnitTypes::Terran_Dropship ||
						j->first==UnitTypes::Terran_Battlecruiser ||
						j->first==UnitTypes::Terran_Science_Vessel ||
						j->first==UnitTypes::Terran_Valkyrie)
						unitsA[i->first][j->first]=&(j->second);
					else
						units[i->first][j->first]=&(j->second);
				}
				if (this->dependencyResolver)
				{
					//check dependencies (required units)
					for(map<BWAPI::UnitType, int>::const_iterator k=j->first.requiredUnits().begin();k!=j->first.requiredUnits().end(); ++k)
					{
						if (k->first == UnitTypes::Zerg_Larva) continue;
						if (this->getPlannedCount(k->first)==0)
						{
							this->build(1,k->first,l->first);
						}
					}
					//also check to see if we have enough gas, or a refinery planned
					if (j->first.gasPrice()>BWAPI::Broodwar->self()->gatheredGas()-this->usedGas)
					{
						if (j->first!=UnitTypes::Zerg_Larva && j->first!=UnitTypes::Zerg_Egg && j->first!=UnitTypes::Zerg_Lurker_Egg && j->first!=UnitTypes::Zerg_Cocoon)
						{
							UnitType refinery=Broodwar->self()->getRace().getRefinery();
							if (this->getPlannedCount(refinery)==0 && BWAPI::Broodwar->getFrameCount() >= takeGas)
								this->build(1,refinery,l->first);
						}
					}
				}
			}
		}
		globalUnitSet=&buildings;
		if (updateUnits()) return;
		globalUnitSet=&unitsA;
		if (updateUnits()) return;
		globalUnitSet=&units;
		if (updateUnits()) return;
		this->removeCompletedItems(&(l->second));
	}
	debug("unit-limited");
}

string BuildOrderManager::getName() const
{
	return "Build Order Manager";
}
void BuildOrderManager::build(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
	if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
	if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
		seedPosition=BWAPI::Broodwar->self()->getStartLocation();
	if (t==UnitTypes::Protoss_Pylon && this->getPlannedCount(t)==0)
	{
		if (!this->buildManager->getBuildingPlacer()->canBuildHereWithSpace(seedPosition, t, 3))
			seedPosition = this->buildManager->getBuildingPlacer()->getBuildLocationNear(seedPosition, t, 3);
	}
	if (items[priority].units[t.whatBuilds().first].find(t)==items[priority].units[t.whatBuilds().first].end())
		items[priority].units[t.whatBuilds().first].insert(make_pair(t,UnitItem(t)));
	items[priority].units[t.whatBuilds().first][t].setNonAdditional(count,seedPosition);
	nextUpdateFrame=0;
}

void BuildOrderManager::buildAdditional(int count, BWAPI::UnitType t, int priority, BWAPI::TilePosition seedPosition)
{
	if (t == BWAPI::UnitTypes::None || t == BWAPI::UnitTypes::Unknown) return;
	if (seedPosition == BWAPI::TilePositions::None || seedPosition == BWAPI::TilePositions::Unknown)
		seedPosition=BWAPI::Broodwar->self()->getStartLocation();

	if (items[priority].units[t.whatBuilds().first].find(t)==items[priority].units[t.whatBuilds().first].end())
		items[priority].units[t.whatBuilds().first].insert(make_pair(t,UnitItem(t)));
	items[priority].units[t.whatBuilds().first][t].addAdditional(count,seedPosition);
	nextUpdateFrame=0;
}

void BuildOrderManager::research(BWAPI::TechType t, int priority)
{
	if (t==BWAPI::TechTypes::None || t==BWAPI::TechTypes::Unknown) return;

	items[priority].techs.push_back(TechItem(t));
	nextUpdateFrame=0;
}

void BuildOrderManager::upgrade(int level, BWAPI::UpgradeType t, int priority)
{
	if (t==BWAPI::UpgradeTypes::None || t==BWAPI::UpgradeTypes::Unknown) return;
	items[priority].techs.push_back(TechItem(t,level));
	nextUpdateFrame=0;
}

bool BuildOrderManager::hasResources(std::pair<int, BuildOrderManager::Resources> res)
{
	bool mineralLimited=false;
	bool gasLimited=false;
	this->reserveResources(res);
	double m=BWAPI::Broodwar->self()->gatheredMinerals()-this->usedMinerals;
	double g=BWAPI::Broodwar->self()->gatheredGas()-this->usedGas;
	for(map<int, Resources>::iterator i=this->reservedResources.begin();i!=this->reservedResources.end(); ++i)
	{
		double t=i->first-Broodwar->getFrameCount();
		m-=i->second.minerals;
		g-=i->second.gas;
		if (m+t*this->workerManager->getMineralRate()<0)
			mineralLimited=true;
		if (g+t*this->workerManager->getGasRate()<0)
			gasLimited=true;
	}
	this->unreserveResources(res);
	this->isMineralLimited = this->isMineralLimited || mineralLimited;
	this->isGasLimited = this->isGasLimited || gasLimited;
	return (!mineralLimited || res.second.minerals==0) && (!gasLimited || res.second.gas==0);
}
bool BuildOrderManager::hasResources(BWAPI::UnitType t)
{
	bool ret=hasResources(t,Broodwar->getFrameCount());
	return ret;
}
bool BuildOrderManager::hasResources(BWAPI::TechType t)
{
	return hasResources(t,Broodwar->getFrameCount());
}
bool BuildOrderManager::hasResources(BWAPI::UpgradeType t)
{
	return hasResources(t,Broodwar->getFrameCount());
}
bool BuildOrderManager::hasResources(BWAPI::UnitType t, int time)
{
	pair<int, Resources> res;
	res.first=time;
	res.second.minerals=t.mineralPrice();
	res.second.gas=t.gasPrice();
	bool ret=hasResources(res);
	return ret;
}

bool BuildOrderManager::hasResources(BWAPI::TechType t, int time)
{
	pair<int, Resources> res;
	res.first=time;
	res.second.minerals=t.mineralPrice();
	res.second.gas=t.gasPrice();
	return hasResources(res);
}

bool BuildOrderManager::hasResources(BWAPI::UpgradeType t, int time)
{
	pair<int, Resources> res;
	res.first=time;
	res.second.minerals=t.mineralPrice(1)+t.mineralPriceFactor()*this->upgradeManager->getPlannedLevel(t);
	res.second.gas=t.gasPrice(1)+t.gasPriceFactor()*this->upgradeManager->getPlannedLevel(t);
	return hasResources(res);
}

void BuildOrderManager::spendResources(BWAPI::UnitType t)
{
	this->usedMinerals+=t.mineralPrice();
	this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::TechType t)
{
	this->usedMinerals+=t.mineralPrice();
	this->usedGas+=t.gasPrice();
}

void BuildOrderManager::spendResources(BWAPI::UpgradeType t)
{
	this->usedMinerals+=t.mineralPrice(1)+t.mineralPriceFactor()*this->upgradeManager->getPlannedLevel(t);
	this->usedGas+=t.gasPrice(1)+t.gasPriceFactor()*this->upgradeManager->getPlannedLevel(t);
}

//cancel all planned building of a given type
int BuildOrderManager::cancelAllBuildingsOfType(BWAPI::UnitType t)
{
	//if (this->buildManager->getPlannedCount(t) - this->buildManager->getCompletedCount(t) <= 0	
	//	|| this->buildManager->getStartedCount(t) - this->buildManager->getCompletedCount(t) <= 0)
	//{
	//	return;
	//}

	if (!savedPlan.empty())
	{
		list<Type>::iterator i2;
		for(list<Type>::iterator i = savedPlan.begin(); i != savedPlan.end(); i = i2)
		{
			i2 = i;
			i2++;
	
			if (i->unitType == t)
			{
				if (t == BWAPI::UnitTypes::Protoss_Nexus)
					baseManager->resetBases();
				
				savedPlan.erase(i);
			}
		}
	}

	int numberCanceled = this->buildManager->cancelAllBuildingsOfType(t);

	UnitType builder = t.whatBuilds().first;

	string sBuilding = t.getName();
	string sBuilder = builder.getName();

	map<int, PriorityLevel>::iterator p2;
	for(map<int, PriorityLevel>::iterator p = items.begin(); p != items.end(); p = p2)
	{
		p2 = p;
		p2++;
		//map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem>> *units = &(p->second.units);
		int priority = p->first;
		if (items[priority].units[builder].find(t) != items[priority].units[builder].end())
		{
			items[priority].units[t.whatBuilds().first][t].nonAdditionalReset();
			items[priority].units[builder].erase(t);
		}
	}	

	return numberCanceled;
}

//returns the BuildOrderManager's planned count of units for this type
int BuildOrderManager::getPlannedCount(BWAPI::UnitType t)
{
	//builder unit type
	UnitType builder=t.whatBuilds().first;

	int c=this->buildManager->getPlannedCount(t);

	//sum all the remaining units for every priority level
	for(map<int, PriorityLevel>::iterator p=items.begin();p!=items.end(); ++p)
	{
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >* units=&(p->second.units);
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=units->find(builder);

		if (i!=units->end())
		{
			map<BWAPI::UnitType, UnitItem >* units2=&(i->second);
			map<BWAPI::UnitType, UnitItem >::iterator j=units2->find(t);
			if (j!=units2->end())
			{
				string str = t.getName();
				c+=j->second.getRemainingCount(c);
			}
		}
	}
	if (t==UnitTypes::Zerg_Hatchery)
		c+=this->getPlannedCount(UnitTypes::Zerg_Lair);
	if (t==UnitTypes::Zerg_Lair)
		c+=this->getPlannedCount(UnitTypes::Zerg_Hive);
	return c;
}

//returns the BuildOrderManager's planned count of units for this type
int BuildOrderManager::getPlannedCount(BWAPI::UnitType t, int minPriority)
{
	//builder unit type
	UnitType builder=t.whatBuilds().first;

	int c=this->buildManager->getPlannedCount(t);

	//sum all the remaining units for every priority level
	for(map<int, PriorityLevel>::iterator p=items.begin();p!=items.end(); ++p)
	{
		if (p->first<minPriority) continue; //don't consider planned units below min priority
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >* units=&(p->second.units);
		map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=units->find(builder);

		if (i!=units->end())
		{
			map<BWAPI::UnitType, UnitItem >* units2=&(i->second);
			map<BWAPI::UnitType, UnitItem >::iterator j=units2->find(t);
			if (j!=units2->end())
			{
				c+=j->second.getRemainingCount(c);
			}
		}
	}
	if (t==UnitTypes::Zerg_Hatchery)
		c+=this->getPlannedCount(UnitTypes::Zerg_Lair);
	if (t==UnitTypes::Zerg_Lair)
		c+=this->getPlannedCount(UnitTypes::Zerg_Hive);
	return c;
}

//reserves resources for this unit type
pair<int, BuildOrderManager::Resources> BuildOrderManager::reserveResources(MetaUnit* builder, UnitType unitType)
{
	int t=Broodwar->getFrameCount();
	if (builder)
		t=nextFreeTime(builder,unitType);
	pair<int, Resources> ret;
	ret.first=t;
	ret.second.minerals=unitType.mineralPrice();
	ret.second.gas=unitType.gasPrice();
	reserveResources(ret);
	return ret;
}
//reserves resources for this tech type
pair<int, BuildOrderManager::Resources> BuildOrderManager::reserveResources(MetaUnit* techUnit, TechType techType)
{
	int t=Broodwar->getFrameCount();
	if (techUnit)
		t=nextFreeTime(techUnit);
	pair<int, Resources> ret;
	ret.first=t;
	ret.second.minerals=techType.mineralPrice();
	ret.second.gas=techType.gasPrice();
	reserveResources(ret);
	return ret;
}
//reserves resources for this upgrade type
pair<int, BuildOrderManager::Resources> BuildOrderManager::reserveResources(MetaUnit* techUnit, UpgradeType upgradeType)
{
	int t=Broodwar->getFrameCount();
	if (techUnit)
		t=nextFreeTime(techUnit);
	pair<int, Resources> ret;
	ret.first=t;
	ret.second.minerals=upgradeType.mineralPrice(1)+upgradeType.mineralPriceFactor()*this->upgradeManager->getPlannedLevel(upgradeType);
	ret.second.gas=upgradeType.gasPrice(1)+upgradeType.gasPriceFactor()*this->upgradeManager->getPlannedLevel(upgradeType);
	reserveResources(ret);
	return ret;
}
void BuildOrderManager::reserveResources(pair<int, BuildOrderManager::Resources> res)
{

	this->reservedResources[res.first].minerals+=res.second.minerals;
	this->reservedResources[res.first].gas+=res.second.gas;
}
//unreserves the given resources
void BuildOrderManager::unreserveResources(pair<int, BuildOrderManager::Resources> res)
{
	this->reservedResources[res.first].minerals-=res.second.minerals;
	this->reservedResources[res.first].gas-=res.second.gas;
	if (this->reservedResources[res.first].minerals==0 && this->reservedResources[res.first].gas == 0)
		this->reservedResources.erase(res.first);
}
void BuildOrderManager::enableDependencyResolver()
{
	this->dependencyResolver=true;
}
void BuildOrderManager::setDebugMode(bool debugMode)
{
	this->debugMode=debugMode;
	this->buildManager->setDebugMode(debugMode);
}


void BuildOrderManager::removeCompletedItems(PriorityLevel* p)
{
	list<TechItem>::iterator i2;
	for(list<TechItem>::iterator i=p->techs.begin();i!=p->techs.end();i=i2)
	{
		i2=i;
		i2++;
		if (i->techType!=TechTypes::None)
		{
			if (this->techManager->planned(i->techType))
			{
				p->techs.erase(i);
			}
		}
		else if (i->upgradeType!=UpgradeTypes::None)
		{
			if (this->upgradeManager->getPlannedLevel(i->upgradeType)==i->level)
			{
				p->techs.erase(i);
			}
		}
	}
	map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i3;
	for(map<BWAPI::UnitType, map<BWAPI::UnitType, UnitItem > >::iterator i=p->units.begin();i!=p->units.end();i=i3)
	{
		i3=i;
		i3++;
		map<BWAPI::UnitType, UnitItem >::iterator j2;
		for(map<BWAPI::UnitType, UnitItem >::iterator j=i->second.begin();j!=i->second.end();j=j2)
		{
			j2=j;
			j2++;
			if (j->second.getRemainingCount()==0)
				i->second.erase(j);
		}
		if (i->second.empty())
			p->units.erase(i);
	}
}

void BuildOrderManager::debug(const char* text, ...)
{
	const int BUFFER_SIZE = 1024;
	char buffer[BUFFER_SIZE];

	va_list ap;
	va_start(ap, text);
	vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE, text, ap);
	va_end(ap);

	if (this->debugMode)
	{
		Broodwar->drawTextScreen(5,y,"%s",buffer);
		y+=15;
	}
}
