#ifndef _NETWORK_ALGORITHM_H_
#define _NETWORK_ALGORITHM_H_

#include "NetworkStructure.h"

namespace Network
{
	/*
	CHANGELOG:

		2023-2-20:
			MODIFY NormalizeData(...); Added offset option and flexibility that comes along; Change output type from double to float
	*/
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

		float* NormalizeData(unsigned char* data, int offset, int dataSize, NormalizationMode mode);

		void SoftMax(NeuronLayer& layer);
		void SoftMaxGetError(NeuronLayer& layer, double* target);
	}
}

#endif
