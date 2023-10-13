#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <fstream>
#include "ArgParse.h"

#define _IRSDK_DEFAULT_VARS {"SessionTick", "Lap", "LapLastLapTime"}


void ArgParser::parseArgs(const int argc, char** argv)
{
    if (argc < 2)
    {
        error = true;
        return;
    }
    // parses the arguments 
    for (int i = 1; i < argc; i++)
    {
        const char* arg = argv[i];
        //std::cout << "Parsing arg " << arg << std::endl;
        if (argv[i][0] == '-')
        {
            // check if it is a valid arg
            if (argvals.find(arg) == argvals.end())
            {
                // invalid argument
                error = true;
                std::cout << "Invalid argument " << arg << std::endl;
                break;
            }

            // ensure that it's not the last argument
            else if (i == argc-1)
            {
                error = true;
                std::cout << "Invalid syntax " << arg << std::endl;
                break;
            }

            // ensure that the next argument does not start with a '-'
            else if (argv[i+1][0] == '-')
            {
                error = true;
                std::cout << "Invalid syntax " << arg << std::endl;
                break;
            }

            // consume the next argv
            argvals[arg] = argv[++i];
            std::cout << "Arg " << arg << " with value of " << argvals[arg] << std::endl;
            continue;
        }
        else if (hasIbtFileName)
        {
            // wrong syntax
            std::cout << "Arg " << arg << " invalid syntax " << std::endl;
            error = true;
            break;
        }

        // else set this as the name of the file
        argvals["ibtFilePath"] = arg;
        hasIbtFileName = true;
    }

    // convert vars from string to vector
    if (argvals["--vars"] != "") 
    {
        splitVars(argvals["--vars"]);
        hasVars = true;
    }
    else
    {
        vars = _IRSDK_DEFAULT_VARS; 
    }

    // set default value of output
    argvals["-o"] = std::move(argvals["ibtFilePath"].substr(0, argvals["ibtFilePath"].size()-4) + ".csv");
}

bool ArgParser::hasError()
{
    return error;
}

std::string& ArgParser::getIbtPath()
{
    return argvals["ibtFilePath"];
}

std::string& ArgParser::getOutputPath()
{
    return argvals["-o"];
}

std::vector<std::string>& ArgParser::getVarList()
{
    return vars;
}

void ArgParser::splitVars(std::string& varPath)
{
    std::ifstream varfile(varPath);
    std::string var;
    while (!varfile.eof())
    {
        varfile >> var;
        vars.push_back(var);
    }
}
