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

#include <BulletGroup.h>

using namespace BWAPI;
using namespace std;

bool passesFlag(Bullet* u, int f)
{
	if (f<0)
		return !passesFlag(u,-f);
	switch(f)
	{
	case existsBullet:
		if(u->exists()) return true;
		break;
	case isVisibleBullet:
		if(u->isVisible()) return true;
		break;
	case Melee:
		if(u->getType()==BulletTypes::Melee) return true;
		break;
	case Fusion_Cutter_Hit:
		if(u->getType()==BulletTypes::Fusion_Cutter_Hit) return true;
		break;
	case Gauss_Rifle_Hit:
		if(u->getType()==BulletTypes::Gauss_Rifle_Hit) return true;
		break;
	case C_10_Canister_Rifle_Hit:
		if(u->getType()==BulletTypes::C_10_Canister_Rifle_Hit) return true;
		break;
	case Gemini_Missiles:
		if(u->getType()==BulletTypes::Gemini_Missiles) return true;
		break;
	case Fragmentation_Grenade:
		if(u->getType()==BulletTypes::Fragmentation_Grenade) return true;
		break;
	case Longbolt_Missile:
		if(u->getType()==BulletTypes::Longbolt_Missile) return true;
		break;
	case ATS_ATA_Laser_Battery:
		if(u->getType()==BulletTypes::ATS_ATA_Laser_Battery) return true;
		break;
	case Burst_Lasers:
		if(u->getType()==BulletTypes::Burst_Lasers) return true;
		break;
	case Arclite_Shock_Cannon_Hit:
		if(u->getType()==BulletTypes::Arclite_Shock_Cannon_Hit) return true;
		break;
	case EMP_Missile:
		if(u->getType()==BulletTypes::EMP_Missile) return true;
		break;
	case Dual_Photon_Blasters_Hit:
		if(u->getType()==BulletTypes::Dual_Photon_Blasters_Hit) return true;
		break;
	case Particle_Beam_Hit:
		if(u->getType()==BulletTypes::Particle_Beam_Hit) return true;
		break;
	case Anti_Matter_Missile:
		if(u->getType()==BulletTypes::Anti_Matter_Missile) return true;
		break;
	case Pulse_Cannon:
		if(u->getType()==BulletTypes::Pulse_Cannon) return true;
		break;
	case Psionic_Shockwave_Hit:
		if(u->getType()==BulletTypes::Psionic_Shockwave_Hit) return true;
		break;
	case Psionic_Storm:
		if(u->getType()==BulletTypes::Psionic_Storm) return true;
		break;
	case Yamato_Gun:
		if(u->getType()==BulletTypes::Yamato_Gun) return true;
		break;
	case Phase_Disruptor:
		if(u->getType()==BulletTypes::Phase_Disruptor) return true;
		break;
	case STA_STS_Cannon_Overlay:
		if(u->getType()==BulletTypes::STA_STS_Cannon_Overlay) return true;
		break;
	case Sunken_Colony_Tentacle:
		if(u->getType()==BulletTypes::Sunken_Colony_Tentacle) return true;
		break;
	case Acid_Spore:
		if(u->getType()==BulletTypes::Acid_Spore) return true;
		break;
	case Glave_Wurm:
		if(u->getType()==BulletTypes::Glave_Wurm) return true;
		break;
	case Seeker_Spores:
		if(u->getType()==BulletTypes::Seeker_Spores) return true;
		break;
	case Queen_Spell_Carrier:
		if(u->getType()==BulletTypes::Queen_Spell_Carrier) return true;
		break;
	case Plague_Cloud:
		if(u->getType()==BulletTypes::Plague_Cloud) return true;
		break;
	case Consume:
		if(u->getType()==BulletTypes::Consume) return true;
		break;
	case Needle_Spine_Hit:
		if(u->getType()==BulletTypes::Needle_Spine_Hit) return true;
		break;
	case Invisible:
		if(u->getType()==BulletTypes::Invisible) return true;
		break;
	case Optical_Flare_Grenade:
		if(u->getType()==BulletTypes::Optical_Flare_Grenade) return true;
		break;
	case Halo_Rockets:
		if(u->getType()==BulletTypes::Halo_Rockets) return true;
		break;
	case Subterranean_Spines:
		if(u->getType()==BulletTypes::Subterranean_Spines) return true;
		break;
	case Corrosive_Acid_Shot:
		if(u->getType()==BulletTypes::Corrosive_Acid_Shot) return true;
		break;
	case Neutron_Flare:
		if(u->getType()==BulletTypes::Neutron_Flare) return true;
		break;
	case NoneBullet:
		if(u->getType()==BulletTypes::None) return true;
		break;
	case Unknown_Bullet:
		if(u->getType()==BulletTypes::Unknown) return true;
		break;
	}
	return false;
}

double getAttribute(Bullet* u, FliterAttributeScalarBullet a)
{
	switch(a)
	{
	case RemoveTimerBullet:
		return u->getRemoveTimer();
		break;
	case PositionXBullet:
		return u->getPosition().x();
		break;
	case PositionYBullet:
		return u->getPosition().y();
		break;
	case AngleBullet:
		return u->getAngle();
		break;
	case VelocityXBullet:
		return u->getVelocityX();
		break;
	case VelocityYBullet:
		return u->getVelocityY();
		break;
	case SourcePositionXBullet:
		return u->getSource()->getPosition().x();
		break;
	case SourcePositionYBullet:
		return u->getSource()->getPosition().y();
		break;
	case TargetPositionXBullet:
		return u->getTargetPosition().x();
		break;
	case TargetPositionYBullet:
		return u->getTargetPosition().y();
		break;
	}
	return 0;
}

Unit* getUnit(Bullet* u, FilterAttributeBullet a)
{
	switch(a)
	{
	case GetSourceBullet:
		return u->getSource();
		break;
	case GetTargetBullet:
		return u->getTarget();
		break;
	}
	return u->getTarget();
}
BulletGroup BulletGroup::operator+(const BulletGroup& other) const
{
	BulletGroup result=*this;
	result+=other;
	return result;
}
BulletGroup BulletGroup::operator*(const BulletGroup& other) const
{
	BulletGroup result=*this;
	result*=other;
	return result;
}
BulletGroup BulletGroup::operator^(const BulletGroup& other) const
{
	BulletGroup result=*this;
	result^=other;
	return result;
}
BulletGroup BulletGroup::operator-(const BulletGroup& other) const
{
	BulletGroup result=*this;
	result-=other;
	return result;
}

BulletGroup BulletGroup::operator()(int f1) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (passesFlag(*i,f1))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::operator()(int f1, int f2) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (passesFlag(*i,f1) || passesFlag(*i,f2))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::operator()(int f1, int f2, int f3) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::operator()(int f1, int f2, int f3, int f4) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3) || passesFlag(*i,f4))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::operator()(int f1, int f2, int f3, int f4, int f5) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3) || passesFlag(*i,f4) || passesFlag(*i,f5))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::operator()(FliterAttributeScalarBullet a, const char* compare, double value) const
{
	BulletGroup result;
	string cmp(compare);
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		double val=getAttribute(*i,a);
		bool passes=false;
		if (cmp=="=" || cmp=="==" || cmp=="<=" || cmp==">=")
			if (val==value)
				passes=true;
		if (cmp=="<" || cmp=="<=" || cmp=="!=" || cmp=="<>")
			if (val<value)
				passes=true;
		if (cmp==">" || cmp==">=" || cmp=="!=" || cmp=="<>")
			if (val>value)
				passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::operator()(FliterAttributeScalarBullet a, const char* compare, int value) const
{
	BulletGroup result;
	string cmp(compare);
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		int val=(int)getAttribute(*i,a);
		bool passes=false;
		if (cmp=="=" || cmp=="==" || cmp=="<=" || cmp==">=")
			if (val==value)
				passes=true;
		if (cmp=="<" || cmp=="<=" || cmp=="!=" || cmp=="<>")
			if (val<value)
				passes=true;
		if (cmp==">" || cmp==">=" || cmp=="!=" || cmp=="<>")
			if (val>value)
				passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::operator()(BWAPI::Player* player) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if ((*i)->getPlayer()==player)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::operator()(FilterAttributeBullet a, BWAPI::Unit* unit) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		Unit *target = getUnit(*i,a);
		if( target == unit )
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::operator()(BWAPI::BulletType type) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		bool passes=false;
		if ((*i)->getType()==type)
			passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::operator()(FilterAttributePositionBullet a, BWAPI::Position position) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		bool passes=false;
		if (a==GetPositionBullet)
			if ((*i)->getPosition()==position)
				passes=true;
		if (a==GetSourcePositionBullet)
			if ((*i)->getSource()->getPosition()==position)
				passes=true;
		if (a==GetTargetPositionBullet)
			if ((*i)->getTargetPosition()==position)
				passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(int f1) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (!(passesFlag(*i,f1)))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(int f1, int f2) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (!(passesFlag(*i,f1) || passesFlag(*i,f2)))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(int f1, int f2, int f3) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (!(passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3)))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(int f1, int f2, int f3, int f4) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (!(passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3) || passesFlag(*i,f4)))
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(int f1, int f2, int f3, int f4, int f5) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (!(passesFlag(*i,f1) || passesFlag(*i,f2) || passesFlag(*i,f3) || passesFlag(*i,f4) || passesFlag(*i,f5)))
			result.insert(*i);
	}
	return result;
}


BulletGroup BulletGroup::not(FliterAttributeScalarBullet a, const char* compare, double value) const
{
	BulletGroup result;
	string cmp(compare);
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		double val=getAttribute(*i,a);
		bool passes=false;
		if (cmp=="=" || cmp=="==" || cmp=="<=" || cmp==">=")
			if (val==value)
				passes=true;
		if (cmp=="<" || cmp=="<=" || cmp=="!=" || cmp=="<>")
			if (val<value)
				passes=true;
		if (cmp==">" || cmp==">=" || cmp=="!=" || cmp=="<>")
			if (val>value)
				passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::not(FliterAttributeScalarBullet a, const char* compare, int value) const
{
	BulletGroup result;
	string cmp(compare);
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		int val=(int)getAttribute(*i,a);
		bool passes=false;
		if (cmp=="=" || cmp=="==" || cmp=="<=" || cmp==">=")
			if (val==value)
				passes=true;
		if (cmp=="<" || cmp=="<=" || cmp=="!=" || cmp=="<>")
			if (val<value)
				passes=true;
		if (cmp==">" || cmp==">=" || cmp=="!=" || cmp=="<>")
			if (val>value)
				passes=true;
		if (passes)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::not(BWAPI::Player* player) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if ((*i)->getPlayer()!=player)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::not(FilterAttributeBullet a, BWAPI::Unit* unit) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		Unit *target = getUnit(*i,a);
		if( target != unit)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::not(BWAPI::BulletType type) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		bool passes=false;
		if ((*i)->getType()==type)
			passes=true;
		if (!passes)
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::not(FilterAttributePositionBullet a, BWAPI::Position position) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		bool passes=false;
		if (a==GetPositionBullet)
			if ((*i)->getPosition()==position)
				passes=true;
		if (a==GetSourcePositionBullet)
			if ((*i)->getSource()->getPosition()==position)
				passes=true;
		if (a==GetTargetPositionBullet)
			if ((*i)->getTargetPosition()==position)
				passes=true;
		if (!passes)
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::inRadius(double radius,BWAPI::Position position) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if ((*i)->getPosition().getApproxDistance(position)<=radius)
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::inRegion(BWTA::Region* region) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (BWTA::getRegion((*i)->getPosition())==region)
			result.insert(*i);
	}
	return result;
}
BulletGroup BulletGroup::onlyNearestChokepoint(BWTA::Chokepoint* choke) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (BWTA::getNearestChokepoint((*i)->getPosition())==choke)
			result.insert(*i);
	}
	return result;
}

BulletGroup BulletGroup::onlyNearestBaseLocation(BWTA::BaseLocation* location) const
{
	BulletGroup result;
	for( set<Bullet*>::const_iterator i = this->begin(); i != this->end(); ++i )
	{
		if (BWTA::getNearestBaseLocation((*i)->getPosition())==location)
			result.insert(*i);
	}
	return result;
}

BulletGroup& BulletGroup::operator+=(const BulletGroup& other)
{
	for(set<Bullet*>::const_iterator i=other.begin();i!=other.end();i++)
		this->insert(*i);
	return *this;
}
BulletGroup& BulletGroup::operator*=(const BulletGroup& other)
{
	set<Bullet*>::iterator i2;
	for(set<Bullet*>::iterator i=this->begin();i!=this->end();i=i2)
	{
		i2=i;
		i2++;
		if (!other.contains(*i))
			this->erase(*i);
	}
	return *this;
}
BulletGroup& BulletGroup::operator^=(const BulletGroup& other)
{
	BulletGroup result=*this;
	for(set<Bullet*>::const_iterator i=other.begin();i!=other.end();i++)
	{
		if (this->contains(*i))
			this->erase(*i);
		else
			this->insert(*i);
	}
	return *this;
}
BulletGroup& BulletGroup::operator-=(const BulletGroup& other)
{
	for(set<Bullet*>::const_iterator i=other.begin();i!=other.end();i++)
		this->erase(*i);
	return *this;
}

BWAPI::Bullet* BulletGroup::getNearest(BWAPI::Position position) const
{
	if (this->empty()) return NULL;
	set<Bullet*>::const_iterator i=this->begin();
	Bullet* result=*i;
	double d=(*i)->getPosition().getApproxDistance(position);
	i++;
	for(;i!=this->end();i++)
	{
		double d2=(*i)->getPosition().getApproxDistance(position);
		if (d2<d)
		{
			d=d2;
			result=*i;
		}
	}
	return result;
}

bool BulletGroup::contains(BWAPI::Bullet* u) const
{
	return this->find(u)!=this->end();
}