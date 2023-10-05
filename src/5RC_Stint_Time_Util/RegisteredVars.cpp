#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <stdexcept>
#include <sstream>
#include "RegisteredVars.h"
#include "irsdk_defines.h"


irsdk_varHeader RegisteredVars::getVarHeader(const char* var)
{
    return registeredVarHeaders[findIndex(var)];

}

irsdk_varHeader RegisteredVars::getVarHeader(int i)
{
    return registeredVarHeaders.at(i);
}
irsdk_varHeader& RegisteredVars::getVarHeaderRef(const char* var)
{
    return registeredVarHeaders.at(findIndex(var));
}

irsdk_varHeader& RegisteredVars::getVarHeaderRef(int i)
{
    return registeredVarHeaders.at(i);
}

void RegisteredVars::registerVar(irsdk_varHeader varHeader)
{
    registeredVarIdx.emplace(varHeader.name, registeredVarHeaders.size());
    offsetsFull.push_back(varHeader.offset);
    
    // update offset for data row and push 
    varHeader.offset = DataRow::getDataSize();
    DataRow::pushSizes(irsdk_VarTypeBytes[varHeader.type]);
    registeredVarHeaders.push_back(varHeader);
    std::cout << "Registered var " << varHeader.name << std::endl;
}

int RegisteredVars::numRegisteredVars()
{
    return registeredVarHeaders.size();
}


int RegisteredVars::findIndex(const char* var)
{
    std::stringstream errors;
    if (registeredVarIdx.find(var) == registeredVarIdx.end())
    {
        // cannot find var 
        errors << "Could not find registered variable name ";
        errors << var << std::endl;
        throw  std::runtime_error(errors.str());
    }

    return registeredVarIdx[var];
}


int RegisteredVars::getOffset(int idx)
{
    return registeredVarHeaders[idx].offset;
}

int RegisteredVars::getOffset(const char* var)
{
    return getOffset(findIndex(var));
}


int RegisteredVars::getOffsetFull(int idx)
{
    return offsetsFull[idx];
}


int RegisteredVars::getOffsetFull(const char* var)
{
    return getOffsetFull(findIndex(var));
}


int RegisteredVars::getNumBytes(const char* var)
{
    return irsdk_VarTypeBytes[getVarHeader(var).type];
}


void RegisteredVars::finalize()
{
    isFinalized = true;
}


const std::vector<irsdk_varHeader>& RegisteredVars::getVars()
{
    return registeredVarHeaders;
}



// Data Row
bool DataRow::isFinal = false;
bool DataRow::isInitialized = false;
int DataRow::varCount = 0;
std::vector<int> DataRow::sizes;
std::vector<DataRow> DataRow::rows;
int DataRow::dataSize = 0;

DataRow::DataRow()
{
    data = nullptr;
}


DataRow::~DataRow()
{
    if (data)
    {
        delete[] data;
        data = nullptr;
    }
}

DataRow::DataRow(DataRow& other)
{
    // copy constructor
    std::cout << this << " using copy constructor" << std::endl;
    isFinal = other.isFinal;
    data = other.data;
}


DataRow::DataRow(DataRow&& other)
{
    std::cout << this << " using move constructor from "<< &other << std::endl;
    isFinal = other.isFinal;
    data = other.data;

    other.data = nullptr;
}

DataRow& DataRow::operator=(DataRow&& other)
{
    if (this != &other)
    {
        delete[] data;
        std::cout << this << " using overloaded = operator";
        isFinal = other.isFinal;
        data = other.data;
        std::cout << " data=" << (void*) data << std::endl;
        other.data = nullptr;
    }
    return *this;
}

void DataRow::init(int count)
{
    isInitialized = true;
    sizes.reserve(count);
}

int DataRow::getDataSize()
{
    return dataSize;
}


bool DataRow::isInit()
{
    return isInitialized;
}

void DataRow::setInit(bool init)
{
    isInitialized = true;
}


const uint8_t* DataRow::getDataP()
{
    return data;
}


void DataRow::finalize(int n)
{
    // allocate memory for all the rows
    rows.reserve(n);
    for (int i = 0; i < n; i++)
    {
        rows.emplace_back();
        rows[i].finalizeRow();
    }
    isFinal = true;
}

void DataRow::finalizeRow()
{
    // allocate memory for this row
    if (data != nullptr)
    {
        delete[] data;
        data = nullptr;
    }
    data = new uint8_t[dataSize];
    std::cout << "DataRow@" << this << ": Finalized size ";
    std::cout << dataSize;
    std::cout << ", datae@" << (void*) data << std::endl;
}

void DataRow::pushSizes(int size)
{
    dataSize += size;
    sizes.push_back(size);
}


DataRow& DataRow::getRow(int n)
{
    return rows[n];
}

void DataRow::clearRows()
{
    rows.clear();
}
