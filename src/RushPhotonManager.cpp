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

#include <RushPhotonManager.h>
#include <BWTA.h>
#include <cstdlib>

using namespace BWAPI;
using namespace std;

pair<list<BWTA::BaseLocation*>, double> bestPathHelper(set<BWTA::BaseLocation*> baseLocations)
{
	pair<list<BWTA::BaseLocation*>, double> shortest_path;
	shortest_path.second = 0;

	if ( baseLocations.empty() )
		return shortest_path;

	if ( baseLocations.size() == 1 )
	{
		shortest_path.first.push_back( *baseLocations.begin() );
		return shortest_path;
	}

	for( set<BWTA::BaseLocation*>::iterator i = baseLocations.begin(); i != baseLocations.end(); ++i)
	{
		BWTA::BaseLocation* node = *i;
		set<BWTA::BaseLocation*> baseLocations2 = baseLocations;
		baseLocations2.erase( *i );
		pair<list<BWTA::BaseLocation*>, double> path_result = bestPathHelper( baseLocations2 );
		double dist = path_result.second+node->getGroundDistance( path_result.first.front() );
		if ( ( dist < shortest_path.second && dist > 0 ) || shortest_path.first.empty() )
		{
			path_result.first.push_front( node );
			shortest_path = make_pair( path_result.first, dist );
		}
	}

	return shortest_path;
}

pair<list<BWTA::BaseLocation*>, double> bestPath( set<BWTA::BaseLocation*> baseLocations )
{
	pair<list<BWTA::BaseLocation*>, double> shortest_path;
	shortest_path.second = 0;

	if ( baseLocations.empty() )
		return shortest_path;
	
	BWTA::BaseLocation* start = BWTA::getStartLocation( Broodwar->self() );
	baseLocations.erase( start );

	if ( baseLocations.size() == 1 )
	{
		shortest_path.first.push_back( *baseLocations.begin() );
		return shortest_path;
	}

	for( set<BWTA::BaseLocation*>::iterator i = baseLocations.begin(); i != baseLocations.end(); ++i )
	{
		BWTA::BaseLocation *node = *i;
		set<BWTA::BaseLocation* > baseLocations2 = baseLocations;
		baseLocations2.erase(*i);
		pair<list<BWTA::BaseLocation*>, double> path_result = bestPathHelper( baseLocations2 );
		double dist = start->getGroundDistance(node) + node->getGroundDistance( path_result.first.front() ) + path_result.second;
		if ( ( dist < shortest_path.second && dist > 0 ) || shortest_path.first.empty() )
		{
			path_result.first.push_front( node );
			shortest_path = make_pair( path_result.first, dist );
		}
	}

	return shortest_path;
}

RushPhotonManager::RushPhotonManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;

	myStartLocation = BWTA::getStartLocation( Broodwar->self() );
	set<BWTA::BaseLocation*> startLocations = BWTA::getStartLocations();
	startLocations.erase( myStartLocation );

	for(set<BWTA::BaseLocation*>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if ( myStartLocation->getGroundDistance( *l ) <= 0 )
			startLocations.erase( *l );

	list<BWTA::BaseLocation*> path = bestPath( startLocations ).first;
	for( list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p )
		baseLocationsToScout.push_back( *p );
	scoutDone = false;

	baseLocationsExplored.insert(myStartLocation);	
	firstPylonDone = false;
	lastPylon = NULL;
	rusher = NULL;
	hasLostControl = false;
	debugMode = false;
}

RushPhotonManager::RushPhotonManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, UnitStates *unitStates) 
	: arbitrator(arbitrator), unitStates(unitStates)
{
	myStartLocation = BWTA::getStartLocation( Broodwar->self() );
	set<BWTA::BaseLocation*> startLocations = BWTA::getStartLocations();
	startLocations.erase( myStartLocation );

	for(set<BWTA::BaseLocation*>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if ( myStartLocation->getGroundDistance( *l ) <= 0 )
			startLocations.erase( *l );

	list<BWTA::BaseLocation*> path = bestPath( startLocations ).first;
	for( list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p )
		baseLocationsToScout.push_back( *p );
	scoutDone = false;

	baseLocationsExplored.insert(myStartLocation);	
	firstPylonDone = false;
	lastPylon = NULL;
	rusher = NULL;
	hasLostControl = false;
	debugMode = false;
}

RushPhotonManager::~RushPhotonManager() {}

void RushPhotonManager::setBuildOrderManager( BuildOrderManager *buildOrderManager )
{
	this->buildOrderManager = buildOrderManager;
	buildingPlacer = buildOrderManager->getBuildManager()->getBuildingPlacer();
}

void RushPhotonManager::setDebugMode( bool debugMode )
{
	this->debugMode = debugMode;
}

void RushPhotonManager::setInformationManager( InformationManager *informationManager )
{
	this->informationManager = informationManager;
}

void RushPhotonManager::setWorkerManager( WorkerManager *workerManager )
{
	this->workerManager = workerManager;
}

string RushPhotonManager::getName() const
{
	return "RushPhoton Manager";
}

string RushPhotonManager::getShortName() const
{
	return "RP";
}

void RushPhotonManager::onOffer(set<Unit*> units)
{
	for each (Unit *u in units)
	{
		if ( rusher == NULL && u->getType().isWorker() )
		{
			workerManager->removeBid( u );
			arbitrator->accept(this, u);
			rusher = u;
		}
	}
}

void RushPhotonManager::onRevoke(Unit *u, double bid)
{
	onRemoveUnit(u);
}

void RushPhotonManager::onRemoveUnit(Unit* u)
{
	if (rusher == u)
		rusher = NULL;
}

Position RushPhotonManager::computeStartingPosition( BWTA::BaseLocation *baseLoc )
{
	poly = baseLoc->getRegion()->getPolygon();
	int addX, addY;

	for( unsigned int i = 0; i < poly.size(); ++i )
	{
		addX = -1;
		addY = -1;

		if( poly[i].x() < poly.getCenter().x() )
			addX = 1;
		
		if( poly[i].y() < poly.getCenter().y() )
			addY = 1;

		poly[i] = Position( poly[i].x() + ( addX * TILE_SIZE ), poly[i].y() + ( addY * TILE_SIZE ) );
	}
	
	double maxDistance = 0.0;
	Position startingPoint = poly[0];

	int sight = Broodwar->enemy()->getRace().getCenter().sightRange();
	if( sight == 0 ) // may append with random opponents
		sight = 352; // Nexus' sight (in pixel), the largest.

	for( unsigned int i = 0; i < poly.size(); ++i )
	{
		if( BWTA::getRegion( poly[i] ) == BWTA::getRegion( baseLoc->getTilePosition() ) )
		{
			double dist = baseLoc->getPosition().getApproxDistance( poly[i] );
			for each( BWTA::Chokepoint *choke in baseLoc->getRegion()->getChokepoints() )
				dist += choke->getCenter().getApproxDistance( poly[i] );
			
			if( dist > maxDistance && baseLoc->getPosition().getApproxDistance( poly[i] ) > sight + ( TILE_SIZE * 5 ) )
			{
				startingPoint = poly[i];
				maxDistance = dist;
			}
		}
	}

	int x = ( baseLoc->getPosition().x() - startingPoint.x() ) / 20;
	int y = ( baseLoc->getPosition().y() - startingPoint.y() ) / 20;

	Position firstPylonHere = startingPoint;

	// The latter, the farest from the enemy base we build our pylon
	double factor = 1.5;
	if( Broodwar->getFrameCount() > 1200  && Broodwar->getFrameCount() <= 2400)
		factor = 1.75;
	else if( Broodwar->getFrameCount() > 2400 )
		factor = 2;

	while( firstPylonHere.getApproxDistance( baseLoc->getPosition() ) > sight * factor )
		firstPylonHere = Position( firstPylonHere.x() + x, firstPylonHere.y() + y );
	
	return firstPylonHere;
}

bool RushPhotonManager::determineIfGoForward( BWTA::BaseLocation *baseLoc, Position start, Position end, int &_sneakyIndex )
{
	int startIndex = -1;
	int endIndex = -1;

	for( unsigned int i = 0; i < poly.size(); ++i )
	{
		if( startIndex != -1 && endIndex != -1 )
			break;

		// Since BWTA::getNearestPoint seems not working well, we approximate start and end points
		if( startIndex == -1 && start.getApproxDistance( poly[i] ) < UnitTypes::Protoss_Probe.sightRange() / 4 )
			startIndex = i;

		if( endIndex == -1 && end.getApproxDistance( poly[i] ) < UnitTypes::Protoss_Probe.sightRange() / 4 )
			endIndex = i;
	}

	_sneakyIndex = startIndex;

	Position current = start;
	int nextIndex = startIndex + 1;
	//while( current != end )
	while( nextIndex != endIndex )
	{
		if( nextIndex == poly.size() )
			nextIndex = 0;

		if( current.getApproxDistance( baseLoc->getPosition() ) < UnitTypes::Protoss_Probe.sightRange() )
			return false;

		if( current.getApproxDistance( poly[nextIndex] ) > UnitTypes::Protoss_Probe.sightRange() )
			return false;

		current = poly[nextIndex];
		++nextIndex;
	}

	return true;
}

void RushPhotonManager::loseControl()
{
	if( rusher != NULL && arbitrator->hasBid( rusher ) )
	{
		hasLostControl = true;
		arbitrator->removeBid( this, rusher );
	}
}

bool RushPhotonManager::isCheesing(Unit *u)
{
	return u == rusher;
}

void RushPhotonManager::setLastPylon(Unit *pylon)
{
	if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Pylon ) > 1 )
		lastPylon = pylon;
}

Position RushPhotonManager::nextsneakyPosition()
{
	Position sneaky;

	//do
	//{
		if( goForward )
		{
			if( sneakyIndex == poly.size() )
			{
				sneakyIndex = 0;
				sneaky = poly[0];
			}
			else
				sneaky = poly[sneakyIndex++];
		}
		else
		{
			if( sneakyIndex == -1 )
			{
				sneakyIndex = poly.size() - 1;
				sneaky = poly[sneakyIndex];
			}
			else
				sneaky = poly[sneakyIndex--];
		}
	//}
	//while ( BWTA::getRegion( TilePosition( sneaky ) ) != BWTA::getRegion( rusher->getTilePosition() ) );

	return sneaky;
}

void RushPhotonManager::updateScoutAssignments()
{
	if( informationManager->isEnemySpotted() && !scoutDone )
	{
		if( debugMode )
		{
			Broodwar->drawLineMap( rusher->getPosition().x(), rusher->getPosition().y(), startingPosition.x(), startingPosition.y(), Colors::Red );
			Broodwar->drawCircleMap( poly.getNearestPoint( startingPosition ).x(), poly.getNearestPoint( startingPosition ).y(), 10, Colors::Red, true );
			Broodwar->drawTextMap( startingPosition.x(), startingPosition.y(), "%d", startingPosition.getApproxDistance(target->getPosition()), Colors::Blue );
			Broodwar->drawLineMap( rusher->getPosition().x(), rusher->getPosition().y(), sneakyPosition.x(), sneakyPosition.y(), Colors::Orange );

			for( unsigned int i = 0; i < target->getRegion()->getPolygon().size(); ++i )
			{
				if( i + 1 < target->getRegion()->getPolygon().size() )
					Broodwar->drawLineMap(	target->getRegion()->getPolygon()[i].x(), target->getRegion()->getPolygon()[i].y(), 
											target->getRegion()->getPolygon()[i+1].x(), target->getRegion()->getPolygon()[i+1].y(), Colors::Cyan );
				else
					Broodwar->drawLineMap(	target->getRegion()->getPolygon()[i].x(), target->getRegion()->getPolygon()[i].y(), 
											target->getRegion()->getPolygon()[0].x(), target->getRegion()->getPolygon()[0].y(), Colors::Cyan );
			}

			for( unsigned int i = 0; i < poly.size(); ++i )
				Broodwar->drawTextMap( poly[i].x(), poly[i].y(), "%d", i, Colors::Purple );
		}

		// if the probe is fucking around, tell him he has work to do.
		if( !rusher->isMoving() || rusher->isAttacking() )
		{
			sneakyPosition = nextsneakyPosition();
			rusher->move( sneakyPosition );
		}

		if( rusher->getDistance( startingPosition ) <= rusher->getType().sightRange() )
		{
			scoutDone = true;
			rusher->move( startingPosition );
		}
		else if( rusher->getDistance( sneakyPosition ) <= rusher->getType().sightRange() / 2 )
		{
			sneakyPosition = nextsneakyPosition();
			rusher->move( sneakyPosition );
		}
	}
	else
	{
		if (baseLocationsToScout.size() > 1) // are there still positions to scout?
		{
			map<double, BWTA::BaseLocation*> distanceMap;
			for each (BWTA::BaseLocation *p in baseLocationsToScout)
			{
				double distance = rusher->getDistance( p->getPosition() );
				distanceMap[distance] = p;
			}

			target = distanceMap.begin()->second;
			rusher->move( target->getPosition() );
		}
		else if (baseLocationsToScout.size() == 1)
		{
			target = *baseLocationsToScout.begin();
			baseLocationsToScout.clear();
			baseLocationsExplored.insert( target );
			informationManager->setEnemyStartLocation( target );
			startingPosition = computeStartingPosition( target );
			sneakyPosition = poly.getNearestPoint( ( *target->getRegion()->getChokepoints().rbegin() )->getCenter() );
			rusher->move( sneakyPosition );
			goForward = determineIfGoForward( target, sneakyPosition, poly.getNearestPoint( startingPosition ), sneakyIndex );
			return;
		}

		if( target != NULL && 
			!baseLocationsToScout.empty() && 
			rusher->getDistance( target->getPosition() ) < rusher->getType().sightRange() )
		{
			bool empty = true;
			for(int x = target->getTilePosition().x(); x < target->getTilePosition().x() + 4; ++x)
			{
				for(int y = target->getTilePosition().y(); y < target->getTilePosition().y() + 3; ++y)
				{
					set<Unit*> getUnitsOnTile = Broodwar->getUnitsOnTile(x, y);
					for each(Unit* u in getUnitsOnTile)
					{
						// u->getType().isInvincible() means it is a mineral field or a vespene geyser.
						if (u->getType().isBuilding() && !(u->getType().isInvincible()))
						{
							empty = false;
							break;
						}
					}
					if (!empty) break;
				}
				if (!empty) break;
			}

			if (empty)
			{
				informationManager->setBaseEmpty( target );
				baseLocationsToScout.remove( target );
				baseLocationsExplored.insert( target );
			}
			else
			{
				baseLocationsToScout.clear();
				baseLocationsExplored.insert( target );
				informationManager->setEnemyStartLocation( target );
				startingPosition = computeStartingPosition( target );
				sneakyPosition = poly.getNearestPoint( ( *target->getRegion()->getChokepoints().rbegin() )->getCenter() );
				rusher->move( sneakyPosition );
				goForward = determineIfGoForward( target, sneakyPosition, poly.getNearestPoint( startingPosition ), sneakyIndex );
			}
		}
	}
}

void RushPhotonManager::update()
{
	if( !hasLostControl )
	{
		if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Probe ) < 8 )
		{
			TilePosition myBasePos = myStartLocation->getTilePosition();
			set<Unit*> bases = Broodwar->getUnitsOnTile( myBasePos.x(), myBasePos.y() );
			Unit *myBase;
			if( !bases.empty() )
				myBase = *bases.begin();
			else
				myBase = NULL;

			if ( myBase != NULL && !myBase->isTraining() && 
				Broodwar->self()->allUnitCount( UnitTypes::Protoss_Probe ) < 8 &&
				Broodwar->self()->minerals() >= 50 )
				myBase->train( UnitTypes::Protoss_Probe );
		}

		if( Broodwar->self()->minerals() >= 100 && Broodwar->self()->allUnitCount( UnitTypes::Protoss_Pylon ) < 1 )
			buildOrderManager->build( 1, UnitTypes::Protoss_Pylon, 90 );

		if( rusher == NULL)
		{
			UnitGroup workers = SelectAll()(isCompleted)(isWorker);
			if( !workers.empty() )
			{
				arbitrator->setBid(this, ( *workers.rbegin() ), 60);
			}
		}
		else
		{
			if( !scoutDone )
			{
				updateScoutAssignments();
				if( !unitStates->isState( rusher, UnitStates::Searching ) )
					unitStates->setState( rusher, UnitStates::Searching );
			}
			else if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Forge ) > 0 )
			{
				if( !unitStates->isState( rusher, UnitStates::Cheesing ) )
					unitStates->setState( rusher, UnitStates::Cheesing );

				if( !firstPylonDone )
				{
					// Position of the first pylon
					if( Broodwar->self()->minerals() >= 100 )
					{
						TilePosition toBuild = buildingPlacer->getBuildLocationNear( TilePosition( startingPosition ), UnitTypes::Protoss_Pylon, 0 );
						rusher->build( toBuild, UnitTypes::Protoss_Pylon );
					}

					if( lastPylon != NULL )
						firstPylonDone = true;
				}
				else
				{ 
					if( Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Pylon ) == Broodwar->self()->allUnitCount( UnitTypes::Protoss_Pylon ) && 
						( Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Pylon ) - 1 ) * 3 > Broodwar->self()->allUnitCount( UnitTypes::Protoss_Photon_Cannon ) )
					{
						if( Broodwar->self()->minerals() >= 150 )
						{
							TilePosition pylonTile = lastPylon->getTilePosition();
							int xDirection = pylonTile.x() - target->getTilePosition().x();
							int yDirection = pylonTile.y() - target->getTilePosition().y();
							int xTile, yTile;
							int xShift, yShift;

							if( abs( xDirection ) > abs( yDirection ) )
							{
								xDirection > 0 ? xTile = pylonTile.x() - 3 : xTile = pylonTile.x() + 3;
								yTile = pylonTile.y();
								xShift = 0;
								yShift = 1;
							}
							else
							{
								yDirection > 0 ? yTile = pylonTile.y() - 3 : yTile = pylonTile.y() + 3;
								xTile = pylonTile.x();
								yShift = 0;
								xShift = 1;
							}

							TilePosition toBuild;

							switch( ( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Photon_Cannon ) % 3 ) )
							{
							case 2:
								toBuild = buildingPlacer->getBuildLocationNear( TilePosition( xTile + xShift*2, yTile + yShift*2 ), UnitTypes::Protoss_Photon_Cannon, 0 );
								break;
							case 1:
								toBuild = buildingPlacer->getBuildLocationNear( TilePosition( xTile, yTile ), UnitTypes::Protoss_Photon_Cannon, 0 );
								break;
							case 0:
								toBuild = buildingPlacer->getBuildLocationNear( TilePosition( xTile - xShift*2, yTile - yShift*2 ), UnitTypes::Protoss_Photon_Cannon, 0 );
								break;
							}

							rusher->build( toBuild, UnitTypes::Protoss_Photon_Cannon );
						}
					}
					else if( Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Photon_Cannon ) == Broodwar->self()->allUnitCount( UnitTypes::Protoss_Photon_Cannon ) && 
						Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Photon_Cannon ) >= 3 * ( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Pylon ) - 1 ) )
					{	
						TilePosition pylonTile = lastPylon->getTilePosition();
						int xDirection = pylonTile.x() - target->getTilePosition().x();
						int yDirection = pylonTile.y() - target->getTilePosition().y();
						int xTile, yTile;

						if( (int)abs( xDirection ) > (int)abs( yDirection ) )
						{
							xDirection > 0 ? xTile = pylonTile.x() - 6 : xTile = pylonTile.x() + 6;
							yTile = pylonTile.y();
						}
						else
						{
							yDirection > 0 ? yTile = pylonTile.y() - 6 : yTile = pylonTile.y() + 6;
							xTile = pylonTile.x();
						}
						TilePosition toBuild = buildingPlacer->getBuildLocationNear( TilePosition( xTile, yTile ), UnitTypes::Protoss_Pylon, 0 );
						rusher->build( toBuild, UnitTypes::Protoss_Pylon );
					}
				}
			}
		}
	}	
}