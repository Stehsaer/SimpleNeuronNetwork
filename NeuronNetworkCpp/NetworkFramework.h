#ifndef _NETWORK_FRAMEWORK_H_
#define _NETWORK_FRAMEWORK_H_

#include <vector>
#include "NetworkStructure.h"
#include "NetworkData.h"

namespace Network
{
	namespace Framework
	{
		class BackPropaNetwork
		{
		public:
			// active functions
			ActivateFunction ForwardActive;
			ActivateFunction BackwardActive;

			// neuron counts
			int inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount;

			// layers
			NeuronLayer inLayer, outLayer;
			std::vector<NeuronLayer> hiddenLayerList;

			// training parameters
			double learningRate, loss;
			double * targetData; 
			
			// construction func
			BackPropaNetwork(int inNeuronCount, int outNeuronCount, int hiddenNeuronCount, int hiddenLayerCount, ActivateFunction ForwardFunc, ActivateFunction BackwardFunc, double learningRate);
			
			double GetLoss();
			void RandomizeAllWeights(double min, double max);
			void SetAllWeights(double weight);
			void PushDataDouble(double* data);
			void PushDataFloat(float* data);
			void ForwardTransmit();
			void BackwardTransmit();
			void UpdateWeights();

			void Train(NetworkData& data, int maxIterCount = 3, double threshold = 0.01);

			// result
			int FindLargestOutput();

			int GetResultNetworkData(NetworkData& data);
			int GetResultDouble(double* data);
			int GetResultFloat(float* data);

			double GetAccuracy(NetworkDataSet& set);

			void Destroy();

		private:
			void ForwardTransmitLayer(NeuronLayer& obj, NeuronLayer& prev);
			void BackwardTransmitLayer(NeuronLayer& obj, NeuronLayer& last);
			void UpdateLayerWeights(NeuronLayer& layer, NeuronLayer& lastLayer);
		};
	}
}

#endif
