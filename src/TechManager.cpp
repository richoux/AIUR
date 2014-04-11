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

#include <TechManager.h>

TechManager::TechManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;
	this->placer = NULL;
}
void TechManager::setBuildingPlacer(BuildingPlacer* placer)
{
	this->placer = placer;
}
void TechManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for(std::set<BWAPI::Unit*>::iterator i=units.begin();i!=units.end(); ++i)
	{
		std::map<BWAPI::UnitType,std::list<BWAPI::TechType> >::iterator q=researchQueues.find((*i)->getType());
		bool used=false;
		if (q!=researchQueues.end() && !q->second.empty())
		{
			for(std::list<BWAPI::TechType>::iterator t=q->second.begin();t!=q->second.end(); ++t)
			{
				if (BWAPI::Broodwar->canResearch(*i,*t) && (*i)->isIdle())
				{
					researchingUnits.insert(std::make_pair(*i,*t));
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

void TechManager::onRevoke(BWAPI::Unit* unit, double bid)
{
	this->onRemoveUnit(unit);
}

void TechManager::update()
{
	if ((lastFrameCheck == 0) || (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 75))
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		std::set<BWAPI::Unit*> myPlayerUnits=BWAPI::Broodwar->self()->getUnits();
		for(std::set<BWAPI::Unit*>::iterator u = myPlayerUnits.begin(); u != myPlayerUnits.end(); ++u)
		{
			std::map<BWAPI::UnitType,std::list<BWAPI::TechType> >::iterator r=researchQueues.find((*u)->getType());
			if ((*u)->isCompleted() && r!=researchQueues.end() && !r->second.empty())
				arbitrator->setBid(this, *u, 50);
		}
		std::map<BWAPI::Unit*,BWAPI::TechType>::iterator i_next;
		//iterate through all the researching units
		for(std::map<BWAPI::Unit*,BWAPI::TechType>::iterator i=researchingUnits.begin();i!=researchingUnits.end();i=i_next)
		{
			i_next=i;
			i_next++;
			if (i->first->isResearching())
			{
				if (i->first->getTech()!=i->second) //if the unit is researching the wrong tech, cancel it
				{
					i->first->cancelResearch();
				}
			}
			else //if the unit is not researching
			{
				if (BWAPI::Broodwar->self()->hasResearched(i->second)) //if we have researched the given tech, we are done
				{
					arbitrator->removeBid(this, i->first);
					researchingUnits.erase(i);
				}
				else //if we haven't researched the given tech, we need to order this unit to research it
				{
					if (i->first->isLifted()) //if the unit is lifted, tell it to land somewhere nearby
					{
						if (i->first->isIdle())
							i->first->land(placer->getBuildLocationNear(i->first->getTilePosition()+BWAPI::TilePosition(0,1),i->first->getType()));
					}
					else //if its landed, we can tell it to research the given tech
					{
						if (BWAPI::Broodwar->canResearch(i->first,i->second)) //only give the order if we can research it
							i->first->research(i->second);
					}
				}
			}
		}
	}
}

std::string TechManager::getName() const
{
	return "Tech Manager";
}

void TechManager::onRemoveUnit(BWAPI::Unit* unit)
{
	std::map<BWAPI::Unit*,BWAPI::TechType>::iterator r=researchingUnits.find(unit);
	if (r!=researchingUnits.end())
	{
		BWAPI::TechType type=r->second;
		if (!BWAPI::Broodwar->self()->hasResearched(type))
			researchQueues[type.whatResearches()].push_front(type);
		researchingUnits.erase(r);
	}
}

bool TechManager::research(BWAPI::TechType type)
{
	//research order starts here
	researchQueues[type.whatResearches()].push_back(type); //add this tech to the end of the research queue
	plannedTech.insert(type); //add this tech to our set of planned tech
	return true;
}

bool TechManager::planned(BWAPI::TechType type) const
{
	std::set<BWAPI::TechType>::const_iterator i=plannedTech.find(type);
	return (i!=plannedTech.end());
}