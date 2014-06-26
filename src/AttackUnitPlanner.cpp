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

#include <AttackUnitPlanner.h>

AttackUnitPlanner *TheAttackUnitPlanner = NULL;
AttackUnitPlanner* AttackUnitPlanner::create()
{
	if (TheAttackUnitPlanner) return TheAttackUnitPlanner;
	return new AttackUnitPlanner();
}
void AttackUnitPlanner::destroy()
{
	if (TheAttackUnitPlanner)
		delete TheAttackUnitPlanner;
}
AttackUnitPlanner::AttackUnitPlanner()
{
	TheAttackUnitPlanner = this;
}
AttackUnitPlanner::~AttackUnitPlanner()
{
	TheAttackUnitPlanner = NULL;
}

void AttackUnitPlanner::setAttackTracker( AttackTracker *attackTracker )
{
	this->attackTracker = attackTracker;
}

double AttackUnitPlanner::computeRatio(BWAPI::Unit *attacker, BWAPI::Unit *target)
{
	if( attacker == NULL || 
		!attacker->exists() || 
		target == NULL || 
		!target->exists() || 
		target->getType() == BWAPI::UnitTypes::Protoss_Interceptor ||
		target->isInvincible() )
	{
		return 0;
	}

	int damage = 0;
	int cooldown = 0;

	BWAPI::UnitType t = target->getType();

	if( t == BWAPI::UnitTypes::Protoss_Carrier )
	{
		BWAPI::UnitType interceptor = BWAPI::UnitTypes::Protoss_Interceptor;
		BWAPI::WeaponType airInterceptor = interceptor.airWeapon();
		damage = 8 * ( ( airInterceptor.damageAmount() + 
			( airInterceptor.damageFactor() * BWAPI::Broodwar->enemy()->getUpgradeLevel(airInterceptor.upgradeType()) ) ) * interceptor.maxAirHits() );
		cooldown = airInterceptor.damageCooldown();
	}
	else
	{
		if( t.maxGroundHits() > 0 )
		{
			BWAPI::WeaponType gw = t.groundWeapon();
			damage = ( gw.damageAmount() + ( gw.damageFactor() * target->getUpgradeLevel( gw.upgradeType() ) ) ) * t.maxGroundHits();
			cooldown = gw.damageCooldown();
		}

		if( t.maxAirHits() > 0 )
		{
			BWAPI::WeaponType aw = t.airWeapon();
			damage += ( aw.damageAmount() + ( aw.damageFactor() * target->getUpgradeLevel( aw.upgradeType() ) ) ) * t.maxAirHits();
			cooldown += aw.damageCooldown();
		}

		if( t.isSpellcaster() )
		{
			damage += 20;
			cooldown += 20;
		}

		if( damage == 0 || cooldown == 0 || target->getHitPoints() == 0 )
			return 0;
	}
	
	// Ratio is: DPS / HP (with shield) / ( distance + 1 )
	return ( ( damage / cooldown ) / ( target->getHitPoints() + target->getShields() ) ) / ( target->getDistance( attacker ) + 1 ); 
}

BWAPI::Unit* AttackUnitPlanner::chooseNextTarget( BWAPI::Unit *attacker )
{
	BWAPI::Unit *nextTarget = NULL;
	double ratio = 0;
	double bestRatio = 0;

	if( attacker->getType().maxGroundHits() > 0 )
	{
		int radius = std::max( attacker->getType().groundWeapon().maxRange(), 30 );
		UnitGroup aroundMe = UnitGroup::getUnitGroup( attacker->getUnitsInRadius( radius ) );
		for each( BWAPI::Unit *u in aroundMe.not( isFlyer ) )
		{
			ratio = computeRatio(attacker, u);
			if( bestRatio < ratio )
			{
				bestRatio = ratio;
				nextTarget = u;
			}
		}
	}

	if( attacker->getType().maxAirHits() > 0 )
	{
		int radius = std::max( attacker->getType().airWeapon().maxRange(), 30 );
		UnitGroup aroundMe = UnitGroup::getUnitGroup( attacker->getUnitsInRadius( radius ) );
		for each( BWAPI::Unit *u in aroundMe( isFlyer ) )
		{
			ratio = computeRatio(attacker, u);
			if( bestRatio < ratio )
			{
				bestRatio = ratio;
				nextTarget = u;
			}
		}
	}

	return nextTarget;
}

bool AttackUnitPlanner::attackNextTarget( BWAPI::Unit *attacker )
{
	BWAPI::Unit *target = chooseNextTarget( attacker );
	if( target != NULL && target->exists() )
	{
		attacker->attack( target );
		return true;
	}

	return false;
}

void AttackUnitPlanner::update()
{
		
}
