#ifndef _BP_NETWORK_H_
#define _BP_NETWORK_H_

#include <vector>
#include <iostream>
#include <cstdlib>
#include <random>

struct Neuron;
struct NeuronLayer;

typedef std::vector<NeuronLayer> layer_list;
typedef std::vector<Neuron> neuron_list;
typedef int integer;

#define RANGE(type, val, range) type val = 0; val < range; val++

#define PAUSE system("pause")

#define DEBUG(str) printf(str),PAUSE;

//void Sigmoid(double);
//void RevSigmoid(double);
//void ReLU(double);
//void RevReLU(double);
//void SoftMax(double);
//void RevSoftMax(double);

struct Neuron
{
	double outActivate;
	double outError;
	double prevTweak;
	
	integer weightCount;
	double* weights;
	
	Neuron(integer count);
	void RandomizeWeights();
};

struct NeuronLayer
{
	integer connCount;
	integer neuronCount;
	
	double bias;
	double response;
	
	neuron_list neurons; 
	
	NeuronLayer() : bias(0), response(0), connCount(0), neuronCount(0) {}
	NeuronLayer(integer count, integer conn);

	void PrintDataDebug();
	void PrintErrorDebug();


	void RandomizeWeights();
};

class BPNetwork
{
public:
	// ����
	
	integer inNeuronCount, outNeuronCount, layerNeuronCount;
	
	integer layerCount;
	
	double learningRate, errorSum, prevTweakCoeff, outTotal;
	
	double* trainTarget;
	
	NeuronLayer inLayer, outLayer;
	layer_list hiddenLayers;
	
public:
	// ����

	BPNetwork(integer inCount, integer outCount, integer layerCount, integer neuronCount, double learningRate, double);
	//~BPNetwork();

	double ForwardActive(double activate, double response);
	double BackActive(double val);
	
	void UpdateNeuronLayerForward(integer id);
	void UpdateNeuronLayerForward();

	//void UpdateErrorBackward(integer id); 

	void PushDataInLayer(double* data);
	double* GetDataOutLayer();

	void Forward(); 
	void Backward(); 
	void UpdateOutLayerError(double* target);

	bool Train(double* in, double* target); 

	void GetErrorSum(); 

	void TweakAllWeights();
	void TweakWeight(NeuronLayer& layer, NeuronLayer& lastLayer);

	int GetResult(double* in);
};


#endif