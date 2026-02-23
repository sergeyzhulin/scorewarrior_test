#include "IO/System/TypeRegistry.hpp"

#include <IO/Commands/CreateMap.hpp>
#include <IO/Commands/March.hpp>
#include <IO/Commands/SpawnHunter.hpp>
#include <IO/Commands/SpawnSwordsman.hpp>
#include <IO/Events/MapCreated.hpp>
#include <IO/Events/MarchEnded.hpp>
#include <IO/Events/MarchStarted.hpp>
#include <IO/Events/UnitAttacked.hpp>
#include <IO/Events/UnitDied.hpp>
#include <IO/Events/UnitMoved.hpp>
#include <IO/Events/UnitSpawned.hpp>
#include <IO/System/CommandParser.hpp>
#include <IO/System/EventLog.hpp>
#include <IO/System/PrintDebug.hpp>
#include <fstream>
#include <iostream>
#include <Core/Map.h>
#include <Features/SmokeTests.h>
#include <Features/Game/GameAboutSwordsmenAndHunters.h>

//..\commands_example.txt
int main(int argc, char** argv)
{

	//tests::SmokeTests::simpleTestECS();
	//tests::SmokeTests::simpleTestForBehsAndActions();
	//tests::SmokeTests::testForActionsAndFilters();

	using namespace sw;

	if (argc != 2)
	{
		throw std::runtime_error("Error: No file specified in command line argument");
	}

	std::ifstream file(argv[1]);
	if (!file)
	{
		throw std::runtime_error("Error: File not found - " + std::string(argv[1]));
	}

	{
		features::GameAboutSwordsmenAndHunters game;
		game.play(file);
	}
	return 0;
}
