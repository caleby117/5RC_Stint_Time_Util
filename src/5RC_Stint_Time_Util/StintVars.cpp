#include <unordered_map>
#include <string>
#include <iostream>
#include <assert.h>
#include "irsdk_defines.h"
#include "StintVars.h"


int StintVarHeaderData::getStintVarHeaderIdx(const char* var)
{
	if (stintVarIndexByName.contains(var))
	{
		return stintVarIndexByName[var];
	}
	return -1;
}

void StintVarHeaderData::registerVars(irsdk_varHeader* vars, int count)
{
	// Overwrites original list of registered vars if any
	if (stintVarHeaders != NULL) { delete[] stintVarHeaders; }
	varCount = count;
	sampleLen = 0;
	stintVarHeaders = new StintVarHeader[count];

	// register each var individually
	for (int i = 0; i < count; i++)
	{
		registerVar(vars[i], i, sampleLen);
		stintVarIndexByName.insert(std::pair<const char*, int>(vars[i].name, i));
		sampleLen += irsdk_VarTypeBytes[vars[i].type];
	}


}

void StintVarHeaderData::registerVar(irsdk_varHeader& var, int idx, int offset)
{
	// registers the variable into this class
	StintVarHeader sVarH;
	sVarH.type = var.type;
	sVarH.offset = offset;
	strncpy(sVarH.name, var.name, IRSDK_MAX_STRING);
	strncpy(sVarH.unit, var.unit, IRSDK_MAX_STRING);
	stintVarHeaders[idx] = sVarH;
}


int StintVarHeaderData::getVarCount() { return varCount; }

StintVarHeader& StintVarHeaderData::getStintVarHeader(const char* var)
{
	// return a reference to the StintVarHeader struct whose name is var
	int idx = getStintVarHeaderIdx(var);
	if (idx > 0)
	{
		return stintVarHeaders[idx];
	}
	std::cout << "getStintVarHeader: no such registered variable " << var << std::endl;
	assert(false);
}


int StintVarHeaderData::getSampleLen() { return sampleLen; }
