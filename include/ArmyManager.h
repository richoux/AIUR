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

#pragma once
#include <Arbitrator.h>
#include <DefenseManager.h>
#include <InformationManager.h>
#include <UnitGroupManager.h>
#include <BaseManager.h>
#include <MoodManager.h>
#include <DragoonManager.h>
#include <AttackUnitPlanner.h>
#include <BWAPI.h>
#include <BWTA.h>
#include <UnitStates.h>

class ArmyManager : Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class ArmyData
	{
	public:
		inline ArmyData(): target(NULL), lastFrameDragoonAttack(0){}
		BWTA::BaseLocation* target;
		int lastFrameDragoonAttack;
	};

	ArmyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	ArmyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>*, UnitStates*);
	~ArmyManager();
	void setDefenseManager		(DefenseManager*);
	void setInformationManager	(InformationManager*);
	void setBaseManager			(BaseManager*);
	void setMoodManager			(MoodManager*);
	void setDragoonManager		(DragoonManager*);
	void setAttackUnitPlanner	(AttackUnitPlanner*);

	void setLastExpandFrame		(int);

	virtual void		onOffer			(std::set<BWAPI::Unit*> unitGroup);
	virtual void		onRevoke		(BWAPI::Unit *p, double bid);
	void				onRemoveUnit	(BWAPI::Unit *unit);
	virtual void		update			();
	virtual	std::string getName			() const;
	virtual	std::string getShortName	() const;

	double			enemyDPS			();
	double			enemyDPStoAir		();
	double			enemyDPStoGround	();
	double			enemyDPSfromAir		();
	double			enemyDPSfromGround	();
	double			myDPS				();
	double			myDPStoAir			();
	double			myDPStoGround		();

	static double	thisGroupDPS		(std::set<BWAPI::Unit*>);
	double			myDPS				(std::map<BWAPI::Unit*, ArmyData>);
	int				enemyHP				();
	int				myHP				(std::map<BWAPI::Unit*, ArmyData>);

	static std::set<BWAPI::Unit*>	whoIsAttacking	(BWAPI::Unit*);
	static BWAPI::Unit*				whoIsTheWeakest	(std::set<BWAPI::Unit*>);
	static bool						containsBuilding(std::set<BWAPI::Unit*>);
	static BWAPI::Unit*				nearestUnit		(BWAPI::Unit*, std::set<BWAPI::Unit*>);

	bool	isInitialized		();
	void	initialize			();
	int		getStartAttack		();
	bool	getFirstAttackDone	();

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator;
	DefenseManager								*defenseManager;
	InformationManager							*informationManager;
	BaseManager									*baseManager;
	MoodManager									*moodManager;
	DragoonManager								*dragoonManager;
	AttackUnitPlanner							*attackUnitPlanner;
	UnitStates									*unitStates;

	Random	*randomEarlyAttack;
	Random	*randomSizeRush;

	std::map<BWAPI::Unit*, ArmyData>	company;
	std::set<BWAPI::Unit*>				fleeing;
	std::vector<BWAPI::Position>		enemyBuildings;
	std::vector<BWTA::BaseLocation*>	unvisitedBases;

	double	sizeRush;
	int		lastFrameCheck;
	int		lastExpandFrame;
	int		lastAttack;
	bool	firstAttackDone;
	bool	initialized;
	int		startAttack;
	int		startAttackInitial;
	int		round;
	int		roundUnvisited;
	double	maxEnemyDPS;
	double	maxEnemyDPStoAir;
	double	maxEnemyDPStoGround;
	double	maxEnemyDPSfromAir;
	double	maxEnemyDPSfromGround;

	BWAPI::Position *dropPosition;
	bool unloadOrder;
};