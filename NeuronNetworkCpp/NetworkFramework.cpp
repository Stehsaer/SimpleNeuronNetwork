#include "NetworkFramework.h"

#include "NetworkStructure.h"
#include "NetworkAlgorithm.h"
#include "NetworkData.h"

#include <float.h>

using namespace Network::Connectivity;
using namespace Network::Algorithm;

typedef Network::float_n float_n;

FullConnNetwork::FullConnNetwork(int inNeuronCount, int outNeuronCount, int hiddenNeuronCount, int hiddenLayerCount, ActivateFunctionType ActivateFunc, float_n learningRate)
{
	if (inNeuronCount <= 0 || outNeuronCount <= 0 || hiddenNeuronCount <= 0 || hiddenLayerCount <= 0 || learningRate < 0.0)
	{
		throw std::exception("Invalid Parameters");
	}

	// Initialize parameters
	this->inNeuronCount = inNeuronCount;
	this->outNeuronCount = outNeuronCount;
	this->hiddenNeuronCount = hiddenNeuronCount;
	this->hiddenLayerCount = hiddenLayerCount;
	this->learningRate = learningRate;
	this->loss = 0.0;
	this->targetData = new float_n[outNeuronCount];

	this->ActivateFunc = ActivateFunc;

	ForwardActive = forwardFuncList[(int)ActivateFunc];
	BackwardActive = backwardFuncList[(int)ActivateFunc];

	inLayer = NeuronLayer(inNeuronCount, 0);
	outLayer = NeuronLayer(outNeuronCount, hiddenNeuronCount);
	
	// add layers
	for (int i = 0; i < hiddenLayerCount; i++)
	{
		hiddenLayerList.push_back(NeuronLayer(hiddenNeuronCount, i == 0 ? inNeuronCount : hiddenNeuronCount));
	}
}

FullConnNetwork::FullConnNetwork(int inNeuronCount, NeuronLayer outLayer, int hiddenNeuronCount, int hiddenLayerCount, ActivateFunctionType activateFunc, float_n learningRate)
{
	if (inNeuronCount <= 0 || outLayer.Count() == 0)
	{
		throw std::exception("Invalid Parameters");
	}

	this->inNeuronCount = inNeuronCount;
	outNeuronCount = outLayer.Count();

	this->hiddenNeuronCount = hiddenNeuronCount;
	this->hiddenLayerCount = hiddenLayerCount;

	ActivateFunc = activateFunc;
	this->learningRate = learningRate;

	this->loss = 0.0;
	this->targetData = new float_n[outNeuronCount];

	ForwardActive = forwardFuncList[(int)ActivateFunc];
	BackwardActive = backwardFuncList[(int)ActivateFunc];

	inLayer = NeuronLayer(inNeuronCount, 0);

	this->outLayer = outLayer;
}

float_n FullConnNetwork::GetLoss()
{
	loss = 0.0;
	
	for (int i = 0; i < outNeuronCount; i++)
	{
		loss += (outLayer[i].value - targetData[i]) * (outLayer[i].value - targetData[i]);
	}

	return loss;
}

void FullConnNetwork::RandomizeAllWeights(float_n min, float_n max)
{
	outLayer.RandomizeWeightAndBias(min, max);

	for (auto& layer : hiddenLayerList)
	{
		layer.RandomizeWeightAndBias(min, max);
	}
}

void FullConnNetwork::SetAllWeights(float_n weight)
{
	inLayer.InitAllWeights(weight);
	outLayer.InitAllWeights(weight);

	for (auto& layer : hiddenLayerList)
	{
		layer.InitAllWeights(weight);
	}
}

void FullConnNetwork::PushDataDouble(double* data)
{
	for (int i = 0; i < inNeuronCount; i++)
	{
		inLayer[i].value = data[i];
	}
}

void FullConnNetwork::PushDataFloat(float *data)
{
	for (int i = 0; i < inNeuronCount; i++)
	{
		inLayer[i].value = (float_n)data[i];
	}
}

void FullConnNetwork::ForwardTransmitLayer(NeuronLayer& obj, NeuronLayer& prev) 
{
	for (int i = 0; i < obj.Count(); i++) 
	{
		obj[i].value = 0.0;

		for (int j = 0; j < obj.prevCount; j++)
		{
			obj[i].value += prev[j].value * obj[i].weights[j];
		}

		obj[i].value += obj.bias;
		obj[i].value = (*ForwardActive)(obj[i].value / (float_n)obj.prevCount);
	}
}

void FullConnNetwork::ForwardTransmitLayer(NeuronLayerInstance& obj, NeuronLayerInstance& prev)
{
	for (int i = 0; i < obj.Count(); i++)
	{
		obj[i].value = 0.0;

		for (int j = 0; j < obj.prevCount; j++)
		{
			obj[i].value += prev[j].value * obj[i].weights[j];
		}

		obj[i].value += obj.bias;
		obj[i].value = (*ForwardActive)(obj[i].value / (float_n)obj.prevCount);
	}
}

void FullConnNetwork::ForwardTransmit()
{
	for (int i = 0; i < hiddenLayerCount; i++)
	{
		ForwardTransmitLayer(hiddenLayerList[i], i == 0 ? inLayer : hiddenLayerList[i - 1]);
	}

	NeuronLayer& lastHiddenLayer = hiddenLayerList[hiddenLayerCount - 1];

	for (int i = 0; i < outLayer.Count(); i++)
	{
		outLayer[i].value = 0.0;

		for (int j = 0; j < outLayer.prevCount; j++)
		{
			outLayer[i].value += lastHiddenLayer[j].value * outLayer[i].weights[j];
		}

		outLayer[i].value += outLayer.bias;
		outLayer[i].value = outLayer[i].value / outLayer.prevCount;
	}

	SoftMax(outLayer);
}

void FullConnNetwork::BackwardTransmitLayer(NeuronLayer& obj, NeuronLayer& last)
{
	for (int i = 0; i < obj.Count(); i++)
	{
		obj[i].error = 0.0;

		for (int j = 0; j < last.Count(); j++)
		{
			obj[i].error += last[j].error * last[j].weights[i];
		}
	}
}

void FullConnNetwork::BackwardTransmitLayer(NeuronLayerInstance& obj, NeuronLayerInstance& last)
{
	for (int i = 0; i < obj.Count(); i++)
	{
		obj[i].error = 0.0;

		for (int j = 0; j < last.Count(); j++)
		{
			obj[i].error += last[j].error * last[j].weights[i];
		}
	}
}

void FullConnNetwork::BackwardTransmit()
{
	SoftMaxGetError(outLayer, targetData);

	for (int i = hiddenLayerCount - 1; i >= 0; i--)
	{
		BackwardTransmitLayer(hiddenLayerList[i], i == hiddenLayerCount - 1 ? outLayer : hiddenLayerList[i + 1]);
	}
}

void FullConnNetwork::UpdateLayerWeights(NeuronLayer& layer, NeuronLayer& lastLayer)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		float_n coeff = learningRate * (*BackwardActive)(layer[i].value) * layer[i].error; // common coeff

		layer.bias += learningRate * (*BackwardActive)(layer.bias) * layer[i].error; // tweak bias

		for (int j = 0; j < layer.prevCount; j++)
		{
			layer[i].weights[j] += coeff * lastLayer[j].value;
		}
	}
}

void FullConnNetwork::UpdateLayerWeights(NeuronLayerInstance& layer, NeuronLayerInstance& lastLayer)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		float_n coeff = learningRate * (*BackwardActive)(layer[i].value) * layer[i].error; // common coeff

		layer.bias += learningRate * (*BackwardActive)(layer.bias) * layer[i].error; // tweak bias

		for (int j = 0; j < layer.prevCount; j++)
		{
			layer[i].weights[j] += coeff * lastLayer[j].value;
		}
	}
}

void FullConnNetwork::computeAverage(float_n count)
{
	for (auto& layer : hiddenLayerList)
	{
		for (auto& neuron : layer.neurons)
		{
			neuron.value /= (float_n)count;
			neuron.error /= (float_n)count;
		}
	}
	for (auto& neuron : inLayer.neurons)
	{
		neuron.value /= (float_n)count;
		neuron.error /= (float_n)count;
	}
	for (auto& neuron : outLayer.neurons)
	{
		neuron.value /= (float_n)count;
		neuron.error /= (float_n)count;
	}
}

void FullConnNetwork::UpdateWeights()
{
	UpdateLayerWeights(outLayer, hiddenLayerList[hiddenLayerCount - 1]); // update outlayer

	for (int i = 0; i < hiddenLayerCount; i++)
	{
		UpdateLayerWeights(hiddenLayerList[i], i == 0 ? inLayer : hiddenLayerList[i - 1]);
	}
}

int FullConnNetwork::FindLargestOutput()
{
	float_n biggest = outLayer[0].value;
	int biggestNeuron = 0;

	for (int i = 1; i < outLayer.Count(); i++)
	{
		if (outLayer[i].value > biggest)
		{
			biggest = outLayer[i].value;
			biggestNeuron = i;
		}
	}

	return biggestNeuron;
}

int FullConnNetwork::GetResultDouble(double* data)
{
	PushDataDouble(data);
	ForwardTransmit();
	return FindLargestOutput();
}

int FullConnNetwork::GetResultFloat(float* data)
{
	PushDataFloat(data);
	ForwardTransmit();
	return FindLargestOutput();
}

int FullConnNetwork::GetResultNetworkData(NetworkData& data)
{
	return GetResultFloat(data.data);
}

float_n FullConnNetwork::GetAccuracy(NetworkDataSet& set)
{
	int correctCount = 0;

	for (auto& data : set.dataSet)
	{
		int label_out = GetResultNetworkData(*data);
		if (label_out == data->label)
		{
			correctCount++;
		}
	}

	return (float_n)correctCount / (float_n)set.Count();
}

float_n FullConnNetwork::GetAccuracyCallbackFloat(NetworkDataSet& set, float* progressVariable)
{
	int correctCount = 0, doneCount = 0;

	for (auto& data : set.dataSet)
	{
		int label_out = GetResultNetworkData(*data);
		if (label_out == data->label)
		{
			correctCount++;
		}

		doneCount++;

		*progressVariable = (float)doneCount / (float)set.Count();
	}

	return (float_n)correctCount / (float_n)set.Count();
}

void FullConnNetwork::PushTargetLabel(int label)
{
	for (int i = 0; i < outNeuronCount; i++)
	{
		targetData[i] = i == label ? 1.0 : 0.0;
	}
}

void ClearNeuron(Network::Neuron* neuron)
{
	if (neuron->weightCount > 0)
		free(neuron->weights);
}

void FullConnNetwork::Destroy()
{
	// Clear neuron data
	inLayer.neurons.clear();

	for (Neuron& neuron : outLayer.neurons)
		ClearNeuron(&neuron);
	outLayer.neurons.clear();

	for (NeuronLayer& layer : hiddenLayerList)
	{
		for (Neuron& neuron : layer.neurons)
			ClearNeuron(&neuron);
		layer.neurons.clear();
	}

	hiddenLayerList.clear();

	// Clear target data
	if (targetData) delete[] targetData;
}
