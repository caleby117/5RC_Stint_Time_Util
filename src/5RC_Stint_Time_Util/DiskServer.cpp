#include <unordered_map>
#include <unordered_set>
#include "irsdk_defines.h"
#include "StintVars.h"
#include <stdint.h>
#include "DiskServer.h"
#include <iostream>
#include <assert.h>

#ifdef _WIN32
// windows install
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifdef __linux__
#include <wchar.h>
#include <cfloat>
#endif


DiskServer::DiskServer(const char* path)
{
    ibtReader.openIbtFile(path);

}

DiskServer::~DiskServer() {}

irsdk_header DiskServer::getHeader()
{
    return ibtReader.getHeader();
}


irsdk_diskSubHeader DiskServer::getDiskHeader()
{
    return ibtReader.getDiskHeader();
}


const std::string DiskServer::getSessionString()
{
    return ibtReader.getSessionString();
}


int DiskServer::registerVars(const char* vars[], int count)
{

    clearHeaderLists();
    initHeaderLists(count);
    DataRow::init(regVars.numRegisteredVars());

    // init vars
    std::unordered_set<std::string> stintVars = std::unordered_set<std::string>();
    for (int i = 0; i < count; i++)
    {
        stintVars.insert(vars[i]);
    }

    irsdk_varHeader curVar;
    for (int i = 0; i < ibtReader.getHeaderRef().numVars; i++)
    {
        ibtReader.readVarHeaderInto(&curVar, i);
        if (stintVars.find(curVar.name) != stintVars.end())
        {
            std::cout << "Registering var " << curVar.name << std::endl;
            // we're interested in this var
            regVars.registerVar(curVar);

            // BACK TO DISKSERVER
            stintVars.erase(curVar.name);
            if (stintVars.empty()) break;
        }
    }

    regVars.finalize();
    
    return 0;
}

int DiskServer::readSamplesNewLap()
{
    // get index of lap number and last lap time
    int lapIdx = regVars.findIndex("Lap");
    int lastLapTimeIdx = regVars.findIndex("LapLastLapTime");

    // lap offset in file
    int fLapOffset = regVars.getOffsetFull("Lap");
    int nBytes = regVars.getNumBytes("Lap");

    // last lap time offset in file
    int fLastLapTimeOffset = regVars.getOffsetFull("LapLastLapTime");
    int nBytesLastLapTime = regVars.getNumBytes("LapLastLapTime");

    // move cursor to second sample in the data
    int start = ibtReader.getDataStart() + ibtReader.getHeaderRef().bufLen;


    // find the first sample of a new lap when driver next crosses the line
    // Look at the next 2 seconds worth of samples and see if
    // LapLastLapTime has been updated
    // Store the index of the sample
    int curLapNum;
    int prevLapNum = -1;
    float curLastLapTime = 0;
    float prevLastLapTime = 0;
    for (int i = 1; i < ibtReader.getDiskHeaderRef().sessionRecordCount; i++)
    {
        curLapNum = ibtReader.readVar<int>(fLapOffset, i);
        if (prevLapNum < 0) prevLapNum = curLapNum;
        
        // check if we have crossed the start/finish and have started a new lap
        if (curLapNum != prevLapNum)
        {
            bool identicalLapTimes = true;
            // new lap. search the next 2 seconds worth of samples for the new time
            int endSample = i+(2*ibtReader.getHeaderRef().tickRate);
            for (;i<endSample;i++)
            {
                if (i >= ibtReader.getDiskHeaderRef().sessionRecordCount)
                {
                    identicalLapTimes = false;
                    break;
                }
                prevLastLapTime = curLastLapTime;
                // find the first sample that has the updated curLastLapTime
                curLastLapTime = ibtReader.readVar<float>(fLastLapTimeOffset, i);
                if (curLastLapTime!=prevLastLapTime)
                {
                    // save the sample index
                    sampleIdxNewLap.push_back(i);
                    identicalLapTimes = false;
                    break;
                }
            }
            
            // identical lap times (extremely unlikely)
            if (identicalLapTimes)
            {
                // pick the first sample of the new lap according to LapNum
                sampleIdxNewLap.push_back(i-(2*ibtReader.getHeaderRef().tickRate));
            }
            std::cout << "Found new lap " << curLapNum << std::endl;
        }
        prevLapNum = curLapNum;
    }
    DataRow::finalize(sampleIdxNewLap.size());
    return 0;
}


int DiskServer::getNumberLaps()
{
    return sampleIdxNewLap.size();
}

void DiskServer::writeSamples()
{
    // write the samples ot the datarow
    for (int samplei = 0; samplei < sampleIdxNewLap.size(); samplei++)
    {
        int sampleIdx = sampleIdxNewLap[samplei];
        DataRow& row = DataRow::getRow(samplei);
        for (int i = 0; i < regVars.numRegisteredVars(); i++)
        {
            irsdk_varHeader vh = regVars.getVarHeader(i);
            int dataRowOffset = vh.offset;
            int type = vh.type;

            switch (type)
            {
                case irsdk_bool:
                {
                    bool valbool = ibtReader.readVar<bool>(regVars.getOffsetFull(vh.name), sampleIdx);
                    row.writeVar<bool>(valbool, dataRowOffset);
                    break;
                }

                case irsdk_char:
                {
                    char valchar = ibtReader.readVar<char>(regVars.getOffsetFull(vh.name), sampleIdx);
                    row.writeVar<char>(valchar, dataRowOffset);
                    break;
                }

                case irsdk_bitField:
                case irsdk_int:
                {
                    int valint = ibtReader.readVar<int>(regVars.getOffsetFull(vh.name), sampleIdx);
                    row.writeVar<int>(valint, dataRowOffset);
                    break;
                }
                
                case irsdk_float:
                {
                    float valfloat = ibtReader.readVar<float>(regVars.getOffsetFull(vh.name), sampleIdx);
                    row.writeVar<float>(valfloat, dataRowOffset);
                    break;
                }
                
                case irsdk_double:
                {
                    double valdouble = ibtReader.readVar<double>(regVars.getOffsetFull(vh.name), sampleIdx);
                    break;
                }
            }
        }

    }
}


void DiskServer::readVarSS(std::stringstream& stream, const char* name, int sampleIdx)
{

    irsdk_varHeader& vh = regVars.getVarHeaderRef(name);
    int dataRowOffset = vh.offset;
    int type = vh.type;

    switch (type)
    {
        case irsdk_bool:
        {
            bool valBool = DataRow::getRow(sampleIdx).readVarAt<bool>(vh.offset);
            stream << valBool;
            break;
        }


        case irsdk_char:
        {
            char valChar = DataRow::getRow(sampleIdx).readVarAt<char>(vh.offset);
            stream << valChar;
            break;
        }

        case irsdk_bitField:
        case irsdk_int:
        {
            int valInt = (DataRow::getRow(sampleIdx)).readVarAt<int>(vh.offset);
            stream << valInt;
            break;
        }
        
        case irsdk_float:
        {
            float valFloat = DataRow::getRow(sampleIdx).readVarAt<float>(vh.offset);
            stream << valFloat;
            break;
        }
        
        case irsdk_double:
        {
            double valDouble = DataRow::getRow(sampleIdx).readVarAt<double>(vh.offset);
            stream << valDouble;
            break;
        }
    }
}

void DiskServer::clearHeaderLists()
{
    // clear all the header lists
    DataRow::clearRows();
    sampleIdxNewLap.clear();
    DataRow::setInit(false);

}

void DiskServer::initHeaderLists(int count)
{
    DataRow::init(count);
    DataRow::setInit(true);
}
