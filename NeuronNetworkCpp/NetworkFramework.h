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
			double (*ForwardActive)(double);
			double (*BackwardActive)(double);

			// neuron counts
			int inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount;

			// layers
			NeuronLayer inLayer, outLayer;
			std::vector<NeuronLayer> hiddenLayerList;

			// training parameters
			double learningRate, loss;
			double * targetData; 
			
			BackPropaNetwork(int inNeuronCount, int outNeuronCount, int hiddenNeuronCount, int hiddenLayerCount, double (*ForwardFunc)(double), double (*BackwardFunc)(double), double learningRate);
			
			double GetLoss();
			void RandomizeAllWeights(double min, double max);
			void SetAllWeights(double weight);
			void PushDataDouble(double* data);
			void PushDataFloat(float* data);
			void ForwardTransmit();
			void BackwardTransmit();
			void UpdateWeights();

			void Train(NetworkData& data, int maxIterCount = 3, double threshold = 0.01);

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
