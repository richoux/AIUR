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
#include <BuildManager.h>
#include <BuildingPlacer.h>
#include <BuildOrderManager.h>
#include <UnitGroupManager.h>
#include <MoodManager.h>
#include <ArmyManager.h>
#include <DefenseManager.h>
#include <DragoonManager.h>
#include <UnderAttackManager.h>
#include <InformationManager.h>

class SpendManager
{
public:
	class Count
	{
	public:
		int		zealot;
		int		dragoon;
		int		corsair;
		int		darkTemplar;
		int		archon;
		int		numberUnitTypes;

		float	zealotRatio;
		float	dragoonRatio;
		float	corsairRatio;
		float	darkTemplarRatio;
		float	archonRatio;

		float	zealotIdealRatio;
		float	dragoonIdealRatio;
		float	corsairIdealRatio;
		float	darkTemplarIdealRatio;
		float	archonIdealRatio;
	};
	SpendManager();
	~SpendManager();

	void		setBuildManager			(BuildManager*);
	void		setBuildOrderManager	(BuildOrderManager*);
	void		setMoodManager			(MoodManager*);
	void		setArmyManager			(ArmyManager*);
	void		setDefenseManager		(DefenseManager*);
	void		setDragoonManager		(DragoonManager*);
	void		setUnderAttackManager	(UnderAttackManager*);
	void		setBaseManager			(BaseManager*);
	void		setInformationManager	(InformationManager*);
	void		setTakeGas				(int);
	void		update					();
	std::string getName					() const;
	std::string getShortName			() const;
	Count*		getUnitCount			();
	int			getTotalUnits			();
	int			getTotalIdealUnits		();
	int			getMineralStock			();
	int			getGasStock				();

private:
	void needMoreGateways (int);

	BuildManager		*buildManager;
	BuildOrderManager	*buildOrderManager;
	MoodManager			*moodManager;
	ArmyManager			*armyManager;
	DefenseManager		*defenseManager;
	DragoonManager		*dragoonManager;
	UnderAttackManager	*underAttackManager;
	BaseManager			*baseManager;
	InformationManager	*informationManager;
	Count				*unitCount;
	int					lastFrameCheck;
	int					weaponUpgradeLevel;
	int					armorUpgradeLevel;
	int					shieldUpgradeLevel;
	int					totalUnits;
	int					totalIdealUnits;
	int					minerals;
	int					gas;
	int					mineralStock;
	int					gasStock;
	int					takeGas;

	std::vector<BWAPI::UnitType>	vecBuildings;
};