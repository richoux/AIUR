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
#include <UnitGroupManager.h>
#include <BWAPI.h>
#include <BWTA.h>
#include <map>
#include <limits>

class InformationManager
{
public:
	class UnitData
	{
	public:
		UnitData();
		BWAPI::Position position;
		BWAPI::UnitType type;
		BWAPI::Player	*player;
		int				lastSeenTime;
		bool			exists;
	};

	const std::vector<BWTA::BaseLocation*>&	getEnemyBases	() const;
	const std::map<BWAPI::Unit*, UnitData>& getSavedData	() const;

	static InformationManager*	create			();
	static void					destroy			();
	void						onUnitDiscover	(BWAPI::Unit*);
	void						onUnitEvade		(BWAPI::Unit*);
	void						onUnitDestroy	(BWAPI::Unit*);
	void						onUnitMorph		(BWAPI::Unit*);
	void						debugCheckpoint	();

	BWAPI::Player*					getPlayer				(BWAPI::Unit*) const;
	BWAPI::UnitType					getType					(BWAPI::Unit*) const;
	BWAPI::Position					getLastPosition			(BWAPI::Unit*) const;
	BWTA::BaseLocation*				getEnemyStartLocation	();
	BWTA::BaseLocation*				getEnemyNatural			();
	BWTA::Polygon					getApproxConvexHull		();
	std::set<BWTA::BaseLocation*>	getUnvisitedBaseLocation();

	int		getLastSeenTime			(BWAPI::Unit*) const;
	bool	exists					(BWAPI::Unit*) const;
	bool	enemyHasBuilt			(BWAPI::UnitType) const;
	int		getBuildTime			(BWAPI::UnitType) const;
	void	setBaseEmpty			(BWTA::BaseLocation*);
	void	setEnemyStartLocation	(BWTA::BaseLocation*);
	void	setEnemyNatural			(BWTA::BaseLocation*);
	void	computeEnemyNatural		();
	int		getNumberBaseCenters	();
	int		getNumberEnemyBases		();
	int		getNumberEnemyBuildings	();
	int		getNumberEvadedUnits	(BWAPI::UnitType);
	bool	isEnemySpotted			();
	void	refreshEnemyBases		();

	bool	getEnemyBaseDestroyed	();
	void	setEnemyBaseDestroyed	(bool);

	int		getNumberPatches		();

	void	setNumberEnemyWorkers	(int);
	int		getNumberEnemyWorkers	();

	bool					canSeeEnemyPatches		();
	bool					hasSeenEnemyPatches		();
	void					setCanSeeEnemyPatches	(bool);
	void					setEnemyPatches			(std::set<BWAPI::Unit*>);
	std::set<BWAPI::Unit*>	getEnemyPatches			();
	std::set<BWAPI::Unit*>	getEnemyBuildings		();
	unsigned int			countEnemyBuildings		(BWAPI::UnitType);

	static std::set<BWAPI::TilePosition>	computeCircle			(BWAPI::Position, double);
	static double							computePossibleDamage	(BWAPI::TilePosition, BWAPI::Unit*);
private:
	InformationManager();
	~InformationManager();

	void updateBuildTime(BWAPI::UnitType, int);

	std::map<BWAPI::Unit*, UnitData>			savedData;
	std::map<BWAPI::UnitType, int>				buildTime;
	std::vector<BWTA::BaseLocation*>			enemyBases;
	std::map<BWTA::BaseLocation*,BWAPI::Unit*>	enemyBaseCenters;
	std::set<BWTA::BaseLocation*>				startLocationCouldContainEnemy;
	std::set<BWTA::BaseLocation*>				unvisitedBaseLocations;
	BWTA::BaseLocation							*enemyStartLocation;
	BWTA::BaseLocation							*enemyNatural;
	std::set<BWAPI::Unit*>						enemyPatches;
	std::set<BWAPI::Unit*>						enemyBuildings;

	bool	scoutedMainEnemyBase;
	bool	enemyBaseDestroyed;
	bool	canSeePatches;
	bool	hasSeenPatches;
	int		numberMineralPatches;
	int		numberEnemyWorkers;

	int		minLeft;
	int		minTop;
	int		maxRight;
	int		maxBottom;
};
extern InformationManager* TheInformationManager;