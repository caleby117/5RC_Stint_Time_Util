
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include "irsdk_defines.h"
#include "IbtHandler.h"
#include "RegisteredVars.h"

IbtHandler::IbtHandler(){}

void IbtHandler::openIbtFile(const char* path)
{
    ibt = fopen(path, "rb");
    std::stringstream errors;

    // terminate if file not found
    if (ibt == NULL)
    {
        errors << "IbtHandler: Unable to open file path " << path << std::endl;
        throw std::runtime_error(errors.str());
    }

    // get file size of ibt
    fseek(ibt, 0, SEEK_END);
    filesize = ftell(ibt);
    fseek(ibt, 0, SEEK_SET);

    // read irsdk header
    if (sizeof(irsdk_header) != fread(&fileHeader, 1, sizeof(irsdk_header), ibt))
    {
        // failed to read all the bytes of the file into header
        errors << "DiskServer::DiskServer: Could not get irsdk_header" << std::endl;
        closeAll();
        throw std::runtime_error(errors.str());
    }

    // read irsdk_diskSubHeader
    if (sizeof(irsdk_diskSubHeader) != fread(&diskHeader, 1, sizeof(irsdk_diskSubHeader), ibt))
    {
        errors << "DiskServer::DiskServer: Could not get irsdk_diskSubHeader" << std::endl;
        closeAll();
        throw std::runtime_error(errors.str());
    }

    // check if file size is correct
    // get the expected total size of all the contents
    size_t expected_size = sizeof(irsdk_header) + sizeof(irsdk_diskSubHeader) + (fileHeader.numVars * sizeof(irsdk_varHeader)) + fileHeader.sessionInfoLen + (diskHeader.sessionRecordCount * fileHeader.bufLen);

    if (expected_size != filesize)
    {
        errors << "IbtHandler: File size does not correspond to header info." << std::endl;
        errors << "Expected Size: " << expected_size << std::endl;
        errors << "Actual Size: " << filesize << std::endl;
        closeAll();
        throw std::runtime_error(errors.str());
    }

    if (fileHeader.sessionInfoOffset + fileHeader.sessionInfoLen >= filesize)
    {
        errors << "IbtHandler: SessionInfo out of bounds" << std::endl;
        errors << "SessionInfoOffset = " << fileHeader.sessionInfoOffset << std::endl;
        errors << "SessionInfo End = " << fileHeader.sessionInfoOffset + fileHeader.sessionInfoLen << std::endl;
        closeAll();
        throw std::runtime_error(errors.str());
    }

    // read session string
    /*
    sessionString = new char[fileHeader.sessionInfoLen+1];
    if (fileHeader.sessionInfoLen != fileReadAt(sessionString, 1, fileHeader.sessionInfoLen, fileHeader.sessionInfoOffset))
    {
        // failed to read session string
        errors << "IbtHandler: Could not read Session String" << std::endl;
        closeAll();
        throw std::runtime_error(errors.str());
    }
    sessionString[fileHeader.sessionInfoLen] = '\0';
    */

    // reset the file position for next operations
    resetFilePos();

    int tickMax = 0;
    int bufNo = 0;
    // find the start of data
    for (int i = 0 ; i < IRSDK_MAX_BUFS; i++)
    {
        int tickCount = fileHeader.varBuf[i].tickCount;
        if (fileHeader.varBuf[i].tickCount > tickMax)
        {
            // this is the latest buffer
            tickMax = tickCount;
            bufNo = i;
        }
    }

    dataStart = fileHeader.varBuf[bufNo].bufOffset;


    // check that there is at least one tick sample here 
    if (dataStart + (fileHeader.bufLen * diskHeader.sessionRecordCount) >  filesize)
    {
        closeAll();
        errors << "IbtHandler: Discrepancy between number of samples and file size" << std::endl;
        errors << "samples end: " << dataStart+(fileHeader.bufLen*diskHeader.sessionRecordCount) << std::endl;
        errors << "filesize: " << filesize << std::endl;
        throw std::runtime_error(errors.str());
    }
    DataRow::setInit(false);
    std::cout << "Read " << path << " successful."<< std::endl;

}

IbtHandler::~IbtHandler()
{
    closeAll();
}

irsdk_header IbtHandler::getHeader()
{
    return fileHeader;
}

irsdk_header& IbtHandler::getHeaderRef()
{
    return fileHeader;
}

irsdk_diskSubHeader IbtHandler::getDiskHeader()
{
    return diskHeader;
}


irsdk_diskSubHeader& IbtHandler::getDiskHeaderRef()
{
    return diskHeader;
}

const std::string IbtHandler::getSessionString()
{
    return sessionString;
}

void IbtHandler::readVarHeaderInto(irsdk_varHeader* buf, int idx)
{
    readValueInto(buf, fileHeader.varHeaderOffset+(idx*sizeof(irsdk_varHeader)), sizeof(irsdk_varHeader));
}

int IbtHandler::getDataStart()
{
    return dataStart;
}


void IbtHandler::closeAll()
{
    // closes all handles
    if (ibt)
    {
        fclose(ibt);
        ibt = NULL;
    }
    if (sessionString)
    {
        delete[] sessionString;
        sessionString = NULL;
    }
}


size_t IbtHandler::fileReadAt(void* buf, size_t typesize, int numberItems, int pos)
{
    if (!ibt) return 0;
    fseek(ibt, pos, SEEK_SET);
    return fread(buf, typesize, numberItems, ibt);
}


void IbtHandler::resetFilePos()
{
    fseek(ibt, 0, SEEK_SET);
}


// DataRow definitions 

// ***** TODO *****
