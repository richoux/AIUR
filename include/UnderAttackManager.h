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
#include <ArmyManager.h>
#include <UnitGroupManager.h>
#include <UnitGroup.h>
#include <DefenseManager.h>
#include <BaseManager.h>
#include <ScoutManager.h>
#include <SpyManager.h>
#include <DragoonManager.h>
#include <MoodManager.h>
#include <RushPhotonManager.h>
#include <AttackUnitPlanner.h>
#include <BWAPI.h>
#include <UnitStates.h>

class UnderAttackManager : Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class UAData
	{
	public:
		enum UAMode
		{
			Idle,
			Worker,
			WorkerDefending,
			Defender,
			Defending
		};
		UAData()			{ mode = Idle; }
		UAData(UAMode mode) { this->mode = mode; }
		UAMode mode;
		BWTA::Region *regionOrigin;
		bool hasReachRegion;
		BWAPI::Unit *needHelp;
	};
	
	UnderAttackManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);
	UnderAttackManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>*, UnitStates*);
	
	virtual void		onOffer				(std::set<BWAPI::Unit*> unitGroup);
	virtual void		onRevoke			(BWAPI::Unit *p, double bid);
	void				onRemoveUnit		(BWAPI::Unit *unit);
	virtual void		update				();
	virtual std::string getName				() const;
	virtual std::string getShortName		() const;
	void				setArmyManager		(ArmyManager*);
	void				setDefenseManager	(DefenseManager*);
	void				setScoutManager		(ScoutManager*);
	void				setSpyManager		(SpyManager*);
	void				setDragoonManager	(DragoonManager*);
	void				setBaseManager		(BaseManager*);
	void				setMoodManager		(MoodManager*);
	void				setRushPhotonManager(RushPhotonManager*);
	void				setAttackUnitPlanner(AttackUnitPlanner*);
	bool				isUnderAttack		();

private:
	bool hasObserver ();

	Arbitrator::Arbitrator<BWAPI::Unit*,double> *arbitrator;
	ArmyManager									*armyManager;
	DefenseManager								*defenseManager;
	ScoutManager								*scoutManager;
	SpyManager									*spyManager;
	DragoonManager								*dragoonManager;
	BaseManager									*baseManager;
	MoodManager									*moodManager;
	RushPhotonManager							*rushPhotonManager;
	AttackUnitPlanner							*attackUnitPlanner;
	UnitStates									*unitStates;

	int											lastFrameCheck;
	bool										underAttack;
	int											lastFrameUnderAttack;

	//set of all units controlled by the UAM
	std::map<BWAPI::Unit*, UAData>				backUp;
};