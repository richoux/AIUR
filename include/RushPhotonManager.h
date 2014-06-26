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

#pragma once
#include <Arbitrator.h>
#include <BuildOrderManager.h>
#include <BuildManager.h>
#include <BuildingPlacer.h>
#include <InformationManager.h>
#include <UnitGroupManager.h>
#include <WorkerManager.h>
#include <UnitStates.h>
#include <BWAPI.h>

using namespace BWAPI;
using namespace std;

class RushPhotonManager : Arbitrator::Controller<Unit*,double>
{
public:
	RushPhotonManager(Arbitrator::Arbitrator<Unit*,double>*);
	RushPhotonManager(Arbitrator::Arbitrator<Unit*,double>*, UnitStates*);
	~RushPhotonManager();

	virtual void		onOffer			(set<Unit*>);
	virtual void		onRevoke		(Unit*, double);
	void				onRemoveUnit	(Unit*);
	virtual void		update			();
	virtual	std::string getName			() const;
	virtual	std::string getShortName	() const;

	void setBuildOrderManager	( BuildOrderManager* );
	void setInformationManager	( InformationManager* );
	void setWorkerManager		( WorkerManager* );
	void setLastPylon			( Unit* );
	void setDebugMode			( bool );
	void loseControl			();
	bool isCheesing				( Unit* );

private:
	void		updateScoutAssignments	();
	Position	computeStartingPosition	( BWTA::BaseLocation* );
	bool		determineIfGoForward	( BWTA::BaseLocation*, Position, Position, int& );
	Position	nextsneakyPosition		();


	Arbitrator::Arbitrator<Unit*,double>		*arbitrator;
	BuildOrderManager							*buildOrderManager;
	BuildingPlacer								*buildingPlacer;
	InformationManager							*informationManager;
	WorkerManager								*workerManager;
	UnitStates									*unitStates;

	Unit										*rusher;
	Unit										*lastPylon;
	list<BWTA::BaseLocation*>					baseLocationsToScout;
	set<BWTA::BaseLocation*>					baseLocationsExplored;
	BWTA::BaseLocation							*myStartLocation;
	BWTA::BaseLocation							*target;
	BWTA::Polygon								poly;
	Position									startingPosition;
	Position									sneakyPosition;
	int											sneakyIndex;
	bool										goForward;
	bool										scoutDone;
	bool										firstPylonDone;
	bool										hasLostControl;
	bool										debugMode;
};