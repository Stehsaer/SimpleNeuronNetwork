#ifndef _NETWORK_FRAMEWORK_H_
#define _NETWORK_FRAMEWORK_H_

#include <vector>
#include "NetworkStructure.h"
#include "NetworkData.h"

namespace Network
{
	namespace Framework
	{
		class FullConnNetwork
		{
		public:
			// active functions
			ActivateFunction ForwardActive;
			ActivateFunction BackwardActive;

			ActivateFunctionType ActivateFunc;

			// neuron counts
			int inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount;

			// layers
			NeuronLayer inLayer, outLayer;
			std::vector<NeuronLayer> hiddenLayerList;

			// training parameters
			double learningRate, loss;
			double* targetData;

			/// <summary>
			/// Initialize a full-connected-network
			/// </summary>
			/// <param name="inNeuronCount">Neuron count of in layer</param>
			/// <param name="outNeuronCount">Neuron count of out layer</param>
			/// <param name="hiddenNeuronCount">Neuron count of each hidden layer (each layer uses a same amount of neuron)</param>
			/// <param name="hiddenLayerCount">Count of hidden layer</param>
			/// <param name="ActivateFunc">Activate Function</param>
			/// <param name="learningRate">Learning Rate</param>
			FullConnNetwork(int inNeuronCount, int outNeuronCount, int hiddenNeuronCount, int hiddenLayerCount, ActivateFunctionType ActivateFunc, double learningRate);

			/// <summary>
			/// Fetch loss value and store it to loss(double) variable
			/// </summary>
			/// <returns>Loss</returns>
			double GetLoss();

			/// <summary>
			/// Randomize all weights in the network in linear fashion
			/// </summary>
			/// <param name="min">Min Value</param>
			/// <param name="max">Max Value</param>
			void RandomizeAllWeights(double min, double max);

			/// <summary>
			/// Set every weight in the network to a value
			/// </summary>
			/// <param name="weight">Input value</param>
			void SetAllWeights(double weight);

			/// <summary>
			/// Push some data into the in layer
			/// </summary>
			/// <param name="data">Pointer for the data</param>
			void PushDataDouble(double* data);

			/// <summary>
			/// Push some data into the in layer
			/// </summary>
			/// <param name="data">Pointer for the data</param>
			void PushDataFloat(float* data);

			/// <summary>
			/// Execute a full forward transmit
			/// </summary>
			void ForwardTransmit();

			/// <summary>
			/// Execute a full backward propagation
			/// </summary>
			void BackwardTransmit();

			/// <summary>
			/// Update all weights in the network based on error values
			/// </summary>
			void UpdateWeights();


			/// <summary>
			/// Train the network using a given NetworkData Instance
			/// </summary>
			/// <param name="data">NetworkData</param>
			/// <param name="maxIterCount">Max Iteration Count in this particular run</param>
			/// <param name="threshold">Threshold when stop iterating</param>
			void Train(NetworkData& data, int maxIterCount = 3, double threshold = 0.01);

			/// <summary>
			/// Find the index of largest output
			/// </summary>
			/// <returns>Index</returns>
			int FindLargestOutput();

			/// <summary>
			/// Fetch the result
			/// </summary>
			/// <param name="data">Input NetworkData Instance</param>
			/// <returns>Index</returns>
			int GetResultNetworkData(NetworkData& data);

			/// <summary>
			/// Fetch the result
			/// </summary>
			/// <param name="data">Input Data Pointer</param>
			/// <returns>Index</returns>
			int GetResultDouble(double* data);

			/// <summary>
			/// Fetch the result
			/// </summary>
			/// <param name="data">Input Data Pointer</param>
			/// <returns>Index</returns>
			int GetResultFloat(float* data);

			/// <summary>
			/// Calculate overall accuracy using a verify/test set
			/// </summary>
			/// <param name="set">Verify/Test Dataset</param>
			/// <returns>Accuracy, 1.0d max.</returns>
			double GetAccuracy(NetworkDataSet& set);

			/// <summary>
			/// Calculate overall accuracy using a verify/test set, but with progress callback
			/// </summary>
			/// <param name="set">Verify/Test Dataset</param>
			/// <param name="progressVariable">Pointer of the progress value</param>
			/// <returns>Accuracy from 0.0 to 1.0</returns>
			double GetAccuracyCallbackFloat(NetworkDataSet& set, float* progressVariable);

			/// <summary>
			/// Clear all data in the network and ready to be destroyed
			/// </summary>
			void Destroy();

		private:
			void ForwardTransmitLayer(NeuronLayer& obj, NeuronLayer& prev);
			void BackwardTransmitLayer(NeuronLayer& obj, NeuronLayer& last);
			void UpdateLayerWeights(NeuronLayer& layer, NeuronLayer& lastLayer);
		};
	}
}

#endif
