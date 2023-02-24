#pragma once

#include<string>
#include<stdio.h>

inline void DebugOutput(std::string msg, bool quit = false)
{
	//printf(("\033[1;31m" + msg + "\033[0m").c_str());
	//if (quit) exit(-1);
}

inline void PrintError(std::string err, bool doVerbose = true)
{
	if(doVerbose) printf("\033[1;7;31m ERR \033[0;1;31m %s\033[0m\n", err.c_str());
}