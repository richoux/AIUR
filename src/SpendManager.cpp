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

#include <SpendManager.h>

#define TAKEGAS 5000

using namespace BWAPI;


SpendManager::SpendManager()
{
	buildOrderManager					= NULL;
	lastFrameCheck						= 0;
	weaponUpgradeLevel					= 0;
	armorUpgradeLevel					= 0;
	shieldUpgradeLevel					= 0;

	unitCount							= new Count();

	unitCount->zealot					= 0;
	unitCount->dragoon					= 0;
	unitCount->corsair					= 0;
	unitCount->darkTemplar				= 0;
	unitCount->archon					= 0;
	unitCount->numberUnitTypes			= 4;

	unitCount->zealotRatio				= 0.;
	unitCount->dragoonRatio				= 0.;
	unitCount->corsairRatio				= 0.;
	unitCount->darkTemplarRatio			= 0.;
	unitCount->archonRatio				= 0.;

	unitCount->zealotIdealRatio			= 0.;
	unitCount->dragoonIdealRatio		= 0.;
	unitCount->corsairIdealRatio		= 0.;
	unitCount->darkTemplarIdealRatio	= 0.;
	unitCount->archonIdealRatio			= 0.;

	totalUnits							= 0;
	totalIdealUnits						= 0;

	minerals							= 0;
	gas									= 0;
	mineralStock						= 0;
	gasStock							= 0;

	vecBuildings.push_back(UnitTypes::Protoss_Nexus);
	vecBuildings.push_back(UnitTypes::Protoss_Pylon);
	vecBuildings.push_back(UnitTypes::Protoss_Assimilator);
	vecBuildings.push_back(UnitTypes::Protoss_Gateway);
	vecBuildings.push_back(UnitTypes::Protoss_Forge);
	vecBuildings.push_back(UnitTypes::Protoss_Cybernetics_Core);
	vecBuildings.push_back(UnitTypes::Protoss_Photon_Cannon);
	vecBuildings.push_back(UnitTypes::Protoss_Robotics_Facility);
	vecBuildings.push_back(UnitTypes::Protoss_Stargate);
	vecBuildings.push_back(UnitTypes::Protoss_Citadel_of_Adun);
	vecBuildings.push_back(UnitTypes::Protoss_Templar_Archives);
	vecBuildings.push_back(UnitTypes::Protoss_Observatory);
}

SpendManager::~SpendManager()
{
	delete unitCount;
}

void SpendManager::setBuildManager(BuildManager *buildManager)
{
	this->buildManager = buildManager;
}

void SpendManager::setBuildOrderManager(BuildOrderManager *buildOrderManager)
{
	this->buildOrderManager = buildOrderManager;
}

void SpendManager::setMoodManager(MoodManager *moodManager)
{
	this->moodManager = moodManager;
}

void SpendManager::setDefenseManager(DefenseManager *defenseManager)
{
	this->defenseManager = defenseManager;
}

void SpendManager::setDragoonManager(DragoonManager *dragoonManager)
{
	this->dragoonManager = dragoonManager;
}

void SpendManager::setUnderAttackManager(UnderAttackManager *underAttackManager)
{
	this->underAttackManager = underAttackManager;
}

void SpendManager::setBaseManager(BaseManager *baseManager)
{
	this->baseManager = baseManager;
}

void SpendManager::setInformationManager(InformationManager *informationManager)
{
	this->informationManager = informationManager;
}

void SpendManager::setArmyManager(ArmyManager *armyManager)
{
	this->armyManager = armyManager;
}

std::string SpendManager::getName() const
{
	return "SpendManager";
}

std::string SpendManager::getShortName() const
{
	return "Spend";
}

SpendManager::Count* SpendManager::getUnitCount()
{
	return unitCount;
}

int SpendManager::getTotalUnits()
{
	return totalUnits;
}

int SpendManager::getTotalIdealUnits()
{
	return totalIdealUnits;
}

int SpendManager::getMineralStock()
{
	return mineralStock;
}

int SpendManager::getGasStock()
{
	return gasStock;
}

void SpendManager::needMoreGateways(int factor)
{
	UnitGroup myGateways = SelectAll(UnitTypes::Protoss_Gateway);
	bool allTraining = true;
	for each (Unit *gateway in myGateways)
	{
		if (!gateway->isTraining())
		{
			allTraining = false;
			break;
		}
	}
	if (allTraining)
	{
		if (myGateways.size() < 10)
		{
			int nbToBuild;
			if (minerals - mineralStock <= factor * 200)
				nbToBuild = 1;
			else if (minerals - mineralStock <= factor * 350)
				nbToBuild = 2;
			else if (minerals - mineralStock <= factor * 500)
				nbToBuild = 3;
			else if (minerals - mineralStock <= factor * 650)
				nbToBuild = 4;
			else 
				nbToBuild = 5;

			buildOrderManager->build(myGateways.size() + nbToBuild, UnitTypes::Protoss_Gateway, 60);
		}
	}
}

void SpendManager::update()
{
	if (Broodwar->getFrameCount() > lastFrameCheck + 24)
	{
		lastFrameCheck			= Broodwar->getFrameCount();
		weaponUpgradeLevel		= Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Weapons);
		armorUpgradeLevel		= Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Ground_Armor);
		shieldUpgradeLevel		= Broodwar->self()->getUpgradeLevel(UpgradeTypes::Protoss_Plasma_Shields);

		unitCount->zealot		= Broodwar->self()->allUnitCount(UnitTypes::Protoss_Zealot);
		unitCount->dragoon		= Broodwar->self()->allUnitCount(UnitTypes::Protoss_Dragoon);
		unitCount->corsair		= Broodwar->self()->allUnitCount(UnitTypes::Protoss_Corsair);
		unitCount->darkTemplar	= Broodwar->self()->allUnitCount(UnitTypes::Protoss_Dark_Templar);
		unitCount->archon		= Broodwar->self()->allUnitCount(UnitTypes::Protoss_Archon);

		totalUnits				= unitCount->zealot + unitCount->dragoon + unitCount->corsair + unitCount->darkTemplar + unitCount->archon;
		totalIdealUnits			= moodManager->getPoints()->zealot + moodManager->getPoints()->dragoon + 
																	moodManager->getPoints()->corsair + 
																	moodManager->getPoints()->darkTemplar +
																	moodManager->getPoints()->archon;

		minerals				= Broodwar->self()->minerals();
		gas						= Broodwar->self()->gas();

		if (totalUnits != 0)
		{
			unitCount->zealotRatio			=	((float)unitCount->zealot)			/ totalUnits;
			unitCount->dragoonRatio			=	((float)unitCount->dragoon)			/ totalUnits;
			unitCount->corsairRatio			=	((float)unitCount->corsair)			/ totalUnits;
			unitCount->darkTemplarRatio		=	((float)unitCount->darkTemplar)		/ totalUnits;
			unitCount->archonRatio			=	((float)unitCount->archon)			/ totalUnits;
		}

		unitCount->zealotIdealRatio			=	((float)moodManager->getPoints()->zealot)		/ totalIdealUnits;
		unitCount->dragoonIdealRatio		=	((float)moodManager->getPoints()->dragoon)		/ totalIdealUnits;
		unitCount->corsairIdealRatio		=	((float)moodManager->getPoints()->corsair)		/ totalIdealUnits;
		unitCount->darkTemplarIdealRatio	=	((float)moodManager->getPoints()->darkTemplar)	/ totalIdealUnits;
		unitCount->archonIdealRatio			=	((float)moodManager->getPoints()->archon)		/ totalIdealUnits;

		//if ((minerals > 300) && (Broodwar->self()->allUnitCount(UnitTypes::Protoss_Forge) != 0) && 
		//	(Broodwar->self()->allUnitCount(UnitTypes::Protoss_Photon_Cannon) == 0))
		//{
		//	buildOrderManager->build(1, UnitTypes::Protoss_Photon_Cannon, 90, 
		//		buildManager->getBuildingPlacer()->getBuildLocationNear(Broodwar->self()->getStartLocation(), UnitTypes::Protoss_Photon_Cannon, 0));
		//}

		mineralStock = 0;
		gasStock = 0;

		for (int i = 0; i < (signed)vecBuildings.size(); i++)
		{
			int count = buildManager->getPlannedCount(vecBuildings[i]) - Broodwar->self()->visibleUnitCount(vecBuildings[i]);
			if (count < 0)
				count = 0;
			mineralStock	+= count * vecBuildings[i].mineralPrice();
			gasStock		+= count * vecBuildings[i].gasPrice();
		}

		// If we rush, don't build a cyber before sending the rush squad.
		if (Broodwar->getFrameCount() > 4500 &&
			Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Cybernetics_Core) == 0 &&
			armyManager->getFirstAttackDone() )
		{
			if ( Broodwar->self()->visibleUnitCount( UnitTypes::Protoss_Assimilator ) == 0 
				 && 
				 buildOrderManager->getPlannedCount( UnitTypes::Protoss_Assimilator ) == 0
				 &&
				 Broodwar->getFrameCount() > TAKEGAS )
			{
				buildOrderManager->build(1, UnitTypes::Protoss_Assimilator, 100);
				baseManager->setRefineryBuildPriority(30);
			}

			buildOrderManager->build(1, UnitTypes::Protoss_Cybernetics_Core, 65);
		}

		if( ( moodManager->getMood() == MoodManager::MoodData::Defensive && Broodwar->getFrameCount() < 13000 )
			// TOFIX: Have some freeing tile problems
			//|| 
			//( ( underAttackManager->isUnderAttack() || defenseManager->isDefending() ) && armyManager->enemyDPS() > armyManager->myDPS() ) 
			)
		{
			/* if we have not at least 2 zealots, training them in emergency */
			if (Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Zealot < 2))
				buildOrderManager->build(2,BWAPI::UnitTypes::Protoss_Zealot,100);

			// DEPRECATED: Forget about gas, except if the enemy is Protoss
			//if ( Broodwar->enemy()->getRace() != Races::Protoss && moodManager->hasChangedMood() && moodManager->getMood() == MoodManager::MoodData::Defensive )
			//	baseManager->setRefineryBuildPriority(0);

			// Forget about everything, except pylons, gateway if we don't have four at leat, as well as cyber and assimilator 
			for (unsigned int i = 0; i < vecBuildings.size(); i++)
			{
				if( vecBuildings[i] != UnitTypes::Protoss_Pylon
					&&
					vecBuildings[i] != UnitTypes::Protoss_Gateway
					&&
					vecBuildings[i] != UnitTypes::Protoss_Cybernetics_Core 
					&& 
					vecBuildings[i] != UnitTypes::Protoss_Assimilator )
				{
					int count = buildOrderManager->cancelAllBuildingsOfType(vecBuildings[i]);
					mineralStock	-= count * vecBuildings[i].mineralPrice();
					gasStock		-= count * vecBuildings[i].gasPrice();

				}
			}

			// If all gateways are busy, build some another ones, depending of our money
			UnitGroup myGateways = SelectAll(UnitTypes::Protoss_Gateway)(isCompleted);
            UnitGroup freeGateways = myGateways.not(isTraining);
            if (freeGateways.empty() && minerals >= mineralStock + 200 )
				needMoreGateways(2);

			// Or, if the enemy is Protoss and has more gateways, build as many as him.
			if (Broodwar->enemy()->getRace() == Races::Protoss)
			{
				unsigned int enemyGates = informationManager->countEnemyBuildings(BWAPI::UnitTypes::Protoss_Gateway);
				if ( enemyGates > myGateways.size() )
					buildOrderManager->build(enemyGates, UnitTypes::Protoss_Gateway, 60);
			}

			for each (Unit *bat in freeGateways)
			{
				if ( unitCount->zealotRatio <= unitCount->zealotIdealRatio && 
					( unitCount->dragoonIdealRatio == 0 || unitCount->dragoonRatio > unitCount->dragoonIdealRatio || Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Cybernetics_Core ) == 0 || gas < gasStock + 50 ) &&
					unitCount->zealotIdealRatio != 0 && minerals >= mineralStock + 100 )
				{
					bat->train(UnitTypes::Protoss_Zealot);
					minerals = Broodwar->self()->minerals();
					continue;
				}

				if (unitCount->dragoonRatio <= unitCount->dragoonIdealRatio && unitCount->dragoonIdealRatio != 0 && minerals >= mineralStock + 125 && gas >= gasStock + 50)
				{
					bat->train(UnitTypes::Protoss_Dragoon);
					minerals = Broodwar->self()->minerals();
					gas = Broodwar->self()->gas();
					continue;
				}
			}
		}
		else
		{
			// built assimilator after 4500 frames if there are no problems (no enemy rush) or if we don't rush ourselves.
			if ( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Assimilator) == 0 
				 && 
				 buildOrderManager->getPlannedCount( UnitTypes::Protoss_Assimilator ) == 0
				 &&
				 Broodwar->getFrameCount() > TAKEGAS 
				 && 
				 armyManager->getFirstAttackDone())
			{
				buildOrderManager->build(1, UnitTypes::Protoss_Assimilator, 100);
				baseManager->setRefineryBuildPriority(30);
			}

			// if we were in defensive mode at frame 13000, expand
			//if( moodManager->getMood() == MoodManager::MoodData::Defensive && Broodwar->getFrameCount() == 13000 &&
			//	armyManager->myDPS() >= armyManager->enemyDPS() && !underAttackManager->isUnderAttack() )
			//{
			//	baseManager->expand(100);
			//}

			// upgrade only if we have at least a little army and a bigger army than enemy's one,
			// and if each gateway is training (unless for Singularity charge and Ground Weapon + 1)
			UnitGroup freeGateways = SelectAll()(Protoss_Gateway)(isCompleted).not(isTraining);
			if (armyManager->myDPStoGround() > 4 && armyManager->myDPStoGround() > armyManager->enemyDPStoGround())
			{

				/**************/
				/** Upgrades **/
				/**************/

				//get forge
				if ((moodManager->getMood() != MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 5100) ||
					(moodManager->getMood() == MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 7000))
				{
					if ( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Forge) == 0
						 &&
						 buildOrderManager->getPlannedCount( UnitTypes::Protoss_Forge ) == 0 )
					{
						buildOrderManager->build(1, UnitTypes::Protoss_Forge, 60);
					}
				}

				//prepare dragoon range upgrade
				//if ((moodManager->getMood() != MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 4300) ||
				//	(moodManager->getMood() == MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 6000))
				if( Broodwar->self()->visibleUnitCount( UnitTypes::Protoss_Dragoon ) >= 3
					&&
					minerals >= 150 //mineralStock + 150 
					&& 
					gas >= 150 //gasStock + 150 
					&&
					Broodwar->self()->getUpgradeLevel( UpgradeTypes::Singularity_Charge ) == 0 )
				{
					//if ( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Cybernetics_Core) == 0
					//	 &&
					//	 buildOrderManager->getPlannedCount( UnitTypes::Protoss_Cybernetics_Core ) == 0 )
					//{
					//	buildOrderManager->build(1, UnitTypes::Protoss_Cybernetics_Core, 65);
					//}

					//if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Cybernetics_Core) > 0 && 
					//	minerals >= mineralStock + 150 && gas >= gasStock + 150)
					//{

					UnitGroup cybers = SelectAll()(Protoss_Cybernetics_Core)(isCompleted);

					// Check if one cyber is not upgrading Singularity Charge
					bool chargeIsGettingUpgraded = false;
					for each (Unit *c in cybers)
						if(c->getUpgrade() == UpgradeTypes::Singularity_Charge )
							chargeIsGettingUpgraded = true;

					// If not, grab a non-upgrading cyber and go for it!
					if( !chargeIsGettingUpgraded )
					{
						for each (Unit *c in cybers)
							if (!c->isUpgrading())
							{
								c->upgrade(UpgradeTypes::Singularity_Charge);
								minerals = Broodwar->self()->minerals();
								gas = Broodwar->self()->gas();
								break;
							}
					}
				}

				//prepare zealot speed upgrade
				if( freeGateways.empty()
					&& 
					( 
						( 
							moodManager->getMood() != MoodManager::MoodData::FastExpo 
							&& 
							Broodwar->getFrameCount() > 7000 
						)
					    ||
					    ( 
							moodManager->getMood() == MoodManager::MoodData::FastExpo 
							&& 
							Broodwar->getFrameCount() > 9000 
						) 
					) 
				  )
				{
					if ( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Citadel_of_Adun) == 0
						 &&
						 buildOrderManager->getPlannedCount( UnitTypes::Protoss_Citadel_of_Adun ) == 0 )
					{
						buildOrderManager->build(1, UnitTypes::Protoss_Citadel_of_Adun, 60);
					}

					if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Citadel_of_Adun) > 0 && 
						minerals >= mineralStock + 150 && gas >= gasStock + 150)
					{
						UnitGroup citadels = SelectAll()(Protoss_Citadel_of_Adun)(isCompleted);
						for each (Unit *c in citadels)
						{
							if (!c->isUpgrading())
							{
								c->upgrade(UpgradeTypes::Leg_Enhancements);
								minerals = Broodwar->self()->minerals();
								gas = Broodwar->self()->gas();
								break;
							}
						}
					}
				}

				//prepare observer sight upgrade
				if ( Broodwar->getFrameCount() > 8000 && freeGateways.empty() )
				{
					if (Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Observatory) > 0 && 
						minerals >= mineralStock + 150 && gas >= gasStock + 150)
					{
						UnitGroup observatories = SelectAll()(Protoss_Observatory)(isCompleted);
						for each (Unit *o in observatories)
						{
							if (!o->isUpgrading())
							{
								o->upgrade(UpgradeTypes::Sensor_Array);
								minerals = Broodwar->self()->minerals();
								gas = Broodwar->self()->gas();
								break;
							}
						}
					}
				}

				//prepare weapon/armor upgrades
				if ((moodManager->getMood() != MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 7000) ||
					(moodManager->getMood() == MoodManager::MoodData::FastExpo && Broodwar->getFrameCount() > 9000))
				{
					//prepare weapon upgrade
					if( weaponUpgradeLevel == 0 
					    && 
					    Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Citadel_of_Adun) == 0 
					    &&
					    buildOrderManager->getPlannedCount( UnitTypes::Protoss_Citadel_of_Adun ) == 0 )
					{
						buildOrderManager->build(1, UnitTypes::Protoss_Citadel_of_Adun, 60);
					}

					if( freeGateways.empty() )
					{
						if( (weaponUpgradeLevel == 1 || Broodwar->self()->isUpgrading(UpgradeTypes::Protoss_Ground_Armor)) && 
							Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Templar_Archives) == 0)
						{
							buildOrderManager->build(1, UnitTypes::Protoss_Templar_Archives, 60);
						}

						//prepare armor upgrade
						if (armorUpgradeLevel < 3 && 
							(weaponUpgradeLevel == 3 || weaponUpgradeLevel > armorUpgradeLevel || 
							(weaponUpgradeLevel == armorUpgradeLevel && Broodwar->self()->isUpgrading(UpgradeTypes::Protoss_Ground_Weapons))) &&
							armorUpgradeLevel == 0 && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Forge) < 2 && 
							Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Cybernetics_Core) > 0)
						{
							buildOrderManager->build(2, UnitTypes::Protoss_Forge, 60);
						}

						//prepare shield upgrade
						if (shieldUpgradeLevel < 3 &&
							(weaponUpgradeLevel == 3 || Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Archon) > 0) &&
							shieldUpgradeLevel == 0 && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Forge) < 3 && 
							Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Cybernetics_Core) > 0)
						{
							buildOrderManager->build(3, UnitTypes::Protoss_Forge, 60);
						}
					}

					UnitGroup freeForges = SelectAll()(Protoss_Forge).not(isUpgrading);
					for each (Unit *bat in freeForges)
					{
						// Weapon
						if( bat->canIssueCommand( UnitCommand::upgrade( bat, UpgradeTypes::Protoss_Ground_Weapons ) ) 
							&& 
							minerals >= mineralStock + UpgradeTypes::Protoss_Ground_Weapons.mineralPrice( weaponUpgradeLevel + 1 )
							&& 
							gas >= gasStock + UpgradeTypes::Protoss_Ground_Weapons.gasPrice( weaponUpgradeLevel + 1 ) 
							&& 
							( freeGateways.empty() || weaponUpgradeLevel == 0 ) )
						{
							bat->upgrade( UpgradeTypes::Protoss_Ground_Weapons );
							minerals = Broodwar->self()->minerals();
							gas = Broodwar->self()->gas();
						}

						// Armor
						if( bat->canIssueCommand( UnitCommand::upgrade( bat, UpgradeTypes::Protoss_Ground_Armor ) )
							&& 
							minerals >= mineralStock + UpgradeTypes::Protoss_Ground_Armor.mineralPrice( armorUpgradeLevel + 1 )
							&& 
							gas >= gasStock + UpgradeTypes::Protoss_Ground_Armor.gasPrice( armorUpgradeLevel + 1 )
							&&
							freeGateways.empty() )
						{
							bat->upgrade(UpgradeTypes::Protoss_Ground_Armor);
							minerals = Broodwar->self()->minerals();
							gas = Broodwar->self()->gas();
						}

						// Shield
						if( Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Forge ) >= 3 
							&&
							bat->canIssueCommand( UnitCommand::upgrade( bat, UpgradeTypes::Protoss_Plasma_Shields ) )
							&& 
							minerals >= mineralStock + UpgradeTypes::Protoss_Plasma_Shields.mineralPrice( shieldUpgradeLevel + 1 )
							&& 
							gas >= gasStock + UpgradeTypes::Protoss_Plasma_Shields.gasPrice( shieldUpgradeLevel + 1 ) 
							&&
							freeGateways.empty() )
						{
							bat->upgrade(UpgradeTypes::Protoss_Plasma_Shields);
							minerals = Broodwar->self()->minerals();
							gas = Broodwar->self()->gas();
						}
					}
				}
			}

			/**************/
			/** Training **/
			/**************/

			freeGateways = SelectAll()(Protoss_Gateway)(isCompleted).not(isTraining);
			if (freeGateways.size() >= 2 && 
				unitCount->archonRatio <= unitCount->archonIdealRatio && 
				unitCount->archonIdealRatio != 0 && 
				minerals >= mineralStock + 100 && gas >= gasStock + 300)
			{
				if( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 
					&&
					buildOrderManager->getPlannedCount( UnitTypes::Protoss_Templar_Archives ) == 0 )
				{
					buildOrderManager->build(1, UnitTypes::Protoss_Templar_Archives, 60);
				}
				else
				{
					std::set<Unit*>::const_iterator bat = freeGateways.begin();
					(*bat)->train(UnitTypes::Protoss_High_Templar);
					bat++;
					(*bat)->train(UnitTypes::Protoss_High_Templar);
					minerals = Broodwar->self()->minerals();
					gas = Broodwar->self()->gas();
				}
			}

			freeGateways = SelectAll()(Protoss_Gateway)(isCompleted).not(isTraining);
			for each (Unit *bat in freeGateways)
			{
				if( ( unitCount->zealotRatio <= unitCount->zealotIdealRatio || 
					( ( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 || gas < gasStock + 100 ) && unitCount->darkTemplarIdealRatio != 0 ) || unitCount->darkTemplarIdealRatio == 0 ) &&
					( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 || gas < gasStock + 300 ) && unitCount->archonIdealRatio != 0 ) || unitCount->archonIdealRatio == 0 ) &&
					( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Stargate) == 0 || gas < gasStock + 100 ) && unitCount->corsairIdealRatio != 0 ) || unitCount->corsairIdealRatio == 0 ) &&
					( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Cybernetics_Core)  == 0 || gas < gasStock + 50 ) && unitCount->dragoonIdealRatio != 0 ) || unitCount->dragoonIdealRatio == 0 ) ) ) &&
					unitCount->zealotIdealRatio != 0 && minerals >= mineralStock + 100 )
				{
					bat->train(UnitTypes::Protoss_Zealot);
					minerals = Broodwar->self()->minerals();
					continue;
				}

				if( ( unitCount->dragoonRatio <= unitCount->dragoonIdealRatio || 
					( ( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 || gas < gasStock + 100 ) && unitCount->darkTemplarIdealRatio != 0 ) || unitCount->darkTemplarIdealRatio == 0 ) &&
					( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 || gas < gasStock + 300 ) && unitCount->archonIdealRatio != 0 ) || unitCount->archonIdealRatio == 0 ) &&
					( ( ( Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Stargate) == 0 || gas < gasStock + 100 ) && unitCount->corsairIdealRatio != 0 ) || unitCount->corsairIdealRatio == 0 ) ) ) &&
					unitCount->dragoonIdealRatio != 0 && minerals >= mineralStock + 125 && gas >= gasStock + 50 )
				{
					bat->train(UnitTypes::Protoss_Dragoon);
					minerals = Broodwar->self()->minerals();
					gas = Broodwar->self()->gas();
					continue;
				}

				if (unitCount->darkTemplarRatio <= unitCount->darkTemplarIdealRatio && 
					unitCount->darkTemplarIdealRatio != 0 && minerals >= mineralStock + 125 && gas >= gasStock + 100)
				{
					if( Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Templar_Archives) == 0 
						&&
						buildOrderManager->getPlannedCount( UnitTypes::Protoss_Templar_Archives ) == 0
						&&
						!underAttackManager->isUnderAttack() 
						&& 
						!defenseManager->isDefending() )
					{
						buildOrderManager->build(1, UnitTypes::Protoss_Templar_Archives, 60);
					}
					else
					{
						bat->train(UnitTypes::Protoss_Dark_Templar);
						minerals = Broodwar->self()->minerals();
						gas = Broodwar->self()->gas();
					}
				}
			}

			if( unitCount->corsairRatio <= unitCount->corsairIdealRatio 
				&& 
				unitCount->corsairIdealRatio != 0 
				&& 
				Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Stargate) == 0 
				&&
				buildOrderManager->getPlannedCount( UnitTypes::Protoss_Stargate ) == 0
				&&
				!underAttackManager->isUnderAttack() 
				&& 
				!defenseManager->isDefending())
			{
				buildOrderManager->build(1, UnitTypes::Protoss_Stargate, 60);
			}

			UnitGroup freeStargates = SelectAll()(Protoss_Stargate)(isCompleted).not(isTraining);
			for each (Unit *bat in freeStargates)
			{
				if (unitCount->corsairRatio <= unitCount->corsairIdealRatio && 
					unitCount->corsairIdealRatio != 0 && minerals >= mineralStock + 150 && gas >= gasStock + 100)
				{
					bat->train(UnitTypes::Protoss_Corsair);
					minerals = Broodwar->self()->minerals();
					gas = Broodwar->self()->gas();
				}
			}

			if (moodManager->getPoints()->corsair > 50 && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Stargate) < 2 &&
				!underAttackManager->isUnderAttack() && !defenseManager->isDefending())
			{
				buildOrderManager->build(2, UnitTypes::Protoss_Stargate, 60);
			}

			if (moodManager->getPoints()->corsair > 80 && Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Stargate) < 3 &&
				!underAttackManager->isUnderAttack() && !defenseManager->isDefending())
			{
				buildOrderManager->build(3, UnitTypes::Protoss_Stargate, 60);
			}

			if (Broodwar->getFrameCount() > 4000 && minerals >= mineralStock + 150)
				needMoreGateways(1);
		}
	}
}