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
	struct NetworkData
	{
	public:
		double* data;
		int label, dataSize;

		NetworkData(double* data, int label, int dataSize);
		void DrawContentDebug(int width, Algorithm::NormalizationMode mode);
	};

	class NetworkDataSet
	{
	public:
		std::vector<NetworkData*> dataSet;
		int dataWidth, dataHeight;

		NetworkDataSet(): dataWidth(0), dataHeight(0) {}
		int Count();
		void AddData(NetworkData* data);
		NetworkData& operator[](int index);
		void Destroy(); // clear all data in dataSet
	};
}

#endif
