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

#include <PhotonManager.h>

void PhotonManager::setBaseManager(BaseManager *baseManager)
{
	this->baseManager = baseManager;
}

void PhotonManager::setBuildManager(BuildManager *buildManager)
{
	this->buildManager = buildManager;
}

void PhotonManager::setBuildOrderManager(BuildOrderManager *buildOrderManager)
{
	this->buildOrderManager = buildOrderManager;
}

void PhotonManager::update()
{
	if (BWAPI::Broodwar->self()->completedUnitCount(BWAPI::UnitTypes::Protoss_Forge) != 0 && (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 80))
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();
		
		for each (Base *b in baseManager->getAllBases())
			if (!b->hasPhoton)
			{
				UnitGroup minerals	= UnitGroup::getUnitGroup(b->getBaseLocation()->getMinerals());
				UnitGroup geysers	= UnitGroup::getUnitGroup(b->getBaseLocation()->getGeysers());

				BWAPI::Position posMineral	= minerals.getCenter();
				BWAPI::Position posGeyser	= geysers.getCenter();

				BWAPI::TilePosition tileMineral	(posMineral);
				BWAPI::TilePosition tileGeyser	(posGeyser);
				BWAPI::TilePosition basePosition = b->getBaseLocation()->getTilePosition();
				bool up = true, right = true, down = true, left = true;

				// minerals or geysers to the left
				if (tileMineral.x() <= basePosition.x() || tileGeyser.x() <= basePosition.x())
				{
					left = false;
				}

				// minerals or geysers to the right
				if (tileMineral.x() >= basePosition.x() + 4 || tileGeyser.x() >= basePosition.x() + 4)
				{
					right = false;
				}

				// minerals or geysers to the top
				if (tileMineral.y() <= basePosition.y() || tileGeyser.y() <= basePosition.y())
				{
					up = false;
				}

				// minerals or geysers to the bottom
				if (tileMineral.y() >= basePosition.y() + 3 || tileGeyser.y() >= basePosition.y() + 3)
				{
					down = false;
				}

				// if we are surrounded by minerals and geysers, choose to build in north.
				if (!left && !right && !down && !up)
					up = true;

				BWAPI::TilePosition *tilePhoton;
				if (up)
				{
					tilePhoton = new BWAPI::TilePosition(basePosition.x() + 1, basePosition.y() - 3);
				}
				else if (right)
				{
					tilePhoton = new BWAPI::TilePosition(basePosition.x() + 5, basePosition.y() + 1);
				}
				else if (down)
				{
					tilePhoton = new BWAPI::TilePosition(basePosition.x() + 1, basePosition.y() + 4);
				}
				else if (left)
				{
					tilePhoton = new BWAPI::TilePosition(basePosition.x() - 3, basePosition.y() + 1);
				}
				// a pylon has a range of 6 tiles, i.e. 6*32 in position coordinate system
				UnitGroup pylons = SelectAll(BWAPI::UnitTypes::Protoss_Pylon)(isCompleted).inRadius(6*32, BWAPI::Position(*tilePhoton));
				if (!pylons.empty())
				{
					buildManager->getBuildingPlacer()->freeTiles(*tilePhoton, 2, 2);
					buildOrderManager->buildAdditional(1, BWAPI::UnitTypes::Protoss_Photon_Cannon, 100, *tilePhoton);
					b->hasPhoton = true;
				}
				delete tilePhoton;

			}
	}
}