#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>
#include <array>
#include <iostream>
#include "irsdk_defines.h"

#ifndef _REGISTEREDVARS_H
#define _REGISTEREDVARS_H

class RegisteredVars
{
public: 

    // get headers
    irsdk_varHeader getVarHeader(const char* var);
    irsdk_varHeader getVarHeader(int i);
    irsdk_varHeader& getVarHeaderRef(const char* var);
    irsdk_varHeader& getVarHeaderRef(int i);

    // register a var
    void registerVar(irsdk_varHeader varHeader);

    // find number of registered vars
    int numRegisteredVars();

    // find index of var
    int findIndex(const char* var);

    // get offset of var in registered vars buffer
    int getOffset(int idx);
    int getOffset(const char* var);

    // get offset of var in full buf row
    int getOffsetFull(int idx);
    int getOffsetFull(const char* var);

    // get number of bytes in var
    int getNumBytes(const char* var);

    // finalize registered vars
    void finalize();

protected:
    // cached variable headers and indexes
    std::unordered_map<std::string, int> registeredVarIdx = std::unordered_map<std::string, int>();
    std::vector<irsdk_varHeader> registeredVarHeaders = std::vector<irsdk_varHeader>();
    std::vector<int> offsetsFull = std::vector<int>();

    // is this list finalized?
    bool isFinalized = false;

};


class DataRow
{
public:
    DataRow();
    ~DataRow();

    // copy constructor
    DataRow(DataRow& other);

    // move constructor
    DataRow(DataRow&& other);

    // = operator
    DataRow& operator=(DataRow&& other);

    // init all the vectors and stuff
    static void init(int count);

    // get current data size
    static int getDataSize();
    // varHeaders that the data rows contain
    static std::vector<irsdk_varHeader> varHeaders;

    // set is init
    static void setInit(bool isInit);
    static bool isInit();

    // push to sizes
    static void pushSizes(int size);
    
    // set var/sample length: initializes the vector to accomodate length of vars
    static DataRow& getRow(int n);
    static void clearRows();

    // finalize data rows
    static void finalize(int n);
    void finalizeRow();

    // pointer to data
    const uint8_t* getDataP();
    const uint8_t* getDataP(int offset);

    // write to data
    template <typename T>
    void writeVar(T val, int offset)
    {
        T valcpy = val;
        if (!isFinal)
        {
            std::stringstream errors;
            errors << "DataRow@" << this << "::writeVar: ";
            errors << "Not finalised" << std::endl;
            throw std::runtime_error(errors.str());
        }
        if (offset >= DataRow::dataSize)
        {
            std::stringstream errors;
            errors << "DataRow::writeVar: offset out of bounds."<<std::endl;
            errors << "offset = " << offset << std::endl;
            errors << "dataSize = " << DataRow::dataSize << std::endl;
            throw std::runtime_error(errors.str());
        }
        //std::cout << "writeVar " << val << " offset " << offset << std::endl;
        //std::cout << "write to " << (void*)(data+offset) << std::endl;
        //std::cout << valcpy << std::endl;
        memcpy((data+offset), &valcpy, sizeof(T));
        return;
    }

    template <typename T>
    T readVarAt(int offset)
    {
        if (offset >= dataSize)
        {
            throw std::runtime_error("DataRow::readVar: offset out of bounds");
        }
        T result;
        memcpy(&result, data+offset, sizeof(T));
        return result;
    }

    static std::vector<DataRow> rows;


protected:
    //std::stringstream errors;
    static bool isFinal;
    static bool isInitialized;
    static int varCount;
    static std::vector<int> sizes;
    static int dataSize;
    uint8_t* data = nullptr;

};

#endif