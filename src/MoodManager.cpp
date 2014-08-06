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

#include <MoodManager.h>

MoodManager::MoodManager()
{
	unitPoints							= new Points();

	unitPoints->zealot					= 100;
	unitPoints->zealotLastValue			= 100;
	unitPoints->dragoon					= 0;
	unitPoints->dragoonLastValue		= 0;
	unitPoints->corsair					= 0;
	unitPoints->corsairLastValue		= 0;
	unitPoints->darkTemplar				= 0;
	unitPoints->darkTemplarLastValue	= 0;
	unitPoints->archon					= 0;
	unitPoints->archonLastValue			= 0;

	restore								= false;
}

MoodManager::~MoodManager()
{
	delete unitPoints;
	delete randomMood;
}

void MoodManager::initialize()
{
	randomMood				= new Random( moodData.numberMoods );
	moodData.mood			= (MoodData::Mood)randomMood->nextInt();
	moodData.previousMood	= moodData.mood;
	moodData.firstMood		= moodData.mood;
	this->setMoodPlan();
}

void MoodManager::initialize(MoodManager::MoodData::Mood mood)
{
	randomMood				= new Random( moodData.numberMoods );
	moodData.mood			= mood;
	moodData.previousMood	= mood;
	moodData.firstMood		= mood;
	this->setMoodPlan();
}

void MoodManager::initialize( int *data, double epsilon )
{
	randomMood				= new Random( data, moodData.numberMoods, epsilon );
	moodData.mood			= (MoodData::Mood)randomMood->nextInt();
	moodData.previousMood	= moodData.mood;
	moodData.firstMood		= moodData.mood;
	this->setMoodPlan();
}

void MoodManager::initializeRoundRobin( int *data, int trainingGames, double epsilon )
{
	for( int i = 1; i < moodData.numberMoods * 2 + 1 ; i += 2 )
		if( data[i] + data[i+1] < trainingGames )
			data[i] = 1;
		else
			data[i]	= 0;

	randomMood				= new Random( data, moodData.numberMoods, epsilon, true );
	moodData.mood			= (MoodData::Mood)randomMood->nextInt();
	moodData.previousMood	= moodData.mood;
	moodData.firstMood		= moodData.mood;
	this->setMoodPlan();
}

std::vector<double>	MoodManager::getDistribution()
{
	return randomMood->getDistribution();
}

int MoodManager::getNumberMoods()
{
	return moodData.numberMoods;
}

double MoodManager::getEpsilon()
{
	return randomMood->getEpsilon();
}

int MoodManager::getFirstMoodRank()
{
	return (int)moodData.firstMood;	
}

MoodManager::MoodData::Mood MoodManager::getMood()
{
	return moodData.mood;
}

MoodManager::MoodData::Mood MoodManager::getPreviousMood()
{
	return moodData.previousMood;
}

MoodManager::Points* MoodManager::getPoints()
{
	return unitPoints;
}

void MoodManager::setMood(MoodManager::MoodData::Mood mood)
{
	moodData.previousMood	= moodData.mood;
	moodData.mood			= mood;
	setMoodPlan();
}

void MoodManager::setRandomMood()
{
	moodData.mood			= (MoodData::Mood)randomMood->nextInt();
	moodData.previousMood	= moodData.mood;
	moodData.firstMood		= moodData.mood;
	setMoodPlan();
}

void MoodManager::setAnotherRandomMood()
{
	moodData.mood			= (MoodData::Mood)randomMood->nextAnotherInt();
	moodData.previousMood	= moodData.mood;
	moodData.firstMood		= moodData.mood;
	setMoodPlan();
}

void MoodManager::setInformationManager(InformationManager *informationManager)
{
	this->informationManager = informationManager;
}

int MoodManager::getStartAttack()
{
	return startAttack;
}

bool MoodManager::getFirstAttackDone()
{
	return firstAttackDone;
}

bool MoodManager::hasChangedMood()
{
	return moodData.mood != moodData.previousMood;
}

void MoodManager::setMoodPlan()
{
	switch(moodData.mood)
	{
	case MoodManager::MoodData::Cheese:
		firstAttackDone = true;
		startAttack = 999;
		break;
	case MoodManager::MoodData::Rush:
		//if( BWAPI::Broodwar->getFrameCount() < 6500 )
		//	firstAttackDone = false;
		//else
			firstAttackDone = false;
		startAttack = 5;
		break;
	case MoodManager::MoodData::Agressive:
		firstAttackDone = true;
		startAttack = 999;
		break;
	case MoodManager::MoodData::FastExpo:
		firstAttackDone = true;
		startAttack = 16;
		break;
	case MoodManager::MoodData::Macro:
		firstAttackDone = true;
		startAttack = 16;
		break;			
	case MoodManager::MoodData::Defensive:
		firstAttackDone = true;
		startAttack = 22;
		break;
	}
}

void MoodManager::update()
{
	if( moodData.mood != MoodManager::MoodData::Agressive &&
		moodData.mood != MoodManager::MoodData::Cheese && 
		( lastFrameCheck == 0 || ( BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 72 ) ) )
	{
		if( moodData.mood == MoodManager::MoodData::Defensive && BWAPI::Broodwar->getFrameCount() < 13000 )
		{
			if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
			{
				unitPoints->dragoon = 20;
			}
		}
		else
		{
			lastFrameCheck = BWAPI::Broodwar->getFrameCount();
			if ((BWAPI::Broodwar->getFrameCount() > 5200) && (unitPoints->dragoon == 0))
				unitPoints->dragoon = 50;

			if ((BWAPI::Broodwar->getFrameCount() > 8500) && (unitPoints->corsair == 0) && (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg))
				unitPoints->corsair = 50;

			if ((BWAPI::Broodwar->getFrameCount() > 10000) && (unitPoints->darkTemplar == 0))
			{
				if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
					unitPoints->darkTemplar = 20;
				else	
					unitPoints->darkTemplar = 50;
			}

			if ((BWAPI::Broodwar->getFrameCount() > 11000) && (unitPoints->archon == 0))
				unitPoints->archon = 50;

			int minerals	= BWAPI::Broodwar->self()->minerals();
			int gas			= BWAPI::Broodwar->self()->gas();

			// If we have too much minerals regarding gas, priority to zealots
			if (minerals > gas + 300 && !restore)
			{
				unitPoints->zealotLastValue = unitPoints->zealot;
				unitPoints->zealot = 200;
				restore = true;
				if (unitPoints->dragoon != 50)
				{
					unitPoints->dragoonLastValue = unitPoints->dragoon;
					unitPoints->dragoon = 50;
				}
			}

			// If we have too much gas regarding minerals, priority to dragoons
			if (gas > minerals + 300 && !restore)
			{
				unitPoints->dragoonLastValue = unitPoints->dragoon;
				unitPoints->dragoon = 100;
				restore = true;
				if (unitPoints->zealot != 100)
				{
					unitPoints->zealotLastValue = unitPoints->zealot;
					unitPoints->zealot = 100;
				}
			}

			// else, restore
			if (minerals <= gas + 300 && gas <= minerals + 300 && restore)
			{
				unitPoints->zealot	= unitPoints->zealotLastValue;
				unitPoints->dragoon = unitPoints->dragoonLastValue;
				restore = false;
			}

			if (unitPoints->corsair < 30 &&
				((informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Starport))		||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Spire))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Greater_Spire))	||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Stargate))))
			{
				unitPoints->corsairLastValue	= unitPoints->corsair;
				unitPoints->corsair				= 30;
			}

			if (unitPoints->corsair < 60 &&
				((informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Wraith))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Valkyrie))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Battlecruiser))	||

				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Corsair))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Scout))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Carrier))			||

				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Guardian))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Queen))))
			{
				unitPoints->corsairLastValue	= unitPoints->corsair;
				unitPoints->corsair				= 60;
			}

			if (unitPoints->corsair < 70 &&
				((informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Mutalisk)) ||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Devourer))))
			{
				unitPoints->corsairLastValue	= unitPoints->corsair;
				unitPoints->corsair				= 70;
			}

			if ((informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Wraith))			||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Terran_Battlecruiser))	||

				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Scout))		||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Protoss_Carrier))		||

				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Mutalisk))		||
				(informationManager->enemyHasBuilt(BWAPI::UnitTypes::Zerg_Devourer)))
			{
				int nbWraith	=	SelectAllEnemy(BWAPI::UnitTypes::Terran_Wraith)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Terran_Wraith);

				int nbBC		=	SelectAllEnemy(BWAPI::UnitTypes::Terran_Battlecruiser)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Terran_Battlecruiser);

				int nbScout		=	SelectAllEnemy(BWAPI::UnitTypes::Protoss_Scout)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Protoss_Scout);

				int nbCarrier	=	SelectAllEnemy(BWAPI::UnitTypes::Protoss_Carrier)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Protoss_Carrier);

				int nbMuta		=	SelectAllEnemy(BWAPI::UnitTypes::Zerg_Mutalisk)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Zerg_Mutalisk);

				int nbDevo		=	SelectAllEnemy(BWAPI::UnitTypes::Zerg_Devourer)(exists).size() + 
					informationManager->getNumberEvadedUnits(BWAPI::UnitTypes::Zerg_Devourer);

				int myCorsairs = SelectAll(BWAPI::UnitTypes::Protoss_Corsair)(exists).size();

				if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Terran)
				{
					if (myCorsairs < nbWraith + nbBC)
					{
						unitPoints->corsairLastValue		= unitPoints->corsair;
						unitPoints->corsair					= 100;

						unitPoints->dragoonLastValue		= unitPoints->dragoon;
						unitPoints->dragoon					= 100;

						unitPoints->zealotLastValue			= unitPoints->zealot;
						unitPoints->zealot					= 50;

						unitPoints->darkTemplarLastValue	= unitPoints->darkTemplar;
						unitPoints->darkTemplar				= 20;
					}
					else if (unitPoints->corsair == 100)
					{
						unitPoints->corsair		= unitPoints->corsairLastValue;
						unitPoints->dragoon		= unitPoints->dragoonLastValue;
						unitPoints->zealot		= unitPoints->zealotLastValue;
						unitPoints->darkTemplar = unitPoints->darkTemplarLastValue;
					}
				}

				if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Protoss)
				{
					if (myCorsairs < nbScout + nbCarrier)
					{
						unitPoints->corsairLastValue		= unitPoints->corsair;
						unitPoints->corsair					= 100;

						unitPoints->dragoonLastValue		= unitPoints->dragoon;
						unitPoints->dragoon					= 100;

						unitPoints->zealotLastValue			= unitPoints->zealot;
						unitPoints->zealot					= 50;

						unitPoints->darkTemplarLastValue	= unitPoints->darkTemplar;
						unitPoints->darkTemplar				= 20;
					}
					else if (unitPoints->corsair == 100)
					{
						unitPoints->corsair		= unitPoints->corsairLastValue;
						unitPoints->dragoon		= unitPoints->dragoonLastValue;
						unitPoints->zealot		= unitPoints->zealotLastValue;
						unitPoints->darkTemplar = unitPoints->darkTemplarLastValue;
					}
				}

				if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg)
				{
					if (myCorsairs < nbMuta + nbDevo)
					{
						unitPoints->corsairLastValue		= unitPoints->corsair;
						unitPoints->corsair					= 100;

						unitPoints->dragoonLastValue		= unitPoints->dragoon;
						unitPoints->dragoon					= 100;

						unitPoints->zealotLastValue			= unitPoints->zealot;
						unitPoints->zealot					= 50;

						unitPoints->darkTemplarLastValue	= unitPoints->darkTemplar;
						unitPoints->darkTemplar				= 20;
					}
					else if (unitPoints->corsair == 100)
					{
						unitPoints->corsair		= unitPoints->corsairLastValue;
						unitPoints->dragoon		= unitPoints->dragoonLastValue;
						unitPoints->zealot		= unitPoints->zealotLastValue;
						unitPoints->darkTemplar = unitPoints->darkTemplarLastValue;
					}
				}
			}
		}
	}
}