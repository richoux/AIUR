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
#include <BWAPI.h>
#include <BWTA.h>
#include <Arbitrator.h>
#include <WorkerManager.h>
#include <SupplyManager.h>
#include <BuildManager.h>
#include <BuildOrderManager.h>
#include <TechManager.h>
#include <UpgradeManager.h>
#include <BaseManager.h>
#include <ScoutManager.h>
#include <DefenseManager.h>
#include <UnderAttackManager.h>
#include <InformationManager.h>
#include <SpyManager.h>
#include <BorderManager.h>
#include <UnitGroupManager.h>
#include <AttackTracker.h>
#include <AttackUnitPlanner.h>
#include <SpendManager.h>
#include <ArmyManager.h>
#include <MoodManager.h>
#include <OpeningManager.h>
#include <PhotonManager.h>
#include <DragoonManager.h>
#include <RushPhotonManager.h>
#include <DropZealotsManager.h>
#include <EnhancedUI.h>
#include <HashMap.h>
#include <MesurePrecision.h>
#include <Random.h>
#include <UnitStates.h>
#include <fstream>

class AiurModule : public BWAPI::AIModule
{
private:
	double	minDist;
	int		frame;
	int		lastFrameExpand;
	int		lastScout;

	BWTA::BaseLocation *closest;
	BWTA::BaseLocation *home;

	int				*dataGame;
	int				*dataGameCopy;
	std::ofstream	inGame;
	std::ifstream	afterGame;
	std::ifstream	directories;
	bool			IOErrorOccurred;
	bool			hasSwitchedToDefensive;

public:
	virtual void onStart		();
	virtual void onEnd			( bool );
	virtual void onFrame		();
	virtual void onUnitDiscover	( BWAPI::Unit* );
	virtual void onUnitEvade	( BWAPI::Unit* );
	virtual void onUnitMorph	( BWAPI::Unit* );
	virtual void onUnitRenegade	( BWAPI::Unit* );
	virtual void onUnitDestroy	( BWAPI::Unit* );
	virtual void onSendText		( std::string );
	virtual void onReceiveText	( BWAPI::Player*, std::string );

	~AiurModule(); //not part of BWAPI::AIModule

	void showStats(); //not part of BWAPI::AIModule
	void showPlayers();
	void showForces();
	bool analyzed;

	std::map<BWAPI::Unit*,BWAPI::UnitType>		buildings;
	Arbitrator::Arbitrator<BWAPI::Unit*,double> arbitrator;

	WorkerManager		*workerManager;
	SupplyManager		*supplyManager;
	BuildManager		*buildManager;
	TechManager			*techManager;
	UpgradeManager		*upgradeManager;
	BaseManager			*baseManager;
	ScoutManager		*scoutManager;
	SpendManager		*spendManager;
	BuildOrderManager	*buildOrderManager;
	DefenseManager		*defenseManager;
	UnderAttackManager	*underAttackManager;
	InformationManager	*informationManager;
	BorderManager		*borderManager;
	UnitGroupManager	*unitGroupManager;
	AttackTracker		*attackTracker;
	AttackUnitPlanner	*attackUnitPlanner;
	SpyManager			*spyManager;
	ArmyManager			*armyManager;
	MoodManager			*moodManager;
	OpeningManager		*openingManager;
	PhotonManager		*photonManager;
	RushPhotonManager	*rushPhotonManager;
	DropZealotsManager	*dropZealotsManager;
	DragoonManager		*dragoonManager;
	EnhancedUI			*enhancedUI;
	Random				*randomExpand;
	UnitStates			*unitStates;
	HashMap				hashMap;
	int					timeToExpand;
	int					baseTimeToExpand;
	bool				showManagerAssignments;
	bool				debugMode;
	bool				debugAttackerMode;
	bool				debugTimersMode;
	bool				debugUI;
	bool				debugState;
	bool				debugOrder;
	bool				destinationMineralSpotted;
	bool				cleaningPath;
	bool				ggAnswered;
	int					ggFrame;

	// time profile
	double diff, top;
	int sup55;
	CMesurePrecision compteur;
	CMesurePrecision timeUI;
	CMesurePrecision timeWorker;
	CMesurePrecision timeBuild;
	CMesurePrecision timeBuildOrder;
	CMesurePrecision timeBase;
	CMesurePrecision timeUA;
	CMesurePrecision timeAT;
	CMesurePrecision timeArbitrator;
	CMesurePrecision timeRush;
	CMesurePrecision timeSupply;
	CMesurePrecision timeTech;
	CMesurePrecision timeUpgrade;
	CMesurePrecision timeMood;
	CMesurePrecision timeSpend;
	CMesurePrecision timeScout;
	CMesurePrecision timeDefense;
	CMesurePrecision timeSpy;
	CMesurePrecision timeBorder;
	CMesurePrecision timeArmy;
	CMesurePrecision timeDragoon;
	CMesurePrecision timePhoton;
	CMesurePrecision timeDrop;

	double diffUI;
	double diffWorker;
	double diffBuild;
	double diffBuildOrder;
	double diffBase;
	double diffUA;
	double diffAT;
	double diffArbitrator;
	double diffRush;
	double diffSupply;
	double diffTech;
	double diffUpgrade;
	double diffMood;
	double diffSpend;
	double diffScout;
	double diffDefense;
	double diffSpy;
	double diffBorder;
	double diffArmy;
	double diffDragoon;
	double diffPhoton;
	double diffDrop;
};