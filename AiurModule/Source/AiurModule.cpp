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

#include "AiurModule.h"
#include "../src/Util.h"
#include <ctime>
#include <string>

using namespace BWAPI;

void AiurModule::onStart()
{
	showManagerAssignments=false;
	if (Broodwar->isReplay()) return;

	Broodwar->setTextSize(0);
	Broodwar->setLocalSpeed(0);

	// Thanks for the trick, Gabriel!
	//Broodwar->setLatCom(false);
	Broodwar->setCommandOptimizationLevel(1);

	// Enable some cheat flags
	Broodwar->enableFlag(Flag::UserInput);
	// Broodwar->enableFlag(Flag::CompleteMapInformation);

	BWTA::readMap();
	BWTA::analyze();
	analyzed = true;

	unitStates			= new UnitStates();
	buildManager		= new BuildManager(&arbitrator);
	techManager			= new TechManager(&arbitrator);
	upgradeManager		= new UpgradeManager(&arbitrator);
	spyManager			= new SpyManager(&arbitrator, unitStates);
	workerManager		= new WorkerManager(&arbitrator);
	supplyManager		= new SupplyManager();
	spendManager		= new SpendManager();
	baseManager			= new BaseManager();
	buildOrderManager	= new BuildOrderManager(buildManager, techManager, upgradeManager, workerManager, supplyManager, baseManager);
	defenseManager		= new DefenseManager(&arbitrator);
	underAttackManager	= new UnderAttackManager(&arbitrator, unitStates);
	armyManager			= new ArmyManager(&arbitrator, unitStates);
	rushPhotonManager	= new RushPhotonManager( &arbitrator, unitStates );
	dropZealotsManager	= new DropZealotsManager( &arbitrator, unitStates );
	informationManager	= InformationManager::create();
	scoutManager		= new ScoutManager(&arbitrator, informationManager, buildManager, unitStates);
	borderManager		= BorderManager::create();
	unitGroupManager	= UnitGroupManager::create();
	attackTracker		= AttackTracker::create();
	//attackUnitPlanner	= AttackUnitPlanner::create();
	moodManager			= new MoodManager();
	openingManager		= new OpeningManager();
	photonManager		= new PhotonManager();
	dragoonManager		= new DragoonManager(unitStates);
	enhancedUI			= new EnhancedUI();
	randomExpand		= new Random(2000);

	timeToExpand		= randomExpand->nextInt();

	buildOrderManager->enableDependencyResolver();

	scoutManager->setArmyManager(armyManager);

	underAttackManager->setArmyManager		( armyManager );
	underAttackManager->setDefenseManager	( defenseManager );
	underAttackManager->setBaseManager		( baseManager );
	underAttackManager->setScoutManager		( scoutManager );
	underAttackManager->setSpyManager		( spyManager );
	underAttackManager->setDragoonManager	( dragoonManager );
	underAttackManager->setMoodManager		( moodManager );
	underAttackManager->setRushPhotonManager( rushPhotonManager );
	//underAttackManager->setAttackUnitPlanner( attackUnitPlanner );

	supplyManager->setBuildManager			( buildManager );
	supplyManager->setBuildOrderManager		( buildOrderManager );

	spendManager->setBuildManager			( buildManager );
	spendManager->setBaseManager			( baseManager );
	spendManager->setBuildOrderManager		( buildOrderManager );
	spendManager->setMoodManager			( moodManager );
	spendManager->setArmyManager			( armyManager );
	spendManager->setDefenseManager			( defenseManager );
	spendManager->setDragoonManager			( dragoonManager );
	spendManager->setUnderAttackManager		( underAttackManager );
	spendManager->setInformationManager		( informationManager );

	techManager->setBuildingPlacer			( buildManager->getBuildingPlacer() );

	upgradeManager->setBuildingPlacer		( buildManager->getBuildingPlacer() );

	workerManager->setBaseManager			( baseManager );
	workerManager->setBuildOrderManager		( buildOrderManager );
	workerManager->setMoodManager			( moodManager );

	baseManager->setBuildOrderManager		( buildOrderManager );
	baseManager->setBuildManager			( buildManager );
	baseManager->setBorderManager			( borderManager );
	baseManager->setMoodManager				( moodManager );

	defenseManager->setBorderManager		( borderManager );
	defenseManager->setBuildOrderManager	( buildOrderManager );
	defenseManager->setBaseManager			( baseManager );
	defenseManager->setInformationManager	( informationManager );
	defenseManager->setSpyManager			( spyManager );
	defenseManager->setDragoonManager		( dragoonManager );
	//defenseManager->setAttackUnitPlanner	( attackUnitPlanner );

	spyManager->setInformationManager		( informationManager );
	spyManager->setBuildOrderManager		( buildOrderManager );
	spyManager->setMoodManager				( moodManager );

	armyManager->setDefenseManager			( defenseManager );
	armyManager->setInformationManager		( informationManager );
	armyManager->setBaseManager				( baseManager );
	armyManager->setMoodManager				( moodManager );
	armyManager->setDragoonManager			( dragoonManager );
	//armyManager->setAttackUnitPlanner		( attackUnitPlanner );

	moodManager->setInformationManager		( informationManager );

	openingManager->setMoodManager			( moodManager );
	openingManager->setBuildOrderManager	( buildOrderManager );
	openingManager->setBaseManager			( baseManager );

	dragoonManager->setInformationManager	( informationManager );
	//dragoonManager->setAttackUnitPlanner	( attackUnitPlanner );

	photonManager->setBaseManager			( baseManager );
	photonManager->setBuildManager			( buildManager );
	photonManager->setBuildOrderManager		( buildOrderManager );

	rushPhotonManager->setBuildOrderManager	( buildOrderManager );
	rushPhotonManager->setInformationManager( informationManager );
	rushPhotonManager->setWorkerManager		( workerManager );

	dropZealotsManager->setBuildOrderManager( buildOrderManager );
	dropZealotsManager->setInformationManager( informationManager );
	dropZealotsManager->setWorkerManager	( workerManager );
	dropZealotsManager->setMoodManager		( moodManager );
	dropZealotsManager->setBaseManager		( baseManager );

	lastFrameExpand				= 0;
	lastScout					= 0;
	debugMode					= false;
	debugAttackerMode			= false;
	debugTimersMode				= false;
	debugUI						= false;
	debugState					= false;
	debugOrder					= false;
	cleaningPath				= false;
	destinationMineralSpotted	= false;
	ggAnswered					= false;
	ggFrame						= -1;

	Race race			= Broodwar->self()->getRace();
	Race enemyRace		= Broodwar->enemy()->getRace();
	UnitType workerType	= race.getWorker();
	home				= BWTA::getStartLocation(Broodwar->self());
	closest				= NULL;


	/******************************/
	/** Begin of common code for **/
	/** persistent file systems  **/
	/******************************/
	// Number of moods - Defensive = 5
	const int dataSizeCopy = moodManager->getNumberMoods() * 2 + 1;
	dataGameCopy = new int[dataSizeCopy];

	// dataSize = ( 5 moods * 2 (#wins; #lose) + 1 (#games) ) * 7 map sizes (ie 2p, ..., 8p) = 88		
	const int dataSize = 7 * dataSizeCopy; 
	dataGame = new int[dataSize];

	// shift needed for I/O files data
	int shift = BWTA::getStartLocations().size() - 2;

	// boolean to skip mood initialization if IO error
	IOErrorOccurred = false;

	// IO files name
	string afterGameFilename;
	string inGameFilename;
	string nameEnemy;


	// StarCraft maps are from 2 till 8 players.
	// But just in case some dude create a map 
	// with more than 8 starting spots
	if( BWTA::getStartLocations().size() > 8 )
	{
		moodManager->initialize();
		// There are no IO errors here, but it is just
		// to avoid wrinting things into the write file
		// at the end of the game
		IOErrorOccurred = true;
	}
	else
	{

		/**************************/
		/**   Persistent files   **/
		/**    for AIIDE / CIG   **/
		/**************************/
		// Read/Write data
		directories.open( (char*)"bwapi-data\\directories.txt", std::ifstream::in );
		if( directories.is_open() && directories.peek() != std::ifstream::traits_type::eof() )
		{
			// Read file
			char pathRead[256];
			directories >> pathRead;
			afterGameFilename = pathRead;
			nameEnemy = Broodwar->enemy()->getName();
			//afterGameFilename += nameEnemy.substr(0, nameEnemy.find("_"));
			afterGameFilename += nameEnemy;

			// Write file
			char pathWrite[256];
			directories >> pathWrite;
			inGameFilename = pathWrite;
			//inGameFilename += nameEnemy.substr(0, nameEnemy.find("_"));
			inGameFilename += nameEnemy;
		}
		// directories.txt is not open or is empty
		else
		{
			moodManager->initialize();
			IOErrorOccurred = true;
		}


		/**************************/
		/**   Persistent files   **/
		/**      for SSCAI       **/
		/**************************/
		//// Read file
		//afterGameFilename = "bwapi-data\\read\\AIUR_";
		//nameEnemy = Broodwar->enemy()->getName();
		//afterGameFilename += nameEnemy + ".txt";

		//// Write file
		//inGameFilename = "bwapi-data\\write\\AIUR_";
		//inGameFilename += nameEnemy + ".txt";


		/**************************/
		/**   Persistent files   **/
		/**   for the BW Ladder  **/
		/**************************/
		//// Read file
		//afterGameFilename = "bwapi-data\\read\\opponent";

		//// Write file
		//inGameFilename = "bwapi-data\\write\\opponent";


		/*****************************/
		/**     End of different    **/
		/** persistent file systems **/
		/*****************************/

		// Take data into consideration to modify 
		// the probability distribution of the mood selection
		if( !IOErrorOccurred )
		{
			afterGame.open( (char*)afterGameFilename.c_str(), std::ifstream::in );

			// If the read-file is empty
			if( afterGame.peek() == std::ifstream::traits_type::eof() )
			{
				for( int i = 0; i < dataSize; ++i )
					dataGame[i] = 0;
			}
			else
			{
				int i = 0;
				while( afterGame >> dataGame[i])
					++i;
			}

			int numberOfGames = dataGame[dataSizeCopy * shift];

			// After 1000 games, stop collecting statistic data
			if( numberOfGames < 1000 )
			{
				inGame.open( (char*)inGameFilename.c_str(), std::ofstream::out );

				inGame.seekp( 0 );
				for( int i = 0; i < dataSize; ++i )
				{
					inGame << dataGame[i] << endl;
					inGame.flush();
				}
			}

			// Pick-up a mood. Change the uniformed distribution after 20 games
			//if( numberOfGames >= 10 )
			//{
				for( int i = 0; i < dataSizeCopy; ++i )
					dataGameCopy[i] = dataGame[i + dataSizeCopy * shift];

				if( numberOfGames >= moodManager->getNumberMoods() )
					moodManager->initialize( dataGameCopy );
				else
					moodManager->initializeRoundRobin( dataGameCopy );
			//}
			//else
			//{
			//	moodManager->initialize( (MoodManager::MoodData::Mood)(numberOfGames % 5) );
			//}
		}

		/***********************/
		/** End of persistent **/
		/** file system code  **/
		/***********************/

	} // end of |map| > 8

	// First, if we play against Zerg and we choose to FE, take another chance to not do so! (unless we play Aztec or Andromeda)
	//if( Broodwar->mapHash() != hashMap.hash( "Andromeda" ) && Broodwar->mapHash() != hashMap.hash( "Aztec" ) &&	
	//		enemyRace == Races::Zerg && moodManager->getMood() == MoodManager::MoodData::FastExpo )
	//{
	//	moodManager->setRandomMood();
	//}
	
	// Second, if we play on Aztec or Andromeda and mood is not FE, force 50% chance to FE - even if the enemy is Zerg
	//if( ( Broodwar->mapHash() == hashMap.hash( "Andromeda" ) || Broodwar->mapHash() == hashMap.hash( "Aztec" ) ) && moodManager->getMood() != MoodManager::MoodData::FastExpo )
	//{
	//	Random *doFE = new Random( 100 );
	//	if( doFE->nextInt() < 50 )
	//	{
	//		moodManager->setMood( MoodManager::MoodData::FastExpo );
	//	}
	//	delete doFE;
	//}

	// Third, if we play on a 2-player map, force 50% chance to cheese, rush or being aggressive if we haven't chosen one of these strategies.
	//if( BWTA::getStartLocations().size() == 2 && 
	//		moodManager->getMood() != MoodManager::MoodData::Cheese && 
	//		moodManager->getMood() != MoodManager::MoodData::Rush && 
	//		moodManager->getMood() != MoodManager::MoodData::Agressive )
	//{
	//	Random *agressivePlay	= new Random( 100 );
	//	Random *headTail		= new Random( 2 );
	//	if( agressivePlay->nextInt() < 50 )
	//	{
	//		int choice = headTail->nextInt();
	//		if (choice == 0)
	//			if( enemyRace == Races::Zerg )
	//				moodManager->setMood(MoodManager::MoodData::Rush);
	//			else
	//				moodManager->setMood(MoodManager::MoodData::Cheese);
	//		else
	//			moodManager->setMood(MoodManager::MoodData::Agressive);
	//	}
	//	delete headTail;
	//	delete agressivePlay;
	//}

	// At the opposite, if we are on a huge map (4-player map), try to avoid agressive plays.
	//if ( BWTA::getStartLocations().size() == 4 &&
	//	( moodManager->getMood() == MoodManager::MoodData::Cheese || 
	//	  moodManager->getMood() == MoodManager::MoodData::Rush || 
	//	  moodManager->getMood() == MoodManager::MoodData::Agressive ) )
	//{
	//	moodManager->setAnotherRandomMood();
	//}

	// Last, if we play on Destination, we don't FE - mining the fake mineral doesn't work...
	//if( Broodwar->mapHash() == hashMap.hash("Destination") && moodManager->getMood() == MoodManager::MoodData::FastExpo )
	//{
	//	moodManager->setAnotherRandomMood();
	//}

	// Defensive mood is only now to face a Zerg rush. do not select it as a starting mood.
	if( moodManager->getMood() == MoodManager::MoodData::Defensive )
	{
		moodManager->setAnotherRandomMood();
	}

	buildManager->setDebugMode(true);
	scoutManager->setDebugMode(false);
	spyManager->setDebugMode(false);
	buildOrderManager->setDebugMode(false);

	openingManager->initialize();

	if( moodManager->getMood() != MoodManager::MoodData::Cheese )
	{
		workerManager->enableAutoBuild();
		workerManager->setAutoBuildPriority(100);
		if( moodManager->getMood() != MoodManager::MoodData::Rush )
			baseManager->setRefineryBuildPriority(30);
	}

	hasSwitchedToDefensive = false;
	top = 0;
	sup55 = 0;
	diffUI = -42;
	diffWorker = -42;
	diffBuild = -42;
	diffBuildOrder = -42;
	diffBase = -42;
	diffUA = -42;
	diffAT = -42;
	diffArbitrator = -42;
	diffRush = -42;
	diffSupply = -42;
	diffTech = -42;
	diffUpgrade = -42;
	diffMood = -42;
	diffSpend = -42;
	diffScout = -42;
	diffDefense = -42;
	diffSpy = -42;
	diffBorder = -42;
	diffArmy = -42;
	diffDragoon = -42;
	diffPhoton = -42;
	diffDrop = -42;
}

AiurModule::~AiurModule()
{
	delete rushPhotonManager;
	delete dropZealotsManager;
	delete buildManager;
	delete techManager;
	delete upgradeManager;
	delete scoutManager;
	delete spyManager;
	delete workerManager;
	delete supplyManager;
	delete spendManager;
	delete buildOrderManager;
	delete baseManager;
	delete defenseManager;
	delete underAttackManager;
	delete armyManager;
	delete moodManager;
	delete randomExpand;
	delete photonManager;
	delete dragoonManager;
	InformationManager::destroy();
	BorderManager::destroy();
	UnitGroupManager::destroy();
	AttackTracker::destroy();
	//AttackUnitPlanner::destroy();
	delete unitStates;
	delete enhancedUI;
}
void AiurModule::onEnd(bool isWinner)
{
	if( !Broodwar->isReplay() && !IOErrorOccurred )
	{
		if( inGame.is_open() )
		{
			const int dataSizeCopy = moodManager->getNumberMoods() * 2 + 1;
			int shift = BWTA::getStartLocations().size() - 2;
			++dataGame[dataSizeCopy * shift];
			int win		= isWinner ? 0 : 1;
			int index	= moodManager->getFirstMoodRank()*2 + win + 1 + ( dataSizeCopy * shift );

			// DEPRECATED: Take only into account games where we didn't win after switching to Defensive mood.
			//if( !hasSwitchedToDefensive || !isWinner )
			++dataGame[index];

			// Write data
			// dataSize = ( 5 moods * 2 (#wins; #lose) + 1 (#games) ) * 7 map sizes (ie 2p, ..., 8p) = 88
			const int dataSize = 7 * ( moodManager->getNumberMoods() * 2 + 1 );

			inGame.seekp( 0 );
			
			for( int i = 0; i < dataSize; ++i )
			{
				inGame << dataGame[i] << endl;
				inGame.flush();
			}

			inGame.close();
		}

		if( afterGame.is_open() )
		{
			afterGame.close();
			delete [] dataGameCopy;
			delete [] dataGame;
		}

		directories.close();
	}

	log("onEnd(%d)\n",isWinner);
}
void AiurModule::onFrame()
{
	if (debugMode)
	{
		compteur.Start();
		informationManager->debugCheckpoint();
	}

	if( debugTimersMode )
		compteur.Start();

 /******************/

	if (Broodwar->isReplay()) return;
	if (!analyzed) return;

	if( Broodwar->getFrameCount() == 200 )
		Broodwar->sendText("gg hf");

	if( !ggAnswered 
		&& 
		ggFrame > -1
		&&
		Broodwar->getFrameCount() == ggFrame + 24 )
	{
		Broodwar->sendText("gg");
		ggAnswered = true;
	}

	// Update WorkerManager
	if( debugTimersMode )
		timeWorker.Start();
	workerManager->update();
	if( debugTimersMode )
		diffWorker = timeWorker.GetTimeFromStart();

	// Update BuildManager
	if( debugTimersMode )
		timeBuild.Start();
	buildManager->update();
	if( debugTimersMode )
		diffBuild = timeBuild.GetTimeFromStart();

	// Update BuildOrderManager
	if( debugTimersMode )
		timeBuildOrder.Start();
	buildOrderManager->update();
	if( debugTimersMode )
		diffBuildOrder = timeBuildOrder.GetTimeFromStart();

	// Update BaseManager
	if( debugTimersMode )
		timeBase.Start();
	baseManager->update();
	if( debugTimersMode )
		diffBase = timeBase.GetTimeFromStart();

	// Update Enhanced UI
	if( debugUI )
	{
		if( debugTimersMode )
			timeUI.Start();
		enhancedUI->update();
		if( debugTimersMode )
			diffUI = timeUI.GetTimeFromStart();
	}

	// Update UnderAttackManager
	if( debugTimersMode )
		timeUA.Start();
	underAttackManager->update();
	if( debugTimersMode )
		diffUA = timeUA.GetTimeFromStart();

	// Update AttackTracker
	if( debugTimersMode )
		timeAT.Start();
	attackTracker->update();
	if( debugTimersMode )
		diffAT = timeAT.GetTimeFromStart();

	// Update Arbitrator
	if( debugTimersMode )
		timeArbitrator.Start();
	arbitrator.update();
	if( debugTimersMode )
		diffArbitrator = timeArbitrator.GetTimeFromStart();

	if( moodManager->getMood() == MoodManager::MoodData::Cheese && Broodwar->getFrameCount() > 8000 )
	{
		buildOrderManager->build(2,BWAPI::UnitTypes::Protoss_Gateway,100);
		buildOrderManager->build(8,BWAPI::UnitTypes::Protoss_Zealot,95);
		if( armyManager->myDPStoGround() > 5 )
			moodManager->setMood( MoodManager::MoodData::Rush );
		rushPhotonManager->loseControl();
	}

	if( moodManager->getMood() == MoodManager::MoodData::Cheese )
	{
		// Update RushPhotonManager
		if( debugTimersMode )
			timeRush.Start();
		rushPhotonManager->update();
		if( debugTimersMode )
			diffRush = timeRush.GetTimeFromStart();
	}
	else if( moodManager->getMood() == MoodManager::MoodData::Agressive )
	{
		// Update DropZealotsManager
		if( debugTimersMode )
			timeDrop.Start();
		dropZealotsManager->update();
		if( debugTimersMode )
			diffDrop = timeDrop.GetTimeFromStart();

		// Update SupplyManager for Drop
		if( debugTimersMode )
			timeSupply.Start();
		supplyManager->update();
		if( debugTimersMode )
			diffSupply = timeSupply.GetTimeFromStart();

		if (informationManager->isEnemySpotted() && Broodwar->getFrameCount() > 3000)
		{
			// Update DefenseManager for Drop
			if( debugTimersMode )
				timeDefense.Start();
			defenseManager->update();
			if( debugTimersMode )
				diffDefense = timeDefense.GetTimeFromStart();
		}

	}
	else
	{
		// Update SupplyManager
		if( debugTimersMode )
			timeSupply.Start();
		supplyManager->update();
		if( debugTimersMode )
			diffSupply = timeSupply.GetTimeFromStart();

		// Update TechManager
		if( debugTimersMode )
			timeTech.Start();
		techManager->update();
		if( debugTimersMode )
			diffTech = timeTech.GetTimeFromStart();

		// Update UpgradeManager
		if( debugTimersMode )
			timeUpgrade.Start();
		upgradeManager->update();
		if( debugTimersMode )
			diffUpgrade = timeUpgrade.GetTimeFromStart();

		// Update MoodManager
		if( debugTimersMode )
			timeMood.Start();
		moodManager->update();
		if( debugTimersMode )
			diffMood = timeMood.GetTimeFromStart();

		// Update SpendManager
		if( debugTimersMode )
			timeSpend.Start();
		spendManager->update();
		if( debugTimersMode )
			diffSpend = timeSpend.GetTimeFromStart();

		// Update ScoutManager
		if( debugTimersMode )
			timeScout.Start();
		scoutManager->update();
		if( debugTimersMode )
			diffScout = timeScout.GetTimeFromStart();

		if (informationManager->isEnemySpotted() && Broodwar->getFrameCount() > 3000)
		{
			// Update DefenseManager
			if( debugTimersMode )
				timeDefense.Start();
			defenseManager->update();
			if( debugTimersMode )
				diffDefense = timeDefense.GetTimeFromStart();

			if (!spyManager->isInitialized())
				spyManager->initialize();

			// Update SpyManager
			if( debugTimersMode )
				timeSpy.Start();
			spyManager->update();
			if( debugTimersMode )
				diffSpy = timeSpy.GetTimeFromStart();
		}

		// Update BorderManager
		if( debugTimersMode )
			timeBorder.Start();
		borderManager->update();
		if( debugTimersMode )
			diffBorder = timeBorder.GetTimeFromStart();

		if (!armyManager->isInitialized())
			armyManager->initialize();

		// Update ArmyManager
		if( debugTimersMode )
			timeArmy.Start();
		armyManager->update();
		if( debugTimersMode )
			diffArmy = timeArmy.GetTimeFromStart();

		// Update DragoonManager
		if( debugTimersMode )
			timeDragoon.Start();
		dragoonManager->update();
		if( debugTimersMode )
			diffDragoon = timeDragoon.GetTimeFromStart();
		
		if( moodManager->getPreviousMood() != MoodManager::MoodData::Cheese )
		{
			// Update PhotonManager
			if( debugTimersMode )
				timePhoton.Start();
			photonManager->update();
			if( debugTimersMode )
				diffPhoton = timePhoton.GetTimeFromStart();
		}

		if (Broodwar->mapHash() == hashMap.hash("Destination") && !destinationMineralSpotted)
		{
			destinationMineralSpotted = true;
			Position *p;

			//Low position
			if (Broodwar->self()->getStartLocation().y() > 50)
			{
				p = new Position(1312,3856);
			}
			//High position
			else
			{
				p = new Position(1760,208);
			}
			for each (Unit *u in Broodwar->getAllUnits())
			{
				if ((u->getType() == UnitTypes::Resource_Mineral_Field) && (u->getPosition().x() == p->x()) && (u->getPosition().y() == p->y()))
				{
					workerManager->setDestinationMineral(u);
					break;
				}
			}
			delete p;
		}

		if (Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe) == 7 && 
			Broodwar->self()->visibleUnitCount(UnitTypes::Protoss_Pylon) == 0)
		{
			TilePosition start	= Broodwar->self()->getStartLocation();
			BWTA::BaseLocation *bl		= BWTA::getNearestBaseLocation(start);
			UnitGroup minerals			= UnitGroup::getUnitGroup(bl->getMinerals());
			UnitGroup geysers			= UnitGroup::getUnitGroup(bl->getGeysers());
			Position posMineral	= minerals.getCenter();
			Position posGeyser	= geysers.getCenter();

			TilePosition tileMineral(posMineral);
			TilePosition tileGeyser(posGeyser);
			bool up = true, right = true, down = true, left = true;

			// minerals or geysers to the left
			if (tileMineral.x() <= start.x() || tileGeyser.x() <= start.x())
			{
				left = false;
			}

			// minerals or geysers to the right
			if (tileMineral.x() >= start.x() + 4 || tileGeyser.x() >= start.x() + 4)
			{
				right = false;
			}

			// minerals or geysers to the top
			if (tileMineral.y() <= start.y() || tileGeyser.y() <= start.y())
			{
				up = false;
			}

			// minerals or geysers to the bottom
			if (tileMineral.y() >= start.y() + 3 || tileGeyser.y() >= start.y() + 3)
			{
				down = false;
			}

			// if we are surrounded by minerals and geysers, choose to build in north.
			if (!left && !right && !down && !up)
				up = true;

			TilePosition *tilePylon;
			if (up)
			{
				tilePylon = new TilePosition(start.x() + 4, start.y() - 3);
				buildManager->getBuildingPlacer()->reserveTiles(TilePosition(start.x() + 1, start.y() - 3), 2, 2);
			}
			else if (right)
			{
				tilePylon = new TilePosition(start.x() + 5, start.y() + 4);
				buildManager->getBuildingPlacer()->reserveTiles(TilePosition(start.x() + 5, start.y() + 1), 2, 2);
			}
			else if (down)
			{
				tilePylon = new TilePosition(start.x() - 2, start.y() + 4);
				buildManager->getBuildingPlacer()->reserveTiles(TilePosition(start.x() + 1, start.y() + 4), 2, 2);
			}
			else if (left)
			{
				tilePylon = new TilePosition(start.x() - 3, start.y() - 2);
				buildManager->getBuildingPlacer()->reserveTiles(TilePosition(start.x() - 3, start.y() + 1), 2, 2);
			}
			buildOrderManager->build(1, UnitTypes::Protoss_Pylon, 1000, *tilePylon);
			delete tilePylon;
		}

		// OLD DEFENSIVE CRITERION
		// if before 3000 frames, #enemy's worker + 4 =< #our workers
		// OR
		// if at 4000/2600/5300 frames resp. for Protoss/Terran/Zerg, the enemy does not built any gas refinery
		// OR
		// if our enemy is Zerg and we do not scout its base before the first 2200 frames
		// then get prepared for a rush! ==> defensive mode
		//if ( moodManager->getMood() != MoodManager::MoodData::Defensive &&
		//	informationManager->getNumberEnemyBases() <= 1 &&
		//	( ( Broodwar->getFrameCount() < 3000 && informationManager->canSeeEnemyPatches() && Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe) >= informationManager->getNumberEnemyWorkers() + 4 )
		//	||
		//	( Broodwar->getFrameCount() < 4500 && informationManager->canSeeEnemyPatches() && Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe) >= informationManager->getNumberEnemyWorkers() + 6 )
			//|| 
			//( Broodwar->enemy()->getRace() == Races::Protoss && Broodwar->getFrameCount() == 4000 && !informationManager->enemyHasBuilt( Broodwar->enemy()->getRace().getRefinery() ) )
			//||
			//( Broodwar->enemy()->getRace() == Races::Terran && Broodwar->getFrameCount() == 2600 && !informationManager->enemyHasBuilt( Broodwar->enemy()->getRace().getRefinery() ) )
			//||
			//( Broodwar->enemy()->getRace() == Races::Zerg && Broodwar->getFrameCount() == 5300 && !informationManager->enemyHasBuilt( Broodwar->enemy()->getRace().getRefinery() ) )
		//	||
		//	( Broodwar->getFrameCount() > 2200 && Broodwar->enemy()->getRace() == Races::Zerg && !informationManager->isEnemySpotted() ) ) )
		//{
		//	moodManager->setMood(MoodManager::MoodData::Defensive);
		//}

		// NEW DEFENSIVE CRITERION
		// Against Zerg only.
		// if before 3000 frames, #enemy's worker + 4 =< #our workers
		// OR
		// if before 4500 frames, #enemy's worker + 6 =< #our workers <== DEPRECATED
		// OR
		// we do not scout enemy's base before the first 2200 frames (holds also against Random players)
		// then get prepared for a rush! ==> defensive mode
		if (
				moodManager->getMood() != MoodManager::MoodData::Defensive &&
				informationManager->getNumberEnemyBases() <= 1 &&
				(
					(
						Broodwar->getFrameCount() < 3000 && 
						Broodwar->enemy()->getRace() == Races::Zerg &&
						informationManager->canSeeEnemyPatches() && 
						Broodwar->self()->allUnitCount( UnitTypes::Protoss_Probe ) >= informationManager->getNumberEnemyWorkers() + 4
					)
					//||
					//(
					//	Broodwar->getFrameCount() < 4500 &&
					//	Broodwar->enemy()->getRace() == Races::Zerg &&
					//	informationManager->canSeeEnemyPatches() &&
					//	Broodwar->self()->allUnitCount( UnitTypes::Protoss_Probe ) >= informationManager->getNumberEnemyWorkers() + 6
					//)
					|| 
					(
						Broodwar->getFrameCount() > 2500 
						&&
						( Broodwar->enemy()->getRace() == Races::Zerg || Broodwar->enemy()->getRace() == Races::Random )						
						&&
						!informationManager->isEnemySpotted()
					)
				)
			)
		{
			moodManager->setMood(MoodManager::MoodData::Defensive);
			hasSwitchedToDefensive = true;
		}

		if ( underAttackManager->isUnderAttack() )
			workerManager->disableAutoBuild();
		else 
			workerManager->enableAutoBuild();

		if ((Broodwar->self()->allUnitCount(UnitTypes::Protoss_Probe) < 5) && !underAttackManager->isUnderAttack())
			buildOrderManager->buildAdditional(1,UnitTypes::Protoss_Probe,90);

		if (Broodwar->self()->allUnitCount(UnitTypes::Protoss_Pylon) == 1 )
		{		
			scoutManager->setScoutCount(1);
		}

		if( moodManager->getMood() == MoodManager::MoodData::Defensive )
			baseTimeToExpand = 10000;
		else
			baseTimeToExpand = 8000;

		// We should expand at least each 9000 [+- 1000] frames (13000 [+- 1000] in a defensive mood)
		if( Broodwar->getFrameCount() >= (signed)baseManager->getAllBases().size() * ( baseTimeToExpand + timeToExpand ) && 
			!underAttackManager->isUnderAttack() &&
			!defenseManager->isDefending() &&
			armyManager->enemyDPStoGround() < armyManager->myDPStoGround() )
		{
			baseManager->expand( 100 );
			armyManager->setLastExpandFrame( Broodwar->getFrameCount() );
		}

		// if we switched to a defensive mood and we are far in the game (i.e. 16000 frames), 
		// or if we have a DPS of at least 12, then set a macro mood
		if( moodManager->getMood() == MoodManager::MoodData::Defensive && 
			moodManager->getPreviousMood() != MoodManager::MoodData::Defensive && 
			( Broodwar->getFrameCount() >= 16000 || armyManager->myDPStoGround() > 12 ) )
		{
			moodManager->setMood( MoodManager::MoodData::Macro );
		}
	}

	if (debugMode)
	{
		// TIMING
		//Broodwar->drawTextScreen(10, 10, "frame=%d", Broodwar->getFrameCount());
		
		switch(moodManager->getMood())
		{
		case MoodManager::MoodData::Cheese:
			Broodwar->drawTextScreen(490,20,"Mood: Cheese (%d)", (int)MoodManager::MoodData::Cheese);
			break;
		case MoodManager::MoodData::Rush:
			Broodwar->drawTextScreen(490,20,"Mood: Rush (%d)", (int)MoodManager::MoodData::Rush);
			break;
		case MoodManager::MoodData::Agressive:
			Broodwar->drawTextScreen(490,20,"Mood: Agressive (%d)", (int)MoodManager::MoodData::Agressive);
			break;
		case MoodManager::MoodData::Defensive:
			Broodwar->drawTextScreen(490,20,"Mood: Defensive (%d)", (int)MoodManager::MoodData::Defensive);
			break;
		case MoodManager::MoodData::FastExpo:
			Broodwar->drawTextScreen(490,20,"Mood: Fast Expo (%d)", (int)MoodManager::MoodData::FastExpo);
			break;
		case MoodManager::MoodData::Macro:
			Broodwar->drawTextScreen(490,20,"Mood: Macro (%d)", (int)MoodManager::MoodData::Macro);
		}

		Broodwar->drawTextScreen(490,30,"Zealots:        %3d%% / %3d%%", (int)(spendManager->getUnitCount()->zealotRatio*100), (int)(spendManager->getUnitCount()->zealotIdealRatio*100));
		Broodwar->drawTextScreen(490,40,"Dragoons:      %3d%% / %3d%%", (int)(spendManager->getUnitCount()->dragoonRatio*100), (int)(spendManager->getUnitCount()->dragoonIdealRatio*100));
		Broodwar->drawTextScreen(490,50,"Corsairs:       %3d%% / %3d%%", (int)(spendManager->getUnitCount()->corsairRatio*100), (int)(spendManager->getUnitCount()->corsairIdealRatio*100));
		Broodwar->drawTextScreen(490,60,"Dark Templars: %3d%% / %3d%%", (int)(spendManager->getUnitCount()->darkTemplarRatio*100), (int)(spendManager->getUnitCount()->darkTemplarIdealRatio*100));
		Broodwar->drawTextScreen(490,70,"Archons:        %3d%% / %3d%%", (int)(spendManager->getUnitCount()->archonRatio*100), (int)(spendManager->getUnitCount()->archonIdealRatio*100));
		Broodwar->drawTextScreen(490,80,"DPS to G:     %.2f", armyManager->myDPStoGround());
		Broodwar->drawTextScreen(490,90,"DPS to A:     %.2f", armyManager->myDPStoAir());
		Broodwar->drawTextScreen(490,100,"Enemy DPS to G:%.2f", armyManager->enemyDPStoGround());
		Broodwar->drawTextScreen(490,110,"Enemy DPS to A:%.2f", armyManager->enemyDPStoAir());
		Broodwar->drawTextScreen(490,120,"StartAttack:     %d", moodManager->getStartAttack());
		Broodwar->drawTextScreen(490,130,"Rush: %s", armyManager->getFirstAttackDone() ? "false" : "true");
		Broodwar->drawTextScreen(490,140,"Enemy Workers: %d", informationManager->getNumberEnemyWorkers());
		Broodwar->drawTextScreen(490,150,"Bases:           %d", baseManager->getAllBases().size());
		Broodwar->drawTextScreen(490,160,"Enemy Bases:   %d", informationManager->getNumberEnemyBases());
		Broodwar->drawTextScreen(490,170,"Enemy Centers:  %d", informationManager->getNumberBaseCenters());
		Broodwar->drawTextScreen(490,180,"Enemy Buildings: %d", informationManager->getNumberEnemyBuildings());
		Broodwar->drawTextScreen(490,190,"Borders: %d", borderManager->getMyBorder().size());
		Broodwar->drawTextScreen(490,200,"M/G:          %3d/%3d", spendManager->getMineralStock(), spendManager->getGasStock());
		Broodwar->drawTextScreen(490,210,"Distrib: ");
		std::vector<double> distrib = moodManager->getDistribution();
		for( unsigned int i = 0; i < distrib.size(); ++i )
			Broodwar->drawTextScreen(490 + i*30,220,"%1.2f ", distrib[i]*10);

		diff = compteur.GetTimeFromStart();
		if (Broodwar->getFrameCount() > 10)
		{
			if (diff > top)
				top = diff;
			if (diff*1000 >= 55)
				sup55++;
		}
		Broodwar->drawTextScreen(280,0,"now: %2.2f ms", diff*1000);
		Broodwar->drawTextScreen(280,10,"top:  %2.2f ms", top*1000);
		Broodwar->drawTextScreen(280,20,"+55: %d times", sup55);
		Broodwar->drawTextScreen(280,30, "%s", Broodwar->mapName().c_str());
	}

	if (debugAttackerMode)
	{
		Broodwar->drawTextScreen(30,10,"My attackers:");
		Broodwar->drawTextScreen(300,10,"Enemy's attackers:");

		int myY = 20;
		int enemyY = 20;

		for each( BWAPI::Unit *u in SelectAllAttacks() )
			if( u != NULL && u->getOrderTarget() != NULL)
			{
				Broodwar->drawTextScreen(30,myY,"From %s to %s", u->getType().getName().c_str(), u->getOrderTarget()->getType().getName().c_str());
				myY += 10;
			}

		for each( BWAPI::Unit *u in SelectAllEnemyAttacks() )
			if( u != NULL && u->getOrderTarget() != NULL)
			{
				Broodwar->drawTextScreen(300,enemyY,"From %s to %s", u->getType().getName().c_str(), u->getOrderTarget()->getType().getName().c_str());
				enemyY += 10;
			}
		
		for each( BWAPI::Unit *u in SelectAll()(isAttacking) )
			if( u != NULL && u->getOrderTarget() != NULL )
			{
				BWAPI::Position uPos = u->getPosition();
				BWAPI::Position tPos = u->getOrderTarget()->getPosition();
				BWAPI::Broodwar->drawLineMap(uPos.x(), uPos.y(), tPos.x(), tPos.y(), BWAPI::Colors::Red);
			}
	}

	if( debugTimersMode )
	{
		Broodwar->drawTextScreen(490,30,"total: %2.2f ms", compteur.GetTimeFromStart()*1000);
		Broodwar->drawTextScreen(490,40,"worker: %2.2f ms", diffWorker*1000);
		Broodwar->drawTextScreen(490,50,"build: %2.2f ms", diffBuild*1000);
		Broodwar->drawTextScreen(490,60,"buildOrder: %2.2f ms", diffBuildOrder*1000);
		Broodwar->drawTextScreen(490,70,"base: %2.2f ms", diffBase*1000);
		Broodwar->drawTextScreen(490,80,"UA: %2.2f ms", diffUA*1000);
		Broodwar->drawTextScreen(490,90,"AT: %2.2f ms", diffAT*1000);
		Broodwar->drawTextScreen(490,100,"arbitrator: %2.2f ms", diffArbitrator*1000);
		Broodwar->drawTextScreen(490,110,"rush: %2.2f ms", diffRush*1000);
		Broodwar->drawTextScreen(490,120,"supply: %2.2f ms", diffSupply*1000);
		Broodwar->drawTextScreen(490,130,"tech: %2.2f ms", diffTech*1000);
		Broodwar->drawTextScreen(490,140,"upgrade: %2.2f ms", diffUpgrade*1000);
		Broodwar->drawTextScreen(490,150,"mood: %2.2f ms", diffMood*1000);
		Broodwar->drawTextScreen(490,160,"spend: %2.2f ms", diffSpend*1000);
		Broodwar->drawTextScreen(490,170,"scout: %2.2f ms", diffScout*1000);
		Broodwar->drawTextScreen(490,180,"defense: %2.2f ms", diffDefense*1000);
		Broodwar->drawTextScreen(490,190,"spy: %2.2f ms", diffSpy*1000);
		Broodwar->drawTextScreen(490,200,"border: %2.2f ms", diffBorder*1000);
		Broodwar->drawTextScreen(490,210,"army: %2.2f ms", diffArmy*1000);
		Broodwar->drawTextScreen(490,220,"dragoon: %2.2f ms", diffDragoon*1000);
		Broodwar->drawTextScreen(490,230,"photon: %2.2f ms", diffPhoton*1000);
		Broodwar->drawTextScreen(490,240,"UI: %2.2f ms", diffUI*1000);
		Broodwar->drawTextScreen(490,250,"Drop: %2.2f ms", diffDrop*1000);
	}

	std::set<Unit*> units=Broodwar->self()->getUnits();

	if (showManagerAssignments)
	{
		for(std::set<Unit*>::iterator i=units.begin();i!=units.end(); ++i)
		{
			if (arbitrator.hasBid(*i))
			{
				int x=(*i)->getPosition().x();
				int y=(*i)->getPosition().y();
				std::list< std::pair< Arbitrator::Controller<Unit*,double>*, double> > bids=arbitrator.getAllBidders(*i);
				int y_off=0;
				bool first = false;
				const char activeColor = '\x07', inactiveColor = '\x16';
				char color = activeColor;
				for(std::list< std::pair< Arbitrator::Controller<Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end(); ++j)
				{
					Broodwar->drawTextMap(x,y+y_off,"%c%s: %d",color,j->first->getShortName().c_str(),(int)j->second);
					y_off+=15;
					//if (j->first->getShortName().compare(1,3,"Con") != 0)
					//{
					//	TilePosition tp((*i)->getTargetPosition());
					//	Broodwar->drawTextMap(x,y+y_off,"%c%dx%d", color, tp.x(), tp.y());
					//	y_off+=15;
					//}
					color = inactiveColor;
				}
			}
		}
	}

	if( debugState )
	{
		for(std::set<Unit*>::iterator i=units.begin();i!=units.end(); ++i)
		{
			int x=(*i)->getPosition().x();
			int y=(*i)->getPosition().y();
			char red	= '\x08';
			char blue	= '\x0E';
			char royal	= '\x1C';
			char cyan	= '\x1F';

			Broodwar->drawTextMap(x - 15, y + 15,"%c%s", red, unitStates->getName( *i ).c_str() );
			//Broodwar->drawTextMap(x - 15, y + 25,"%c%s", blue, ( *i )->isIdle() ? "true" : "false" );
			//Broodwar->drawTextMap(x - 15, y + 35,"%c%s", royal, ( *i )->isMoving() ? "true" : "false" );
			//Broodwar->drawTextMap(x - 15, y + 45,"%c%s", cyan, ( *i )->isAttacking() ? "true" : "false" );
		}
	}

	if( debugOrder )
	{		
		Broodwar->drawTextScreen(30,10,"Orders: ");
		int myY = 20;

		for(std::set<Unit*>::iterator i=units.begin();i!=units.end(); ++i)
			if( (*i) != NULL && (*i)->isSelected() && (*i)->getOrder() != NULL && (*i)->getOrderTarget() != NULL )
			{
				BWAPI::Position uPos = (*i)->getPosition();
				BWAPI::Position tPos = (*i)->getOrderTarget()->getPosition();
				Broodwar->drawTextScreen( 30, myY, "%s, target %s", (*i)->getOrder().getName().c_str(), (*i)->getOrderTarget()->getType().getName().c_str() );
				BWAPI::Broodwar->drawLineMap( uPos.x(), uPos.y(), tPos.x(), tPos.y(), BWAPI::Colors::Red );
				BWAPI::Broodwar->drawCircleMap(tPos.x(), tPos.y(), 15, BWAPI::Colors::Purple);
				myY += 10;
			}
	}
}

void AiurModule::onUnitDestroy(Unit* unit)
{
	if (Broodwar->isReplay()) return;
	arbitrator.onRemoveObject(unit);
	buildManager->onRemoveUnit(unit);
	techManager->onRemoveUnit(unit);
	upgradeManager->onRemoveUnit(unit);
	workerManager->onRemoveUnit(unit);
	scoutManager->onRemoveUnit(unit);
	spyManager->onRemoveUnit(unit);
	defenseManager->onRemoveUnit(unit);
	informationManager->onUnitDestroy(unit);
	baseManager->onRemoveUnit(unit);
	underAttackManager->onRemoveUnit(unit);
	armyManager->onRemoveUnit(unit);
	if( moodManager->getMood() == MoodManager::MoodData::Agressive )
		dropZealotsManager->onRemoveUnit(unit);

	if( moodManager->getMood() == MoodManager::MoodData::Cheese && unit->getType().isWorker() )
		rushPhotonManager->onRemoveUnit(unit);

	// we rebuild a lost pylon at the same position (except pylon in enemy base)
	if ( unit->getPlayer() == Broodwar->self() && unit->getType() == UnitTypes::Protoss_Pylon )
	{
		buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Pylon, 1000, unit->getTilePosition());
	}
	// we rebuild a lost photon cannon at the same position
	//if (unit->getType() == UnitTypes::Protoss_Photon_Cannon)
	//{
	//	buildOrderManager->buildAdditional(1, UnitTypes::Protoss_Photon_Cannon, 60, unit->getTilePosition());
	//}

	if (unit->getType() == UnitTypes::Protoss_Dragoon && unit->getPlayer() == Broodwar->self())
	{
		dragoonManager->deleteDragoon(unit);
	}

	if( unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding() )
	{
		unitStates->remove( unit );
	}
}

void AiurModule::onUnitDiscover(Unit* unit)
{
	if (Broodwar->isReplay()) return;
	informationManager->onUnitDiscover(unit);
	unitGroupManager->onUnitDiscover(unit);

	if (unit->getType() == UnitTypes::Protoss_Dragoon && unit->getPlayer() == Broodwar->self())
	{
		dragoonManager->addDragoon(unit);
	}

	if( moodManager->getMood() == MoodManager::MoodData::Cheese && unit->getType() == UnitTypes::Protoss_Pylon && unit->getPlayer() == Broodwar->self() )
	{
		rushPhotonManager->setLastPylon(unit);
	}

	if( unit->getPlayer() == BWAPI::Broodwar->self() && !unit->getType().isBuilding() )
	{
		unitStates->setState( unit, UnitStates::Idle );
	}
}
void AiurModule::onUnitEvade(Unit* unit)
{
	if (Broodwar->isReplay()) return;
	informationManager->onUnitEvade(unit);
	unitGroupManager->onUnitEvade(unit);
}

void AiurModule::onUnitMorph(Unit* unit)
{
	if (Broodwar->isReplay()) return;
	unitGroupManager->onUnitMorph(unit);
	informationManager->onUnitMorph(unit);
}
void AiurModule::onUnitRenegade(Unit* unit)
{
	if (Broodwar->isReplay()) return;
	unitGroupManager->onUnitRenegade(unit);
}

void AiurModule::onReceiveText( Player *player, std::string text )
{
	if( player == Broodwar->enemy()
		&& 
		( text.find("gg") != text.npos || text.find("GG") != text.npos ) 
		&&
		Broodwar->getFrameCount() > 3000 )
	{
		ggFrame = Broodwar->getFrameCount();
	}
}

void AiurModule::onSendText(std::string text)
{
	if (Broodwar->isReplay())
	{
		Broodwar->sendText("%s",text.c_str());
		return;
	}
	UnitType type=UnitTypes::getUnitType(text);
	if( text == "help" )
	{
		Broodwar->sendText("debug");
		Broodwar->sendText("ui");
		Broodwar->sendText("state");
		Broodwar->sendText("expand");
		Broodwar->sendText("attack");
		Broodwar->sendText("time");
		Broodwar->sendText("order");
	}
	if (text=="debug")
	{
		if( !showManagerAssignments )
		{
			showManagerAssignments=true;
			buildOrderManager->setDebugMode(true);
			scoutManager->setDebugMode(true);
			spyManager->setDebugMode(true);
			rushPhotonManager->setDebugMode(true);
			debugMode = true;
			debugAttackerMode = false;
			debugTimersMode = false;
		}
		else
		{
			showManagerAssignments=false;
			buildOrderManager->setDebugMode(false);
			scoutManager->setDebugMode(false);
			spyManager->setDebugMode(false);
			rushPhotonManager->setDebugMode(false);
			debugMode = false;
		}
		Broodwar->printf("%s",text.c_str());
		return;
	}
	if (text=="ui")
	{
		debugUI ^= true;
		return;
	}
	if (text=="state")
	{
		showManagerAssignments ^= true;
		debugState ^= true;
		return;
	}
	if (text=="expand")
	{
		baseManager->expand();
		Broodwar->printf("%s",text.c_str());
		return;
	}
	if (text=="attack")
	{
		if(!debugAttackerMode)
		{
			if(debugMode)
			{
				showManagerAssignments=false;
				buildOrderManager->setDebugMode(false);
				scoutManager->setDebugMode(false);
				spyManager->setDebugMode(false);
				rushPhotonManager->setDebugMode(false);
				debugMode = false;
			}
			debugTimersMode = false;
			debugAttackerMode = true;
		}
		else
			debugAttackerMode = false;
		return;
	}
	if( text == "time")
	{
		if(!debugAttackerMode)
		{
			if(debugMode)
			{
				showManagerAssignments=false;
				buildOrderManager->setDebugMode(false);
				scoutManager->setDebugMode(false);
				spyManager->setDebugMode(false);
				rushPhotonManager->setDebugMode(false);
				debugMode = false;
			}
			debugAttackerMode = false;
			debugTimersMode = true;
		}
		else
			debugTimersMode = false;
		return;
	}
	if( text == "order" )
	{
		debugOrder ^= true;
		return;
	}
	if (type!=UnitTypes::Unknown)
	{
		buildOrderManager->buildAdditional(1,type,300);
	}
	else
	{
		TechType type=TechTypes::getTechType(text);
		if (type!=TechTypes::Unknown)
		{
			techManager->research(type);
		}
		else
		{
			UpgradeType type=UpgradeTypes::getUpgradeType(text);
			if (type!=UpgradeTypes::Unknown)
			{
				upgradeManager->upgrade(type);
			}
			else
				Broodwar->printf("You typed '%s'!",text.c_str());
		}
	}
	Broodwar->sendText("%s",text.c_str());
}
