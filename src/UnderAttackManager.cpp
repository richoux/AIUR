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

#include <UnderAttackManager.h>
#include <UnitGroup.h>

UnderAttackManager::UnderAttackManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator)
{
	this->arbitrator = arbitrator;
	lastFrameCheck = 0;
	lastFrameUnderAttack = 0;
	underAttack = false;
}

UnderAttackManager::UnderAttackManager(Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator, UnitStates *unitStates)
: arbitrator(arbitrator), unitStates(unitStates)
{
	lastFrameCheck = 0;
	lastFrameUnderAttack = 0;
	underAttack = false;
}

bool UnderAttackManager::hasObserver()
{
	for( std::map<BWAPI::Unit*, UAData>::iterator it = backUp.begin(); it != backUp.end(); ++it )
	{
		if( it->first->getType() == BWAPI::UnitTypes::Protoss_Observer )
			return true;
	}

	return false;
}

void UnderAttackManager::update()
{
	if (BWAPI::Broodwar->getFrameCount() > lastFrameCheck + 20)
	{
		lastFrameCheck = BWAPI::Broodwar->getFrameCount();

		// spot and free idle backups
		for (std::map<BWAPI::Unit*, UAData>::iterator it = backUp.begin(); it != backUp.end(); ++it)
		{
			// to fix a bug with UAM workers mining gas
			if( ( it->second.mode == UAData::WorkerDefending || it->second.mode == UAData::Worker ) && it->first->isGatheringGas() )
				it->first->stop();

			if ((it->second.mode == UAData::WorkerDefending || it->second.mode == UAData::Defending) && 
				it->first->isIdle())
			{
				UnitGroup intrudersInMyRegion = SelectAllEnemy().inRegion(it->second.regionOrigin).not(isBuilding);
				if (!intrudersInMyRegion.empty())
				{
					//if( !attackUnitPlanner->attackNextTarget( it->first ) )
					//{
						BWAPI::Unit *weakest = ArmyManager::whoIsTheWeakest(intrudersInMyRegion);
						// For dragoons micro
						if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
							it->first->attack(weakest->getPosition());
					//}
				}
				else
					it->second.mode = UAData::Idle;
			}

			// if we reach the attacked region, turn hasReachRegion to true
			if ((it->second.mode == UAData::WorkerDefending || it->second.mode == UAData::Defending) && 
				!it->second.hasReachRegion && it->second.regionOrigin == BWTA::getRegion(it->first->getTilePosition()))
			{
				it->second.hasReachRegion = true;
			}

			// if we are leaving the attacked region, cancel everything: the enemy is fleeing
			if ((it->second.mode == UAData::WorkerDefending || it->second.mode == UAData::Defending) && 
				it->second.hasReachRegion && it->second.regionOrigin != BWTA::getRegion(it->first->getTilePosition()))
			{
				it->second.mode = UAData::Idle;	
			}

			if( (it->second.mode == UAData::Worker || it->second.mode == UAData::Defender) && it->first->isIdle() )
			{
				bool toRelease = true;
				
				for( std::map<BWAPI::Unit*, UAData>::iterator ju = backUp.begin(); ju != backUp.end(); ++ju)
				{
					if( ju->first->isUnderAttack() )
					{
						toRelease = false;
						break;
					}
				}
				
				if( toRelease )
					it->second.mode = UAData::Idle;
			}
			
			if (it->second.mode == UAData::Idle)
			{	
				it->first->stop();
				arbitrator->removeBid(this,it->first);
			}
		}

		UnitGroup myPrecious = SelectAll()(isBuilding) + SelectAll()(isWorker);
		for each (BWAPI::Unit *u in myPrecious)
		{
			if ( u->isUnderAttack() && 
				( !u->getType().isWorker() || !scoutManager->isScouting( u ) || ( moodManager->getMood() == MoodManager::MoodData::Cheese && rushPhotonManager->isCheesing( u ) ) ) )
			{
				if (!underAttack)
					underAttack = true;
				lastFrameUnderAttack = BWAPI::Broodwar->getFrameCount();

				// if there are cloaked enemy units in the same region as our unit
				// make sure we have observers
				if( !hasObserver() )
				{
					UnitGroup invisiblesInRegion = SelectAllEnemy()(isCloaked).inRegion( BWTA::getRegion( u->getTilePosition() ) );
					if( !invisiblesInRegion.empty() )
					{
						if( spyManager->getNumberSpies() > 0 )
							spyManager->removeBid();
						else
							spyManager->buildObserver(true);
					}
				}

				// If I am an under attacked worker, myCoWorkers are my buddy workers from the same base
				UnitGroup myCoWorkers;

				std::set<BWAPI::Unit*> idleDef = defenseManager->getIdleDefenders();
				for each (BWAPI::Unit *def in idleDef)
				{					
					arbitrator->setBid(this, def, 50);
				}

				// if a non-scouting worker is attacked OR if our DPS is less than enemy's
				if (u->getType().isWorker() || (armyManager->enemyDPStoGround() > armyManager->myDPStoGround()))
				{
					//std::set<BWAPI::Unit*> aroundMe = u->getUnitsInRadius(u->getType().sightRange());
					//myCoWorkers = UnitGroup::getUnitGroup(aroundMe);
					//myCoWorkers = myCoWorkers(BWAPI::Broodwar->self())(isWorker)(isCompleted);
					UnitGroup myCoWorkers = SelectAll()(BWAPI::Broodwar->self())(isWorker)(isCompleted);
					
					myCoWorkers = myCoWorkers.inRegion(BWTA::getRegion(u->getTilePosition()));

					// If only one enemy worker is harassing, just take some probes. Otherwise, everyone.
					// attackers == 0 means there is great chance the attacker is an harassing worker not attacking continually
					set<BWAPI::Unit*> attackers = ArmyManager::whoIsAttacking(u);
					UnitGroup groundAttackers = UnitGroup::getUnitGroup( attackers ).not( isFlyer );
					if (attackers.size() == 0 || (attackers.size() == 1 && (*attackers.begin())->getType().isWorker()))
					{
						std::set<BWAPI::Unit*>::iterator it = myCoWorkers.begin();
						
						int helpers = 1;
						//if( moodManager->getMood() == MoodManager::MoodData::Cheese)
						//	helpers = 1;
						//else
						//	helpers = 3;

						for (int i = 0; i < helpers && it != myCoWorkers.end(); i++)
						{
							arbitrator->setBid(this, (*it), 60);
							it++;
						}
					}
					// anyway, always leave 5 workers mining if we have at least 8 workers
					// well, if there are ground enemies of course
					else if( !groundAttackers.empty() )
					{	
						if (myCoWorkers.size() < 8)
						{
							for each (BWAPI::Unit *def in myCoWorkers)
							{
								arbitrator->setBid(this, def, 60);
							}
						}
						else
						{
							std::set<BWAPI::Unit*>::iterator it = myCoWorkers.begin();
							for (unsigned int i = 0; i < myCoWorkers.size() - 5; i++)
							{
								arbitrator->setBid(this, (*it), 60);
								it++;
							}
						}
					}
				}

				// If I am an under attacked building and I am researching/upgrading/training/being constructed
				// and my life is below 50 hp, cancel everything.
				if (u->getHitPoints() < 50)
				{
					if (u->isResearching())
					{
						u->cancelResearch();
					}
					if (u->isUpgrading())
					{
						u->cancelUpgrade();
					}
					if (u->isTraining())
					{
						u->cancelTrain();
					}
					if ( u->isBeingConstructed() && 
						( moodManager->getMood() != MoodManager::MoodData::Cheese || ( u->getType() != UnitTypes::Protoss_Photon_Cannon &&  u->getType() != UnitTypes::Protoss_Pylon ) ) )
					{
						u->cancelConstruction();
					}
				}

				// if we have no defenders and an enemy worker attack our pylons, take some probes and screw him!
				if (u->getType() == BWAPI::UnitTypes::Protoss_Pylon && defenseManager->getIdleDefenders().empty())
				{
					UnitGroup enemyWorkers = SelectAllEnemy()(isWorker).inRadius(50, u->getPosition());
					if (!enemyWorkers.empty())
					{
						UnitGroup myCoWorkers = SelectAll()(BWAPI::Broodwar->self())(isWorker)(isCompleted).inRegion(BWTA::getRegion(u->getTilePosition()));
						std::set<BWAPI::Unit*>::iterator it = myCoWorkers.begin();
						for (int i = 0; i < 3 && it != myCoWorkers.end(); i++)
						{
							arbitrator->setBid(this, (*it), 60);
							it++;
						}
					}
				}

				// Go help my buddies!
				for (std::map<BWAPI::Unit*, UAData>::iterator it = backUp.begin(); it != backUp.end(); ++it)
				{
					it->second.regionOrigin = BWTA::getRegion(u->getTilePosition());

					set<BWAPI::Unit*> attackers = ArmyManager::whoIsAttacking(u);

					if( it->second.mode == UAData::Defender
						&& ( moodManager->getMood() != MoodManager::MoodData::Rush 
						     ||
							 it->first->getType() != BWAPI::UnitTypes::Protoss_Dark_Templar
							 ||
							 Broodwar->getFrameCount() > 9000 ) )
					{
						// For dragoons micro
						if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
						{
							//if( !attackUnitPlanner->attackNextTarget( it->first ) )
							//{
								if (attackers.empty())
									it->first->attack(u->getPosition());
								else
									it->first->attack( (*attackers.begin())->getPosition() );
							//}
							it->second.mode = UAData::Defending;
						}
					}

					if (it->second.mode == UAData::Worker)
					{
						//if( !attackUnitPlanner->attackNextTarget( it->first ) )
						//{
							if (attackers.empty())
								it->first->attack(u->getPosition());
							else
								it->first->attack( ( *attackers.begin() ) );
						//}
						it->second.mode = UAData::WorkerDefending;
					}
				}
			}
			else if (underAttack && BWAPI::Broodwar->getFrameCount() > lastFrameUnderAttack + 48) // if we have not been attacked since 2 sec
				underAttack = false;
		}

		// Anti gas steal (and anti in-base enemy building)
		UnitGroup intruders = SelectAllEnemy()( isBuilding );
		for each (BWAPI::Unit *u in intruders)
		{
			// if build u is placed inside one of our region, blast it!
			if (baseManager->getMyRegions().find(BWTA::getRegion(u->getTilePosition())) != baseManager->getMyRegions().end())
			{
				std::set<BWAPI::Unit*> idleDef = defenseManager->getIdleDefenders();
				for each (BWAPI::Unit *def in idleDef)
				{
					arbitrator->setBid(this, def, 50);
				}

				for (std::map<BWAPI::Unit*, UAData>::iterator it = backUp.begin(); it != backUp.end(); ++it)
				{
					if (it->second.mode == UAData::Defender)
					{
						// For dragoons micro
						if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
						{
							//if( !attackUnitPlanner->attackNextTarget( it->first ) )
								it->first->attack(u->getPosition());
							it->second.mode = UAData::Defending;
						}
					}
				}								
			}
		}

		// if someone is entering into our territory
		intruders = SelectAllEnemy().not( isBuilding );
		for each( BWAPI::Unit *u in intruders )
		{
			// if u is cloaked, make sure we have observers
			if( u->isCloaked() )
			{
				if( spyManager->getNumberSpies() > 0 )
					spyManager->removeBid();
				else
					spyManager->buildObserver(true);
			}

			BWTA::Region *uRegion = BWTA::getRegion(u->getTilePosition());
			// if u is moving inside one of our region, blast it!
			if( baseManager->getMyRegions().find( uRegion ) != baseManager->getMyRegions().end() )
			{
				// if u is not a worker and belongs to a enemy group much stronger than our defense, add our workers to defend!
				UnitGroup intrudersInMyRegion = SelectAllEnemy().inRegion( uRegion ).not( isBuilding );
				if( !u->getType().isWorker() && ArmyManager::thisGroupDPS( intrudersInMyRegion ) > armyManager->myDPS() )
				{
					UnitGroup myCoWorkers = SelectAll()( BWAPI::Broodwar->self() )( isWorker )( isCompleted ).inRegion( uRegion );
					if( myCoWorkers.size() < 8 )
					{
						for each( BWAPI::Unit *def in myCoWorkers )
						{
							arbitrator->setBid( this, def, 60 );
						}
					}
					else
					{
						std::set<BWAPI::Unit*>::iterator it = myCoWorkers.begin();
						for( unsigned int i = 0; i < myCoWorkers.size() - 5; i++ )
						{
							arbitrator->setBid( this, (*it), 60 );
							it++;
						}
					}
				}
					
				std::set<BWAPI::Unit*> idleDef = defenseManager->getIdleDefenders();
				for each( BWAPI::Unit *def in idleDef )
				{
					if( ( u->getType().isFlyer() && def->getType().airWeapon() != WeaponTypes::None ) 
						||
						( !u->getType().isFlyer() && def->getType().groundWeapon() != WeaponTypes::None ) )
						
						arbitrator->setBid(this, def, 50);
				}

				//BWAPI::Unit *weakest = u;
				BWAPI::Unit *weakest = ArmyManager::whoIsTheWeakest(intrudersInMyRegion.not(isWorker));
				if (weakest == NULL)
					weakest = u;

				for (std::map<BWAPI::Unit*, UAData>::iterator it = backUp.begin(); it != backUp.end(); ++it)
				{
					if (it->second.mode == UAData::Worker)
					{
						//if( !attackUnitPlanner->attackNextTarget( it->first ) )
							it->first->attack(weakest->getPosition());				
						it->second.mode = UAData::WorkerDefending;
						it->second.regionOrigin = BWTA::getRegion(u->getTilePosition());
					}

					if (it->second.mode == UAData::Defender)
					{
						// For dragoons micro
						if( it->first->getType() != BWAPI::UnitTypes::Protoss_Dragoon || dragoonManager->availableToAttack(it->first) )
						{
							//if( !attackUnitPlanner->attackNextTarget( it->first ) )
								it->first->attack(weakest->getPosition());
							it->second.mode = UAData::Defending;
							it->second.regionOrigin = BWTA::getRegion(u->getTilePosition());
						}
					}
				}								
			}
		}
	}		
}

void UnderAttackManager::setArmyManager(ArmyManager *armyManager)
{
	this->armyManager = armyManager;
}

void UnderAttackManager::setAttackUnitPlanner(AttackUnitPlanner *attackUnitPlanner)
{
	this->attackUnitPlanner = attackUnitPlanner;
}

void UnderAttackManager::setDefenseManager(DefenseManager *defenseManager)
{
	this->defenseManager = defenseManager;
}

void UnderAttackManager::setScoutManager(ScoutManager *scoutManager)
{
	this->scoutManager = scoutManager;
}

void UnderAttackManager::setSpyManager(SpyManager *spyManager)
{
	this->spyManager = spyManager;
}

void UnderAttackManager::setMoodManager(MoodManager *moodManager)
{
	this->moodManager = moodManager;
}

void UnderAttackManager::setRushPhotonManager(RushPhotonManager *rushPhotonManager)
{
	this->rushPhotonManager = rushPhotonManager;
}

bool UnderAttackManager::isUnderAttack()
{
	return underAttack;
}

void UnderAttackManager::setBaseManager(BaseManager *baseManager)
{
	this->baseManager = baseManager;
}

void UnderAttackManager::setDragoonManager(DragoonManager *dragoonManager)
{
	this->dragoonManager = dragoonManager;
}

std::string UnderAttackManager::getName() const
{
	return "UnderAttack Manager";
}

std::string UnderAttackManager::getShortName() const
{
	return "UAM";
}

void UnderAttackManager::onOffer(std::set<BWAPI::Unit*> units)
{
	for each (BWAPI::Unit *u in units)
	{
		if (backUp.find(u) == backUp.end())
		{
			arbitrator->accept(this, u);
			
			UAData temp;
			if (arbitrator->getBid(this, u) == 50)
			{
				temp.mode = UAData::Defender;
			}
			else
			{
				temp.mode = UAData::Worker;
			}

			temp.hasReachRegion = false;
			backUp.insert(std::make_pair(u, temp));
		}
	}
}

void UnderAttackManager::onRevoke(BWAPI::Unit *u, double bid)
{
	onRemoveUnit(u);
}

void UnderAttackManager::onRemoveUnit(BWAPI::Unit* u)
{
	backUp.erase(u);
}
