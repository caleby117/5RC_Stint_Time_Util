#pragma once
#ifndef STINTVARS_H
#define STINTVARS_H
#include "irsdk_defines.h"
#include <unordered_map>



struct StintVarHeader
{
	int type;
	int offset;
	int count;
	char pad[4] = { 0 };
	char name[IRSDK_MAX_STRING];
	char unit[IRSDK_MAX_STRING];
};

class StintVarHeaderData
{
public:

	void registerVars(irsdk_varHeader* vars, int count);
	int getStintVarHeaderIdx(std::string var);
	int getVarCount();
	StintVarHeader& getStintVarHeader(const char* var);
	int getSampleLen();
	void setValidLaps(int laps);
	int getValidLaps();

protected:
	void registerVar(irsdk_varHeader& var, int idx, int offset);
	irsdk_varHeader* irVarHeaderArr;
	int varCount = 0;
	int registeredVars = 0;
	std::unordered_map < std::string, int > stintVarIndexByName;
	StintVarHeader* stintVarHeaders = NULL;
	size_t sampleLen;
	int validLaps = -1;
	
};

#endif
