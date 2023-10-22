// 5RC_Stint_Time_Util.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <unordered_set>
#include <string>
#include "irsdk_defines.h"
#include "DiskServer.h"
#include "ArgParse.h"

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
static irsdk_header header;
static irsdk_diskSubHeader diskHeader;

int main(int argc, char** argv)
{
    // parse command line arguments
    ArgParser::getInstance().parseArgs(argc, argv);
    if (ArgParser::getInstance().hasError())
    {
        // print help message
        std::cout << ArgParser::getInstance().help;
        // stop execution
        return 1;
    }

    std::string& ibtPath = ArgParser::getInstance().getIbtPath();
    std::string& outputPath = ArgParser::getInstance().getOutputPath();
    std::vector<std::string>& varList = ArgParser::getInstance().getVarList();

    DiskServer server = {ibtPath};

    server.registerVars(varList);
    server.readSamplesNewLap();         // saves the sample index of new laps
    server.saveSamplesNewLap();         // saves the samples themselves
    std::cout << "samples written" << std::endl;


    server.writeCSV(outputPath);

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
