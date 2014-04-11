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

#include <BulletGroupManager.h>

BulletGroup allBullets;
BulletGroupManager* TheBulletGroupManager = NULL;

BulletGroupManager* BulletGroupManager::create()
{
	if (TheBulletGroupManager) return TheBulletGroupManager;
	return new BulletGroupManager();
}
void BulletGroupManager::destroy()
{
	if (TheBulletGroupManager)
		delete TheBulletGroupManager;
}
BulletGroupManager::BulletGroupManager()
{
	TheBulletGroupManager = this;
}
BulletGroupManager::~BulletGroupManager()
{
	TheBulletGroupManager = NULL;
}

void BulletGroupManager::update()
{
	for(std::set<BWAPI::Bullet*>::iterator i=BWAPI::Broodwar->getBullets().begin(); i!=BWAPI::Broodwar->getBullets().end(); ++i)
		if( (*i)->exists() && 
			(*i)->getSource() != NULL && 
			( !(*i)->getSource()->getType().isWorker() ||
			  !( (*i)->getSource()->isGatheringMinerals() ||
			     (*i)->getSource()->isRepairing() ||
			     (*i)->getSource()->isConstructing() 
			   ) 
			) 
		  )
		{
			allBullets.insert(*i);
		}
		
	// Cleaning
	BulletGroup bulletsToRemove = allBullets.not(existsBullet);
	for each( BWAPI::Bullet *b in bulletsToRemove )
		allBullets.erase(b);
}

BulletGroup AllBullets()
{
	return allBullets;
}
BulletGroup SelectAllBullet()
{
	return allBullets( BWAPI::Broodwar->self() );
}
BulletGroup SelectAllBullet(BWAPI::BulletType type)
{
	return allBullets( BWAPI::Broodwar->self() )( type );
}
BulletGroup SelectAllEnemyBullet()
{
	return allBullets( BWAPI::Broodwar->enemy() );
}
BulletGroup SelectAllEnemyBullet(BWAPI::BulletType type)
{
	return allBullets( BWAPI::Broodwar->enemy() )( type );
}
BulletGroup SelectAllBullet(BWAPI::Player* player, BWAPI::BulletType type)
{
	return allBullets( player )( type );
}