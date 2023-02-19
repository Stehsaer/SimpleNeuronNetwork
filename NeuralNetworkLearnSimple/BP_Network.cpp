#include "BP_Network.h"

#define LEAKY_RELU_INDEX 0.01

#define _TANH_

Neuron::Neuron(integer count)
{
	outActivate = 0.0;
	outError = 0.0;
	prevTweak = 0.0;

	weightCount = count;
	weights = new double[count];
}

void Neuron::RandomizeWeights()
{
	for (RANGE(integer, i, weightCount))
	{
		srand((((intptr_t) & weights[i]) << 2) ^ time(0));

		//weights[i] = rand() / double(RAND_MAX) *0.1 + .5;
		//weights[i] = 0.0;
		std::random_device rd;
		std::mt19937 g((((intptr_t) & weights[i]) << 2) ^ time(0));

		std::uniform_real_distribution<double> dist(.2, .5);

		weights[i] = dist(g);
		
		//printf("%f,", weights[i]);
	}
	//printf("\n");
	//PAUSE;
}

NeuronLayer::NeuronLayer(integer count, integer conn = 0)
{
	response = 1.0;
	bias = 0.0;

	connCount = conn;
	neuronCount = count;

	// ������Ԫ�б�
	for (RANGE(integer, i, count))
	{
		neurons.push_back(Neuron(conn));
	}
}

void NeuronLayer::PrintDataDebug()
{
	std::cout << "[";
	for (auto &neuron : neurons)
	{
		printf("%.4f,", neuron.outActivate);
	}
	std::cout << "\b]" << std::endl;
}

void NeuronLayer::PrintErrorDebug()
{
	std::cout << "[";
	for (auto &neuron : neurons)
	{
		std::cout << neuron.outError << "; ";
	}
	std::cout << "\b]" << std::endl;
}

void NeuronLayer::RandomizeWeights()
{
	for (auto &neuron : neurons)
	{
		neuron.RandomizeWeights();
	}
}

BPNetwork::BPNetwork(integer inCount, integer outCount, integer layerCount, integer neuronCount, double learningRate, double prevTweak = 0)
{
	inLayer = NeuronLayer(inCount);				   // ��������
	outLayer = NeuronLayer(outCount, neuronCount); // ��������
	
	outLayer.response/=neuronCount;

	inNeuronCount = inCount;
	outNeuronCount = outCount;
	layerNeuronCount = neuronCount;
	
	prevTweakCoeff = prevTweak;

	this->layerCount = layerCount;
	this->learningRate = learningRate;

	for (integer n = 0; n < layerCount; n++)
	{
		hiddenLayers.push_back(NeuronLayer(neuronCount, n == 0 ? inCount : neuronCount));

		hiddenLayers[n].RandomizeWeights();
		if(n == 0)
		{
			hiddenLayers[n].response /= inNeuronCount;
		}
		else
		{
			hiddenLayers[n].response /= neuronCount;
		}
	}

	outLayer.RandomizeWeights();
}

double BPNetwork::ForwardActive(double activate, double response)
{
	double in = activate * response;
#ifdef _RELU_
	if (in > 0)
		return in;
	else
		return 0;
#endif
#ifdef _TANH_
	return (1.0 / (1.0 + exp(-in)) - 0.5) * 2.0;
#endif
#ifdef _SIGMOID_
	return 1.0 / (1.0 + exp(-in));
#endif
#ifdef _LEAKY_RELU_
	if (in > 0)
		return in;
	else
		return in * LEAKY_RELU_INDEX;
#endif
}

double BPNetwork::BackActive(double x)
{
#ifdef _RELU_
	if (x > 0)
		return 1;
	else
		return 0;
#endif
#ifdef _TANH_
	double _x = x / 2.0 + 0.5;
	return _x * (1.0 - _x);
#endif
#ifdef _SIGMOID_
	return x * (1.0 - x);
#endif
#ifdef _LEAKY_RELU_
	if (x > 0)
		return 1;
	else
		return LEAKY_RELU_INDEX;
#endif
	/**/
}

void BPNetwork::UpdateNeuronLayerForward(integer id)
{
	NeuronLayer &layer = hiddenLayers[id];
	NeuronLayer &prevLayer = id == 0 ? inLayer : hiddenLayers[id - 1]; // Ϊ0����������㣬����Ϊ���ز�
	//printf("%f,%f,%d\n",hiddenLayers[0].response,layer.response, id);

	for (auto &neuron : layer.neurons)
	{
		neuron.outActivate = 0.0;

		for (RANGE(integer, i, neuron.weightCount))
			neuron.outActivate += neuron.weights[i] * prevLayer.neurons[i].outActivate;
			
		//printf("%f,", neuron.outActivate);

		neuron.outActivate = ForwardActive(neuron.outActivate, layer.response); 
		//printf("%f,", neuron.outActivate);

		//printf("%f, ", neuron.outActivate);
	}

	//printf("\n");
	//PAUSE;
}

void BPNetwork::UpdateNeuronLayerForward()
{
	NeuronLayer &layer = outLayer;
	NeuronLayer &prevLayer = hiddenLayers[layerCount - 1]; // ���һ�����ز�
	
	double totalOutput = 0.;
	double biggest = layer.neurons[0].outActivate;
	
	for(auto& neuron:layer.neurons)
	{
		neuron.outActivate = 0.0;

		for (RANGE(integer, i, neuron.weightCount))
			neuron.outActivate += neuron.weights[i] * prevLayer.neurons[i].outActivate * layer.response;
			
		if(neuron.outActivate > biggest)
		biggest = neuron.outActivate;
	}

	for (auto &neuron : layer.neurons)
	{
		//printf("%f,", neuron.outActivate);
			
		totalOutput += exp(neuron.outActivate - biggest);
	}
	//printf("\n");
	
	for(auto &neuron:layer.neurons)
	{
		neuron.outActivate = exp(neuron.outActivate - biggest) / totalOutput;
		
		//printf("%f;", neuron.outActivate);
	}
	
	//printf("\n");
	
	//PAUSE;
}

void BPNetwork::PushDataInLayer(double *data)
{
	for (RANGE(integer, i, inNeuronCount))
	{
		inLayer.neurons[i].outActivate = data[i];
	}
}

double *BPNetwork::GetDataOutLayer()
{
	double *out = new double[outNeuronCount];

	for (RANGE(integer, i, outNeuronCount))
	{
		out[i] = outLayer.neurons[i].outActivate;
	}

	return out;
}

void BPNetwork::Forward()
{
	for (RANGE(integer, i, layerCount))
		UpdateNeuronLayerForward(i);

	UpdateNeuronLayerForward();
}

void BPNetwork::Backward()
{
	for (RANGE(integer, num, layerCount))
	{
		NeuronLayer &layer = hiddenLayers[num];
		NeuronLayer &next = num == layerCount - 1 ? outLayer : hiddenLayers[num + 1];

		for (RANGE(integer, neuronId, layer.neuronCount))
		{
			Neuron &neuron = layer.neurons[neuronId];

			for (RANGE(integer, nextNeuronId, next.neuronCount))
			{
				neuron.outError += next.neurons[nextNeuronId].outError * next.neurons[nextNeuronId].weights[neuronId];
			}
		}
	}
}

void BPNetwork::UpdateOutLayerError(double *target)
{
	for (RANGE(integer, n, outNeuronCount))
	{
		outLayer.neurons[n].outError = - outLayer.neurons[n].outActivate + target[n];
	}
}

void networkDebug(BPNetwork *network)
{
	/*printf("in:%f, mid:%f, out%f, midErr:%f, outErr:%f, midW:%f, outW:%f\n",
		network->inLayer.neurons[0].outActivate,
		network->hiddenLayers[0].neurons[0].outActivate,
		network->outLayer.neurons[0].outActivate,
		network->hiddenLayers[0].neurons[0].outError,
		network->outLayer.neurons[0].outError,
		network->hiddenLayers[0].neurons[0].weights[0],
		network->outLayer.neurons[0].weights[0]
	);*/

	printf("error: %f\n", network->errorSum);

	/*for (auto& neuron : network->hiddenLayers[0].neurons)
	{
		printf("[%f,%f]\n", neuron.weights[0], neuron.weights[1]);
	}

	network->outLayer.PrintDataDebug();

	std::cout << std::endl;*/
}

bool BPNetwork::Train(double *in, double *target)
{
	PushDataInLayer(in);

	integer iterCount = 0;
	
	trainTarget = target;

	while (iterCount < 5)
	{
		//networkDebug(this);

		Forward();

		UpdateOutLayerError(trainTarget);

		//double prevError = errorSum;
		GetErrorSum();

		if (errorSum < 0.01)
		{
			if (iterCount != 0)
			{
				//printf("Stop Training, error=%.8f, iter = %d\n", errorSum, iterCount);
				//outLayer.PrintDataDebug();
			}
			return true;
		}

		Backward();

		TweakAllWeights();

		iterCount++;

		//outLayer.PrintDataDebug();
	}

	//printf("Stop Training, error=%.8f\n", errorSum);
	return false;
}

void BPNetwork::GetErrorSum()
{
	errorSum = 0.0;

	for (auto &neuron : outLayer.neurons)
		errorSum += pow(neuron.outError, 2.0);
}

void BPNetwork::TweakAllWeights()
{
	TweakWeight(outLayer, hiddenLayers[layerCount - 1]);

	for (integer i = 0; i < layerCount; i++)
	{
		TweakWeight(hiddenLayers[i], i == 0 ? inLayer : hiddenLayers[i - 1]);
	}
}

void BPNetwork::TweakWeight(NeuronLayer &layer, NeuronLayer &lastLayer)
{
	if(&layer == &outLayer)
	{
		for (integer count = 0; count < layer.neuronCount; count++)
	{
		Neuron &neuron = layer.neurons[count];
		double temp = -neuron.outActivate + trainTarget[count];
		
		temp*=learningRate;

		for (integer i = 0; i < neuron.weightCount; i++)
		{
			neuron.weights[i] += temp * lastLayer.neurons[i].outActivate + prevTweakCoeff * neuron.prevTweak;
		}
			
		neuron.prevTweak = temp;
	}
	}
	else
	for (integer count = 0; count < layer.neuronCount; count++)
	{
		Neuron &neuron = layer.neurons[count];
		double temp = BackActive(neuron.outActivate) * learningRate * neuron.outError;

		for (integer i = 0; i < neuron.weightCount; i++)
		{
			neuron.weights[i] += temp * lastLayer.neurons[i].outActivate + prevTweakCoeff * neuron.prevTweak;
		}
			
		neuron.prevTweak = temp;
	}
}

int BPNetwork::GetResult(double *in)
{
	PushDataInLayer(in);

	Forward();

	int biggest = 0;
	double biggestResult = 0.0;

	for (int i = 0; i < outNeuronCount; i++)
	{
		if (outLayer.neurons[i].outActivate > biggestResult)
		{
			biggestResult = outLayer.neurons[i].outActivate;
			biggest = i;
		}
	}

	//outLayer.PrintDataDebug();
	return biggest;
}