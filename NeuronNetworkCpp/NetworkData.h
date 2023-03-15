#pragma once

#ifndef _NETWORK_DATA_H_
#define _NETWORK_DATA_H

#include <filesystem>
#include<vector>
#include<string>
#include<iostream>

#include "NetworkStructure.h"
#include "NetworkAlgorithm.h"

namespace Network
{
	/* 
	CHANGE LOG:
		
		2023-2-20:
			CHANGE data storing type from double to float; cut down memory space needed
			CHANGE NetworkData(double*,int,int) to NetworkData(); move initialization function to NetworkDataParser
	*/
	struct NetworkData
	{
	public:
		float* data;
		int label, dataSize;

		NetworkData();
		void DrawContentDebug(int width, Algorithm::NormalizationMode mode);
	};

	class NetworkDataSet
	{
	public:
		std::vector<NetworkData*> dataSet;
		int dataWidth, dataHeight;
		std::string name;

		Network::Algorithm::NormalizationMode mode;

		NetworkDataSet(std::string name = "") : dataWidth(0), dataHeight(0), name(name) {}
		int Count();
		void AddData(NetworkData* data);
		NetworkData& operator[](int index);
		void Destroy(); // clear all data in dataSet
	};
}

#endif
