#include "NetworkDataParser.h"
#include "DebugOutput.h"

#define _FILE_OP_SAFE_

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

int GetInt32(unsigned char* data, int offset)
{
	int out;
	memcpy(&out, data + offset, sizeof(int));
	return out;
}

void WriteInt32(int src, unsigned char* dst, int offset)
{
	int _src = src;
	memcpy(dst + offset, &src, sizeof(int));
}

int GetInt32Reverse(unsigned char* data, int offset)
{
	int out;
	memcpy(&out, data + offset, sizeof(int));
	Reverse(&out);
	return out;
}

void NetworkDataParser::ReadMNISTData(NetworkDataSet* dataSet, std::string dataPath, std::string labelPath, Network::Algorithm::NormalizationMode mode)
{
	// check if file is valid
	if (!is_regular_file(dataPath) || !is_regular_file(labelPath) || !exists(dataPath) || !exists(labelPath))
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

	int dataMagicNumber = GetInt32Reverse(data, 0);
	int labelMagicNumber = GetInt32Reverse(label, 0);

	// check magic number
	if (dataMagicNumber != DataMagicNumber || labelMagicNumber != LabelMagicNumber)
	{
		throw "Magic numbers mismatch.";
	}

	int dataCount = GetInt32Reverse(data, DataCountOffset);
	int labelCount = GetInt32Reverse(label, LabelCountOffset);

	// check if counts match
	if (dataCount != labelCount)
	{
		throw "Data and label count mismatch.";
	}

	int dataWidth = GetInt32Reverse(data, DataWidthOffset);
	int dataHeight = GetInt32Reverse(data, DataHeightOffset);

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

/*
=====================================================
|| DATA FORMAT FOR NEURON NETWORK STORAGE & EXPORT ||
=====================================================

Metadata Format: (Offsets are absolute)

	OFFSET		TYPE		DESCRIPTION
	-----------------------------------
	0x00		int			Magic Number (1949)
	0x04		int			inNeuronCount
	0x08		int			outNeuronCount
	0x0c		int			hiddenNeuronCount
	0x10		int			hiddenLayerCount
	0x14		int		ForwardActiveFunction (NOTE: Undefined = -1, Sigmoid=0, ShiftedSigmoid=1, ReLU=2)
	0x18		...			Actual Data

Actual Data Storage:

	NO			ITEM
	----------------
	BEFORE		Metadata
	1			outLayer
	2			hiddenLayer[0]
	3			hiddenLayer[1]
	...			...
	AFTER		End Data

NeuronLayer Data Format: (Offsets are relative)

	OFFSET		TYPE		DESCRIPTION
	-----------------------------------
	0x00		int			Magic Number (1978)
	0x04		int			prevCount
	0x08		int			neuronCount
	0x0c		double		bias
	0x14~0x1f	byte		Gap
	0x20		...			Neuron Data

Neuron Data Format: (Offsets are relative)

	OFFSET		TYPE		DESCRIPTION
	-----------------------------------
	0x00		int			Magic Number (2023)
	0x04		int			weightCount
	0x08~...	double		weights (LSB)

End Data Format: (Offsets are relative)

	OFFSET		TYPE		DESCRIPTION
	-----------------------------------
	0x00		int		Magic Number (0xc5c57f7f)
*/

const int MetadataMN = 1949;
const int NeuronLayerMN = 1978;
const int NeuronDataMN = 2023;
const int EndDataMN = 0xc5c57f7f;

int LengthRequiredByLayer(NeuronLayer* layer)
{
	int out = 0;

	out += 0x20; // Metadata

	for (auto& neuron : layer->neurons)
	{
		out += 0x08 + neuron.weightCount * sizeof(double);
	}

	return out;
}

void WriteData_Neuron(Neuron* neuron, unsigned char** data)
{
	unsigned char* ptr = *data;

	WriteInt32(NeuronDataMN, ptr, 0x00);
	WriteInt32(neuron->weightCount, ptr, 0x04);

	memcpy(ptr + 0x08, neuron->weights, sizeof(double) * neuron->weightCount);

	*data = ptr + 0x08 + sizeof(double) * neuron->weightCount; // shift original ptr
}

void WriteData_Layer(NeuronLayer* layer, unsigned char** data)
{
	unsigned char* ptr = *data;

	// Metadata
	WriteInt32(NeuronLayerMN, ptr, 0x00);
	WriteInt32(layer->prevCount, ptr, 0x04);
	WriteInt32(layer->Count(), ptr, 0x08);
	memcpy(ptr + 0x0c, &(layer->bias), sizeof(double)); // bias

	// Neuron Data
	unsigned char* ptrNeuron = ptr + 0x20;
	for (Neuron& neuron : layer->neurons)
	{
		WriteData_Neuron(&neuron, &ptrNeuron);
	}

	*data = ptrNeuron;
}

void NetworkDataParser::SaveNetworkData(Network::Framework::BackPropaNetwork* network, std::string path)
{
	// Calculate required space
	int lengthRequired = 0;
	lengthRequired += 0x18; // Metadata

	lengthRequired += LengthRequiredByLayer(&(network->outLayer)); // outLayer

	for (NeuronLayer& layer : network->hiddenLayerList)
	{
		lengthRequired += LengthRequiredByLayer(&layer);
	}

	lengthRequired += sizeof(int); // End data

	// DEBUG
	//printf("lengthRequired: %d\n", lengthRequired);
	//system("pause");

	unsigned char* data = new unsigned char[lengthRequired]; // Allocate space

	// Write Metadata
	WriteInt32(MetadataMN, data, 0x00);
	WriteInt32(network->inNeuronCount, data, 0x04);
	WriteInt32(network->outNeuronCount, data, 0x08);
	WriteInt32(network->hiddenNeuronCount, data, 0x0c);
	WriteInt32(network->hiddenLayerCount, data, 0x10);

	// Active Function
	if (network->ForwardActive == &Network::Algorithm::Sigmoid)
	{
		WriteInt32(0, data, 0x14);
	}
	else if (network->ForwardActive == &Network::Algorithm::ShiftedSigmoid)
	{
		WriteInt32(1, data, 0x14);
	}
	else if (network->ForwardActive == &Network::Algorithm::ReLU)
	{
		WriteInt32(2, data, 0x14);
	}
	else
	{
		WriteInt32(-1, data, 0x14);
	}

	// Actual Data
	unsigned char* ptr = data + 0x18;
	WriteData_Layer(&(network->outLayer), &ptr);

	for (NeuronLayer& layer : network->hiddenLayerList)
	{
		WriteData_Layer(&layer, &ptr);
	}

	WriteInt32(EndDataMN, ptr, 0x00);

	// DEBUG
	//printf("len: %d", (int)(ptr + 0x04 - data));
	//system("pause");

	// Write data to file

	FILE* file;

#ifdef _FILE_OP_SAFE_
	fopen_s(&file, path.c_str(), "wb");
#else
	file = fopen(path.c_str(), "wb");
#endif

	if (!file)
	{
		DebugOutput("Can't open file.", true);
	}

	fwrite(data, 1, lengthRequired, file);
	fclose(file);

	// free
	delete[] data;
}

#define READ_ERR_MSG DebugOutput("Error while parsing file.", true)

void ReadNetwork_Neuron(unsigned char** data, Neuron* neuron)
{
	unsigned char* ptr = *data;

	// check magic number
	if (GetInt32(ptr, 0x00) != NeuronDataMN)
	{
		READ_ERR_MSG;
	}

	// check weightCount
	if (neuron->weightCount != GetInt32(ptr, 0x04))
	{
		READ_ERR_MSG;
	}

	ptr += 0x08; // go to data section
	memcpy(neuron->weights, ptr, sizeof(double) * neuron->weightCount);

	*data = ptr + sizeof(double) * neuron->weightCount;
}

void ReadNetwork_Layer(unsigned char** data, NeuronLayer* layer)
{
	unsigned char* ptr = *data;

	// check magic number
	if (GetInt32(ptr, 0x00) != NeuronLayerMN)
	{
		READ_ERR_MSG;
	}

	// check count
	if (layer->prevCount != GetInt32(ptr, 0x04)
		|| layer->Count() != GetInt32(ptr, 0x08))
	{
		READ_ERR_MSG;
	}

	memcpy(&(layer->bias), ptr + 0x0c, sizeof(double)); // read bias

	ptr += 0x20; // go to neuron data

	for (Neuron& neuron : layer->neurons)
	{
		ReadNetwork_Neuron(&ptr, &neuron);
	}

	*data = ptr;
}

Network::Framework::BackPropaNetwork NetworkDataParser::ReadNetworkData(std::string _path)
{
	if (!exists(std::filesystem::path(_path)))
	{
		DebugOutput("File doesn't exists.", true);
	}

	// fetch file size
	int filesize = file_size(path(_path));

	// open file
	FILE* file;

#ifdef _FILE_OP_SAFE_
	fopen_s(&file, _path.c_str(), "rb");
#else
	file = fopen(_path.c_str(), "rb");
#endif

	if (!file)
	{
		DebugOutput("Can't open file.", true);
	}

	unsigned char* data = new unsigned char[filesize];

	fread(data, 1, filesize, file);
	fclose(file);
	
	if (GetInt32(data, 0x00) != MetadataMN)
		READ_ERR_MSG;

	int inNeuronCount = GetInt32(data, 0x04);
	int outNeuronCount = GetInt32(data, 0x08);
	int hiddenNeuronCount = GetInt32(data, 0x0c);
	int hiddenLayerCount = GetInt32(data, 0x10);
	int ForwardActiveFunction = GetInt32(data, 0x14);

	// select activation function
	double (*ForwardActive)(double);
	double (*BackwardActive)(double);

	switch (ForwardActiveFunction)
	{
	case 0:
		ForwardActive = &Network::Algorithm::Sigmoid;
		BackwardActive = &Network::Algorithm::Sigmoid_D;
		break;
	case 1:
		ForwardActive = &Network::Algorithm::ShiftedSigmoid;
		BackwardActive = &Network::Algorithm::ShiftedSigmoid_D;
		break;
	case 2:
		ForwardActive = &Network::Algorithm::ReLU;
		BackwardActive = &Network::Algorithm::ReLU_D;
		break;
	default:
		ForwardActive = &Network::Algorithm::Sigmoid;
		BackwardActive = &Network::Algorithm::Sigmoid_D;
		break;
	}


	Network::Framework::BackPropaNetwork outNetwork(inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount, ForwardActive, BackwardActive, DBL_MAX);

	unsigned char* ptr = data + 0x18;

	ReadNetwork_Layer(&ptr, &(outNetwork.outLayer));

	for (auto& layer : outNetwork.hiddenLayerList)
	{
		ReadNetwork_Layer(&ptr, &layer);
	}

	// check end data
	if (GetInt32(ptr, 0x00) != EndDataMN)
	{
		READ_ERR_MSG;
	}

	// free space
	delete[] data;
	return outNetwork;
}