#include "NetworkStructure.h"
#include <random>

using namespace Network;

Neuron::Neuron(int _weightCount)
{
	// initialize parameters
	weightCount = _weightCount; 
	value = 0.0;
	error = 0.0;

	if (weightCount > 0)
		weights = new float_n[weightCount];
	else
		weights = nullptr;
}

void Neuron::InitWeights(float_n val)
{
	for (int i = 0; i < weightCount; i++)
	{
		weights[i] = val;
	}
}

NeuronLayer::NeuronLayer(int neuronCount, int prevCount)
{
	// initialize parameters
	bias = 0.0;
	this->prevCount = prevCount;

	for (int i = 0; i < neuronCount; i++)
	{
		neurons.push_back(Neuron(prevCount));
		neurons[i].InitWeights(0.0);
	}
}

NeuronLayer::NeuronLayer()
{
	// initialize parameters
	bias = 0.0;
	prevCount = 0;
}

int NeuronLayer::Count()
{
	return neurons.size();
}

void NeuronLayer::InitAllWeights(float_n weight)
{
	for (auto& neuron : neurons)
	{
		neuron.InitWeights(weight);
	}
}

Neuron& NeuronLayer::operator[](int index)
{
	return neurons[index];
}

void NeuronLayer::RandomizeWeightAndBias(float_n min, float_n max)
{
	std::mt19937 rnd(std::random_device{}());
	std::uniform_real_distribution<float_n> dist(min, max);

	for (auto& neuron : neurons)
	{
		for (int i = 0; i < neuron.weightCount; i++)
		{
			neuron.weights[i] = dist(rnd);
		}
	}

	bias = dist(rnd);
}
