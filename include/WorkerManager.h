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
#include <BaseManager.h>
#include <HashMap.h>
#include <MoodManager.h>

class BuildOrderManager;
class WorkerManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class WorkerData
	{
	public:
		WorkerData() {resource = NULL; lastFrameSpam = 0;}
		BWAPI::Unit* resource;
		BWAPI::Unit* newResource;
		int lastFrameSpam;
	};

	WorkerManager();
	WorkerManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator);

	void				setBaseManager		( BaseManager* );
	void				setBuildOrderManager( BuildOrderManager* );
	void				setMoodManager		( MoodManager* );
	virtual void		onOffer				(std::set<BWAPI::Unit*> units);
	virtual void		onRevoke			(BWAPI::Unit* unit, double bid);
	virtual void		update				();
	virtual std::string getName				() const;
	virtual std::string getShortName		() const;

	void	onRemoveUnit			(BWAPI::Unit* unit);
	void	setWorkersPerGas		(int count);
	double	getMineralRate			() const;
	double	getGasRate				() const;
	int		getOptimalWorkerCount	() const;
	bool	isAutoBuild				();
	void	enableAutoBuild			();
	void	disableAutoBuild		();
	void	setAutoBuildPriority	(int priority);
	void	setDestinationMineral	(BWAPI::Unit*);
	void	removeBid				(BWAPI::Unit*);

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double>		*arbitrator;
	BaseManager										*baseManager;
	BuildOrderManager								*buildOrderManager;
	MoodManager										*moodManager;
	std::map<BWAPI::Unit*,WorkerData>				workers;
	std::map<BWAPI::Unit*, std::set<BWAPI::Unit*> > currentWorkers;
	std::map<BWAPI::Unit*, Base*>					resourceBase;
	std::map<BWAPI::Unit*, int>						desiredWorkerCount;
	std::vector<std::pair<BWAPI::Unit*, int> >		mineralOrder;
	BWAPI::Unit										*destinationMineral;
	BWAPI::Unit										*cleaner;
	int												mineralOrderIndex;
	int												lastSCVBalance;
	std::set<Base*>									basesCache;
	int												WorkersPerGas;
	HashMap											hashMap;

	void rebalanceWorkers();
	void updateWorkerAssignments();

	double	mineralRate;
	double	gasRate;
	bool	autoBuild;
	int		autoBuildPriority;
	int		optimalWorkerCount;
	bool	cleaningInProgress;
};