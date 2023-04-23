#include "NetworkAlgorithm.h"

#include<math.h>

using namespace Network::Algorithm;
typedef Network::float_n float_n;

float_n Network::Algorithm::Sigmoid(float_n x)
{
	return 1.0 / (1.0 + exp(-x));
}

float_n Network::Algorithm::Sigmoid_D(float_n x)
{
	return x * (1.0 - x);
}

float_n Network::Algorithm::ShiftedSigmoid(float_n x)
{
	return Sigmoid(x) * 2.0 - 1.0;
}

float_n Network::Algorithm::ShiftedSigmoid_D(float_n x)
{
	return Sigmoid_D((x + 1.0) / 2.0);
}

float_n Network::Algorithm::ReLU(float_n x)
{
	return x > 0.0 ? x : 0.0;
}

float_n Network::Algorithm::ReLU_D(float_n x)
{
	return x > 0.0 ? 1.0 : 0.0;
}

#define LEAKY_RELU_CONST 0.01

float_n Network::Algorithm::LeakyReLU(float_n x)
{
	return x > 0.0 ? x : x * LEAKY_RELU_CONST;
}

float_n Network::Algorithm::LeakyReLU_D(float_n x)
{
	return x > 0.0 ? 1.0 : LEAKY_RELU_CONST;
}

// NOTE: Added offset (2023-2-20)
void Normalization_ZeroToOne(float* dataOut, int offset, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i + offset] / 256.0;
	}
}

void Normalization_MinusOneToOne(float* dataOut, int offset, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i + offset] / 128.0 - 1.0;
	}
}

void Normalization_NoNormalization(float* dataOut, int offset, unsigned char* data, int dataSize)
{
	for (int i = 0; i < dataSize; i++)
	{
		dataOut[i] = data[i + offset];
	}
}

float* Network::Algorithm::NormalizeData(unsigned char* data, int offset, int dataSize, NormalizationMode mode)
{
	float* dataOut = new float[dataSize];

	switch (mode)
	{
	case NormalizationMode::ZeroToOne:
		Normalization_ZeroToOne(dataOut, offset, data, dataSize);
		break;

	case NormalizationMode::MinusOneToOne:
		Normalization_MinusOneToOne(dataOut, offset, data, dataSize);
		break;

	default:
		Normalization_NoNormalization(dataOut, offset, data, dataSize);
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

void Network::Algorithm::SoftMax(Network::NeuronLayerInstance& layer)
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

void Network::Algorithm::SoftMaxGetError(NeuronLayer& layer, float_n* target)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		layer[i].error = target[i] - layer[i].value;
	}
}

void Network::Algorithm::SoftMaxGetError(NeuronLayerInstance& layer, float_n* target)
{
	for (int i = 0; i < layer.Count(); i++)
	{
		layer[i].error = target[i] - layer[i].value;
	}
}