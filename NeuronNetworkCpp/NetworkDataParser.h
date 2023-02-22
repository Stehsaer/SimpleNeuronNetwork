#pragma once

#ifndef _NETWORK_DATA_PARSER_H_
#define _NETWORK_DATA_PARSER_H_

#include<string>
#include<filesystem>
#include"NetworkFramework.h"

namespace Network
{
	/*
	CHANGELOG:

		2023-2-20:
			MODIFY ReadMnistData(...) Logic; cut down memory space needed
	*/
	class NetworkDataParser
	{
	public:
		static const int DataCountOffset = 4;
		static const int DataWidthOffset = 8;
		static const int DataHeightOffset = 12;
		static const int DataOffset = 16;

		static const int LabelCountOffset = 4;
		static const int LabelOffset = 8;

		static const int DataMagicNumber = 2051;
		static const int LabelMagicNumber = 2049;

		static void ReadMNISTData(NetworkDataSet* dataSet, std::string dataPath, std::string labelPath, Network::Algorithm::NormalizationMode mode);
		static void SaveNetworkData(Network::Framework::BackPropaNetwork* network, std::string path);
		static Network::Framework::BackPropaNetwork ReadNetworkData(std::string path);
	};
}

#endif

