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
#include <InformationManager.h>
#include <UnitGroupManager.h>
#include <Random.h>

class MoodManager
{
public:
	class MoodData
	{
	public:
		enum Mood
		{
			Cheese,
			Rush,
			Agressive,
			FastExpo,
			Macro,
			Defensive
		};
		Mood mood;
		Mood previousMood;
		Mood firstMood;

		// Take Defensive into account
		static const int numberMoods = 6;
	};

	class Points
	{
	public:
		int					zealot;
		int					zealotLastValue;
		int					dragoon;
		int					dragoonLastValue;
		int					corsair;
		int					corsairLastValue;
		int					darkTemplar;
		int					darkTemplarLastValue;
		int					archon;
		int					archonLastValue;
	};

	MoodManager();
	~MoodManager();

	MoodData::Mood	getMood					(void);
	MoodData::Mood	getPreviousMood			(void);
	Points*			getPoints				(void);

	int				getFirstMoodRank		();
	int				getNumberMoods			();

	std::vector<double>	getDistribution	();

	int				getStartAttack			();
	bool			getFirstAttackDone		();
	bool			hasChangedMood			();

	void			setInformationManager	(InformationManager*);
	void			setMood					(MoodData::Mood);
	void			initialize				();
	void			initialize				(MoodData::Mood);
	void			initialize				(int*);
	void			initializeRoundRobin	(int*);
	void			setRandomMood			(void);
	void			setAnotherRandomMood	(void);
	void			update					(void);

private:
	void setMoodPlan();

	MoodData			moodData;
	Points				*unitPoints;
	InformationManager	*informationManager;
	Random				*randomMood;
	int					lastFrameCheck;
	int					startAttack;
	bool				restore;
	bool				firstAttackDone;
};
