#ifndef _NETWORK_STRUCTURE_H_
#define _NETWORK_STRUCTURE_H_

#include <vector>

namespace Network
{
	struct Neuron
	{
	public:
		double value, error;

		double* weights;
		int weightCount;

		Neuron(int weightCount);
		void InitWeights(double val);
	};

	class NeuronLayer
	{
	public:
		std::vector<Neuron> neurons;
		int prevCount; // count of neurons of previous layer

		double response, bias;

		NeuronLayer(int neuronCount, int prevCount);
		NeuronLayer();

		int Count(); // get count of neurons in this layer
		void InitAllWeights(double weight);
		void RandomizeAllWeights(double min, double max);
		Neuron& operator[](int index);
	};
}

#endif