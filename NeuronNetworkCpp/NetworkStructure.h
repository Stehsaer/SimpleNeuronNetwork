#ifndef _NETWORK_STRUCTURE_H_
#define _NETWORK_STRUCTURE_H_

#include <vector>

namespace Network
{
	typedef float float_n;

	struct Neuron
	{
	public:
		float_n value, error;

		float_n* weights;
		int weightCount;

		Neuron(int weightCount);
		void InitWeights(float_n val);
	};

	class NeuronLayer
	{
	public:
		std::vector<Neuron> neurons;
		int prevCount; // count of neurons of previous layer

		float_n bias;

		NeuronLayer(int neuronCount, int prevCount);
		NeuronLayer();

		int Count(); // get count of neurons in this layer
		void InitAllWeights(float_n weight);
		void RandomizeWeightAndBias(float_n min, float_n max);
		Neuron& operator[](int index);
	};
}

#endif