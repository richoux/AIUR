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

#include <BWTA.h>
#include <ScoutManager.h>
#include <BuildingPlacer.h>
#include <UnitGroupManager.h>

std::pair<std::list<BWTA::BaseLocation*>, double> getBestPathHelper(std::set<BWTA::BaseLocation* > baseLocations)
{
	std::pair<std::list<BWTA::BaseLocation*>, double> shortest_path;
	shortest_path.second=0;
	if (baseLocations.empty()) return shortest_path;
	if (baseLocations.size()==1)
	{
		shortest_path.first.push_back(*baseLocations.begin());
		return shortest_path;
	}
	for(std::set<BWTA::BaseLocation*>::iterator i=baseLocations.begin();i!=baseLocations.end(); ++i)
	{
		BWTA::BaseLocation* node=*i;
		std::set<BWTA::BaseLocation* > baseLocations2=baseLocations;
		baseLocations2.erase(*i);
		std::pair<std::list<BWTA::BaseLocation*>, double> path_result=getBestPathHelper(baseLocations2);
		double dist=path_result.second+node->getGroundDistance(path_result.first.front());
		if( ( dist < shortest_path.second && dist > 0 ) || shortest_path.first.empty())
		{
			path_result.first.push_front(node);
			shortest_path=std::make_pair(path_result.first,dist);
		}
	}
	return shortest_path;
}

std::pair<std::list<BWTA::BaseLocation*>, double> getBestPath(std::set<BWTA::BaseLocation* > baseLocations)
{
	std::pair<std::list<BWTA::BaseLocation*>, double> shortest_path;
	shortest_path.second=0;
	if (baseLocations.empty()) return shortest_path;
	BWTA::BaseLocation* start=BWTA::getStartLocation(BWAPI::Broodwar->self());
	baseLocations.erase(start);
	if (baseLocations.size()==1)
	{
		shortest_path.first.push_back(*baseLocations.begin());
		return shortest_path;
	}
	for(std::set<BWTA::BaseLocation*>::iterator i = baseLocations.begin(); i != baseLocations.end(); ++i)
	{
		BWTA::BaseLocation *node = *i;
		std::set<BWTA::BaseLocation* > baseLocations2 = baseLocations;
		baseLocations2.erase(*i);
		std::pair<std::list<BWTA::BaseLocation*>, double> path_result = getBestPathHelper(baseLocations2);
		double dist=start->getGroundDistance(node) + node->getGroundDistance(path_result.first.front()) + path_result.second;
		if( ( dist<shortest_path.second && dist > 0 ) || shortest_path.first.empty())
		{
			path_result.first.push_front(node);
			shortest_path=std::make_pair(path_result.first,dist);
		}
	}
	return shortest_path;
}

ScoutManager::ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator, InformationManager* infoManager, BuildManager *build)
{
	this->arbitrator = arbitrator;
	this->informationManager = infoManager;
	this->buildManager = build;
	//randomPylon = new Random(100);

	desiredScoutCount = 0;

	myStartLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
	std::set<BWTA::BaseLocation *> startLocations = BWTA::getStartLocations();
	startLocations.erase(myStartLocation);
	for(std::set<BWTA::BaseLocation *>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if (myStartLocation->getGroundDistance(*l) <= 0)
			startLocations.erase(*l);

	std::list<BWTA::BaseLocation*> path = getBestPath(startLocations).first;
	for(std::list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p)
		baseLocationsToScout.push_back(*p);
	scoutDone = false;

	baseLocationsExplored.insert(myStartLocation);	
	this->debugMode=false;

	mustContinue = false;
}

ScoutManager::ScoutManager(Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator, 
						   InformationManager* infoManager, 
						   BuildManager *build,
						   UnitStates *unitStates)
						   : arbitrator(arbitrator), informationManager(infoManager), buildManager(build), unitStates(unitStates)
{
	//randomPylon = new Random(100);

	desiredScoutCount = 0;

	myStartLocation = BWTA::getStartLocation(BWAPI::Broodwar->self());
	std::set<BWTA::BaseLocation *> startLocations = BWTA::getStartLocations();
	startLocations.erase(myStartLocation);
	for(std::set<BWTA::BaseLocation *>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if (myStartLocation->getGroundDistance(*l) <= 0)
			startLocations.erase(*l);

	std::list<BWTA::BaseLocation*> path = getBestPath(startLocations).first;
	for(std::list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p)
		baseLocationsToScout.push_back(*p);
	scoutDone = false;

	baseLocationsExplored.insert(myStartLocation);	
	this->debugMode=false;

	mustContinue = false;
}

ScoutManager::~ScoutManager()
{
	//delete randomPylon;
}

void ScoutManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for(std::set<BWAPI::Unit*>::iterator u = units.begin(); u != units.end(); ++u)
	{
		if( targetOf.find(*u) != targetOf.end() )
			arbitrator->accept(this, *u);
		else if( (*u)->getType().isWorker() && needMoreScouts() )
		{
			arbitrator->accept(this, *u);
			targetOf[*u] = NULL;
		}
		else
			arbitrator->decline(this, *u, 0);
	}
}

void ScoutManager::onRevoke(BWAPI::Unit *unit, double bid)
{
	onRemoveUnit(unit);
}

void ScoutManager::update()
{
	if (!scoutDone)
	{
		if (needMoreScouts())
		{
			requestScout(30); // Bid 30.
		}
		else
		{
			int sCount = desiredScoutCount;
			while ((int)targetOf.size() > sCount)
			{
				arbitrator->removeBid(this, targetOf.begin()->first);
				targetOf.erase(targetOf.begin());
			}
		}
		updateScoutAssignments();
		if (debugMode)
			drawAssignments();
	}
}

std::string ScoutManager::getName() const
{
	return "Scout Manager";
}

std::string ScoutManager::getShortName() const
{
	return "Scout";
}

void ScoutManager::onRemoveUnit(BWAPI::Unit* unit)
{
	if (targetOf.find(unit) != targetOf.end())
	{
		// If the scout was intercepted before being into the enemy base, resend a scout.
		// If it was destroyed in the target area, do not.
		if( unitStates->isState( unit, UnitStates::Searching) &&
			BWTA::getRegion(unit->getTilePosition()) != targetOf[unit]->getRegion())
		{
			BWTA::BaseLocation* lostTarget = targetOf[unit];
			if (baseLocationsExplored.find(lostTarget) == baseLocationsExplored.end())
				baseLocationsToScout.push_back(lostTarget);
		}
		targetOf.erase(unit);
	}
}

void ScoutManager::setScoutCount(int count)
{
	this->desiredScoutCount=count;
}

void ScoutManager::setDebugMode(bool debugMode)
{
	this->debugMode = debugMode;
}

void ScoutManager::drawAssignments()
{
	//draw target vector for each scout
	for (std::map<BWAPI::Unit*, BWTA::BaseLocation*>::iterator s = targetOf.begin(); s != targetOf.end(); ++s)
	{
		if( !unitStates->isState( s->first, UnitStates::Idle ) )
		{
			BWAPI::Position scoutPos = s->first->getPosition();
			BWAPI::Position targetPos = s->second->getPosition();
			BWAPI::Broodwar->drawLineMap(scoutPos.x(), scoutPos.y(), targetPos.x(), targetPos.y(), BWAPI::Colors::Yellow);
			BWAPI::Broodwar->drawCircleMap(scoutPos.x(), scoutPos.y(), 6, BWAPI::Colors::Yellow);
			BWAPI::Broodwar->drawCircleMap(targetPos.x(), targetPos.y(), (*s).first->getType().sightRange(), BWAPI::Colors::Yellow);
		}
	}

	//draw scouting tour
	if (!poly.empty() && !mustContinue)
		for (BWTA::Polygon::iterator it = poly.begin(); it != poly.end(); ++it)
		{
			if (it == itPoly)
				BWAPI::Broodwar->drawCircleMap((*it).x(), (*it).y(), 6, BWAPI::Colors::Green);
			else 
				BWAPI::Broodwar->drawCircleMap((*it).x(), (*it).y(), 6, BWAPI::Colors::Blue);
		}

	if (mineralsCenter.isValid())
		BWAPI::Broodwar->drawCircleMap(mineralsCenter.x(), mineralsCenter.y(), 12, BWAPI::Colors::Red);
}

bool ScoutManager::isScouting() const
{
	return targetOf.size() >= 1;
}

bool ScoutManager::isScouting(BWAPI::Unit *u)
{
	for (std::map<BWAPI::Unit*, BWTA::BaseLocation*>::iterator it = targetOf.begin(); it != targetOf.end(); ++it)
		if (it->first->getID() == u->getID())
			return true;

	return false;
}

bool ScoutManager::needMoreScouts() const
{
	if (informationManager->isEnemySpotted())
		return false;

	int sCount = desiredScoutCount;
	if (baseLocationsExplored.size()==BWAPI::Broodwar->getStartLocations().size())
		sCount = 0;
	return (int)targetOf.size() < sCount;
}

void ScoutManager::requestScout(double bid)
{
	// Bid on all completed workers.
	std::set<BWAPI::Unit*> usefulUnits=SelectAll()(isWorker,Zerg_Overlord)(isCompleted).not(isCarryingMinerals,isCarryingGas,isGatheringGas);
	arbitrator->setBid(this,usefulUnits,bid);
}

void ScoutManager::setArmyManager(ArmyManager *armyManager)
{
	this->armyManager = armyManager;
}

void ScoutManager::updateScoutAssignments()
{
	// Remove scout positions if the enemy is not there.
	std::map<BWAPI::Unit*, BWTA::BaseLocation*>::iterator u;

	for(u = targetOf.begin(); u != targetOf.end(); u++)
	{
		if( unitStates->isState( u->first, UnitStates::Searching )
			&& u->second != NULL // should never happen, but it does actually, when we cannot scout enemy's main.
			&& u->first->getPosition().getApproxDistance( u->second->getPosition() ) < u->first->getType().sightRange() - 2
			&& ( !baseLocationsToScout.empty() || mustContinue ) )
		{
			bool empty = true;

			for(int x = u->second->getTilePosition().x(); x < u->second->getTilePosition().x() + 4; x++)
			{
				for(int y = u->second->getTilePosition().y(); y < u->second->getTilePosition().y() + 3; y++)
				{
					std::set<BWAPI::Unit*> getUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile( x, y );
					for each(BWAPI::Unit* b in getUnitsOnTile)
					{
						// b->getType().isInvincible() means it is a mineral field or a vespene geyser.
						if (b->getType().isBuilding() && !(b->getType().isInvincible()))
						{
							empty = false;
							break;
						}
					}
					if (!empty) break;
				}
				if (!empty) break;
			}

			BWTA::BaseLocation* exploredBaseLocation = u->second;

			// Second check: Are some buildings around me?
			bool existBuildings = false;
			// getUnitsInRadius broken; use UnitGroup::inRadius
			//std::set<BWAPI::Unit*> aroundMe = (*u).first->getUnitsInRadius((*u).first->getType().sightRange());
			UnitGroup aroundMe = SelectAllEnemy()( isBuilding ).inRadius( u->first->getType().sightRange(),
																		  u->first->getPosition() );
			for each(BWAPI::Unit* b in aroundMe)
			{
				// b->getType().isInvincible() means it is a mineral field or a vespene geyser.
				if (!(b->getType().isInvincible()))
				{
					existBuildings = true;
					break;
				}
			}

			if( empty && !existBuildings )
			{
				informationManager->setBaseEmpty( u->second );
				baseLocationsToScout.remove( exploredBaseLocation );
				baseLocationsExplored.insert( exploredBaseLocation );
				unitStates->setState( u->first, UnitStates::Idle );
			}
			else
			{
				if( !informationManager->isEnemySpotted() )
				{
					baseLocationsToScout.clear();
					baseLocationsExplored.insert( exploredBaseLocation );
					informationManager->setEnemyStartLocation( exploredBaseLocation );
				}

				UnitGroup ug = SelectAllEnemy()( isBuilding ).inRegion( exploredBaseLocation->getRegion() );
				if ( informationManager->hasSeenEnemyPatches() )
					ug = ug + UnitGroup::getUnitGroup( informationManager->getEnemyPatches() );
				poly = informationManager->getApproxConvexHull();

				BWAPI::Unit *bat = NULL;
				for each( BWAPI::Unit *unit in ug )
				{
					if( unit->getType().isResourceDepot() )
						bat = unit;
				}

				// get closer to mineral fields
				// 45 is approx TILE_SIZE * sqrt(2)
				if (!poly.empty() && bat != NULL &&
					( u->first->getDistance( BWAPI::Position( bat->getLeft(),  bat->getTop() ) ) < 45 ||
					  u->first->getDistance( BWAPI::Position( bat->getRight(), bat->getTop() ) ) < 45 ||
					  u->first->getDistance( BWAPI::Position( bat->getLeft(),	 bat->getBottom() ) ) < 45 ||
					  u->first->getDistance( BWAPI::Position( bat->getRight(), bat->getBottom() ) ) < 45 ) )
				{
					unitStates->setState( u->first, UnitStates::Roaming );
					mustContinue = false;

					// Start by the closest position to mineral fields
					// in order to count enemy workers
					UnitGroup minerals = UnitGroup::getUnitGroup( exploredBaseLocation->getMinerals() );
					mineralsCenter = minerals.getCenter();
					double minDist = std::numeric_limits<double>::max();
					
					for( BWTA::Polygon::iterator it = poly.begin(); it != poly.end(); ++it )
						if( it->getApproxDistance( mineralsCenter ) < minDist )
						{
							minDist = it->getApproxDistance( mineralsCenter );
							itPoly = it;
						}
				}
				else
					mustContinue = true;
			}
		}

		// Scout should not attack
			if( unitStates->isState( u->first, UnitStates::Scouting ) || unitStates->isState( u->first, UnitStates::Roaming ) )
			{
				// count enemy workers when we see all mineral patches
				UnitGroup mineralPatches = UnitGroup::getUnitGroup( u->second->getMinerals() );
				mineralPatches = mineralPatches( isVisible );
				if( mineralPatches.size() == informationManager->getNumberPatches() )
				{
					informationManager->setCanSeeEnemyPatches( true );
					informationManager->setEnemyPatches( u->second->getMinerals() );
					UnitGroup enemyWorkers = SelectAllEnemy()( isWorker );
					informationManager->setNumberEnemyWorkers( enemyWorkers.size() );
				}
				else
					informationManager->setCanSeeEnemyPatches( false );

				if( u->first->isAttacking() )
				{
					itPoly++;
					if( itPoly == poly.end() )
					{
						poly = informationManager->getApproxConvexHull();
						itPoly = poly.begin();
					}
					u->first->move(*itPoly);
					unitStates->setState( u->first, UnitStates::Scouting );
				}
			}

		if( unitStates->isState( u->first, UnitStates::Roaming ) )
		{			
			u->first->move( *itPoly );
			unitStates->setState( u->first, UnitStates::Scouting );
		}

		if( unitStates->isState( u->first, UnitStates::Scouting ) && 
			u->first->getPosition().getApproxDistance(*itPoly) <= TILE_SIZE )
		{
			do
			{
				itPoly++;
			}
			while( itPoly != poly.end() && !BWAPI::Broodwar->isWalkable( itPoly->x() / 8, itPoly->y() / 8) );

			if( itPoly == poly.end() )
			{
				poly = informationManager->getApproxConvexHull();
				itPoly = poly.begin();
			}
			
			unitStates->setState( u->first, UnitStates::Roaming );
		}
	}

	if( baseLocationsToScout.size() >= 1 ) // are there still positions to scout?
	{
		if( baseLocationsToScout.size() == 1 && BWTA::getStartLocations().size() > 2 )
			informationManager->setEnemyStartLocation( *baseLocationsToScout.begin() );

		std::list<BWTA::BaseLocation*>::iterator p;
		for( u = targetOf.begin(); u != targetOf.end() && !baseLocationsToScout.empty(); u++ )
		{ 
			if( unitStates->isState(u->first, UnitStates::Idle) || ( !u->first->isMoving() ) )
			{
				std::map<double, BWTA::BaseLocation*> distanceMap;
				
				for( p = baseLocationsToScout.begin(); p != baseLocationsToScout.end(); p++ )
				{
					double distance = u->first->getPosition().getApproxDistance( (*p)->getPosition() );
					distanceMap[distance] = *p;
				}

				BWTA::BaseLocation *target = distanceMap.begin()->second;
				unitStates->setState( u->first, UnitStates::Searching );
				u->first->rightClick( target->getPosition() );
				u->second = target;
			}
		} 
	}
}