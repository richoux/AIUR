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
#include <InformationManager.h>
#include <BuildOrderManager.h>
#include <BuildManager.h>
#include <BaseManager.h>
#include <BuildingPlacer.h>
#include <WorkerManager.h>
#include <MoodManager.h>
#include <UnitGroupManager.h>
#include <UnitStates.h>
#include <BWAPI.h>

using namespace BWAPI;
using namespace std;

class DropZealotsManager : Arbitrator::Controller<Unit*,double>
{
public:
	DropZealotsManager(Arbitrator::Arbitrator<Unit*,double>*);
	DropZealotsManager(Arbitrator::Arbitrator<Unit*,double>*, UnitStates*);
	~DropZealotsManager();

	virtual void		onOffer			(set<Unit*>);
	virtual void		onRevoke		(Unit*, double);
	void				onRemoveUnit	(Unit*);
	virtual void		update			();
	virtual	std::string getName			() const;
	virtual	std::string getShortName	() const;

	void setInformationManager	( InformationManager* );
	void setBuildOrderManager	( BuildOrderManager* );
	void setBaseManager			( BaseManager* );
	void setWorkerManager		( WorkerManager* );
	void setMoodManager			( MoodManager* );

private:
	void updateScoutAssignments	();

	Arbitrator::Arbitrator<Unit*,double>	*arbitrator;
	InformationManager						*informationManager;
	BuildOrderManager						*buildOrderManager;
	BuildingPlacer							*buildingPlacer;
	BaseManager								*baseManager;
	WorkerManager							*workerManager;
	MoodManager								*moodManager;
	UnitStates								*unitStates;

	BWAPI::Unit				*scout;
	BWAPI::Unit				*shuttle;
	std::set<BWAPI::Unit*>	zealots;

	BWAPI::Unit				*gate;
	BWAPI::Unit				*robo;


	list<BWTA::BaseLocation*>	baseLocationsToScout;
	set<BWTA::BaseLocation*>	baseLocationsExplored;
	BWTA::BaseLocation			*myStartLocation;
	BWTA::BaseLocation			*target;
	BWAPI::Position				dropPoint;

	int mineralStock;
	int gasStock;

	bool hasBuildGate;
	bool hasBuildCyber;
	bool hasBuildRobo;
	bool hasBuildGas;

	bool askForGate;
	bool askForCyber;
	bool askForRobo;
	bool askForGas;

	bool hasExpanded;
};