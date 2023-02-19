#include "NetworkFramework.h"

#include "NetworkStructure.h"
#include "NetworkAlgorithm.h"
#include "NetworkData.h"

using namespace Network::Framework;
using namespace Network::Algorithm;

BackPropaNetwork::BackPropaNetwork(int inNeuronCount, int outNeuronCount, int hiddenNeuronCount, int hiddenLayerCount, double (*ForwardFunc)(double), double (*BackwardFunc)(double), double learningRate)
{
	if (inNeuronCount <= 0 || outNeuronCount <= 0 || hiddenNeuronCount <= 0 || hiddenLayerCount <= 0 || learningRate <= 0.0)
	{
		throw "Invalid input.";
	}

	// Initialize parameters
	this->inNeuronCount = inNeuronCount;
	this->outNeuronCount = outNeuronCount;
	this->hiddenNeuronCount = hiddenNeuronCount;
	this->hiddenLayerCount = hiddenLayerCount;
	this->learningRate = learningRate;
	this->loss = 0.0;
	this->inputData = nullptr;
	this->targetData = nullptr;

	ForwardActive = ForwardFunc;
	BackwardActive = BackwardFunc;

	inLayer = NeuronLayer(inNeuronCount, 0);
	outLayer = NeuronLayer(outNeuronCount, hiddenNeuronCount);
	
	// add layers
	for (int i = 0; i < hiddenLayerCount; i++)
	{
		hiddenLayerList.push_back(NeuronLayer(hiddenNeuronCount, i == 0 ? inNeuronCount : hiddenNeuronCount));
	}
}

double BackPropaNetwork::GetLoss()
{
	loss = 0.0;
	
	for (int i = 0; i < outNeuronCount; i++)
	{
		loss += (outLayer[i].value - targetData[i]) * (outLayer[i].value - targetData[i]);
	}

	return loss;
}

void BackPropaNetwork::RandomizeAllWeights(double min, double max)
{
	inLayer.RandomizeAllWeights(min, max);
	outLayer.RandomizeAllWeights(min, max);

	for (auto& layer : hiddenLayerList)
	{
		layer.RandomizeAllWeights(min, max);
	}
}

void BackPropaNetwork::SetAllWeights(double weight)
{
	inLayer.InitAllWeights(weight);
	outLayer.InitAllWeights(weight);

	for (auto& layer : hiddenLayerList)
	{
		layer.InitAllWeights(weight);
	}
}

void BackPropaNetwork::PushData(double* data)
{
	if (inputData) delete[] inputData;

	inputData = new double[inNeuronCount];
	memcpy(inputData, data, inNeuronCount * sizeof(double)); // copy data

	for (int i = 0; i < inNeuronCount; i++)
	{
		inLayer[i].value = inputData[i];
	}
}

void BackPropaNetwork::PushTarget(double* target)
{
	if (targetData) delete[] targetData;

	targetData = new double[outNeuronCount];
	memcpy(targetData, target, outNeuronCount * sizeof(double)); // copy data
}

void BackPropaNetwork::ForwardTransmitLayer(NeuronLayer& obj, NeuronLayer& prev) 
{
	for (int i = 0; i < obj.Count(); i++) 
	{
		obj[i].value = 0.0;

		for (int j = 0; j < obj.prevCount; j++)
		{
			obj[i].value += prev[j].value * obj[i].weights[j];
		}

		obj[i].value += obj.bias;
		obj[i].value = (*ForwardActive)(obj[i].value / (double)obj.prevCount);
	}
}

void BackPropaNetwork::ForwardTransmit()
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
		outLayer[i].value /= outLayer.prevCount;
	}

	SoftMax(outLayer);
}

void BackPropaNetwork::BackwardTransmitLayer(NeuronLayer& obj, NeuronLayer& last)
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

void BackPropaNetwork::BackwardTransmit()
{
	SoftMaxGetError(outLayer, targetData);

	for (int i = hiddenLayerCount - 1; i >= 0; i--)
	{
		BackwardTransmitLayer(hiddenLayerList[i], i == hiddenLayerCount - 1 ? outLayer : hiddenLayerList[i + 1]);
	}
}

void BackPropaNetwork::UpdateLayerWeights(NeuronLayer& layer, NeuronLayer& lastLayer)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		double coeff = learningRate * (*BackwardActive)(layer[i].value) * layer[i].error; // common coeff

		layer.bias += learningRate * (*BackwardActive)(layer.bias) * layer[i].error; // tweak bias

		for (int j = 0; j < layer.prevCount; j++)
		{
			layer[i].weights[j] += coeff * lastLayer[j].value;
		}
	}
}

void BackPropaNetwork::UpdateWeights()
{
	UpdateLayerWeights(outLayer, hiddenLayerList[hiddenLayerCount - 1]); // update outlayer

	for (int i = 0; i < hiddenLayerCount; i++)
	{
		UpdateLayerWeights(hiddenLayerList[i], i == 0 ? inLayer : hiddenLayerList[i - 1]);
	}
}

int BackPropaNetwork::FindLargestOutput()
{
	double biggest = outLayer[0].value;
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

int BackPropaNetwork::GetResult(double* data)
{
	PushData(data);
	ForwardTransmit();
	return FindLargestOutput();
}

int BackPropaNetwork::GetResult(NetworkData& data)
{
	return GetResult(data.data);
}

double BackPropaNetwork::GetAccuracy(NetworkDataSet& set)
{
	int correctCount = 0;

	for (auto data : set.dataSet)
	{
		int label_out = GetResult(*data);
		if (label_out == data->label)
		{
			correctCount++;
		}
	}

	return (double)correctCount / (double)set.Count();
}

void BackPropaNetwork::Train(NetworkData& data, int maxIterCount, double threshold)
{
	PushData(data.data);

	// generate one-hot label
	targetData = new double[outNeuronCount];

	for (int i = 0; i < outNeuronCount; i++)
	{
		targetData[i] = i == data.label ? 1.0 : 0.0;
	}

	for (int iterCount = 0; iterCount < maxIterCount; iterCount++)
	{
		ForwardTransmit();
		GetLoss();

		if (loss < threshold)
			break;

		BackwardTransmit();
		UpdateWeights();
	}
}