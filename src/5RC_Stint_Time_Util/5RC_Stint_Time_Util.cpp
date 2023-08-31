// 5RC_Stint_Time_Util.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <unordered_set>
#include <string>
#include "irsdk_defines.h"


#define SEEK_SET 0
#pragma warning(disable:4996) //_CRT_SECURE_NO_WARNINGS

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

void GetFirstSamplesOfLaps(int *firstSamplesOfLaps, irsdk_varHeader &lastLapTime, FILE* ibt)
{
	int idx = 0;
	int len = sizeof(float);
	float curr;
	float prev = -INFINITY;
	int dataStart = header.sessionInfoLen + header.sessionInfoOffset;
	fseek(ibt, dataStart+lastLapTime.offset, SEEK_SET);
	int count = 0;
	while (len == fread(&curr, 1, irsdk_float, ibt))
	{
		if (curr != prev)
		{
			firstSamplesOfLaps[idx++] = count;
			std::cout << "Sample of new lap: " << count << std::endl;
		}
		prev = curr;
		fseek(ibt, header.bufLen-sizeof(int), SEEK_CUR);
		count++;
	}

}


int main(int argc, char** argv)
{

	// List of variables that we are interested in
	std::unordered_set<std::string> importantVars = std::unordered_set<std::string>();
	
	std::string varStrings[] = {
		"PlayerCarMyIncidentCount",
		"PlayerCarWeightPenalty",
		"PlayerCarPowerAdjust",
		"LapCompleted",
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

	for (std::string v : varStrings)
	{
		importantVars.insert(v);
	}

	
	if (argc < 2)
	{
		// no args
		std::cout << "Usage: 5RC_Stint_Time_Util.exe <ibt file path>" << std::endl;
		return 1;
	}

	// open the file for reading
	FILE* ibt = fopen(argv[1], "rb");
	const int importantVarsSize = 26;
	irsdk_varHeader importantVarHeaders[importantVarsSize];
	std::byte* samples;
	int sampleRowLength;


	if (ibt)
	{
		// file opened successfully
		// read header length and read header
		int len = sizeof(irsdk_header);
		if (len == fread(&header, 1, len, ibt))
		{
			// successfully read header
			// read disk header length and header
			len = sizeof(irsdk_diskSubHeader);
			if (len == fread(&diskHeader, 1, len, ibt))
			{
				// successfully read disk header
				// varheaders: length header.numVars, offset to this part: header.varHeaderOffset

				// Get the headers that we are interested in 
				// linear search through until we populate the vars list
				int currHeaderOffset = 0;
				
				// length of the list of variables
				len = sizeof(irsdk_varHeader);
				irsdk_varHeader curVar;
				fseek(ibt, header.varHeaderOffset, SEEK_SET);
				int lastLapTimeidx;
				for (int i = 0; i < header.numVars; i++)
				{
					if (len == fread(&curVar, 1, len, ibt))
					{
						// Read into curVar. 
						if (importantVars.contains(curVar.name))
						{
							// Found the var that we are interested in 
							importantVarHeaders[currHeaderOffset] = curVar;
							if (strcmp(curVar.name, "LapLastLapTime") == 0 ) lastLapTimeidx = currHeaderOffset;
							currHeaderOffset++;
							importantVars.erase(curVar.name);
							if (importantVars.empty()) break;
						}
					}
				}

				for (int i = 0; i < importantVarsSize; i++)
				{
					std::cout << importantVarHeaders[i].name << " - " << importantVarHeaders[i].count << std::endl;
				}

				// Read all the vars that we are interested in. Grab the variables that we are interested in 
				// Laptime comes in only after about 5 ticks 

				// set up a new row of data only for the vars that we are interested in 
				int newRowSize = 0;
				for (irsdk_varHeader& h : importantVarHeaders)
				{
					newRowSize += irsdk_VarTypeBytes[h.type];
				}


				// Compare the LastLap time between samples, get samples that are at the start of a new lap
				int* firstSampleRowOfLap = new int[diskHeader.sessionLapCount];
				GetFirstSamplesOfLaps(firstSampleRowOfLap, importantVarHeaders[lastLapTimeidx], ibt);
				sampleRowLength = diskHeader.sessionLapCount * newRowSize;
				samples = new std::byte[sampleRowLength];
				int start = header.sessionInfoLen + header.sessionInfoOffset;
				int curPosBuffer = 0;
				for (int lap = 0; lap < diskHeader.sessionLapCount; lap++)
				{
					int startOfCurrentRow = start + (header.bufLen * firstSampleRowOfLap[lap]);
	
					for (irsdk_varHeader& vHeader : importantVarHeaders)
					{
						fseek(ibt, startOfCurrentRow+vHeader.offset, SEEK_SET);
						fread(&samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type], 1, ibt);
						curPosBuffer += irsdk_VarTypeBytes[vHeader.type];
					}
				}


				// for testing and verification
				curPosBuffer = 0;
				for (int i = 0; i < diskHeader.sessionLapCount; i++)
				{
					std::cout << "\n\n Lap " << i << std::endl;
					for (irsdk_varHeader& vHeader : importantVarHeaders)
					{
						irPossibleTypes currentType;
						std::cout << vHeader.name << ": ";
						switch (vHeader.type)
						{
						case irsdk_char:
							currentType.charValue = (char)samples[curPosBuffer];
							std::cout << currentType.charValue << std::endl;
							break;

						case irsdk_bool:
							currentType.charValue = (bool)samples[curPosBuffer];
							std::cout << currentType.charValue << std::endl;
							break;

						case irsdk_int:
							std::memcpy(&currentType.intValue, &samples[curPosBuffer], irsdk_VarTypeBytes[irsdk_int]);
							std::cout << currentType.intValue << std::endl;
							break;

						case irsdk_bitField:
							std::memcpy(&currentType.intValue, &samples[curPosBuffer], irsdk_VarTypeBytes[irsdk_bitField]);
							std::cout << std::hex << currentType.intValue << std::endl;
							break;

						case irsdk_float:
							std::memcpy(&currentType.floatValue, &samples[curPosBuffer], irsdk_VarTypeBytes[irsdk_float]);
							std::cout << currentType.floatValue << std::endl;
							break;

						case irsdk_double:
							std::memcpy(&currentType.doubleValue, &samples[curPosBuffer], irsdk_VarTypeBytes[irsdk_double]);
							std::cout << currentType.doubleValue << std::endl;
							break;
						}

						curPosBuffer += irsdk_VarTypeBytes[vHeader.type];
					}
				}
			}
		}
		fclose(ibt);
	}

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
