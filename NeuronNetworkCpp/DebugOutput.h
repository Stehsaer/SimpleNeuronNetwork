#pragma once

#include<string>
#include<stdio.h>

void DebugOutput(std::string msg, bool quit = false)
{
	printf(("\033[1;31m" + msg + "\033[0m").c_str());
	if (quit) exit(-1);
}