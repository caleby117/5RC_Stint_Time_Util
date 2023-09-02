// 5RC_Stint_Time_Util.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <unordered_set>
#include <string>
#include "irsdk_defines.h"
#include "StintVars.h"

#define DUMP_TO_STDOUT
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

std::string varStrings[] = {
	"SessionTick",
	"PlayerCarPosition",
	"PlayerCarClassPosition",
	"PlayerCarClass",
	"PlayerCarTeamIncidentCount",
	"PlayerCarMyIncidentCount",
	"PlayerCarWeightPenalty",
	"PlayerCarPowerAdjust",
	"Lap",
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

int GetDataOffset()
{
	int curLatest = 0;
	// get the latest varBuf
	for (int i = 1; i < header.numBuf; i++)
	{
		if (header.varBuf[i].tickCount > header.varBuf[curLatest].tickCount)
			curLatest = i;
	}
	return header.varBuf[curLatest].bufOffset;
}



void ReadFirstSamplesOfLaps(int *firstSamplesOfLaps, irsdk_varHeader &lastLapTime, FILE* ibt)
{
	int idx = 0;
	int len = irsdk_VarTypeBytes[lastLapTime.type];
	float currLastLapTime;
	float prevLastLapTime = -INFINITY;

	// ignore first lap/entry
	int dataStart = GetDataOffset() + header.bufLen;
	bool ignore = true;
	
	fseek(ibt, dataStart+lastLapTime.offset, SEEK_SET);
	int count = 1;
	while (len == fread(&currLastLapTime, 1, irsdk_float, ibt))
	{
		if (currLastLapTime != prevLastLapTime)
		{
			std::cout << currLastLapTime << ", " << prevLastLapTime << std::endl;
			if (ignore)
			{
				ignore = false;
				prevLastLapTime = currLastLapTime;
				fseek(ibt, header.bufLen-len, SEEK_CUR);
				count++;
				continue;
			}
			firstSamplesOfLaps[idx++] = count;
			std::cout << "Sample of new lap: " << count << std::endl;
		}
		prevLastLapTime = currLastLapTime;
		fseek(ibt, header.bufLen-len, SEEK_CUR);
		count++;
	}

}




int main(int argc, char** argv)
{
	if (argc < 2)
	{
		// no args
		std::cout << "Usage: 5RC_Stint_Time_Util.exe <ibt file path>" << std::endl;
		return 1;
	}

	// List of variables that we are interested in
	std::unordered_set<std::string> stintVars = std::unordered_set<std::string>();
	

	for (std::string v : varStrings)
	{
		stintVars.insert(v);
	}

	

	// open the file for reading
	FILE* ibt = fopen(argv[1], "rb");
	const int stintVarsCount = stintVars.size();
	irsdk_varHeader* irStintVarHeaders = new irsdk_varHeader[stintVarsCount];
	StintVarHeaderData sVarData = StintVarHeaderData();


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
				int currHeaderIdx = 0;
				
				len = sizeof(irsdk_varHeader);
				irsdk_varHeader curVar;
				fseek(ibt, header.varHeaderOffset, SEEK_SET);
				for (int i = 0; i < header.numVars; i++)
				{
					if (len == fread(&curVar, 1, len, ibt))
					{
						// Read into curVar. 
						if (stintVars.contains(curVar.name))
						{
							// Found the var that we are interested in 
							irStintVarHeaders[currHeaderIdx++] = curVar;
							stintVars.erase(curVar.name);
							if (stintVars.empty()) break;
						}
					}
				}

#ifdef DUMP_TO_STDOUT
				for (int i = 0; i < stintVarsCount; i++)
				{
					std::cout << irStintVarHeaders[i].name << " - " << irStintVarHeaders[i].count << std::endl;
				}
#endif

				// Read all the vars that we are interested in. Grab the variables that we are interested in 
				// Laptime comes in only after about 5 ticks 

				// set up a new row of data only for the vars that we are interested in 
				sVarData.registerVars(irStintVarHeaders, stintVarsCount);
				

				// Get the sample indices of "first" sample of every new lap
				// ** var LapLastLapTime only updates about 10 ticks after crossing the line, take first sample when this variable updates.
				int* firstSampleRowOfLap = new int[diskHeader.sessionLapCount];
				int lastLapTime_i = sVarData.getStintVarHeaderIdx("LapLastLapTime");
				ReadFirstSamplesOfLaps(firstSampleRowOfLap, irStintVarHeaders[lastLapTime_i], ibt);
				std::byte* samples = new std::byte[diskHeader.sessionLapCount * sVarData.getSampleLen()];

				// Get the data from the selected samples
				int start = GetDataOffset();
				int curPosBuffer = 0;
				for (int lap = 0; lap < diskHeader.sessionLapCount; lap++)
				{
					int startOfCurrentRow = start + (header.bufLen * firstSampleRowOfLap[lap]);
	
					for (int i = 0; i < stintVarsCount; i++)
					{
						irsdk_varHeader& vHeader = irStintVarHeaders[i];
						fseek(ibt, startOfCurrentRow+vHeader.offset, SEEK_SET);
						fread(&samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type], 1, ibt);
						curPosBuffer += irsdk_VarTypeBytes[vHeader.type];
					}
				}

				// TODO: Create a class to hold information about each variable in this new data row etc.

#ifdef DUMP_TO_STDOUT
				// for testing and verification
				curPosBuffer = 0;
				for (int i = 0; i < diskHeader.sessionLapCount; i++)
				{
					std::cout << "\n\nLap " << i << std::endl;
					for (int j = 0; j < stintVarsCount; j++)
					{
						irsdk_varHeader& vHeader = irStintVarHeaders[j];
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
							std::memcpy(&currentType.intValue, &samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type]);
							std::cout << currentType.intValue << std::endl;
							break;

						case irsdk_bitField:
							std::memcpy(&currentType.intValue, &samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type]);
							std::cout << std::hex << currentType.intValue << std::endl;
							break;

						case irsdk_float:
							std::memcpy(&currentType.floatValue, &samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type]);
							std::cout << currentType.floatValue << std::endl;
							break;

						case irsdk_double:
							std::memcpy(&currentType.doubleValue, &samples[curPosBuffer], irsdk_VarTypeBytes[vHeader.type]);
							std::cout << currentType.doubleValue << std::endl;
							break;
						}

						curPosBuffer += irsdk_VarTypeBytes[vHeader.type];
					}
					
				}
#endif

				// Remove laps at the end with jumbled data
				

				delete[] samples;
				delete[] firstSampleRowOfLap;
				
			}
		}
		fclose(ibt);
	}

	delete[] irStintVarHeaders;

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
