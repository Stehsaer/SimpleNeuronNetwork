#pragma once

#ifndef _NETWORK_DATA_PARSER_H_
#define _NETWORK_DATA_PARSER_H_

#include<string>
#include<filesystem>
#include"NetworkFramework.h"

namespace Network
{
	struct ProcessState // A struct to indicate success or not
	{
		std::string msg;
		bool success;

		ProcessState(bool success, std::string msg = "") : msg(msg), success(success) {}
	};

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

		static ProcessState ReadMNISTData(NetworkDataSet* dataSet, std::string dataPath, std::string labelPath, Network::Algorithm::NormalizationMode mode);
		static ProcessState SaveNetworkData(Network::Framework::BackPropaNetwork* network, std::string path);
		static ProcessState ReadNetworkData(Network::Framework::BackPropaNetwork** network, std::string path);
	};
}

#endif

