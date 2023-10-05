#pragma once 

#include "irsdk_defines.h"
#include "StintVars.h"
#include <string>
#include <vector>
#include <unordered_map>

#ifndef IBTHANDLER_H
#define IBTHANDLER_H

class IbtHandler
{
public:
    IbtHandler();
    ~IbtHandler();

    void openIbtFile(const char* path);

    // get Headers
    irsdk_header getHeader();
    irsdk_header& getHeaderRef();
    irsdk_diskSubHeader getDiskHeader();
    irsdk_diskSubHeader& getDiskHeaderRef();

    // get session string
    const std::string getSessionString();

    // read single variable of sample
    template <typename T>
    T readVar(int offset, int sampleIdx)
    {
        T val;

        readValueInto(&val, dataStart+(sampleIdx * fileHeader.bufLen)+offset, sizeof(T));

        return val;
    }

    // read varHeader into buffer
    void readVarHeaderInto(irsdk_varHeader* buf, int idx);

    // get data start
    int getDataStart();


protected:
    void closeAll();

    template <typename T>
    void readValueInto(T* buf, size_t pos, int expectedSize)
    {
        if (!ibt)
        {
            closeAll();
            throw std::runtime_error("IbtHandler::readValueInto: bad ibt file");
        }

        fseek(ibt, pos, SEEK_SET);
        int read_bytes = fread(buf, 1, sizeof(T), ibt);
        if (expectedSize != read_bytes)
        {
            closeAll();
            throw std::runtime_error("IbtHandler::readValueInto: Did not read expected number of bytes into buffer");
        }

    }

    // file operations
    size_t fileReadAt(void* buf, size_t typesize, int numberItems, int pos);
    void resetFilePos();

    // ibt file*
    FILE* ibt = NULL;
    size_t filesize;

    // irsdk_header
    irsdk_header fileHeader;

    // irsdk_diskSubHeader
    irsdk_diskSubHeader diskHeader;

    // session String
    char* sessionString = NULL;

    // data start
    int dataStart;


};


#endif

