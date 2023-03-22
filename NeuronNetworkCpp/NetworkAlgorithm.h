#ifndef _NETWORK_ALGORITHM_H_
#define _NETWORK_ALGORITHM_H_

#include "NetworkStructure.h"

namespace Network
{
	typedef double (*ActivateFunction)(double);

	namespace Algorithm
	{
		enum NormalizationMode
		{
			ZeroToOne = 0,
			MinusOneToOne = 1,
			NoNormalization = -1
		};

		/// <summary>
		/// Sigmoid
		/// </summary>
		double Sigmoid(double x);

		/// <summary>
		/// Derivative of Sigmoid
		/// </summary>
		double Sigmoid_D(double x);

		/// <summary>
		/// Shifted Sigmoid
		/// </summary>
		double ShiftedSigmoid(double x);

		/// <summary>
		/// D: Shifted Sigmoid
		/// </summary>
		double ShiftedSigmoid_D(double x);

		/// <summary>
		/// ReLU
		/// </summary>
		double ReLU(double x);

		/// <summary>
		/// D: ReLU
		/// </summary>
		double ReLU_D(double x);

		/// <summary>
		/// Leaky ReLU
		/// </summary>
		double LeakyReLU(double x);

		/// <summary>
		/// D: Leaky ReLU
		/// </summary>
		double LeakyReLU_D(double x);

		/// <summary>
		/// Normalize data in a given data range
		/// </summary>
		/// <param name="data">Original Data</param>
		/// <param name="offset">Offset</param>
		/// <param name="dataSize">Size of the data</param>
		/// <param name="mode">Normalization Mode</param>
		/// <returns>Processed Data Pointer</returns>
		float* NormalizeData(unsigned char* data, int offset, int dataSize, NormalizationMode mode);

		/// <summary>
		/// Softmax
		/// </summary>
		void SoftMax(NeuronLayer& layer);

		/// <summary>
		/// Softmax
		/// </summary>
		/// <param name="target">Target Data used for error calculation</param>
		void SoftMaxGetError(NeuronLayer& layer, double* target);
	}

	enum class ActivateFunctionType:int
	{
		Sigmoid, SigmoidShifted, ReLU, LeakyReLU
	};

	const int ActivateFunctionCount = 4;

	inline const Network::ActivateFunction forwardFuncList[] =
	{ &Network::Algorithm::Sigmoid,
	&Network::Algorithm::ShiftedSigmoid ,
	&Network::Algorithm::ReLU ,
	&Network::Algorithm::LeakyReLU
	};

	inline const Network::ActivateFunction backwardFuncList[] =
	{ &Network::Algorithm::Sigmoid_D,
	&Network::Algorithm::ShiftedSigmoid_D ,
	&Network::Algorithm::ReLU_D ,
	&Network::Algorithm::LeakyReLU_D
	};
}

#endif
