#ifndef _NETWORK_STRUCTURE_H_
#define _NETWORK_STRUCTURE_H_

#include <vector>
#include <atomic>

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

	// Used for batched gradient descent, doesn't affect original Neuron
	struct NeuronInstance
	{
	public:
		Neuron* source;

		float_n value, error;

		float_n* weights;
		int weightCount;

		NeuronInstance(Neuron* source);

		void FeedBack();
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
		void ClearValues();
		Neuron& operator[](int index);
	};

	// Used for batched gradient descent, doesn't affect original NeuronLayer
	class NeuronLayerInstance
	{
	public:
		NeuronLayer* source;

		std::vector<NeuronInstance*> neurons;
		int prevCount;

		float_n bias;

		NeuronLayerInstance(NeuronLayer* source);

		int Count();
		NeuronInstance& operator[](int index);
		void FeedBack();

		void Destroy();
		void ClearSourceValue();
		void FetchBias();
		void PushDataFloat(float_n* data);
	};
}

#endif