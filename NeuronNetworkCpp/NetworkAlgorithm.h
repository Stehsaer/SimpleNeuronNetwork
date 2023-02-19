#ifndef _NETWORK_ALGORITHM_H_
#define _NETWORK_ALGORITHM_H_

#include "NetworkStructure.h"

namespace Network
{
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

		double* NormalizeData(unsigned char* data, int dataSize, NormalizationMode mode);

		void SoftMax(NeuronLayer& layer);
		void SoftMaxGetError(NeuronLayer& layer, double* target);
	}
}

#endif
