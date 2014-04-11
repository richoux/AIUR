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

#include <InformationManager.h>
#include "Util.h"
#include <cmath>
using namespace std;
using namespace BWAPI;

const double PI = 3.141592;

InformationManager* TheInformationManager = NULL;

InformationManager* InformationManager::create()
{
	if (TheInformationManager) return TheInformationManager;
	return new InformationManager();
}

void InformationManager::destroy()
{
	if (TheInformationManager)
		delete TheInformationManager;
}

InformationManager::InformationManager()
{
	TheInformationManager = this;
	buildTime[Broodwar->enemy()->getRace().getCenter()]=0;
	buildTime[Broodwar->enemy()->getRace().getWorker()]=0;
	if (Broodwar->enemy()->getRace()==Races::Zerg)
	{
		buildTime[UnitTypes::Zerg_Larva]=0;
		buildTime[UnitTypes::Zerg_Overlord]=0;
	}
	startLocationCouldContainEnemy = BWTA::getStartLocations();
	startLocationCouldContainEnemy.erase(BWTA::getStartLocation(Broodwar->self()));
	scoutedMainEnemyBase = false;
	enemyNatural = NULL;

	enemyStartLocation = *startLocationCouldContainEnemy.begin();
	enemyBaseDestroyed = false;
	for each (BWTA::BaseLocation *bl in BWTA::getBaseLocations())
		unvisitedBaseLocations.insert(bl);

	UnitGroup patches = UnitGroup::getUnitGroup(BWTA::getStartLocation(Broodwar->self())->getMinerals());
	numberMineralPatches = patches.size();
	canSeePatches = false;
	hasSeenPatches = false;
	numberEnemyWorkers = 0;

	minLeft = std::numeric_limits<int>::max();
	minTop = std::numeric_limits<int>::max();
	maxRight = -1;
	maxBottom = -1;
}

InformationManager::~InformationManager()
{
	TheInformationManager = NULL;
}

void InformationManager::debugCheckpoint()
{
	for each(BWAPI::Unit *u in enemyBuildings)
	{
		int ray = (u->getRight() - u->getLeft()) / 2;
		BWAPI::Broodwar->drawCircleMap(u->getPosition().x(), u->getPosition().y(), ray, BWAPI::Colors::Green);
	}
}

BWTA::Polygon InformationManager::getApproxConvexHull()
{
	if (!scoutedMainEnemyBase)
		return BWTA::Polygon();

	// Corners
	BWAPI::Position ur(maxRight + TILE_SIZE/2, minTop - 2*TILE_SIZE);
	BWAPI::Position ul(minLeft - TILE_SIZE/2, minTop - 2*TILE_SIZE);
	BWAPI::Position br(maxRight + TILE_SIZE/2, maxBottom + 2*TILE_SIZE);
	BWAPI::Position bl(minLeft - TILE_SIZE/2, maxBottom + 2*TILE_SIZE);

	ur.makeValid();
	ul.makeValid();
	br.makeValid();
	bl.makeValid();

	BWTA::Polygon poly;

	poly.push_back(ul);
	poly.push_back(bl);
	poly.push_back(br);
	poly.push_back(ur);

	return poly;
}

void InformationManager::onUnitMorph(Unit *unit)
{
	this->onUnitDiscover(unit);
}

void InformationManager::onUnitDiscover(Unit *unit)
{
	savedData[unit].exists = true;

	// take only into account enemy's units and all mineral fields
	if (!Broodwar->self()->isEnemy(unit->getPlayer()) && !unit->getType().isMineralField()) 
		return;

	int time = Broodwar->getFrameCount();
	UnitType type=unit->getType();
	updateBuildTime(type,time-type.buildTime());

	if (!scoutedMainEnemyBase)
	{
		// add main base's buildings and minerals to enemyBuilding if enemyStartLocation is not defined yet
		BWTA::BaseLocation* b = BWTA::getNearestBaseLocation(unit->getTilePosition());
		if (unit->getType().isBuilding() && !unit->getType().isInvincible() &&
			startLocationCouldContainEnemy.find(b) != startLocationCouldContainEnemy.end() && 
			enemyBuildings.find(unit) == enemyBuildings.end())
		{
			enemyBuildings.insert(unit);
			if (unit->getTop() < minTop)
				minTop = unit->getTop();
			if (unit->getLeft() < minLeft)
				minLeft = unit->getLeft();
			if (unit->getBottom() > maxBottom)
				maxBottom = unit->getBottom();
			if (unit->getRight() > maxRight)
				maxRight = unit->getRight();
		}

		if (unit->getType().isBuilding() && !unit->getType().isMineralField())
		{
			BWTA::Region* r=BWTA::getRegion(unit->getTilePosition());
			if (r->getBaseLocations().size()==1)
			{
				BWTA::BaseLocation* b = *(r->getBaseLocations().begin());
				bool toAdd = true;
				for (std::vector<BWTA::BaseLocation*>::const_iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
				{
					if ((*it) == b)
					{
						toAdd = false;
						break;
					}
				}
				if (toAdd)
					enemyBases.push_back(b);
			}
		}
	}
	
	if (unit->getType().isResourceDepot())
	{
		BWTA::BaseLocation* b = BWTA::getNearestBaseLocation(unit->getTilePosition());
		bool toAdd = true;
		for (std::vector<BWTA::BaseLocation*>::const_iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
		{
			if ((*it) == b)
			{
				toAdd = false;
				break;
			}
		}
		
		if (toAdd)
			enemyBases.push_back(b);

		// test if b is not too far from unit (can happen with macro hatch)
		if ( b->getPosition().getApproxDistance( unit->getPosition() ) <= 3 * TILE_SIZE )
		{
			if ( enemyBaseCenters[b] == NULL || !enemyBaseCenters[b]->exists() )
				enemyBaseCenters[b] = unit;
			
			if (unvisitedBaseLocations.find(b) != unvisitedBaseLocations.end())
				unvisitedBaseLocations.erase(b);
		}
	}

	// if unit is a new building or mineral field in the same region than enemy main base
	if (scoutedMainEnemyBase && enemyStartLocation->getRegion() == BWTA::getRegion(unit->getTilePosition()) &&
		(unit->getType().isBuilding() || unit->getType().isMineralField()) &&
		enemyBuildings.find(unit) == enemyBuildings.end())
	{
		enemyBuildings.insert(unit);
		if (unit->getTop() < minTop)
			minTop = unit->getTop();
		if (unit->getLeft() < minLeft)
			minLeft = unit->getLeft();
		if (unit->getBottom() > maxBottom)
			maxBottom = unit->getBottom();
		if (unit->getRight() > maxRight)
			maxRight = unit->getRight();
	}
}

void InformationManager::setNumberEnemyWorkers(int number)
{
	numberEnemyWorkers = number;
}

int InformationManager::getNumberEnemyWorkers()
{
	return numberEnemyWorkers;
}

bool InformationManager::canSeeEnemyPatches()
{
	return canSeePatches;
}

bool InformationManager::hasSeenEnemyPatches()
{
	return hasSeenPatches;
}

void InformationManager::setCanSeeEnemyPatches(bool can)
{
	canSeePatches = can;
	if (!hasSeenPatches && can)
		hasSeenPatches = true;
}

void InformationManager::setEnemyPatches (std::set<BWAPI::Unit*> patches)
{
	enemyPatches.clear();

	for (std::set<BWAPI::Unit*>::iterator it = patches.begin(); it != patches.end(); ++it)
		enemyPatches.insert(*it);
}

std::set<BWAPI::Unit*> InformationManager::getEnemyPatches()
{
	return enemyPatches;
}

std::set<BWAPI::Unit*> InformationManager::getEnemyBuildings()
{
	return enemyBuildings;
}

unsigned int InformationManager::countEnemyBuildings(BWAPI::UnitType ut)
{
	unsigned int total = 0;
	for each (BWAPI::Unit *u in enemyBuildings)
		if (u->getType() == ut)
			++total;
	return total;
}

double InformationManager::computePossibleDamage(BWAPI::TilePosition t, BWAPI::Unit *myUnit)
{
	BWAPI::Position pos( t );
	UnitGroup enemies = SelectAllEnemy()(canAttack)(isCompleted)
		.inRadius((double)BWAPI::WeaponTypes::Arclite_Shock_Cannon.maxRange(), pos);

	if (enemies.empty())
		return 0;

	double totalDamage = 0.0;
	for each (BWAPI::Unit *u in enemies)
	{
		if( u->getType().maxGroundHits() > 0 )
		{
			BWAPI::WeaponType groundWeapon = u->getType().groundWeapon();
			double dist = u->getDistance( pos );
			int maxR = groundWeapon.maxRange();
			int minR = groundWeapon.minRange();

			if( u->getDistance( pos ) <= groundWeapon.maxRange() &&
				u->getDistance( pos ) >= groundWeapon.minRange() )
			{
				double damage = (double)groundWeapon.damageAmount();
				if (groundWeapon.damageType() == BWAPI::DamageTypes::Concussive)
				{
					if (myUnit->getType().size() == BWAPI::UnitSizeTypes::Medium)
						totalDamage += damage * 0.5;
					else if (myUnit->getType().size() == BWAPI::UnitSizeTypes::Large)
						totalDamage += damage * 0.25;
					else
						totalDamage += damage;
				}
				else if (groundWeapon.damageType() == BWAPI::DamageTypes::Explosive)
				{
					if (myUnit->getType().size() == BWAPI::UnitSizeTypes::Small)
						totalDamage += damage * 0.5;
					else if (myUnit->getType().size() == BWAPI::UnitSizeTypes::Medium)
						totalDamage += damage * 0.75;
					else
						totalDamage += damage;
				}
				else
					totalDamage += damage;
			}
		}
	}

	return totalDamage;
}

std::set<BWAPI::TilePosition> InformationManager::computeCircle(BWAPI::Position p, double ray)
{
	std::set<BWAPI::TilePosition> circle;
	double radian;
	int dx, dy;

	for (int degree = 0; degree < 360; degree += 10)
	{
		radian = (double)degree * PI / 180;
		// divided by 8 to have walk tile size
		dx = (int)( cos( radian ) * ray );
		dy = (int)( sin( radian ) * ray );
		BWAPI::TilePosition tp( ( p.x() + dx ) / TILE_SIZE, ( p.y() + dy ) / TILE_SIZE );
		circle.insert( tp );
		//circle.insert(BWAPI::TilePosition(p.x() + dx, p.y() + dy));
	}

	return circle;
}

bool InformationManager::isEnemySpotted()
{
	return scoutedMainEnemyBase;
}

int InformationManager::getNumberBaseCenters()
{
	return enemyBaseCenters.size();
}

int InformationManager::getNumberEnemyBases()
{
	return enemyBases.size();
}

int InformationManager::getNumberEnemyBuildings()
{
	return enemyBuildings.size();
}

int InformationManager::getNumberEvadedUnits(BWAPI::UnitType unitType)
{
	int count = 0;
	
	for (std::map<BWAPI::Unit*, UnitData>::const_iterator it = savedData.begin(); it != savedData.end(); ++it)
		if (it->second.type == unitType)
			++count;

	return count;
}

std::set<BWTA::BaseLocation*> InformationManager::getUnvisitedBaseLocation()
{
	return unvisitedBaseLocations;
}

void InformationManager::onUnitEvade(Unit* unit)
{
	savedData[unit].player=unit->getPlayer();
	savedData[unit].type=unit->getType();
	savedData[unit].position=unit->getPosition();
	savedData[unit].lastSeenTime=Broodwar->getFrameCount();
}

void InformationManager::onUnitDestroy(Unit* unit)
{
	if( savedData.find( unit ) != savedData.end() )
	{
		this->onUnitEvade(unit);
		savedData[unit].exists=false;
		if (!Broodwar->self()->isEnemy(unit->getPlayer())) return;
		if (unit->getType().isResourceDepot())
		{
			BWTA::BaseLocation* b = BWTA::getNearestBaseLocation(unit->getTilePosition());
			if ( enemyBaseCenters[b] == unit )
			{
				for (std::vector<BWTA::BaseLocation*>::const_iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
				{
					if ((*it) == b)
					{
						this->enemyBases.erase(it);
						break;
					}
				}			
				this->enemyBaseCenters.erase(b);
				enemyBaseDestroyed = true;
			}
		}

		for (std::set<BWAPI::Unit*>::iterator it = enemyBuildings.begin(); it != enemyBuildings.end(); ++it)
			if ((*it)->getID() == unit->getID())
			{
				enemyBuildings.erase(it);
				break;
			}
	}
}

Player* InformationManager::getPlayer(Unit* unit) const
{
	if (unit->exists())
		return unit->getPlayer();

	map<Unit*,UnitData>::const_iterator i=savedData.find(unit);
	
	if (i==savedData.end())
		return NULL;
	
	return (*i).second.player;
}

UnitType InformationManager::getType(Unit* unit) const
{
	if (unit->exists())
		return unit->getType();

	map<Unit*,UnitData>::const_iterator i=savedData.find(unit);
	
	if (i==savedData.end())
		return UnitTypes::None;

	return (*i).second.type;
}

Position InformationManager::getLastPosition(Unit* unit) const
{
	if (unit->exists())
		return unit->getPosition();

	map<Unit*,UnitData>::const_iterator i=savedData.find(unit);
	
	if (i==savedData.end())
		return Positions::None;

	return (*i).second.position;
}

int InformationManager::getLastSeenTime(Unit* unit) const
{
	if (unit->exists())
		return Broodwar->getFrameCount();

	map<Unit*,UnitData>::const_iterator i=savedData.find(unit);
	
	if (i==savedData.end())
		return -1;

	return (*i).second.lastSeenTime;
}

bool InformationManager::exists(Unit* unit) const
{
	if (unit->exists())
		return true;

	map<Unit*,UnitData>::const_iterator i=savedData.find(unit);
	
	if (i==savedData.end())
		return false;

	return (*i).second.exists;
}

bool InformationManager::enemyHasBuilt(UnitType type) const
{
	return (buildTime.find(type)!=buildTime.end());
}

int InformationManager::getBuildTime(UnitType type) const
{
	map<UnitType, int>::const_iterator i=buildTime.find(type);

	if (i==buildTime.end())
		return -1;

	return i->second;
}

const vector<BWTA::BaseLocation*>& InformationManager::getEnemyBases() const
{
	return this->enemyBases;
}

const std::map<BWAPI::Unit*, InformationManager::UnitData>& InformationManager::getSavedData() const
{
	return this->savedData;
}

void InformationManager::setBaseEmpty(BWTA::BaseLocation* base)
{
	this->startLocationCouldContainEnemy.erase(base);
	if (startLocationCouldContainEnemy.size()==1)
	{
		bool toAdd = true;
		for (std::vector<BWTA::BaseLocation*>::const_iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
		{
			if ((*it) == *startLocationCouldContainEnemy.begin())
			{
				toAdd = false;
				break;
			}
		}
		if (toAdd)
			enemyBases.push_back(*startLocationCouldContainEnemy.begin());
	}
}

void InformationManager::refreshEnemyBases()
{
	std::set<std::vector<BWTA::BaseLocation*>::iterator> toErase;
	for (std::vector<BWTA::BaseLocation*>::iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
	{
		if (unvisitedBaseLocations.find(*it) == unvisitedBaseLocations.end())
		{
			UnitGroup visibleBuildings = SelectAllEnemy()(isBuilding).not(isInvincible).inRegion((*it)->getRegion());
			bool noHiddenBuildings = true;
			for (std::map<BWAPI::Unit*, UnitData>::const_iterator it2 = savedData.begin(); it2 != savedData.end(); ++it2)
			{
				if (it2->second.type.isBuilding() && it2->second.player == BWAPI::Broodwar->enemy() && it2->second.exists && !it2->first->isInvincible())
				{
					BWAPI::TilePosition tp(it2->second.position);
					BWTA::Region *r = BWTA::getRegion(tp);
					if (r == (*it)->getRegion())
					{
						noHiddenBuildings = false;
						break;
					}
				}
			}

			if (visibleBuildings.empty() && noHiddenBuildings)
				toErase.insert(it);
		}		
	}

	for each (std::vector<BWTA::BaseLocation*>::iterator it in toErase)
		enemyBases.erase(it);
}

void InformationManager::setEnemyStartLocation(BWTA::BaseLocation* base)
{
	enemyStartLocation = base;
	scoutedMainEnemyBase = true;

	bool toAdd = true;
	for (std::vector<BWTA::BaseLocation*>::const_iterator it = enemyBases.begin(); it != enemyBases.end(); ++it)
	{
		if ((*it) == enemyStartLocation)
		{
			toAdd = false;
			break;
		}
	}
	if (toAdd)
		enemyBases.push_back(enemyStartLocation);
}

BWTA::BaseLocation* InformationManager::getEnemyStartLocation()
{
	return enemyStartLocation;
}

void InformationManager::setEnemyNatural(BWTA::BaseLocation* base)
{
	enemyNatural = base;
}

BWTA::BaseLocation* InformationManager::getEnemyNatural()
{
	return enemyNatural;
}

void InformationManager::computeEnemyNatural()
{
	if (enemyStartLocation != NULL && enemyNatural == NULL)
	{
		std::set<BWTA::BaseLocation *> allBaseLocations = BWTA::getBaseLocations();
		allBaseLocations.erase(enemyStartLocation);
		double minDistance = std::numeric_limits<double>::max();
		double distance;
		for each (BWTA::BaseLocation *bl in allBaseLocations)
		{
			distance = enemyStartLocation->getGroundDistance(bl);
			if (distance > 0 && distance < minDistance && !bl->isIsland())
			{
				minDistance = distance;
				enemyNatural = bl;
			}
		}
	}
}

bool InformationManager::getEnemyBaseDestroyed()
{
	return enemyBaseDestroyed;
}

void InformationManager::setEnemyBaseDestroyed(bool enemyBaseDestroyed)
{
	this->enemyBaseDestroyed = enemyBaseDestroyed;
}

void InformationManager::updateBuildTime(UnitType type, int time)
{
	map<UnitType, int>::iterator i=buildTime.find(type);

	if (i!=buildTime.end() && (i->second<=time || i->second==0)) return;

	buildTime[type]=time;

	if (time<0) return;

	for(map< UnitType,int>::const_iterator i=type.requiredUnits().begin();i!=type.requiredUnits().end(); ++i)
	{
		updateBuildTime(i->first,time-i->first.buildTime());
	}
}

int InformationManager::getNumberPatches()
{
	return numberMineralPatches;
}

InformationManager::UnitData::UnitData()
{
	position     = Positions::Unknown;
	type         = UnitTypes::Unknown;
	player       = NULL;
	lastSeenTime = -1;
	exists       = false;
}