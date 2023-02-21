#include "NetworkDataParser.h"

//#include "json/json.h"

//#define _FILE_OP_SAFE_

using namespace std::filesystem;
using namespace Network;

void Reverse(int* x) // convert MSB & LSB
{
	int y = 0;
	y = y | (0xff000000 & *x) >> 24;
	y = y | (0x00ff0000 & *x) >> 8;
	y = y | (0x0000ff00 & *x) << 8;
	y = y | (0x000000ff & *x) << 24;
	*x = y;
}

int GetInt32(unsigned char* data, int offset, bool reverse = true)
{
	int out;
	memcpy(&out, data + offset, sizeof(int));
	if (reverse) Reverse(&out);
	return out;
}

void NetworkDataParser::ReadMNISTData(NetworkDataSet* dataSet, std::string dataPath, std::string labelPath, Network::Algorithm::NormalizationMode mode)
{
	// check if file is valid
	if (!is_regular_file(dataPath) || !is_regular_file(labelPath))
	{
		throw "Invalid file input.";
	}

	// fetch data from file
	FILE* dataFileObj;
	FILE* labelFileObj;

	// open file
#ifdef _FILE_OP_SAFE_
	fopen_s(&dataFileObj, dataPath.c_str(), "rb");
	fopen_s(&labelFileObj, labelPath.c_str(), "rb");
#else
	dataFileObj = fopen(dataPath.c_str(), "rb");
	labelFileObj = fopen(labelPath.c_str(), "rb");
#endif

	// check if files get opened correctly
	if (!dataFileObj || !labelFileObj)
	{
		throw "Can't open files.";
	}

	// read data
	unsigned char* data, * label;

	int dataFileSize = file_size(path(dataPath));
	int labelFileSize = file_size(path(labelPath));

	data = new unsigned char[dataFileSize];
	label = new unsigned char[labelFileSize];

	fread(data, sizeof(unsigned char), dataFileSize, dataFileObj);
	fread(label, sizeof(unsigned char), labelFileSize, labelFileObj);

	fclose(dataFileObj); // close file streams correctly
	fclose(labelFileObj);

	int dataMagicNumber = GetInt32(data, 0);
	int labelMagicNumber = GetInt32(label, 0);

	// check magic number
	if (dataMagicNumber != DataMagicNumber || labelMagicNumber != LabelMagicNumber)
	{
		throw "Magic numbers mismatch.";
	}

	int dataCount = GetInt32(data, DataCountOffset);
	int labelCount = GetInt32(label, LabelCountOffset);

	// check if counts match
	if (dataCount != labelCount)
	{
		throw "Data and label count mismatch.";
	}

	int dataWidth = GetInt32(data, DataWidthOffset);
	int dataHeight = GetInt32(data, DataHeightOffset);

	// push data
	for (int i = 0; i < dataCount; i++)
	{
		// NOTE: Added 2023-2-20
		NetworkData* networkdata = new NetworkData();
		networkdata->label = *(label + LabelOffset + i);
		networkdata->data = Algorithm::NormalizeData(data, DataOffset + i * dataWidth * dataHeight, dataWidth * dataHeight, mode);
		networkdata->dataSize = dataWidth * dataHeight;
		
		dataSet->AddData(networkdata);
	}

	dataSet->dataHeight = dataHeight;
	dataSet->dataWidth = dataWidth;

	// free allocated spaces
	delete[] label;
	delete[] data;
}

/*void NetworkDataParser::SaveNetworkData(std::string outputPath, Network::Framework::BackPropaNetwork& network)
{
	Json::Value outValue;

	outValue["inNeuronCount"] = network.inNeuronCount;
	outValue["outNeuronCount"] = network.outNeuronCount;
	outValue["hiddenLayerCount"] = network.hiddenLayerCount;
	outValue["hiddenNeuronCount"] = network.hiddenNeuronCount;

	for (int i = 0; i < network.hiddenLayerCount; i++)
	{
		outValue["HiddenLayers"]["bias"] = network.hiddenLayerList[i].bias;
		outValue["HiddenLayers"]["prevCount"] = network.hiddenLayerList[i].prevCount;

		for (int j = 0; j < network.hiddenNeuronCount; j++)
		{
			outValue["hiddenLayers"]["neurons"][j]["weightCount"] = network.hiddenLayerList[i][j].weightCount;

			for (int m = 0; m < network.hiddenLayerList[i].prevCount; m++)
			{
				outValue["hiddenLayers"]["neurons"][j]["weights"][m] = network.hiddenLayerList[i][j].weights[m];
			}
		}
	}
}*/
