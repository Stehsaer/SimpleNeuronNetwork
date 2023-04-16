#include "NetworkDataParser.h"
#include "ProgressTimer.h"
#include "FileHelper.h"

#include <format>
#include <json/json.h>

using namespace Network;
using FCNetwork = Network::Framework::FullConnNetwork;

/// <summary>
/// Tell if the computer runs on a little-endian system
/// </summary>
/// <returns>True for little-endian, False for big-endian</returns>
bool isLittleEndian()
{
	union U
	{
		int  i;
		char c;
	};
	U u;
	u.i = 1;
	return u.c == 1;
}

/// <summary>
/// Convert a number between MSB and LSB
/// </summary>
/// <param name="x">Target int</param>
void Reverse(int* x)
{
	int y = (0xff000000 & *x) >> 24;
	y = y | (0x00ff0000 & *x) >> 8;
	y = y | (0x0000ff00 & *x) << 8;
	y = y | (0x000000ff & *x) << 24;
	*x = y;
}

/// <summary>
/// Get int number from a specfic memory location
/// </summary>
/// <param name="data">Data pointer</param>
/// <param name="offset">Offset from the starting position</param>
/// <returns>Int</returns>
int GetInt32NoReverse(unsigned char* data, int offset)
{
	int out;
	memcpy(&out, data + offset, sizeof(int));
	return out;
}

/// <summary>
/// Get int number from a specfic memory location
/// </summary>
/// <param name="data">Data pointer</param>
/// <param name="offset">Offset from the starting position</param>
/// <returns>Int</returns>
int GetInt32Reverse(unsigned char* data, int offset)
{
	int out;
	memcpy(&out, data + offset, sizeof(int));
	Reverse(&out);
	return out;
}

/// <summary>
/// Write an int into a specfic memory location
/// </summary>
/// <param name="src">Source number</param>
/// <param name="dst">Target data</param>
/// <param name="offset">Offset from start</param>
void WriteInt32(int src, unsigned char* dst, int offset)
{
	int _src = src;
	if (isLittleEndian()) Reverse(&_src);
	memcpy(dst + offset, &_src, sizeof(int));
}

/// <summary>
/// Get an int, endian tackled
/// </summary>
/// <param name="data">Data pointer</param>
/// <param name="offset">Offset</param>
/// <returns></returns>
int GetInt32(unsigned char* data, int offset)
{
	if (isLittleEndian())
		return GetInt32Reverse(data, offset);
	else
		return GetInt32NoReverse(data, offset);
}

ProcessState NetworkDataParser::ReadMNISTData(NetworkDataSet* dataSet, std::string dataPath, std::string labelPath, Network::Algorithm::NormalizationMode mode)
{
	// read file
	File dataFileObj(dataPath);
	File labelFileObj(labelPath);

	unsigned char* data, * label;

	uintmax_t dataFileSize;
	uintmax_t labelFileSize;

	dataFileObj.ReadAllBytes(&data, &dataFileSize);
	labelFileObj.ReadAllBytes(&label, &labelFileSize);

	// check file size, current limited at 4G;
	if (dataFileSize > UINT32_MAX || labelFileSize > UINT32_MAX)
	{
		return ProcessState(false, "File Too Large");
	}

	int dataMagicNumber = GetInt32(data, 0);
	int labelMagicNumber = GetInt32(label, 0);

	// check magic number
	if (dataMagicNumber != DataMagicNumber || labelMagicNumber != LabelMagicNumber)
	{		
		delete[] data;
		delete[] label;
		return ProcessState(false, "Can't Parse File");
	}

	int dataCount = GetInt32(data, DataCountOffset);
	int labelCount = GetInt32(label, LabelCountOffset);

	// check if counts match
	if (dataCount != labelCount)
	{
		delete[] data;
		delete[] label;
		return ProcessState(false, "Can't Parse File");
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

	dataSet->mode = mode;

	// free allocated spaces
	delete[] label;
	delete[] data;

	return ProcessState(true);
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

ProcessState NetworkDataParser::SaveNetworkData(FCNetwork* network, std::string path)
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

	unsigned char* data = new unsigned char[lengthRequired]; // Allocate space

	if (!data)
	{
		return ProcessState(false, "Running Out Of Memory Space");
	}

	// Write Metadata
	WriteInt32(MetadataMN, data, 0x00);
	WriteInt32(network->inNeuronCount, data, 0x04);
	WriteInt32(network->outNeuronCount, data, 0x08);
	WriteInt32(network->hiddenNeuronCount, data, 0x0c);
	WriteInt32(network->hiddenLayerCount, data, 0x10);

	WriteInt32((int)network->ActivateFunc, data, 0x14);

	// Actual Data
	unsigned char* ptr = data + 0x18;
	WriteData_Layer(&(network->outLayer), &ptr);

	for (NeuronLayer& layer : network->hiddenLayerList)
	{
		WriteData_Layer(&layer, &ptr);
	}

	WriteInt32(EndDataMN, ptr, 0x00);

	File file(path);
	if (!file.WriteAllBytes(data, lengthRequired))
	{
		delete[] data;

		return ProcessState(false, "Can't Write File");
	}

	// free
	delete[] data;

	return ProcessState(true);
}

bool ReadNetwork_Neuron(unsigned char** data, Neuron* neuron)
{
	unsigned char* ptr = *data;

	// check magic number
	if (GetInt32(ptr, 0x00) != NeuronDataMN)
	{
		return false;
	}

	// check weightCount
	if (neuron->weightCount != GetInt32(ptr, 0x04))
	{
		return false;
	}

	ptr += 0x08; // go to data section
	memcpy(neuron->weights, ptr, sizeof(double) * neuron->weightCount);

	*data = ptr + sizeof(double) * neuron->weightCount;
	return true;
}

bool ReadNetwork_Layer(unsigned char** data, NeuronLayer* layer)
{
	unsigned char* ptr = *data;

	// check magic number
	if (GetInt32(ptr, 0x00) != NeuronLayerMN)
	{
		return false;
	}

	// check count
	if (layer->prevCount != GetInt32(ptr, 0x04)
		|| layer->Count() != GetInt32(ptr, 0x08))
	{
		return false;
	}

	memcpy(&(layer->bias), ptr + 0x0c, sizeof(double)); // read bias

	ptr += 0x20; // go to neuron data

	for (Neuron& neuron : layer->neurons)
	{
		if (!ReadNetwork_Neuron(&ptr, &neuron))
		{
			return false;
		}
	}

	*data = ptr;
	return true;
}

ProcessState NetworkDataParser::ReadNetworkData(FCNetwork** network, std::string _path)
{
	unsigned char* data = nullptr;
	size_t size;

	File file(_path);

	if (!file.Exists())
	{
		return ProcessState(false, "No Such File");
	}

	if (!file.ReadAllBytes(&data, &size))
	{
		return ProcessState(false, "File Access failed");
	}
	
	if (GetInt32(data, 0x00) != MetadataMN)
	{
		delete[] data;

		return ProcessState(false, "Network Reading Error (Magic Number Mismatch)");
	}

	int inNeuronCount = GetInt32(data, 0x04);
	int outNeuronCount = GetInt32(data, 0x08);
	int hiddenNeuronCount = GetInt32(data, 0x0c);
	int hiddenLayerCount = GetInt32(data, 0x10);
	int ActivateFunctionCode = GetInt32(data, 0x14);

	FCNetwork * nwk = new FCNetwork(inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount, (ActivateFunctionType)ActivateFunctionCode, DBL_MAX);

	unsigned char* ptr = data + 0x18;

	if (!ReadNetwork_Layer(&ptr, &(nwk->outLayer)))
	{
		nwk->Destroy();

		delete nwk;
		delete[] data;

		return ProcessState(false, "Network Reading Error (Out Layer)");
	}

	for (auto& layer : nwk->hiddenLayerList)
	{
		if (!ReadNetwork_Layer(&ptr, &layer))
		{
			nwk->Destroy();

			delete nwk;
			delete[] data;

			return ProcessState(false, "Network Reading Error (Hidden Layer)");
		}
	}

	// check end data
	if (GetInt32(ptr, 0x00) != EndDataMN)
	{
		nwk->Destroy();

		delete nwk;
		delete[] data;

		return ProcessState(false, "Network Reading Error (End Data Mismatch)");
	}

	// free space
	delete[] data;

	*network = nwk;

	return ProcessState(true);
}

Json::Value SaveNeuronJSON(Neuron* neuron)
{
	Json::Value root;

	root["weight_count"] = neuron->weightCount;

	for (int i = 0; i < neuron->weightCount; i++)
	{
		root["weights"].append(neuron->weights[i]);
	}

	return root;
}

Json::Value SaveLayerJSON(NeuronLayer* layer)
{
	Json::Value root;

	// root parameters
	root["prev_count"] = layer->prevCount;
	root["bias"] = layer->bias;
	root["neuron_count"] = layer->Count();

	// neuron data
	for (auto& neuron : layer->neurons)
		root["neurons"].append(SaveNeuronJSON(&neuron));

	return root;
}

ProcessState NetworkDataParser::SaveNetworkDataJSON(FCNetwork* network, std::string path)
{
	try
	{
		ProgressTimer timer;

		Json::Value root;

		// Save Root Parameters
		root["activate_func"] = (int)network->ActivateFunc;
		root["in_count"] = network->inNeuronCount;
		root["out_count"] = network->outNeuronCount;
		root["hidden_neuron_count"] = network->hiddenNeuronCount;
		root["hidden_layer_count"] = network->hiddenLayerCount;

		// out layer
		root["out_layer_data"] = SaveLayerJSON(&network->outLayer);

		// hidden layers
		for (auto& layer : network->hiddenLayerList)
			root["hidden_layer_data"].append(SaveLayerJSON(&layer));

		// write json to string
		std::string content = Json::FastWriter().write(root);
		
		// write string to file
		if (!File(path).WriteAllText(content))
		{
			// fail
			return ProcessState(false, "Failed to save network.");
		}

		return ProcessState(true, "Network Saved.", timer.Count());
	}
	catch (std::exception e)
	{
		return ProcessState(false, std::format("Unhandled Exception: {}", e.what()));
	}
}

void ThrowLogicError(std::string what)
{
	throw Json::LogicError(what);
}

Json::Value GetMember(Json::Value& root, std::string key)
{
	if (root.isMember(key))
		return root[key];
	else
		ThrowLogicError("Can't find member: " + key);
}

Network::Neuron GetNeuronJSON(Json::Value val)
{
	int weightCount = GetMember(val, "weight_count").asInt();
	Json::Value weights = GetMember(val, "weights");

	Network::Neuron neuron(weightCount);

	for (int i = 0; i < weightCount; i++)
	{
		neuron.weights[i] = weights[i].asDouble();
	}

	return neuron;
}

Network::NeuronLayer GetLayerJSON(Json::Value val)
{
	int prevCount = GetMember(val, "prev_count").asInt();
	int neuronCount = GetMember(val, "neuron_count").asInt();
	double bias = GetMember(val, "bias").asDouble();

	Network::NeuronLayer layer;
	
	layer.prevCount = prevCount;
	layer.bias = bias;

	if (!GetMember(val, "neurons").isArray())
	{
		ThrowLogicError("Can't read key: neurons");
	}

	if (GetMember(val, "neurons").size() != neuronCount)
	{
		ThrowLogicError("Can't parse data: array size mismatch");
	}

	for (int i = 0; i < neuronCount; i++)
	{
		layer.neurons.push_back(GetNeuronJSON(val["neurons"][i]));
	}

	return layer;
}

ProcessState NetworkDataParser::ReadNetworkDataJSON(FCNetwork** network, std::string path)
{
	try
	{
		ProgressTimer timer;

		std::string content;

		// read string from file
		if (!File(path).ReadAllText(&content))
		{
			// fail
			return ProcessState(false, "Failed to read file!");
		}

		Json::Value root;
		
		// parse raw json string
		if (!Json::Reader().parse(content, root))
		{
			// fail
			return ProcessState(false, "Not a valid json file!");
		}

		Network::ActivateFunctionType activateFunc = (Network::ActivateFunctionType)GetMember(root, "activate_func").asInt();
		int in_count = GetMember(root, "in_count").asInt();
		int out_count = GetMember(root, "out_count").asInt();
		int hidden_neuron_count = GetMember(root, "hidden_neuron_count").asInt();
		int hidden_layer_count = GetMember(root, "hidden_layer_count").asInt();

		if (GetMember(root, "hidden_layer_data").size() != hidden_layer_count)
		{
			return ProcessState(false, "Not a valid network!");
		}

		FCNetwork* net = new FCNetwork(in_count, GetLayerJSON(GetMember(root, "out_layer_data")), hidden_neuron_count, hidden_layer_count, activateFunc);

		for (int i = 0; i < hidden_layer_count; i++)
		{
			net->hiddenLayerList.push_back(GetLayerJSON(root["hidden_layer_data"][i]));
		}

		if (*network)
		{
			(**network).Destroy();
		}

		*network = net;

		return ProcessState(true, "", timer.Count());

	}
	catch (Json::Exception e)
	{
		return ProcessState(false, std::format("Can't parse json file. Message: {}", e.what()));
	}
	catch (std::exception e)
	{
		return ProcessState(false, std::format("Unhandled Exception: {}", e.what()));
	}
}