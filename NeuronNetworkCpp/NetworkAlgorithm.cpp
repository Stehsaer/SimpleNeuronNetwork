#include "NetworkAlgorithm.h"

#include<math.h>

using namespace Network::Algorithm;

double Network::Algorithm::Sigmoid(double x)
{
	return 1.0 / (1.0 + exp(-x));
}

double Network::Algorithm::Sigmoid_D(double x)
{
	return x * (1.0 - x);
}

double Network::Algorithm::ShiftedSigmoid(double x)
{
	return Sigmoid(x) * 2.0 - 1.0;
}

double Network::Algorithm::ShiftedSigmoid_D(double x)
{
	return Sigmoid_D((x + 1.0) / 2.0);
}

double Network::Algorithm::ReLU(double x)
{
	return x > 0.0 ? x : 0.0;
}

double Network::Algorithm::ReLU_D(double x)
{
	return x > 0.0 ? 1.0 : 0.0;
}

void Normalization_ZeroToOne(double* dataOut, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i] / 256.0;
	}
}

void Normalization_MinusOneToOne(double* dataOut, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i] / 128.0 - 1.0;
	}
}

void Normalization_NoNormalization(double* dataOut, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i];
	}
}

double* Network::Algorithm::NormalizeData(unsigned char* data, int dataSize, NormalizationMode mode)
{
	double* dataOut = new double[dataSize];

	switch (mode)
	{
	case NormalizationMode::ZeroToOne:
		Normalization_ZeroToOne(dataOut, data, dataSize);
		break;

	case NormalizationMode::MinusOneToOne:
		Normalization_MinusOneToOne(dataOut, data, dataSize);
		break;

	default:
		Normalization_NoNormalization(dataOut, data, dataSize);
		break;
	}

	return dataOut;
}

void Network::Algorithm::SoftMax(Network::NeuronLayer& layer)
{
	double sum = 0.0;
	double biggestValue = layer[0].value;

	// find largest value in neurons
	for (int i = 0; i < layer.Count(); i++)
	{
		if (layer[i].value > biggestValue)
			biggestValue = layer[i].value;
	}

	// add up sums
	for (int i = 0; i < layer.Count(); i++)
	{
		sum += exp(layer[i].value - biggestValue);
	}

	// set values
	for (int i = 0; i < layer.Count(); i++)
	{
		layer[i].value = exp(layer[i].value - biggestValue) / sum;
	}
}

void Network::Algorithm::SoftMaxGetError(NeuronLayer& layer, double* target)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		layer[i].error = target[i] - layer[i].value;
	}
}