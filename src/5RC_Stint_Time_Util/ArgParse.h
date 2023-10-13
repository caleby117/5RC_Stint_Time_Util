#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>


#ifndef _ARGPARSER_H
#define _ARGPARSER_H
class ArgParser
{
public:
    // singleton class
    static ArgParser& getInstance()
    {
        static ArgParser instance;
        return instance;
    };

    // create vector of arguments and set the argument values
    void parseArgs(const int argc, char** argv);

    std::string& getIbtPath();
    std::string& getOutputPath();
    std::vector<std::string>& getVarList();

    // for future - verbosity levels
    int getVerbose();

    bool hasError();

    // help string
    const std::string help = 
    "usage: 5RC_Stint_Time_Util [option] <ibt file path>\n"
    "<ibt file path> : File path for .ibt file to be read from (default: ./telemetry.ibt)\n"
    "-o path         : Output file path for .csv file (default: ./laps.csv) \n"
    "--vars path     : Path to txt file indicating which telemetry variables to "
    "read. Refer to irsdk for more details and full list of telemetry "
    "variables available. (default: SessionTick, Lap, LapLastLapTime)\n";

private:
    ArgParser() = default;
    ArgParser(const ArgParser&) = delete;
    ArgParser& operator=(const ArgParser&) = delete;
    static ArgParser instance;


    // stores the vars array
    std::vector<std::string> vars;
    bool error = false;

    bool hasIbtFileName = false;
    bool hasVars = false;

    // stores the values for the args
    std::unordered_map<std::string, std::string> argvals = 
    {
        {"-o", ""},
        {"--vars", ""},
        {"-v", "0"},
        {"ibtFilePath", "./telemetry.ibt"}
    };

    void splitVars(std::string& varPath);


};
#endif