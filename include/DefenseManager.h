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
#include <BWAPI.h>
#include <BWTA.h>
#include <BaseManager.h>
#include <InformationManager.h>
#include <BuildOrderManager.h>
#include <UnitGroupManager.h>
#include <SpyManager.h>
#include <AttackUnitPlanner.h>
#include <DragoonManager.h>

class BorderManager;
class DefenseManager : Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class DefenseData
	{
	public:
		enum DefenseMode
		{
			Idle,
			Moving,
			Defending
		};
		DefenseData(){ mode = Idle; }
		DefenseMode mode;
		BWTA::Region *regionOrigin;
		bool hasReachRegion;
	};
	DefenseManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);

	void setBorderManager		(BorderManager *borderManager);
	void setBaseManager			(BaseManager *baseManager);
	void setInformationManager	(InformationManager *informationManager);
	void setBuildOrderManager	(BuildOrderManager *buildOrderManager);
	void setSpyManager			(SpyManager *spyManager);
	void setDragoonManager		(DragoonManager *dragoonManager);
	void setAttackUnitPlanner	(AttackUnitPlanner*);

	std::set<BWAPI::Unit*>& getIdleDefenders	();
	std::set<BWAPI::Unit*>& getDefenders		();
	virtual void			onOffer				(std::set<BWAPI::Unit*> units);
	virtual void			onRevoke			(BWAPI::Unit* unit, double bid);
	void					onRemoveUnit		(BWAPI::Unit* unit);
	virtual void			update				();
	virtual std::string		getName				() const;
	virtual std::string		getShortName		() const;
	bool					isDefending			();

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double>	*arbitrator;
	BorderManager		*borderManager;
	BuildOrderManager	*buildOrderManager;
	BaseManager			*baseManager; 
	InformationManager	*informationManager;
	SpyManager			*spyManager;
	DragoonManager		*dragoonManager;
	AttackUnitPlanner	*attackUnitPlanner;

	std::set<BWTA::Chokepoint*>			myBorder;
	std::vector<BWTA::Chokepoint*>		myBorderVector;
	std::map<BWAPI::Unit*,DefenseData>	defenders;
	std::set<BWAPI::Unit*>				idleDefenders;
	std::set<BWAPI::Unit*>				allDefenders;
	bool								hasObserver;
	bool								hasWorker;
	int									lastFrameCheck;
	int									round;
};