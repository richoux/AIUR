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
#include <BWTA.h>
enum FilterFlagBullet
{
	existsBullet,
	isVisibleBullet,

    Melee,
    Fusion_Cutter_Hit,
    Gauss_Rifle_Hit,
    C_10_Canister_Rifle_Hit,
    Gemini_Missiles,
    Fragmentation_Grenade,
    Longbolt_Missile,
    ATS_ATA_Laser_Battery,
    Burst_Lasers,
    Arclite_Shock_Cannon_Hit,
    EMP_Missile,
    Dual_Photon_Blasters_Hit,
    Particle_Beam_Hit,
    Anti_Matter_Missile,
    Pulse_Cannon,
    Psionic_Shockwave_Hit,
    Psionic_Storm,
    Yamato_Gun,
    Phase_Disruptor,
    STA_STS_Cannon_Overlay,
    Sunken_Colony_Tentacle,
    Acid_Spore,
    Glave_Wurm,
    Seeker_Spores,
    Queen_Spell_Carrier,
    Plague_Cloud,
    Consume,
    Needle_Spine_Hit,
    Invisible,
    Optical_Flare_Grenade,
    Halo_Rockets,
    Subterranean_Spines,
    Corrosive_Acid_Shot,
    Neutron_Flare,
    NoneBullet,
    Unknown_Bullet
};
enum FliterAttributeScalarBullet
{
	RemoveTimerBullet,
	PositionXBullet,
	PositionYBullet,
	AngleBullet,
	VelocityXBullet,
	VelocityYBullet,
	SourcePositionXBullet,
	SourcePositionYBullet,
	TargetPositionXBullet,
	TargetPositionYBullet
};

enum FilterAttributeBullet
{
	GetSourceBullet,
	GetTargetBullet
};

enum FilterAttributePositionBullet
{
	GetPositionBullet,
	GetSourcePositionBullet,
	GetTargetPositionBullet
};

class BulletGroup : public std::set<BWAPI::Bullet*>
{
public:
	static BulletGroup getBulletGroup(const std::set<BWAPI::Bullet*> bullets)
	{
		BulletGroup u;
		u.insert(bullets.begin(), bullets.end());
		return u;
	}

	BulletGroup operator+(const BulletGroup& other) const;//union
	BulletGroup operator*(const BulletGroup& other) const;//intersection
	BulletGroup operator^(const BulletGroup& other) const;//symmetric difference
	BulletGroup operator-(const BulletGroup& other) const;//difference
	BulletGroup inRadius(double radius,BWAPI::Position position) const;
	BulletGroup inRegion(BWTA::Region* region) const;
	BulletGroup onlyNearestChokepoint(BWTA::Chokepoint* choke) const;
	BulletGroup onlyNearestBaseLocation(BWTA::BaseLocation* location) const;
	BulletGroup operator()(int f1) const;
	BulletGroup operator()(int f1, int f2) const;
	BulletGroup operator()(int f1, int f2, int f3) const;
	BulletGroup operator()(int f1, int f2, int f3, int f4) const;
	BulletGroup operator()(int f1, int f2, int f3, int f4, int f5) const;
	BulletGroup operator()(FliterAttributeScalarBullet a, const char* compare, double value) const;
	BulletGroup operator()(FliterAttributeScalarBullet a, const char* compare, int value) const;
	BulletGroup operator()(BWAPI::Player* player) const;
	BulletGroup operator()(FilterAttributeBullet a, BWAPI::Unit* Unit) const;
	BulletGroup operator()(BWAPI::BulletType type) const;
	BulletGroup operator()(FilterAttributePositionBullet a, BWAPI::Position position) const;

	BulletGroup not(int f1) const;
	BulletGroup not(int f1, int f2) const;
	BulletGroup not(int f1, int f2, int f3) const;
	BulletGroup not(int f1, int f2, int f3, int f4) const;
	BulletGroup not(int f1, int f2, int f3, int f4, int f5) const;
	BulletGroup not(FliterAttributeScalarBullet a, const char* compare, double value) const;
	BulletGroup not(FliterAttributeScalarBullet a, const char* compare, int value) const;
	BulletGroup not(BWAPI::Player* player) const;
	BulletGroup not(FilterAttributeBullet a, BWAPI::Unit* Unit) const;
	BulletGroup not(BWAPI::BulletType type) const;
	BulletGroup not(FilterAttributePositionBullet a, BWAPI::Position position) const;

	BulletGroup& operator+=(const BulletGroup& other);//union
	BulletGroup& operator*=(const BulletGroup& other);//intersection
	BulletGroup& operator^=(const BulletGroup& other);//symmetric difference
	BulletGroup& operator-=(const BulletGroup& other);//difference
	BWAPI::Bullet* getNearest(BWAPI::Position position) const;
	bool contains(BWAPI::Bullet* u) const;
};

