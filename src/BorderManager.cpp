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

#include <BorderManager.h>
#include <InformationManager.h>
BorderManager* TheBorderManager = NULL;

BorderManager* BorderManager::create()
{
	if (TheBorderManager) return TheBorderManager;
	return new BorderManager();
}

void BorderManager::destroy()
{
	if (TheBorderManager)
		delete TheBorderManager;
}

BorderManager::BorderManager()
{
	lastFrameCheck = 0;
	TheBorderManager = this;
}

BorderManager::~BorderManager()
{
	TheBorderManager = NULL;
}

void BorderManager::addMyBase(BWTA::BaseLocation* location)
{
	this->myBases.insert(location);
	recalculateBorders();
}

void BorderManager::removeMyBase(BWTA::BaseLocation* location)
{
	this->myBases.erase(location);
	recalculateBorders();
}

void BorderManager::reset()
{
	this->myBases.clear();
	this->myRegions.clear();
	this->myBorder.clear();
	this->enemyRegions.clear();
	this->enemyBorder.clear();
}

const std::set<BWTA::Chokepoint*>& BorderManager::getMyBorder() const
{
	return this->myBorder;
}

const std::set<BWTA::Chokepoint*>& BorderManager::getEnemyBorder() const
{
	return this->enemyBorder;
}
void BorderManager::update()
{
	if ((lastFrameCheck == 0) || (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 100))
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		if (TheInformationManager->getEnemyBases()!=this->enemyBases)
		{
			this->enemyBases=TheInformationManager->getEnemyBases();
			recalculateBorders();
		}

		//for(std::set<BWTA::Chokepoint*>::const_iterator c=myBorder.begin();c!=myBorder.end(); ++c)
		//{
		//	BWAPI::Position point1=(*c)->getSides().first;
		//	BWAPI::Position point2=(*c)->getSides().second;
		//	BWAPI::Broodwar->drawLineMap(point1.x(),point1.y(),point2.x(),point2.y(),BWAPI::Colors::Red);
		//}
		//for(std::set<BWTA::Chokepoint*>::const_iterator c=enemyBorder.begin();c!=enemyBorder.end(); ++c)
		//{
		//	BWAPI::Position point1=(*c)->getSides().first;
		//	BWAPI::Position point2=(*c)->getSides().second;
		//	BWAPI::Broodwar->drawLineMap(point1.x(),point1.y(),point2.x(),point2.y(),BWAPI::Colors::Orange);
		//}
	}
}

void BorderManager::recalculateBorders()
{
	this->myRegions.clear();
	this->myBorder.clear();
	this->enemyRegions.clear();
	this->enemyBorder.clear();
	std::set<BWTA::Region*> canReachSelf;
	std::set<BWTA::Region*> canReachEnemy;
	for(std::set<BWTA::BaseLocation*>::iterator i=myBases.begin();i!=myBases.end(); ++i)
	{
		myRegions.insert((*i)->getRegion());
		canReachSelf.insert((*i)->getRegion());
	}
	for(std::vector<BWTA::BaseLocation*>::iterator i=enemyBases.begin();i!=enemyBases.end(); ++i)
	{
		enemyRegions.insert((*i)->getRegion());
		canReachEnemy.insert((*i)->getRegion());
	}
	if (!enemyBases.empty())
	{
		bool exploring=true;
		while (exploring)
		{
			exploring=false;
			for(std::set<BWTA::Region*>::const_iterator i=BWTA::getRegions().begin();i!=BWTA::getRegions().end(); ++i)
			{
				BWTA::Region* r=*i;
				for(std::set<BWTA::Chokepoint*>::const_iterator c=r->getChokepoints().begin();c!=r->getChokepoints().end(); ++c)
				{
					BWTA::Region* r2=(*c)->getRegions().first;
					if (r==r2)
						r2=(*c)->getRegions().second;

					if (canReachSelf.find(r)!=canReachSelf.end() && enemyRegions.find(r2)==enemyRegions.end() && canReachSelf.find(r2)==canReachSelf.end())
					{
						canReachSelf.insert(r2);
						exploring=true;
					}
					if (canReachEnemy.find(r)!=canReachEnemy.end() && myRegions.find(r2)==myRegions.end() && canReachEnemy.find(r2)==canReachEnemy.end())
					{
						canReachEnemy.insert(r2);
						exploring=true;
					}
				}
			}
		}
	}
	for(std::set<BWTA::Region*>::const_iterator i=BWTA::getRegions().begin();i!=BWTA::getRegions().end(); ++i)
	{
		BWTA::Region* r=*i;
		if (canReachSelf.find(r)!=canReachSelf.end() && canReachEnemy.find(r)==canReachEnemy.end())
			myRegions.insert(r);
		if (canReachSelf.find(r)==canReachSelf.end() && canReachEnemy.find(r)!=canReachEnemy.end())
			enemyRegions.insert(r);
	}
	for(std::set<BWTA::Region*>::iterator i=myRegions.begin();i!=myRegions.end(); ++i)
	{
		BWTA::Region* r=*i;
		for(std::set<BWTA::Chokepoint*>::const_iterator c=r->getChokepoints().begin();c!=r->getChokepoints().end(); ++c)
		{
			if (myBorder.find(*c)==myBorder.end())
				myBorder.insert(*c);
			else
				myBorder.erase(*c);
		}
	}
	for(std::set<BWTA::Region*>::iterator i=enemyRegions.begin();i!=enemyRegions.end(); ++i)
	{
		BWTA::Region* r=*i;
		for(std::set<BWTA::Chokepoint*>::const_iterator c=r->getChokepoints().begin();c!=r->getChokepoints().end(); ++c)
		{
			if (enemyBorder.find(*c)==enemyBorder.end())
				enemyBorder.insert(*c);
			else
				enemyBorder.erase(*c);
		}
	}
}