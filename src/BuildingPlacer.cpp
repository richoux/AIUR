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

#include <BuildingPlacer.h>
BuildingPlacer::BuildingPlacer()
{
	reserveMap.resize(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight());
	reserveMap.setTo(false);
	this->buildDistance=1;
}
bool BuildingPlacer::canBuildHere(BWAPI::TilePosition position, BWAPI::UnitType type, bool force) const
{
	//returns true if we can build this type of unit here. Takes into account reserved tiles.
	if (!BWAPI::Broodwar->canBuildHere(NULL, position, type))
		return false;
	
	for(int x = position.x(); x < position.x() + type.tileWidth(); x++)
		for(int y = position.y(); y < position.y() + type.tileHeight(); y++)
			if (reserveMap[x][y])
				return false;
	
	// if type is not a Nexus, we cannot build on a base location (+1 tilespace)
	// since all buildings in Broodwar have a rectangular shape, we just check the four corners
	if (!force && type != BWAPI::UnitTypes::Protoss_Nexus)
		for each (BWTA::BaseLocation *bl in BWTA::getBaseLocations())
		{
			UnitGroup minerals					= UnitGroup::getUnitGroup(bl->getMinerals());
			UnitGroup geysers						= UnitGroup::getUnitGroup(bl->getGeysers());
			BWAPI::Position posMineral	= minerals.getCenter();
			BWAPI::Position posGeyser		= geysers.getCenter();
			BWAPI::TilePosition tileMineral(posMineral);
			BWAPI::TilePosition tileGeyser(posGeyser);
			int up = 1, right = 1, down = 1, left = 1;

			// minerals or geysers to the left
			if (tileMineral.x() <= bl->getTilePosition().x() || tileGeyser.x() <= bl->getTilePosition().x())
			{
				left = 3;
			}

			// minerals or geysers to the right
			if (tileMineral.x() >= bl->getTilePosition().x() + 4 || tileGeyser.x() >= bl->getTilePosition().x() + 4)
			{
				right = 3;
			}

			// minerals or geysers to the top
			if (tileMineral.y() <= bl->getTilePosition().y() || tileGeyser.y() <= bl->getTilePosition().y())
			{
				up = 3;
			}

			// minerals or geysers to the bottom
			if (tileMineral.y() >= bl->getTilePosition().y() + 3 || tileGeyser.y() >= bl->getTilePosition().y() + 3)
			{
				down = 3;
			}

			// if we are surrounded by minerals and geysers, choose to build in north.
			if (left == 3 && right == 3 && down == 3 && up == 3)
				up = 1;


			if ((bl->getTilePosition().x() - left <= position.x() && 
					 position.x() < bl->getTilePosition().x() + BWAPI::UnitTypes::Protoss_Nexus.tileWidth() + right &&
					 bl->getTilePosition().y() - up <= position.y() && 
					 position.y() < bl->getTilePosition().y() + BWAPI::UnitTypes::Protoss_Nexus.tileHeight() + down) ||

					(bl->getTilePosition().x() - left <= position.x() && 
					 position.x() < bl->getTilePosition().x() + BWAPI::UnitTypes::Protoss_Nexus.tileWidth() + right &&
					 bl->getTilePosition().y() - up <= position.y() + type.tileHeight() - 1 && 
					 position.y() + type.tileHeight() - 1 < bl->getTilePosition().y() + BWAPI::UnitTypes::Protoss_Nexus.tileHeight() + down) ||

					(bl->getTilePosition().x() - left <= position.x() + type.tileWidth() - 1 && 
					 position.x() + type.tileWidth() - 1 < bl->getTilePosition().x() + BWAPI::UnitTypes::Protoss_Nexus.tileWidth() + right &&
					 bl->getTilePosition().y() - up <= position.y() && 
					 position.y() < bl->getTilePosition().y() + BWAPI::UnitTypes::Protoss_Nexus.tileHeight() + down) ||

					(bl->getTilePosition().x() - left <= position.x() + type.tileWidth() - 1 && 
					 position.x() + type.tileWidth() - 1 < bl->getTilePosition().x() + BWAPI::UnitTypes::Protoss_Nexus.tileWidth() + right &&
					 bl->getTilePosition().y() - up <= position.y() + type.tileHeight() - 1 && 
					 position.y() + type.tileHeight() - 1 < bl->getTilePosition().y() + BWAPI::UnitTypes::Protoss_Nexus.tileHeight() + down))
			{
				return false;
			}
		}

	return true;
}
bool BuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
	return canBuildHereWithSpace(position,type,this->buildDistance);
}
bool BuildingPlacer::canBuildHereWithSpace(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const
{
	//returns true if we can build this type of unit here with the specified amount of space.
	//space value is stored in this->buildDistance.

	//if we can't build here, we of course can't build here with space
	if (!this->canBuildHere(position, type))
		return false;

	int width=type.tileWidth();
	int height=type.tileHeight();

	//make sure we leave space for add-ons. These types of units can have addons:
	if (type==BWAPI::UnitTypes::Terran_Command_Center ||
		type==BWAPI::UnitTypes::Terran_Factory || 
		type==BWAPI::UnitTypes::Terran_Starport ||
		type==BWAPI::UnitTypes::Terran_Science_Facility)
	{
		width+=2;
	}
	int startx = position.x() - buildDist;
	if (startx<0) return false;
	int starty = position.y() - buildDist;
	if (starty<0) return false;
	int endx = position.x() + width + buildDist;
	if (endx>BWAPI::Broodwar->mapWidth()) return false;
	int endy = position.y() + height + buildDist;
	if (endy>BWAPI::Broodwar->mapHeight()) return false;

	if (!type.isRefinery())
	{
		for(int x = startx; x < endx; x++)
			for(int y = starty; y < endy; y++)
				if (!buildable(x, y) || reserveMap[x][y])
					return false;
	}

	if (position.x()>3)
	{
		int startx2=startx-2;
		if (startx2<0) startx2=0;
		for(int x = startx2; x < startx; x++)
			for(int y = starty; y < endy; y++)
			{
				std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(x, y);
				for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); ++i)
				{
					if (!(*i)->isLifted())
					{
						BWAPI::UnitType type=(*i)->getType();
						if (type==BWAPI::UnitTypes::Terran_Command_Center ||
							type==BWAPI::UnitTypes::Terran_Factory || 
							type==BWAPI::UnitTypes::Terran_Starport ||
							type==BWAPI::UnitTypes::Terran_Science_Facility)
						{
							return false;
						}
					}
				}
			}
	}
	return true;
}
BWAPI::TilePosition BuildingPlacer::getBuildLocation(BWAPI::UnitType type) const
{
	//returns a valid build location if one exists, scans the map left to right
	for(int x = 0; x < BWAPI::Broodwar->mapWidth(); x++)
		for(int y = 0; y < BWAPI::Broodwar->mapHeight(); y++)
			if (this->canBuildHere(BWAPI::TilePosition(x, y), type))
				return BWAPI::TilePosition(x, y);
	return BWAPI::TilePositions::None;
}

BWAPI::TilePosition BuildingPlacer::getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type) const
{
	return getBuildLocationNear(position, type, this->buildDistance);
}

BWAPI::TilePosition BuildingPlacer::getBuildLocationNear(BWAPI::TilePosition position, BWAPI::UnitType type, int buildDist) const
{
	//returns a valid build location near the specified tile position.
	//searches outward in a spiral.
	int x      = position.x();
	int y      = position.y();
	int length = 1;
	int j      = 0;
	bool first = true;
	int dx     = 0;
	int dy     = 1;
	while (length < BWAPI::Broodwar->mapWidth()) //We'll ride the spiral to the end
	{
		//if we can build here, return this tile position
		if (x >= 0 && x < BWAPI::Broodwar->mapWidth() && y >= 0 && y < BWAPI::Broodwar->mapHeight())
			if (this->canBuildHereWithSpace(BWAPI::TilePosition(x, y), type, buildDist))
				return BWAPI::TilePosition(x, y);

		//otherwise, move to another position
		x = x + dx;
		y = y + dy;
		//count how many steps we take in this direction
		j++;
		if (j == length) //if we've reached the end, its time to turn
		{
			//reset step counter
			j = 0;

			//Spiral out. Keep going.
			if (!first)
				length++; //increment step counter if needed

			//first=true for every other turn so we spiral out at the right rate
			first =! first;

			//turn counter clockwise 90 degrees:
			if (dx == 0)
			{
				dx = dy;
				dy = 0;
			}
			else
			{
				dy = -dx;
				dx = 0;
			}
		}
		//Spiral out. Keep going.
	}
	return BWAPI::TilePositions::None;
}

bool BuildingPlacer::buildable(int x, int y) const
{
	//returns true if this tile is currently buildable, takes into account units on tile
	if (!BWAPI::Broodwar->isBuildable(x,y)) return false;
	std::set<BWAPI::Unit*> units = BWAPI::Broodwar->getUnitsOnTile(x, y);
	for(std::set<BWAPI::Unit*>::iterator i = units.begin(); i != units.end(); ++i)
		if ((*i)->getType().isBuilding() && !(*i)->isLifted())
			return false;
	return true;
}

void BuildingPlacer::reserveTiles(BWAPI::TilePosition position, int width, int height)
{
	for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); ++x)
		for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); ++y)
			reserveMap[x][y] = true;
}

void BuildingPlacer::freeTiles(BWAPI::TilePosition position, int width, int height)
{
	for(int x = position.x(); x < position.x() + width && x < (int)reserveMap.getWidth(); ++x)
		for(int y = position.y(); y < position.y() + height && y < (int)reserveMap.getHeight(); ++y)
			reserveMap[x][y] = false;
}

void BuildingPlacer::setBuildDistance(int distance)
{
	this->buildDistance=distance;
}
int BuildingPlacer::getBuildDistance() const
{
	return this->buildDistance;
}
bool BuildingPlacer::isReserved(int x, int y) const
{
	if (x<0 || y<0 || x>=(int)reserveMap.getWidth() || y>=(int)reserveMap.getHeight())
		return false;
	return reserveMap[x][y];
}