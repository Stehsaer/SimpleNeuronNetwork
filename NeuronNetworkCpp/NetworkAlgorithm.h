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

		double Sigmoid(double x);
		double Sigmoid_D(double x);
		double ShiftedSigmoid(double x);
		double ShiftedSigmoid_D(double x);
		double ReLU(double x);
		double ReLU_D(double x);
		double LeakyReLU(double x);
		double LeakyReLU_D(double x);

		float* NormalizeData(unsigned char* data, int offset, int dataSize, NormalizationMode mode);

		void SoftMax(NeuronLayer& layer);
		void SoftMaxGetError(NeuronLayer& layer, double* target);
	}

	enum class ActivateFunctionType
	{
		Sigmoid, SigmoidShifted, ReLU, LeackyReLU
	};

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
