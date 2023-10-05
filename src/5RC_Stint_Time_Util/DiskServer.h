#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <sstream>
#include "irsdk_defines.h"
#include "RegisteredVars.h"
#include "IbtHandler.h"

#ifndef DISKSERVER_H
#define DISKSERVER_H


class DiskServer
{
public:
    // constructor and destructor
    DiskServer(const char* path);
    ~DiskServer();


    // get Headers
    irsdk_header getHeader();
    irsdk_diskSubHeader getDiskHeader();

    // get session string
    const std::string getSessionString();

    // init vars: do the initial processing and search of the vars;
    int registerVars(const char* vars[], int count);

    // populate sampleIdxNewLap
    int readSamplesNewLap();

    int getNumberLaps();

    // write samples whose idx in sampleIdxNewLap to datarows
    void saveSamplesNewLap();

    // read a single variable of a particular sample
    template <typename T>
    T readVar(const char* var, int sampleIdx)
    {
        /** TODO **/
        DataRow& sample = DataRow::getRow(sampleIdx);

        int offset = regVars.getOffset(var);
        return sample.readVarAt<T>(offset);
    }

    // read var into a stream
    size_t readVarToStream(std::iostream& stream, const char* name, int sampleIdx);
    size_t readVarToStream(std::iostream& stream, const int varIdx, int sampleIdx);
    size_t readVarToStream(std::iostream& stream, irsdk_varHeader& vh, int sampleIdx);

    // output the file into csv
    size_t writeCSV(std::string& path);
    size_t writeCSV();

protected:
    // file path
    std::string ibtPath;


    bool is_init = false;

    // clear the samples and lists
    void clearHeaderLists();

    void initHeaderLists(int count);

    // ibt reader
    IbtHandler ibtReader;

    // samples
    DataRow samples;

    // sample indices of new lap 
    std::vector<int> sampleIdxNewLap = std::vector<int>();

    // information about registered variables
    RegisteredVars regVars = RegisteredVars();

};


#endif