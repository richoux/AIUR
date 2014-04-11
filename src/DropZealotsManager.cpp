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

#include <DropZealotsManager.h>

pair<list<BWTA::BaseLocation*>, double> bestDropHelper(set<BWTA::BaseLocation*> baseLocations)
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
		pair<list<BWTA::BaseLocation*>, double> path_result = bestDropHelper( baseLocations2 );
		double dist = path_result.second+node->getGroundDistance( path_result.first.front() );
		if ( ( dist < shortest_path.second && dist > 0 ) || shortest_path.first.empty() )
		{
			path_result.first.push_front( node );
			shortest_path = make_pair( path_result.first, dist );
		}
	}

	return shortest_path;
}

pair<list<BWTA::BaseLocation*>, double> bestDrop( set<BWTA::BaseLocation*> baseLocations )
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
		pair<list<BWTA::BaseLocation*>, double> path_result = bestDropHelper( baseLocations2 );
		double dist = start->getGroundDistance(node) + node->getGroundDistance( path_result.first.front() ) + path_result.second;
		if ( ( dist < shortest_path.second && dist > 0 ) || shortest_path.first.empty() )
		{
			path_result.first.push_front( node );
			shortest_path = make_pair( path_result.first, dist );
		}
	}

	return shortest_path;
}

DropZealotsManager::DropZealotsManager(Arbitrator::Arbitrator<Unit*,double> *arbitrator)
:arbitrator(arbitrator)
{
	myStartLocation = BWTA::getStartLocation( Broodwar->self() );
	set<BWTA::BaseLocation*> startLocations = BWTA::getStartLocations();
	startLocations.erase( myStartLocation );

	for(set<BWTA::BaseLocation*>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if( myStartLocation->getGroundDistance( *l ) <= 0 )
			startLocations.erase( *l );

	list<BWTA::BaseLocation*> path = bestDrop( startLocations ).first;
	for( list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p )
		baseLocationsToScout.push_back( *p );

	baseLocationsExplored.insert(myStartLocation);

	scout = NULL;
	shuttle = NULL;

	gate = NULL;
	robo = NULL;

	mineralStock = 0;
	gasStock = 0;

	hasBuildGate = false;
	hasBuildCyber = false;
	hasBuildRobo = false;
	hasBuildGas = false;

	askForGate = false;
	askForCyber = false;
	askForRobo = false;
	askForGas = false;

	hasExpanded = false;
}

DropZealotsManager::DropZealotsManager(Arbitrator::Arbitrator<Unit*,double> *arbitrator, UnitStates *unitStates)
:arbitrator(arbitrator), unitStates(unitStates)
{
	myStartLocation = BWTA::getStartLocation( Broodwar->self() );
	set<BWTA::BaseLocation*> startLocations = BWTA::getStartLocations();
	startLocations.erase( myStartLocation );

	for(set<BWTA::BaseLocation*>::iterator l = startLocations.begin(); l != startLocations.end(); ++l)
		if( myStartLocation->getGroundDistance( *l ) <= 0 )
			startLocations.erase( *l );

	list<BWTA::BaseLocation*> path = bestDrop( startLocations ).first;
	for( list<BWTA::BaseLocation*>::iterator p = path.begin(); p != path.end(); ++p )
		baseLocationsToScout.push_back( *p );

	baseLocationsExplored.insert(myStartLocation);

	scout = NULL;
	shuttle = NULL;

	gate = NULL;
	robo = NULL;

	mineralStock = 0;
	gasStock = 0;

	hasBuildGate = false;
	hasBuildCyber = false;
	hasBuildRobo = false;
	hasBuildGas = false;

	askForGate = false;
	askForCyber = false;
	askForRobo = false;
	askForGas = false;

	hasExpanded = false;
}

DropZealotsManager::~DropZealotsManager() {}

void DropZealotsManager::onOffer(std::set<Unit*> units)
{
	for each (Unit *u in units)
	{
		if ( scout == NULL && u->getType().isWorker() )
		{
			workerManager->removeBid( u );
			arbitrator->accept(this, u);
			scout = u;
		}
		else if( u->getType() == BWAPI::UnitTypes::Protoss_Shuttle && shuttle == NULL )
		{
			shuttle = u;
			arbitrator->accept(this, u);
		}
		else if( u->getType() == BWAPI::UnitTypes::Protoss_Zealot && zealots.size() < 4 )
		{
			zealots.insert( u );
			arbitrator->accept(this, u);
		}
		else
			arbitrator->decline(this, u, 0);
	}
}

void DropZealotsManager::onRevoke(Unit *unit, double bid)
{
	this->onRemoveUnit(unit);
}

void DropZealotsManager::onRemoveUnit(Unit *unit)
{
	if( unit == shuttle)
		shuttle = NULL;
	if( zealots.find( unit ) != zealots.end() )
		zealots.erase( unit );
}

std::string DropZealotsManager::getName() const
{
	return "Drop Zealots Manager";
}

std::string DropZealotsManager::getShortName() const
{
	return "Drop";
}

void DropZealotsManager::setInformationManager( InformationManager *informationManager )
{
	this->informationManager = informationManager;
}

void DropZealotsManager::setBaseManager( BaseManager *baseManager )
{
	this->baseManager = baseManager;
}

void DropZealotsManager::setBuildOrderManager( BuildOrderManager *buildOrderManager )
{
	this->buildOrderManager = buildOrderManager;
	buildingPlacer = buildOrderManager->getBuildManager()->getBuildingPlacer();
}

void DropZealotsManager::setWorkerManager( WorkerManager *workerManager )
{
	this->workerManager = workerManager;
}

void DropZealotsManager::setMoodManager( MoodManager *moodManager )
{
	this->moodManager = moodManager;
}

void DropZealotsManager::updateScoutAssignments()
{
	if( baseLocationsToScout.size() > 1 ) // are there still positions to scout?
	{
		map<double, BWTA::BaseLocation*> distanceMap;
		for each (BWTA::BaseLocation *p in baseLocationsToScout)
		{
			double distance = scout->getDistance( p->getPosition() );
			distanceMap[distance] = p;
		}

		target = distanceMap.begin()->second;
		scout->move( target->getPosition() );
	}
	else
	{
		informationManager->setEnemyStartLocation( *baseLocationsToScout.begin() );
		baseLocationsToScout.clear();
		scout->move( baseManager->getLastTakenBaseLocation()->getPosition() );
		arbitrator->removeBid( this, scout );
		return;
	}

	if ( target != NULL && scout->getDistance( target->getPosition() ) < scout->getType().sightRange() )
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

		if( empty )
		{
			informationManager->setBaseEmpty( target );
			baseLocationsToScout.remove( target );
			baseLocationsExplored.insert( target );

			if( baseLocationsToScout.size() == 1 )
			{
				informationManager->setEnemyStartLocation( *baseLocationsToScout.begin() );
				baseLocationsToScout.clear();
				scout->move( baseManager->getLastTakenBaseLocation()->getPosition() );
				arbitrator->removeBid( this, scout );
				return;
			}
		}
		else
		{
			baseLocationsToScout.clear();
			baseLocationsExplored.insert( target );
			informationManager->setEnemyStartLocation( target );
			scout->move( baseManager->getLastTakenBaseLocation()->getPosition() );
			arbitrator->removeBid( this, scout );
			return;
		}
	}
}

void DropZealotsManager::update()
{
	// Set bid on scout
	if( scout == NULL && BWAPI::Broodwar->self()->allUnitCount(UnitTypes::Protoss_Pylon) == 1 )
	{
		UnitGroup workers = SelectAll()(isCompleted)(isWorker);
		if( !workers.empty() )
		{
			arbitrator->setBid(this, ( *workers.rbegin() ), 60);
		}
	}

	// Set bid on zealots and shuttle
	if( shuttle == NULL && 
		Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Zealot ) >= 4 && 
		Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Shuttle ) >= 1 )
	{
		UnitGroup group = SelectAll()(isCompleted).not(isBuilding, isWorker);
		for each( BWAPI::Unit *u in group )
			arbitrator->setBid(this, u, 100);
	}

	// Scout if we don't know where the opponent is
	if( !informationManager->isEnemySpotted() && scout != NULL )
		updateScoutAssignments();

	// Build a gateway and save money for it
	if( !askForGate && 
		Broodwar->self()->minerals() >= 130 && 
		Broodwar->self()->allUnitCount( UnitTypes::Protoss_Gateway ) < 1 )
	{
		buildOrderManager->build( 1, UnitTypes::Protoss_Gateway, 90 );
		mineralStock += 150;
		askForGate = true;
	}

	// release money when the gateway is under construction
	if( !hasBuildGate && Broodwar->self()->allUnitCount( UnitTypes::Protoss_Gateway ) == 1 )
	{
		hasBuildGate = true;
		mineralStock -= 150;
	}

	// Build an assimilator and save money for it
	if( !askForGas && 
		Broodwar->self()->minerals() >= 80 + mineralStock && 
		//Broodwar->self()->allUnitCount( UnitTypes::Protoss_Gateway ) == 1 &&
		Broodwar->getFrameCount() >= 2800 &&
		Broodwar->self()->allUnitCount( UnitTypes::Protoss_Assimilator ) < 1 )
	{
		buildOrderManager->build( 1, UnitTypes::Protoss_Assimilator, 90 );
		mineralStock += 100;
		askForGas = true;
	}

	// release money when the assimilator is under construction
	if( !hasBuildGas && Broodwar->self()->allUnitCount( UnitTypes::Protoss_Assimilator ) == 1 )
	{
		hasBuildGas = true;
		mineralStock -= 100;
	}

	// train 4 zealots if we have money and a free gateway
	if( gate == NULL )
	{
		UnitGroup myGateway = SelectAll(UnitTypes::Protoss_Gateway)(isCompleted);
		if( !myGateway.empty() )
			gate = ( *myGateway.begin() );
	}
	else if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Zealot ) < 4 && Broodwar->self()->minerals() >= 100 + mineralStock )
		if( !gate->isTraining() )
			gate->train( UnitTypes::Protoss_Zealot );

	// Build a cybernetics core and save money for it
	if( !askForCyber &&
		Broodwar->self()->allUnitCount( UnitTypes::Protoss_Assimilator ) == 1 &&
		Broodwar->self()->allUnitCount( UnitTypes::Protoss_Cybernetics_Core ) < 1 )
	{
		buildOrderManager->build( 1, UnitTypes::Protoss_Cybernetics_Core, 90 );
		mineralStock += 200;
		askForCyber = true;
	}

	// release money when the cybernetics core is under construction
	if( !hasBuildCyber && Broodwar->self()->allUnitCount( UnitTypes::Protoss_Cybernetics_Core ) == 1 )
	{
		hasBuildCyber = true;
		mineralStock -=200;
	}

	// Build a robotic facility and save money for it
	if( Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Cybernetics_Core ) == 1 ) 
	{
		if( !askForRobo && 
			Broodwar->self()->allUnitCount( UnitTypes::Protoss_Robotics_Facility ) < 1 )
		{
			buildOrderManager->build( 1, UnitTypes::Protoss_Robotics_Facility, 90 );
			mineralStock += 200;
			askForRobo = true;
		}
		else if( robo == NULL )
		{		
			UnitGroup myRobo = SelectAll(UnitTypes::Protoss_Robotics_Facility)(isCompleted);
			if( !myRobo.empty() )
				robo = ( *myRobo.begin() );
		}
		else if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Shuttle ) < 1 && Broodwar->self()->minerals() >= 200 + mineralStock )
			if( !robo->isTraining() )
				robo->train( UnitTypes::Protoss_Shuttle );
	}

	// release money when the robotic facility is under construction
	if( !hasBuildRobo && Broodwar->self()->allUnitCount( UnitTypes::Protoss_Robotics_Facility ) == 1 )
	{
		hasBuildRobo = true;
		mineralStock -=200;
	}

	// if we have a shuttle (at least in training) and money, build gateways and train units
	if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Shuttle ) == 1 )
	{
		if( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Gateway ) < 2 && 
			( 
				( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Nexus ) >= 2 && Broodwar->self()->minerals() >= 150 ) || 
				( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Nexus ) < 2 && Broodwar->self()->minerals() >= 550 ) 
			) 
		  )
		{
			buildOrderManager->build( Broodwar->self()->allUnitCount( UnitTypes::Protoss_Gateway ) + 1, UnitTypes::Protoss_Gateway, 90 );
		}

		// and expand!
		if( !hasExpanded )
		{
			baseManager->expand( 100 );
			hasExpanded = true;
		}

		UnitGroup myGateways = SelectAll(UnitTypes::Protoss_Gateway)(isCompleted).not(isTraining);
		for each( BWAPI::Unit* u in myGateways )
		{
			if( Broodwar->self()->minerals() >= 125 && Broodwar->self()->gas() >= 50 )
				u->train( BWAPI::UnitTypes::Protoss_Dragoon );
			else if( Broodwar->self()->minerals() >= 100 )
				u->train( BWAPI::UnitTypes::Protoss_Zealot );
		}		
	}

	// load zealots into the shuttle, and finish this manager
	if( shuttle != NULL )
	{
		if( UnitGroup::getUnitGroup( zealots )(isLoaded).size() != 4 )
		{
			for each( BWAPI::Unit *u in zealots )
				if( !u->isLoaded() && u->getOrderTarget() != shuttle )
					u->rightClick( shuttle );
		}
		else
		{
			unitStates->setState( shuttle, UnitStates::Dropping);
			arbitrator->removeBid( this, shuttle );
			for each( BWAPI::Unit *u in zealots )
			{
				unitStates->setState( u, UnitStates::Dropping);
				arbitrator->removeBid( this, u );
			}
			moodManager->setMood( MoodManager::MoodData::Macro );
		}								
	}
}