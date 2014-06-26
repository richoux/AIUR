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
enum FilterFlag
{
	exists = 1,
	isAccelerating,
	isAttacking,
	isBeingConstructed,
	isBeingGathered,
	isBeingHealed,
	isBlind,
	isBraking,
	isBurrowed,
	isCarryingGas,
	isCarryingMinerals,
	isCloaked,
	isCompleted,
	isConstructing,
	isDefenseMatrixed,
	isDetected,
	isEnsnared,
	isFollowing,
	isGatheringGas,
	isGatheringMinerals,
	isHallucination,
	isHoldingPosition,
	isIdle,
	isInterruptible,
	isIrradiated,
	isLifted,
	isLoaded,
	isLockedDown,
	isMaelstrommed,
	isMorphing,
	isMoving,
	isParasited,
	isPatrolling,
	isPlagued,
	isRepairing,
	isResearching,
	isSelected,
	isSieged,
	isStartingAttack,
	isStasised,
	isStimmed,
	isStuck,
	isTraining,
	isUnderStorm,
	isUnpowered,
	isUpgrading,
	isVisible,

	canProduce,
	canAttack,
	canMove,
	isFlyer,
	regeneratesHP,
	isSpellcaster,
	hasPermanentCloak,
	isInvincible,
	isOrganic,
	isMechanical,
	isRobotic,
	isDetector,
	isResourceContainer,
	isResourceDepot,
	isRefinery,
	isWorker,
	requiresPsi,
	requiresCreep,
	isTwoUnitsInOneEgg,
	isBurrowable,
	isCloakable,
	isBuilding,
	isAddon,
	isFlyingBuilding,
	isNeutral,
	isHero,
	isPowerup,
	isBeacon,
	isFlagBeacon,
	isSpecialBuilding,
	isSpell,

	Terran_Firebat,
	Terran_Ghost,
	Terran_Goliath,
	Terran_Marine,
	Terran_Medic,
	Terran_SCV,
	Terran_Siege_Tank,
	Terran_Vulture,
	Terran_Vulture_Spider_Mine,
	Terran_Battlecruiser,
	Terran_Dropship,
	Terran_Nuclear_Missile,
	Terran_Science_Vessel,
	Terran_Valkyrie,
	Terran_Wraith,
	Hero_Alan_Schezar,
	Hero_Alexei_Stukov,
	Hero_Arcturus_Mengsk,
	Hero_Edmund_Duke,
	Hero_Gerard_DuGalle,
	Hero_Gui_Montag,
	Hero_Hyperion,
	Hero_Jim_Raynor_Marine,
	Hero_Jim_Raynor_Vulture,
	Hero_Magellan,
	Hero_Norad_II,
	Hero_Samir_Duran,
	Hero_Sarah_Kerrigan,
	Hero_Tom_Kazansky,
	Terran_Civilian,
	Terran_Academy,
	Terran_Armory,
	Terran_Barracks,
	Terran_Bunker,
	Terran_Command_Center,
	Terran_Engineering_Bay,
	Terran_Factory,
	Terran_Missile_Turret,
	Terran_Refinery,
	Terran_Science_Facility,
	Terran_Starport,
	Terran_Supply_Depot,
	Terran_Comsat_Station,
	Terran_Control_Tower,
	Terran_Covert_Ops,
	Terran_Machine_Shop,
	Terran_Nuclear_Silo,
	Terran_Physics_Lab,
	Special_Crashed_Norad_II,
	Special_Ion_Cannon,
	Special_Power_Generator,
	Special_Psi_Disrupter,

	Protoss_Archon,
	Protoss_Dark_Archon,
	Protoss_Dark_Templar,
	Protoss_Dragoon,
	Protoss_High_Templar,
	Protoss_Probe,
	Protoss_Reaver,
	Protoss_Scarab,
	Protoss_Zealot,
	Protoss_Arbiter,
	Protoss_Carrier,
	Protoss_Corsair,
	Protoss_Interceptor,
	Protoss_Observer,
	Protoss_Scout,
	Protoss_Shuttle,
	Hero_Aldaris,
	Hero_Artanis,
	Hero_Danimoth,
	Hero_Dark_Templar,
	Hero_Fenix_Dragoon,
	Hero_Fenix_Zealot,
	Hero_Gantrithor,
	Hero_Mojo,
	Hero_Raszagal,
	Hero_Tassadar,
	Hero_Tassadar_Zeratul_Archon,
	Hero_Warbringer,
	Hero_Zeratul,
	Protoss_Arbiter_Tribunal,
	Protoss_Assimilator,
	Protoss_Citadel_of_Adun,
	Protoss_Cybernetics_Core,
	Protoss_Fleet_Beacon,
	Protoss_Forge,
	Protoss_Gateway,
	Protoss_Nexus,
	Protoss_Observatory,
	Protoss_Photon_Cannon,
	Protoss_Pylon,
	Protoss_Robotics_Facility,
	Protoss_Robotics_Support_Bay,
	Protoss_Shield_Battery,
	Protoss_Stargate,
	Protoss_Templar_Archives,
	Special_Khaydarin_Crystal_Form,
	Special_Protoss_Temple,
	Special_Stasis_Cell_Prison,
	Special_Warp_Gate,
	Special_XelNaga_Temple,

	Zerg_Broodling,
	Zerg_Defiler,
	Zerg_Drone,
	Zerg_Egg,
	Zerg_Hydralisk,
	Zerg_Infested_Terran,
	Zerg_Larva,
	Zerg_Lurker,
	Zerg_Lurker_Egg,
	Zerg_Ultralisk,
	Zerg_Zergling,
	Zerg_Cocoon,
	Zerg_Devourer,
	Zerg_Guardian,
	Zerg_Mutalisk,
	Zerg_Overlord,
	Zerg_Queen,
	Zerg_Scourge,
	Hero_Devouring_One,
	Hero_Hunter_Killer,
	Hero_Infested_Duran,
	Hero_Infested_Kerrigan,
	Hero_Kukulza_Guardian,
	Hero_Kukulza_Mutalisk,
	Hero_Matriarch,
	Hero_Torrasque,
	Hero_Unclean_One,
	Hero_Yggdrasill,
	Zerg_Creep_Colony,
	Zerg_Defiler_Mound,
	Zerg_Evolution_Chamber,
	Zerg_Extractor,
	Zerg_Greater_Spire,
	Zerg_Hatchery,
	Zerg_Hive,
	Zerg_Hydralisk_Den,
	Zerg_Infested_Command_Center,
	Zerg_Lair,
	Zerg_Nydus_Canal,
	Zerg_Queens_Nest,
	Zerg_Spawning_Pool,
	Zerg_Spire,
	Zerg_Spore_Colony,
	Zerg_Sunken_Colony,
	Zerg_Ultralisk_Cavern,
	Special_Cerebrate,
	Special_Cerebrate_Daggoth,
	Special_Mature_Chrysalis,
	Special_Overmind,
	Special_Overmind_Cocoon,
	Special_Overmind_With_Shell,

	Critter_Bengalaas,
	Critter_Kakaru,
	Critter_Ragnasaur,
	Critter_Rhynadon,
	Critter_Scantid,
	Critter_Ursadon,

	Resource_Mineral_Field,
	Resource_Vespene_Geyser,

	Spell_Dark_Swarm,
	Spell_Disruption_Web,
	Spell_Scanner_Sweep,

	Special_Protoss_Beacon,
	Special_Protoss_Flag_Beacon,
	Special_Terran_Beacon,
	Special_Terran_Flag_Beacon,
	Special_Zerg_Beacon,
	Special_Zerg_Flag_Beacon,

	Powerup_Data_Disk,
	Powerup_Flag,
	Powerup_Khalis_Crystal,
	Powerup_Khaydarin_Crystal,
	Powerup_Psi_Emitter,
	Powerup_Uraj_Crystal,
	Powerup_Young_Chrysalis,

	None,
	Unknown_Unit
};
enum FliterAttributeScalar
{
	HitPoints,
	InitialHitPoints,
	Shields,
	Energy,
	Resources,
	InitialResources,
	KillCount,
	GroundWeaponCooldown,
	AirWeaponCooldown,
	SpellCooldown,
	DefenseMatrixPoints,
	DefenseMatrixTimer,
	EnsnareTimer,
	IrradiateTimer,
	LockdownTimer,
	MaelstromTimer,
	PlagueTimer,
	RemoveTimer,
	StasisTimer,
	StimTimer,
	PositionX,
	PositionY,
	InitialPositionX,
	InitialPositionY,
	TilePositionX,
	TilePositionY,
	InitialTilePositionX,
	InitialTilePositionY,
	Angle,
	VelocityX,
	VelocityY,
	TargetPositionX,
	TargetPositionY,
	OrderTimer,
	RemainingBuildTime,
	RemainingTrainTime,
	TrainingQueueCount,
	LoadedUnitsCount,
	InterceptorCount,
	ScarabCount,
	SpiderMineCount,
	RemainingResearchTime,
	RemainingUpgradeTime,
	RallyPositionX,
	RallyPositionY,
	maxAirHits,
	maxGroundHits
};

enum FilterAttributeUnit
{
	GetTarget,
	GetOrderTarget,
	GetBuildUnit,
	GetTransport,
	GetRallyUnit,
	GetAddon
};
enum FilterAttributeType
{
	GetType,
	GetInitialType,
	GetBuildType,
	GetTech,
	GetUpgrade
};

enum FilterAttributePosition
{
	GetPosition,
	GetInitialPosition,
	GetTargetPosition,
	GetRallyPosition
};

enum FilterAttributeTilePosition
{
	GetTilePosition,
	GetInitialTilePosition,
};
enum FilterAttributeOrder
{
	GetOrder,
	GetSecondaryOrder
};


class UnitGroup : public std::set<BWAPI::Unit*>
{
public:
	static UnitGroup getUnitGroup(const std::set<BWAPI::Unit*> units)
	{
		UnitGroup u;
		u.insert(units.begin(), units.end());
		return u;
	}

	UnitGroup operator+(const UnitGroup& other) const;//union
	UnitGroup operator*(const UnitGroup& other) const;//intersection
	UnitGroup operator^(const UnitGroup& other) const;//symmetric difference
	UnitGroup operator-(const UnitGroup& other) const;//difference
	UnitGroup inRadius(double radius,BWAPI::Position position) const;
	UnitGroup inRegion(BWTA::Region* region) const;
	UnitGroup onlyNearestChokepoint(BWTA::Chokepoint* choke) const;
	UnitGroup onlyNearestBaseLocation(BWTA::BaseLocation* location) const;
	UnitGroup onlyNearestUnwalkablePolygon(BWTA::Polygon* polygon) const;
	UnitGroup operator()(int f1) const;
	UnitGroup operator()(int f1, int f2) const;
	UnitGroup operator()(int f1, int f2, int f3) const;
	UnitGroup operator()(int f1, int f2, int f3, int f4) const;
	UnitGroup operator()(int f1, int f2, int f3, int f4, int f5) const;
	UnitGroup operator()(FliterAttributeScalar a, const char* compare, double value) const;
	UnitGroup operator()(FliterAttributeScalar a, const char* compare, int value) const;
	UnitGroup operator()(BWAPI::Player* player) const;
	UnitGroup operator()(FilterAttributeUnit a, BWAPI::Unit* unit) const;
	UnitGroup operator()(FilterAttributeType a, BWAPI::UnitType type) const;
	UnitGroup operator()(FilterAttributeType a, BWAPI::TechType type) const;
	UnitGroup operator()(FilterAttributeType a, BWAPI::UpgradeType type) const;
	UnitGroup operator()(FilterAttributeOrder a, BWAPI::Order type) const;
	UnitGroup operator()(FilterAttributePosition a, BWAPI::Position position) const;
	UnitGroup operator()(FilterAttributeTilePosition a, BWAPI::TilePosition position) const;

	UnitGroup not(int f1) const;
	UnitGroup not(int f1, int f2) const;
	UnitGroup not(int f1, int f2, int f3) const;
	UnitGroup not(int f1, int f2, int f3, int f4) const;
	UnitGroup not(int f1, int f2, int f3, int f4, int f5) const;
	UnitGroup not(FliterAttributeScalar a, const char* compare, double value) const;
	UnitGroup not(FliterAttributeScalar a, const char* compare, int value) const;
	UnitGroup not(BWAPI::Player* player) const;
	UnitGroup not(FilterAttributeUnit a, BWAPI::Unit* unit) const;
	UnitGroup not(FilterAttributeType a, BWAPI::UnitType type) const;
	UnitGroup not(FilterAttributeType a, BWAPI::TechType type) const;
	UnitGroup not(FilterAttributeType a, BWAPI::UpgradeType type) const;
	UnitGroup not(FilterAttributeOrder a, BWAPI::Order type) const;
	UnitGroup not(FilterAttributePosition a, BWAPI::Position position) const;
	UnitGroup not(FilterAttributeTilePosition a, BWAPI::TilePosition position) const;

	UnitGroup& operator+=(const UnitGroup& other);//union
	UnitGroup& operator*=(const UnitGroup& other);//intersection
	UnitGroup& operator^=(const UnitGroup& other);//symmetric difference
	UnitGroup& operator-=(const UnitGroup& other);//difference
	BWAPI::Unit* getNearest(BWAPI::Position position) const;
	bool contains(BWAPI::Unit* u) const;
	BWAPI::Position getCenter() const;

	bool attack(BWAPI::Position position, bool enqueue=false) const;
	bool attack(BWAPI::Unit* target, bool enqueue=false) const;
	bool rightClick(BWAPI::Position position) const;
	bool rightClick(BWAPI::Unit* target) const;
	bool train(BWAPI::UnitType type) const;
	bool build(BWAPI::TilePosition position, BWAPI::UnitType type) const;
	bool buildAddon(BWAPI::UnitType type) const;
	bool research(BWAPI::TechType tech) const;
	bool upgrade(BWAPI::UpgradeType upgrade) const;
	bool stop() const;
	bool holdPosition() const;
	bool patrol(BWAPI::Position position) const;
	bool follow(BWAPI::Unit* target) const;
	bool setRallyPoint(BWAPI::Position target) const;
	bool setRallyPoint(BWAPI::Unit* target) const;
	bool repair(BWAPI::Unit* target) const;
	bool morph(BWAPI::UnitType type) const;
	bool burrow() const;
	bool unburrow() const;
	bool siege() const;
	bool unsiege() const;
	bool cloak() const;
	bool decloak() const;
	bool lift() const;
	bool land(BWAPI::TilePosition position) const;
	bool load(BWAPI::Unit* target) const;
	bool unload(BWAPI::Unit* target) const;
	bool unloadAll() const;
	bool unloadAll(BWAPI::Position position) const;
	bool cancelConstruction() const;
	bool haltConstruction() const;
	bool cancelMorph() const;
	bool cancelTrain() const;
	bool cancelTrain(int slot) const;
	bool cancelAddon() const;
	bool cancelResearch() const;
	bool cancelUpgrade() const;
	bool useTech(BWAPI::TechType tech) const;
	bool useTech(BWAPI::TechType tech, BWAPI::Position position) const;
	bool useTech(BWAPI::TechType tech, BWAPI::Unit* target) const;
};

