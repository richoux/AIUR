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

#include <UpgradeManager.h>

UpgradeManager::UpgradeManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;
	this->placer = NULL;
	for(std::set<BWAPI::UpgradeType>::const_iterator i=BWAPI::UpgradeTypes::allUpgradeTypes().begin();i!=BWAPI::UpgradeTypes::allUpgradeTypes().end(); ++i)
	{
		plannedLevel[*i]=0;
		startedLevel[*i]=0;
	}
}
void UpgradeManager::setBuildingPlacer(BuildingPlacer* placer)
{
	this->placer = placer;
}
void UpgradeManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end(); ++i)
	{
		std::map<BWAPI::UnitType,std::list<Upgrade> >::iterator q=upgradeQueues.find((*i)->getType());
		bool used=false;
		if (q!=upgradeQueues.end() && !q->second.empty())
		{
			for(std::list<Upgrade>::iterator t=q->second.begin();t!=q->second.end(); ++t)
			{
				if (BWAPI::Broodwar->canUpgrade(*i,t->type))
				{
					upgradingUnits.insert(std::make_pair(*i,*t));
					q->second.erase(t);

					//tell the arbitrator we accept the unit, and raise the bid to hopefully prevent other managers from using it
					arbitrator->accept(this,*i);
					arbitrator->setBid(this,*i,100.0);
					used=true;
					break;
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

void UpgradeManager::onRevoke(BWAPI::Unit* unit, double bid)
{
	this->onRemoveUnit(unit);
}

void UpgradeManager::update()
{
	if ((lastFrameCheck == 0) || (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 75))
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
		for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); ++u)
		{
			std::map<BWAPI::UnitType,std::list<Upgrade> >::iterator r=upgradeQueues.find((*u)->getType());
			if ((*u)->isCompleted() && r!=upgradeQueues.end() && !r->second.empty())
			{
				arbitrator->setBid(this, *u, 50);
			}
		}
		std::map<BWAPI::Unit*,Upgrade>::iterator i_next;
		//iterate through all the upgrading units
		for(std::map<BWAPI::Unit*,Upgrade>::iterator i=upgradingUnits.begin();i!=upgradingUnits.end();i=i_next)
		{
			i_next=i;
			i_next++;
			if (i->first->isUpgrading())
			{
				if (i->first->getUpgrade()!=i->second.type) //if our unit is upgrading the wrong thing, cancel it
				{
					i->first->cancelUpgrade();
				}
				else
				{
					if (startedLevel[i->second.type]<i->second.level)
						startedLevel[i->second.type]=i->second.level;
				}
			}
			else
			{
				if (BWAPI::Broodwar->self()->getUpgradeLevel(i->second.type)>=i->second.level) //if we have reached the desired upgrade level, we are done
				{
					arbitrator->removeBid(this, i->first);
					upgradingUnits.erase(i);
				}
				else //otherwise, we need to tell this unit to upgrade
				{
					if (i->first->isLifted()) //the unit is lifted, tell it to land nearby
					{
						if (i->first->isIdle())
							i->first->land(placer->getBuildLocationNear(i->first->getTilePosition()+BWAPI::TilePosition(0,1),i->first->getType()));
					}
					else
					{
						if (i->first->isResearching())
						{
							i->first->cancelResearch();
						}
						else
						{
							if (BWAPI::Broodwar->canUpgrade(i->first,i->second.type))
								i->first->upgrade(i->second.type);
						}
					}
				}
			}
		}
	}
}

std::string UpgradeManager::getName() const
{
	return "Upgrade Manager";
}

void UpgradeManager::onRemoveUnit(BWAPI::Unit* unit)
{
	std::map<BWAPI::Unit*,Upgrade>::iterator r=upgradingUnits.find(unit);
	if (r!=upgradingUnits.end())
	{
		Upgrade type=r->second;
		if (BWAPI::Broodwar->self()->getUpgradeLevel(type.type)<type.level)
			upgradeQueues[type.type.whatUpgrades()].push_front(type);
		upgradingUnits.erase(r);
	}
}

bool UpgradeManager::upgrade(BWAPI::UpgradeType type, int level)
{
	if (level<=0)
		level=this->getPlannedLevel(type)+1;
	if (level>type.maxRepeats()) return false;
	Upgrade newUpgrade;
	newUpgrade.type=type;
	newUpgrade.level=level;
	upgradeQueues[type.whatUpgrades()].push_back(newUpgrade);
	plannedLevel[type]=level;
	return true;
}

int UpgradeManager::getPlannedLevel(BWAPI::UpgradeType type) const
{
	std::map<BWAPI::UpgradeType, int>::const_iterator i=plannedLevel.find(type);
	if (i!=plannedLevel.end())
		return i->second;
	return 0;
}

int UpgradeManager::getStartedLevel(BWAPI::UpgradeType type) const
{
	std::map<BWAPI::UpgradeType, int>::const_iterator i=startedLevel.find(type);
	if (i!=startedLevel.end())
		return i->second;
	return 0;
}

int UpgradeManager::getCompletedLevel(BWAPI::UpgradeType type) const
{
	return BWAPI::Broodwar->self()->getUpgradeLevel(type);
}
