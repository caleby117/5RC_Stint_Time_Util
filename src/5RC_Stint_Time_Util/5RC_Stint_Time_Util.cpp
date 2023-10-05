// 5RC_Stint_Time_Util.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <unordered_set>
#include <string>
#include "irsdk_defines.h"
#include "DiskServer.h"
#include "StintVars.h"

#define DUMP_TO_STDOUT

// Windows implementations
#ifdef _WIN32
#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS
#define byte std::byte
#endif

// Linux implementations
#ifdef __linux__
#include <cstring>
#include <wchar.h>
#include <stdint.h>
#include <cfloat>
#endif 
/* 
	Get relevant telemetry information from IBT file for laptime, fuel consumption, weather, tyre wear. 
*/

union irPossibleTypes
{
	char charValue;

	int intValue;
	float floatValue;

	double doubleValue;
};

static irsdk_header header;
static irsdk_diskSubHeader diskHeader;

const char* varStrings[] = {
	"Lap",
	"SessionTick",
	"PlayerCarPosition",
	"PlayerCarClassPosition",
	"PlayerCarClass",
	"PlayerCarTeamIncidentCount",
	"PlayerCarMyIncidentCount",
	"PlayerCarWeightPenalty",
	"PlayerCarPowerAdjust",
	"LapLastLapTime",
	"TrackTemp",
	"TrackTempCrew",
	"AirTemp",
	"WeatherType",
	"Skies",
	"AirDensity",
	"AirPressure",
	"WindVel",
	"FuelLevelPct",
	"LFwearL",
	"LFwearM",
	"LFwearR",
	"RFwearL",
	"RFwearM",
	"RFwearR",
	"LRwearL",
	"LRwearM",
	"LRwearR",
	"RRwearL",
	"RRwearM",
	"RRwearR"
	};


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		// no args
		std::cout << "Usage: ./5RC_Stint_Time_Util <ibt file path>" << std::endl;
		return 1;
	}


	DiskServer server = {argv[1]};

	server.registerVars(varStrings, 31);
	server.readSamplesNewLap();
	server.saveSamplesNewLap();
	std::cout << "samples written" << std::endl;


	server.writeCSV();

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
