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

#include <ArmyManager.h>

ArmyManager::ArmyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;
	lastFrameCheck = 0;
	lastExpandFrame = 0;
	lastAttack = 0;
	initialized = false;
	randomEarlyAttack = new Random(100);
	randomSizeRush = new Random(7);
	sizeRush = 0.0;
	maxEnemyDPS = 0.0;
	dropPosition = NULL;
	unloadOrder = false;
}

ArmyManager::ArmyManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, UnitStates *unitStates)
: 	arbitrator(arbitrator), unitStates(unitStates)
{
	lastFrameCheck		= 0;
	lastExpandFrame		= 0;
	lastAttack			= 0;
	initialized			= false;
	randomEarlyAttack	= new Random(100);
	randomSizeRush		= new Random(7);
	sizeRush			= 0.0;
	maxEnemyDPS			= 0.0;
	maxEnemyDPStoAir	= 0.0;
	maxEnemyDPStoGround = 0.0;
	maxEnemyDPSfromAir	= 0.0;
	maxEnemyDPSfromGround = 0.0;
	dropPosition = NULL;
	unloadOrder = false;
}

ArmyManager::~ArmyManager()
{
	delete randomEarlyAttack;
	delete randomSizeRush;
	if( dropPosition != NULL )
		delete dropPosition;
}

void ArmyManager::setDefenseManager(DefenseManager* defenseManager)
{
	this->defenseManager = defenseManager;
}

void ArmyManager::setMoodManager(MoodManager* moodManager)
{
	this->moodManager = moodManager;
}

void ArmyManager::setDragoonManager(DragoonManager* dragoonManager)
{
	this->dragoonManager = dragoonManager;
}

void ArmyManager::setAttackUnitPlanner( AttackUnitPlanner *attackUnitPlanner )
{
	this->attackUnitPlanner = attackUnitPlanner;
}

void ArmyManager::setInformationManager(InformationManager* infoManager)
{
	this->informationManager = infoManager;
}

void ArmyManager::setBaseManager(BaseManager *baseManager)
{
	this->baseManager = baseManager;
}

bool ArmyManager::getFirstAttackDone()
{
	return firstAttackDone;
}

int ArmyManager::getStartAttack()
{
	return startAttack;
}

bool ArmyManager::isInitialized()
{
  return initialized;
}

void ArmyManager::initialize()
{
	MoodManager::MoodData::Mood mood = moodManager->getMood();

	firstAttackDone = moodManager->getFirstAttackDone();

	// if the enemy is Zerg, let's have 75% to rush him (if we don't FE).
	//if (BWAPI::Broodwar->enemy()->getRace() == BWAPI::Races::Zerg && moodManager->getFirstAttackDone() && mood != MoodManager::MoodData::FastExpo)
	//{
	//	if ( randomEarlyAttack->nextInt() < 75 )
	//		firstAttackDone = false;
	//	else
	//		firstAttackDone = true;
	//}

	startAttackInitial = startAttack = moodManager->getStartAttack();
	initialized			= true;
}

double ArmyManager::enemyDPS()
{
	double total = 0;
	UnitGroup enemies = SelectAllEnemy()(canMove)(isCompleted).not(isWorker);

	// Ground
	for each (BWAPI::Unit *u in enemies)
		if( u->getType().maxGroundHits() > 0 )
			total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();

	// Air
	for each (BWAPI::Unit *u in enemies)
		if( u->getType().maxAirHits() > 0 || u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
		{
			BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
			BWAPI::WeaponType airInterceptor = interceptor.airWeapon();
			if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
			{				
				int damage = ( airInterceptor.damageAmount() + 
							 ( airInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(airInterceptor.upgradeType()) ) ) * interceptor.maxAirHits();
				total += ( (double)damage * u->getInterceptorCount() ) / airInterceptor.damageCooldown();
			}
			else if( u->getType() != interceptor )
				total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
		}

	if (total > maxEnemyDPS)
		maxEnemyDPS = total;

	return maxEnemyDPS;
}

double ArmyManager::enemyDPStoAir()
{
	double total = 0;
	UnitGroup enemies = SelectAllEnemy()(canMove)(isCompleted).not(isWorker);

	for each (BWAPI::Unit *u in enemies)
		if( u->getType().maxAirHits() > 0 || u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
		{
			BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
			BWAPI::WeaponType airInterceptor = interceptor.airWeapon();
			if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
			{				
				int damage = ( airInterceptor.damageAmount() + 
							 ( airInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(airInterceptor.upgradeType()) ) ) * interceptor.maxAirHits();
				total += ( (double)damage * u->getInterceptorCount() ) / airInterceptor.damageCooldown();
			}
			else if( u->getType() != interceptor )
				total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
		}

	if (total > maxEnemyDPStoAir)
		maxEnemyDPStoAir = total;

	return maxEnemyDPStoAir;
}

double ArmyManager::enemyDPStoGround()
{
	double total = 0;
	UnitGroup enemies = SelectAllEnemy()(canMove)(isCompleted).not(isWorker);

	for each (BWAPI::Unit *u in enemies)
		if( u->getType().maxGroundHits() > 0 || u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
		{
			BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
			BWAPI::WeaponType groundInterceptor = interceptor.groundWeapon();
			if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
			{				
				int damage = ( groundInterceptor.damageAmount() + 
							 ( groundInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(groundInterceptor.upgradeType()) ) ) * interceptor.maxGroundHits();
				total += ( (double)damage * u->getInterceptorCount() ) / groundInterceptor.damageCooldown();
			}
			else if( u->getType() != interceptor )
				total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
		}

	if (total > maxEnemyDPStoGround)
		maxEnemyDPStoGround = total;

	return maxEnemyDPStoGround;
}

double ArmyManager::enemyDPSfromAir()
{
	double total = 0;
	for each (BWAPI::Unit *u in SelectAllEnemy()(canMove)(isCompleted)(isFlyer))
	{
		BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
		
		if( u->getType().maxGroundHits() > 0 || u->getType() == BWAPI::UnitTypes::Protoss_Carrier)
		{
			BWAPI::WeaponType groundInterceptor = interceptor.groundWeapon();
			if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
			{
				int damage = ( groundInterceptor.damageAmount() + 
							 ( groundInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(groundInterceptor.upgradeType()) ) ) * interceptor.maxGroundHits();
				total += ( (double)damage * u->getInterceptorCount() ) / groundInterceptor.damageCooldown();
			}
			else if( u->getType() != BWAPI::UnitTypes::Protoss_Interceptor )
				total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
		}
		if( u->getType().maxAirHits() > 0 || u->getType() == BWAPI::UnitTypes::Protoss_Carrier)
		{
			BWAPI::WeaponType airInterceptor = interceptor.airWeapon();
			if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
			{
				int damage = ( airInterceptor.damageAmount() + 
							 ( airInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(airInterceptor.upgradeType()) ) ) * interceptor.maxAirHits();
				total += ( (double)damage * u->getInterceptorCount() ) / airInterceptor.damageCooldown();
			}
			else if( u->getType() != interceptor )
				total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
		}
	}

	if (total > maxEnemyDPSfromAir)
		maxEnemyDPSfromAir = total;

	return maxEnemyDPSfromAir;
}

double ArmyManager::enemyDPSfromGround()
{
	double total = 0;
	for each (BWAPI::Unit *u in SelectAllEnemy()(canMove)(isCompleted).not(isFlyer).not(isWorker))
	{
		if( u->getType().maxGroundHits() > 0)
			total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
		if( u->getType().maxAirHits() > 0)
			total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
	}

	if (total > maxEnemyDPSfromGround)
		maxEnemyDPSfromGround = total;

	return maxEnemyDPSfromGround;
}

double ArmyManager::myDPS()
{
  double total = 0;

	// Ground
	for each (BWAPI::Unit *u in SelectAll()(canMove)(isCompleted)(maxGroundHits,">",0).not(isWorker))
	{
		total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
	}

	// Air
	for each (BWAPI::Unit *u in SelectAll()(canMove)(isCompleted)(maxAirHits,">",0))
	{
		total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
	}

	return total;
}

double ArmyManager::myDPStoAir()
{
	double total = 0;
	for each (BWAPI::Unit *u in SelectAll()(canMove)(isCompleted)(maxAirHits,">",0))
		total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();

	return total;
}

double ArmyManager::myDPStoGround()
{
	double total = 0;
	for each (BWAPI::Unit *u in SelectAll()(canMove)(isCompleted)(maxGroundHits,">",0).not(isWorker))
		total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();

	return total;
}

double ArmyManager::thisGroupDPS(std::set<BWAPI::Unit*> group)
{
  double total = 0;

	for each (BWAPI::Unit *u in group)
	{
		BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
		if( !u->getType().isFlyer() )
			total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
		else if( u->getType() == BWAPI::UnitTypes::Protoss_Carrier )
		{			
			BWAPI::WeaponType airInterceptor = interceptor.airWeapon();
			int damage = ( airInterceptor.damageAmount() + 
						 ( airInterceptor.damageFactor() * Broodwar->enemy()->getUpgradeLevel(airInterceptor.upgradeType()) ) ) * interceptor.maxAirHits();
			total += ( (double)damage * u->getInterceptorCount() ) / airInterceptor.damageCooldown();
		}
		else if( u->getType() != interceptor )
			total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
	}

	return total;
}

double ArmyManager::myDPS(std::map<BWAPI::Unit*, ArmyData> group)
{
  double total = 0;
	std::set<BWAPI::Unit*> groupSet;

	for (std::map<BWAPI::Unit*, ArmyData>::const_iterator it = group.begin(); it != group.end(); ++it)
	{
		groupSet.insert(it->first);
	}

	for each (BWAPI::Unit *u in groupSet)
	{
		if ( u->exists() )
		{
			if( !u->getType().isFlyer() )
				total += ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
			else
				total += ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
		}
	}

	return total;
}

int ArmyManager::enemyHP()
{
  int total = 0;

	for each (BWAPI::Unit *u in SelectAllEnemy()(canMove)(isCompleted)(maxGroundHits,">",0).not(isWorker))
	{
		if (u->exists())
		{
			total += u->getHitPoints();			
			total += u->getShields();
		}
	}

	return total;
}

int ArmyManager::myHP(std::map<BWAPI::Unit*, ArmyData> group)
{
  int total = 0;
	std::set<BWAPI::Unit*> groupSet;

	for (std::map<BWAPI::Unit*, ArmyData>::const_iterator it = group.begin(); it != group.end(); ++it)
	{
		groupSet.insert(it->first);
	}

	for each (BWAPI::Unit *u in groupSet)
	{
		if (u->exists())
		{
			total += u->getHitPoints();
			total += u->getShields();
		}
	}

	return total;
}

std::set<BWAPI::Unit*>	ArmyManager::whoIsAttacking(BWAPI::Unit *unit)
{
	std::set<BWAPI::Unit*> listAttack;
	// get units around our target unit under the attack range of a siege-mode tank
	UnitGroup aroundUnit;

	if (unit->getPlayer() == BWAPI::Broodwar->self())
		aroundUnit = SelectAllEnemy()(isAttacking).inRadius((double)BWAPI::WeaponTypes::Arclite_Shock_Cannon.maxRange(), unit->getPosition());
	else 
		aroundUnit = SelectAll()(isAttacking).inRadius((double)BWAPI::WeaponTypes::Arclite_Shock_Cannon.maxRange(), unit->getPosition());
	
	for each (BWAPI::Unit *u in aroundUnit)
		if (u->getOrderTarget() == unit)
			listAttack.insert(u);

	return listAttack;
}

BWAPI::Unit* ArmyManager::whoIsTheWeakest(std::set<BWAPI::Unit*> group)
{
	int min = std::numeric_limits<int>::max();
	int hp;
	
	if (group.empty())
		return NULL;

	BWAPI::Unit *weakest = (*group.begin());

	for each (BWAPI::Unit *u in group)
	{
		hp = u->getHitPoints();
		if (u->getType().getRace() == BWAPI::Races::Protoss)
			hp += u->getShields();

		if (hp < min)
		{
			min = hp;
			weakest = u;
		}
	}

	return weakest;
}

bool ArmyManager::containsBuilding(std::set<BWAPI::Unit*> list)
{
	for each (BWAPI::Unit *u in list)
		if (u->getType().isBuilding())
			return true;

	return false;
}

BWAPI::Unit* ArmyManager::nearestUnit(BWAPI::Unit *unit, std::set<BWAPI::Unit*> list)
{
	int minimum = std::numeric_limits<int>::max();
	BWAPI::Unit *nearest = NULL;

	for each (BWAPI::Unit *u in list)
		if (u->getDistance(unit) < minimum)
		{
			minimum = u->getDistance(unit);
			nearest = u;
		}

	return nearest;
}

void ArmyManager::setLastExpandFrame(int lastExpandFrame)
{
	this->lastExpandFrame = lastExpandFrame;
}

void ArmyManager::update()
{
	// after 6500 frames, no rush
	//if (BWAPI::Broodwar->getFrameCount() > 6500 && !firstAttackDone)
	//	firstAttackDone = true;

	// if we have change our mood, we must change the startAttack value
	if( startAttackInitial != moodManager->getStartAttack() )
		startAttackInitial = startAttack = moodManager->getStartAttack();

	// each 15000 frames, increase the startAttack by 3 until reaching 22
	if (BWAPI::Broodwar->getFrameCount() != 0 && BWAPI::Broodwar->getFrameCount() % 15000 == 0)
	{
		if (startAttack + 3 < 22)
		{
			startAttack += 3;
		}
		else if (startAttack != 22)
		{
			startAttack = 22;
		}
	}

	// each time we destroy an enemy base, we descrease startAttack by 3.
	if (informationManager->getEnemyBaseDestroyed())
	{
		if (startAttack - 3 > 6)
		{
			startAttack -= 3;
		}
		else if (startAttack != 6)
		{
			startAttack = 6;
		}
		informationManager->setEnemyBaseDestroyed(false);
	}

	if (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 12)
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		//if (!firstAttackDone && sizeRush == 0.0)
		//{
		//	sizeRush = 0.7 + (((double)randomSizeRush->nextInt()) / 10);
		//}

		if( BWAPI::Broodwar->self()->completedUnitCount( UnitTypes::Protoss_Shuttle ) == 1 && dropPosition == NULL )
		{
			UnitGroup group = SelectAll()(isCompleted).not(isBuilding, isWorker);
			for each( BWAPI::Unit *u in group )
			{
				if( unitStates->isState( u, UnitStates::Dropping ) )
					arbitrator->setBid(this, u, 100);
			}
		}

		// We attack if:
		// - our DPS is higher than the value starAttack and than enemy's DPS,
		// - or if we rush
		// - or if we didn't attack since 18000 frames (which would be weird BTW)
		// - or we have 200 of population
		if ( ( myDPStoGround() > startAttack && myDPStoGround() > enemyDPStoGround() + 2 ) 
			|| (myDPStoGround() >= sizeRush && !firstAttackDone) 
			|| BWAPI::Broodwar->getFrameCount() > lastAttack + 18000 
			|| Broodwar->self()->supplyUsed() >= 400 ) // in BWAPI, supply are multiplied by 2
		{
			std::set<BWAPI::Unit*> idleDefenders;
			if (BWAPI::Broodwar->getFrameCount() > lastAttack + 18000)
				idleDefenders = defenseManager->getDefenders();
			else
				idleDefenders = defenseManager->getIdleDefenders();

			lastAttack = BWAPI::Broodwar->getFrameCount();
			 
			//int count = 0;
			if (!idleDefenders.empty())
			{
				for (std::set<BWAPI::Unit*>::iterator it = idleDefenders.begin(); it != idleDefenders.end(); ++it)
				{
					if (firstAttackDone)
						arbitrator->setBid(this, *it, 70);
					else
						arbitrator->setBid(this, *it, 40);
				}
			}
		}

		// if we have more than 2 HT, merge them!
		UnitGroup highTemplars = SelectAll(BWAPI::UnitTypes::Protoss_High_Templar)(isCompleted).not(isMoving);
		// while (highTemplars.size() > 1) works whatever highTemplars.size() is odd or even
		while (highTemplars.size() > 1)
		{
			std::set<BWAPI::Unit*>::iterator ht1 = highTemplars.begin();
			std::set<BWAPI::Unit*>::reverse_iterator ht2 = highTemplars.rbegin();
			(*ht1)->useTech(BWAPI::TechTypes::Archon_Warp, (*ht2));
			highTemplars.erase(*ht1);				
			highTemplars.erase(*ht2);
		}

		std::map<BWAPI::Unit*, InformationManager::UnitData> savedData = informationManager->getSavedData();
		enemyBuildings.clear();
		for (std::map<BWAPI::Unit*, InformationManager::UnitData>::const_iterator sd = savedData.begin(); sd != savedData.end(); ++sd)
		{
			if (sd->second.exists && sd->second.player == BWAPI::Broodwar->enemy() && sd->second.type.isBuilding() && !sd->first->isInvincible())
			{
				enemyBuildings.push_back(sd->second.position);
			}
		}
		round = (int)enemyBuildings.size() - 1;

		std::set<BWTA::BaseLocation*> toVisit = informationManager->getUnvisitedBaseLocation();
		unvisitedBases.clear();
		for each (BWTA::BaseLocation *bl in toVisit)
			unvisitedBases.push_back(bl);
		roundUnvisited = (int)unvisitedBases.size() - 1;

		BWAPI::Position pos = informationManager->getEnemyStartLocation()->getPosition();
		if (myDPStoGround() >= sizeRush && !firstAttackDone && !company.empty())
		{
			//if( BWAPI::Broodwar->enemy()->getRace() != BWAPI::Races::Protoss || informationManager->getNumberBaseCenters() > 1)
			//{
				// attack natural first (if it doesn't exist, units will attack the main)
			    // DISABLED AIUR v2.2 
				//if (informationManager->getEnemyNatural() == NULL)
				//	informationManager->computeEnemyNatural();
				//BWTA::BaseLocation *natural = informationManager->getEnemyNatural();

				//if (natural != NULL)
				//	pos = natural->getPosition();

				//for(std::map<BWAPI::Unit*,ArmyManager::ArmyData>::iterator it = company.begin(); it != company.end(); ++it)
				//{
				//	it->first->move(pos);
				//	unitStates->setState( it->first, UnitStates::Moving );
				//	it->second.target = natural;
				//}

				if( informationManager->getEnemyStartLocation() != NULL )
				{
					pos = informationManager->getEnemyStartLocation()->getPosition();

					for(std::map<BWAPI::Unit*,ArmyManager::ArmyData>::iterator it = company.begin(); it != company.end(); ++it)
					{
						it->first->attack(pos);
						unitStates->setState( it->first, UnitStates::Moving );
					}
				}

			//}
			firstAttackDone = true;
		}

		fleeing.clear();

		//for each (BWAPI::Unit *u in company)
		for(std::map<BWAPI::Unit*,ArmyManager::ArmyData>::iterator it = company.begin(); it != company.end(); ++it)
		{
			if( unitStates->isState( it->first, UnitStates::Dropping ) )
			{
				if( it->first->getType() == BWAPI::UnitTypes::Protoss_Shuttle )
				{
					if( it->first->getLoadedUnits().size() == 4 )
					{
						if( dropPosition == NULL )
						{
							BWTA::BaseLocation* enemyBase = informationManager->getEnemyStartLocation();
							BWAPI::Position baseCenter = enemyBase->getPosition();
							dropPosition = new Position( baseCenter.x(), baseCenter.y() - (int)( 2.5 * TILE_SIZE ) );
						}
						else if( !unloadOrder )
						{
							it->first->unloadAll( *dropPosition );
							unloadOrder = true;
						}
					}
					else
					{
						if(	it->first->getLoadedUnits().size() == 0 && dropPosition != NULL )
						{
							it->first->attack( baseManager->getLastTakenBaseLocation()->getPosition() );
							unitStates->setState( it->first, UnitStates::Idle );
							arbitrator->removeBid( this, it->first );
						}
					}
				}
				//else if( it->first->getType() == BWAPI::UnitTypes::Protoss_Zealot && !it->first->isLoaded() )
				//{
				//	UnitGroup enemies = SelectAllEnemy().inRadius( BWAPI::UnitTypes::Protoss_Zealot.sightRange() * 1.5, it->first->getPosition() ).not(isInvincible);
				//	if( !enemies.empty() )
				//	{
				//		UnitGroup workers = enemies(isWorker, isCompleted);
				//		if( !workers.empty() )
				//		{
				//			if( !attackUnitPlanner->attackNextTarget( it->first ) )
				//				it->first->attack( nearestUnit( it->first, workers ) );
				//		}
				//		else if( it->first->isIdle() )
				//		{
				//			UnitGroup pylons = enemies( BWAPI::UnitTypes::Protoss_Pylon );
				//			if( !pylons.empty() )
				//			{
				//				if( !attackUnitPlanner->attackNextTarget( it->first ) )
				//					it->first->attack( nearestUnit( it->first, pylons ) );
				//			}
				//			else 
				//			{
				//				if( !attackUnitPlanner->attackNextTarget( it->first ) )
				//					it->first->attack( nearestUnit( it->first, enemies ) );
				//			}
				//		}
				//	}
				//}
			}
			else
			{
				if( unitStates->isState( it->first, UnitStates::Fleeing ) && 
					it->first->getPosition().getApproxDistance( it->second.target->getPosition() ) < it->first->getType().sightRange() * 2 )
				{
					fleeing.insert( it->first );
					unitStates->setState( it->first, UnitStates::Idle );
				}

				// if *it belongs to the rush squad, attack the natural (if any) and then the main, aiming workers
				if( arbitrator->getBid(this, it->first) == 40 && firstAttackDone)
				{
					// If the enemy army is too big, flee courageously!
					//if( unitStates->isState( it->first, UnitStates::Moving ) || 
					//	unitStates->isState( it->first, UnitStates::Attacking ) )
					//{
						//if ((double)myHP(company) / enemyDPS() < (double)enemyHP() / myDPS(company) && it->first->exists())
						//{
						//	BWAPI::Position myPosition(BWAPI::Broodwar->self()->getStartLocation());
						//	it->first->move( baseManager->getLastTakenBaseLocation()->getPosition() );
						//	unitStates->setState( it->first, UnitStates::Fleeing );
						//	it->second.target = baseManager->getLastTakenBaseLocation();
						//}
						//else if (!it->first->isAttacking())
						//{
						//	std::set<BWAPI::Unit*> aroundMe = it->first->getUnitsInRadius(it->first->getType().sightRange());
						//	for each (BWAPI::Unit *am in aroundMe)
						//	{
						//		if (am->getType().isWorker() && am->getPlayer() == BWAPI::Broodwar->enemy())
						//			it->first->attack(am);
						//	}
						//}
					//}

					BWAPI::Position positionToAttack = informationManager->getEnemyStartLocation()->getPosition();

					//look if the natural is empty or not
					if (informationManager->getEnemyNatural() == NULL)
						informationManager->computeEnemyNatural();
					bool emptyNatural = true;
					bool arrivedNatural = false;
					if (it->second.target == informationManager->getEnemyNatural() &&
						it->first->getPosition().getApproxDistance(it->second.target->getPosition()) < it->first->getType().sightRange())
					{
						arrivedNatural = true;
						for(int x = it->second.target->getTilePosition().x(); x < it->second.target->getTilePosition().x() + 4; x++)
						{
							for(int y = it->second.target->getTilePosition().y(); y < it->second.target->getTilePosition().y() + 3; y++)
							{
								std::set<BWAPI::Unit*> getUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(x,y);
								for each(BWAPI::Unit* u in getUnitsOnTile)
								{
									if (u->getType().isResourceDepot())
									{
										emptyNatural = false;
										UnitGroup minerals = UnitGroup::getUnitGroup(BWTA::getNearestBaseLocation(u->getTilePosition())->getMinerals());
										positionToAttack = minerals.getCenter();
										break;
									}
								}
								if (!emptyNatural) break;
							}
							if (!emptyNatural) break;
						}
					}

					//look if we have reached the main
					bool arrivedMain = false;
					if (it->second.target == informationManager->getEnemyStartLocation() &&
						it->first->getPosition().getApproxDistance(it->second.target->getPosition()) < it->first->getType().sightRange())
					{					
						for(int x = it->second.target->getTilePosition().x(); x < it->second.target->getTilePosition().x() + 4; x++)
						{
							for(int y = it->second.target->getTilePosition().y(); y < it->second.target->getTilePosition().y() + 3; y++)
							{
								std::set<BWAPI::Unit*> getUnitsOnTile = BWAPI::Broodwar->getUnitsOnTile(x,y);
								for each(BWAPI::Unit* u in getUnitsOnTile)
								{
									if (u->getType().isResourceDepot())
									{
										arrivedMain = true;
										UnitGroup minerals = UnitGroup::getUnitGroup(BWTA::getNearestBaseLocation(u->getTilePosition())->getMinerals());
										positionToAttack = minerals.getCenter();
										break;
									}
								}
								if (arrivedMain) break;
							}
							if (arrivedMain) break;
						}
					}

					if( it->first->isIdle() )
					{
						unitStates->setState( it->first, UnitStates::Idle );
					}
					if( unitStates->isState( it->first, UnitStates::Idle ) )
					{
						//if (it->second.target == informationManager->getEnemyNatural())
						//	it->second.mode = ArmyManager::ArmyData::Harassing;
						//else if (it->second.target == informationManager->getEnemyStartLocation())
						//	it->second.mode = ArmyManager::ArmyData::Attacking;
						//else if (it->second.mode != ArmyManager::ArmyData::Fleeing)
						//	it->second.mode = ArmyManager::ArmyData::Idle;

						if( it->second.target == NULL )
						{
							// For dragoons micro
							if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
							{
								it->first->attack( informationManager->getEnemyNatural()->getPosition() );
								unitStates->setState( it->first, UnitStates::Moving );
								it->second.target = informationManager->getEnemyNatural();
							}
						}

						if (emptyNatural && arrivedNatural && it->second.target == informationManager->getEnemyNatural())
						{
							// For dragoons micro
							if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )

							{
								it->first->attack( positionToAttack );
								unitStates->setState( it->first, UnitStates::Moving );
								it->second.target = informationManager->getEnemyStartLocation();
							}
						}

						if (it->first->isUnderAttack())
						{
							UnitGroup list = UnitGroup::getUnitGroup( whoIsAttacking(it->first) );
							// to not attack interceptors, if any
							list -= list(BWAPI::UnitTypes::Protoss_Interceptor);
							if( !containsBuilding(list) )
							{
								// For dragoons micro
								if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
								{
									//if( !attackUnitPlanner->attackNextTarget( it->first ) )
									//{
										BWAPI::Unit *toAttack = nearestUnit( it->first, list );
										if ( toAttack == NULL )
											it->first->attack( positionToAttack );
										else
											//it->first->attack( toAttack->getPosition() );
											it->first->attack( toAttack );
									//}
									unitStates->setState( it->first, UnitStates::Attacking );
								}
							}
						}

						if (((!emptyNatural && arrivedNatural && it->second.target == informationManager->getEnemyNatural()) ||
							(	arrivedMain && it->second.target == informationManager->getEnemyStartLocation())))
						{
							// For dragoons micro
							if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack( it->first ) )
							{
								//if( !attackUnitPlanner->attackNextTarget( it->first ) )
									it->first->attack( positionToAttack );
								unitStates->setState( it->first, UnitStates::Attacking );
							}
						}
					}
				}
				// if it is a regular attack, not a rush
				else
				{
					// if some units from rush squad are lost, retake them under control
					if( moodManager->getMood() == MoodManager::MoodData::Rush )
					{
						std::set<BWAPI::Unit*> group = arbitrator->getObjects(this);
						for each (BWAPI::Unit *u in group)
							if (arbitrator->getBid(this, u) == 40)
								arbitrator->setBid(this, u, 70);
					}

					// hit enemy units first
					UnitGroup enemies = SelectAllEnemy().inRadius( 2 * TILE_SIZE * Broodwar->self()->sightRange( it->first->getType() ), it->first->getPosition() );
					UnitGroup units = enemies.inRadius( it->first->getType().seekRange(), it->first->getPosition() )(isDetected).not(isBuilding);
					UnitGroup airUnits = units(isFlyer);
					UnitGroup groundUnits = units.not(isFlyer);
					if( ( !airUnits.empty() && it->first->getType().maxAirHits() > 0 ) 
						|| 
						( !groundUnits.empty() && it->first->getType().maxGroundHits() > 0 ) )
					{
						it->first->attack( nearestUnit( it->first, airUnits+groundUnits ) );
						if( !unitStates->isState( it->first, UnitStates::Attacking ) )
							unitStates->setState( it->first, UnitStates::Attacking );
					}


					// focus on pylons
					if( Broodwar->enemy()->getRace() == Races::Protoss && 
						//( unitStates->isState( it->first, UnitStates::Moving ) || 
						unitStates->isState( it->first, UnitStates::Attacking ) ) //)
					{
						UnitGroup pylons = enemies( UnitTypes::Protoss_Pylon );
						if( !pylons.empty() )
						{
							it->first->attack( nearestUnit( it->first, pylons ) );
							if( !unitStates->isState( it->first, UnitStates::Attacking ) )
								unitStates->setState( it->first, UnitStates::Attacking );
						}
					}

					if( it->first->isIdle() )
					{
						unitStates->setState( it->first, UnitStates::Idle );
					}
					if(unitStates->isState( it->first, UnitStates::Idle ) )
					{
						// go destroy scouted bases.
						informationManager->refreshEnemyBases();
						if (!informationManager->getEnemyBases().empty())
						{
							BWTA::BaseLocation *bl = informationManager->getEnemyBases().back();
							it->second.target = bl;
							pos = it->second.target->getPosition(); 
							if ((BWAPI::Broodwar->getFrameCount() > lastExpandFrame + 4000) && (myDPStoGround() > 5))
							{
								lastExpandFrame = BWAPI::Broodwar->getFrameCount();
								baseManager->expand(90);
							}
						}
						else
						{
							// go destroy buildings in the light fog of war.
							if ((int)enemyBuildings.size() != 0)
							{
								pos = enemyBuildings[round];
								--round;
								if (round < 0)
									round = (int)enemyBuildings.size() - 1;
							}
							// go destroy remaining visible buildings.
							else
							{
								UnitGroup enemyBases = SelectAllEnemy()(isBuilding)(exists).not(isInvincible);
								if (!enemyBases.empty())
								{
									std::set<BWAPI::Unit*>::const_iterator first = enemyBases.begin();
									pos = (*first)->getPosition();
								}
								// go destroy hidden and unscouted bases in the dark fog of war.
								else
								{
									pos = unvisitedBases[roundUnvisited]->getPosition();
									--roundUnvisited;
									if (roundUnvisited < 0)
										roundUnvisited = (int)unvisitedBases.size() - 1;
								}
							}
						}
						if (it->first->getType() == BWAPI::UnitTypes::Protoss_Observer && !company.empty())
						{
							std::map<BWAPI::Unit*,ArmyManager::ArmyData>::iterator leader = company.begin();
							if (leader->first->getType() != BWAPI::UnitTypes::Protoss_Observer)
							{
								it->first->follow(leader->first);
								unitStates->setState( it->first, UnitStates::Following );
							}
							else
							{
								leader++;
								if( leader != company.end() )
								{
									it->first->follow(leader->first);
									unitStates->setState( it->first, UnitStates::Following );
								}
								else
								{
									it->first->move( BWTA::getStartLocation( BWAPI::Broodwar->self() )->getPosition() );
									unitStates->setState( it->first, UnitStates::Moving );
								}
							}
							unitStates->setState( it->first, UnitStates::Moving );
						}
						else
						{
							// For dragoons micro
							if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
							{
								//if( !attackUnitPlanner->attackNextTarget( it->first ) )
									it->first->attack( pos );
								unitStates->setState( it->first, UnitStates::Attacking );
							}
						}					
					}
				}
			}
		}
		for each (BWAPI::Unit *u in fleeing)
		{
			if (company.find(u) != company.end())
				company.erase(u);
			arbitrator->removeBid(this, u);
		}
	}
}

std::string ArmyManager::getName() const
{
	return "Army Manager";
}

std::string ArmyManager::getShortName() const
{
	return "Arm";
}

void ArmyManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for each (BWAPI::Unit *u in units)
	{
		if (company.find(u) == company.end())
		{
			arbitrator->accept(this, u);
			ArmyData ad;
			company[u] = ad;
		}
	}
}

void ArmyManager::onRevoke(BWAPI::Unit *u, double bid)
{
	onRemoveUnit(u);
}

void ArmyManager::onRemoveUnit(BWAPI::Unit* u)
{
	if (u->getPlayer() == BWAPI::Broodwar->self())
		company.erase(u);
	else if (u->getType().canMove() && !u->getType().isWorker())
	{
		if( u->getType().maxGroundHits() > 0 )
		{
			maxEnemyDPS -= ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
			if (maxEnemyDPS < 0)
				maxEnemyDPS = 0;

			maxEnemyDPStoGround -= ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
			if (maxEnemyDPStoGround < 0)
				maxEnemyDPStoGround = 0;

			maxEnemyDPSfromAir -= ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
			if (maxEnemyDPSfromAir < 0)
				maxEnemyDPSfromAir = 0;

			maxEnemyDPSfromGround -= ( (double)u->getType().groundWeapon().damageAmount() * u->getType().maxGroundHits() ) / u->getType().groundWeapon().damageCooldown();
			if (maxEnemyDPSfromGround < 0)
				maxEnemyDPSfromGround = 0;
		}

		if( u->getType().maxAirHits() > 0 )
		{
			maxEnemyDPS -= ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
			if (maxEnemyDPS < 0)
				maxEnemyDPS = 0;

			maxEnemyDPStoAir -= ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
			if (maxEnemyDPStoAir < 0)
				maxEnemyDPStoAir = 0;

			maxEnemyDPSfromAir -= ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
			if (maxEnemyDPSfromAir < 0)
				maxEnemyDPSfromAir = 0;

			maxEnemyDPSfromGround -= ( (double)u->getType().airWeapon().damageAmount() * u->getType().maxAirHits() ) / u->getType().airWeapon().damageCooldown();
			if (maxEnemyDPSfromGround < 0)
				maxEnemyDPSfromGround = 0;
		}
	}
}
