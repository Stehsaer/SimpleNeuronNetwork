/*
File name: MainNetworkInstance.h
Function: Contains main network objects needed for main thread
*/

#pragma once
#ifndef _NETWORK_INSTANCE_H_
#define _NETWORK_INSTANCE_H_

#include "Network.h"
#include "ProcessState.h"

#include <string>
#include <vector>

#define MAX_DATASET_COUNT 4

enum class serverStatus
{
	Idle,
	Query,
	Working,
	Done
};

enum class serverTask
{
	None = 0,
	ReadModel = 1,
	ReadDataset = 2,
	TrainModel = 3,
	RecognizeInput = 4
};

struct TrainSnapshot
{
	int index;
	double loss;

	TrainSnapshot(int index, double loss): index(index),loss(loss){}
};

extern std::string status_text;
extern float serverProgress;
extern std::string serverProgressDisplay;
extern serverTask serverCurrentTask;
extern serverStatus server_status;
extern int progressSuccess; // -1: none; 0: fail; 1:success;
extern bool trainTerminate;

extern Network::Connectivity::FullConnNetwork* network;

extern Network::NetworkDataSet datasets[MAX_DATASET_COUNT];

const std::string datasetPath = "web_res/datasets/";
const std::string modelPath = "web_res/models/";
const std::string webPagePath = "web/";

void LoadDatasetWork(std::string image, std::string label, int slot, std::string name, bool flip);
void ClearDataset(int slot);
void CreateModelWork(int inNeuronCount, int outNeuronCount, int layerCount, int layerNeuronCount, Network::ActivateFunctionType func);
void TrainModelWork(int slot, int maxIter, double learningRate, double threshold, int verifySlot);
void LoadModelWork(std::string name);
void SaveModelWork(std::string name);

#endif