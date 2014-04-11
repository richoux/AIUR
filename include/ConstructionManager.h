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
#include <BWAPI.h>
#include <BuildingPlacer.h>
class ConstructionManager : public Arbitrator::Controller<BWAPI::Unit*,double>
{
public:
	class Building
	{
	public:
		BWAPI::TilePosition goalPosition;
		BWAPI::TilePosition tilePosition;
		BWAPI::Position		position;
		BWAPI::UnitType		type;
		BWAPI::Unit*		buildingUnit;
		BWAPI::Unit*		builderUnit;
		int					lastOrderFrame;
		bool				started;
	};
	
	ConstructionManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, BuildingPlacer* placer);

	virtual void		onOffer(std::set<BWAPI::Unit*> units);
	virtual void		onRevoke(BWAPI::Unit* unit, double bid);
	virtual void		update();
	virtual std::string getName() const;
	virtual std::string getShortName() const;

	void	onRemoveUnit			(BWAPI::Unit* unit);
	bool	build					(BWAPI::UnitType type, BWAPI::TilePosition goalPosition, bool forcePosition);
	int		getPlannedCount			(BWAPI::UnitType type) const;
	int		getStartedCount			(BWAPI::UnitType type) const;
	int		cancelAllBuildingsOfType(BWAPI::UnitType);
	void	setDebugMode			(bool);

private:
	Arbitrator::Arbitrator<BWAPI::Unit*,double>		*arbitrator;
	BuildingPlacer									*placer;
	std::map<BWAPI::Unit*,Building*>				builders;
	std::list<Building>								incompleteBuildings;
	std::map<BWAPI::UnitType,std::set<Building*> >	buildingsNeedingBuilders;
	std::map<BWAPI::UnitType, int>					plannedCount;
	std::map<BWAPI::UnitType, int>					startedCount;

	bool debugMode;
};