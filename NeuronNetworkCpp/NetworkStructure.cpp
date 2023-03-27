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
		weights = new double[weightCount];
	else
		weights = nullptr;
}

void Neuron::InitWeights(double val)
{
	for (int i = 0; i < weightCount; i++)
	{
		weights[i] = val;
	}
}

NeuronLayer::NeuronLayer(int neuronCount, int prevCount)
{
	// initialize parameters
	response = 0.0;
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
	response = 0.0;
	bias = 0.0;
	prevCount = 0;
}

int NeuronLayer::Count()
{
	return neurons.size();
}

void NeuronLayer::InitAllWeights(double weight)
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

void NeuronLayer::RandomizeWeightAndBias(double min, double max)
{
	std::mt19937 rnd(std::random_device{}());
	std::uniform_real_distribution<double> dist(min, max);

	for (auto& neuron : neurons)
	{
		for (int i = 0; i < neuron.weightCount; i++)
		{
			neuron.weights[i] = dist(rnd);
		}
	}

	bias = dist(rnd);
}
