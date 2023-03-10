/*
File name: MainNetworkInstance.h
Function: Contains main network objects needed for main thread
*/

#pragma once
#ifndef _NETWORK_INSTANCE_H_
#define _NETWORK_INSTANCE_H_

#include "Network.h"
#include <string>

#define MAX_DATASET_COUNT 4

enum class serverStatus
{
	Idle,
	Query,
	Working
};

enum class serverTask
{
	None = 0,
	ReadModel = 1,
	ReadDataset = 2,
	TrainModel = 3,
	RecognizeInput = 4
};

extern std::string status_text;
extern float serverProgress;
extern std::string serverProgressDisplay;
extern serverTask server_task;
extern serverStatus server_status;
extern int progressSuccess; // -1: none; 0: fail; 1:success;

extern Network::Framework::BackPropaNetwork* network;

extern Network::NetworkDataSet datasets[MAX_DATASET_COUNT];

const std::string datasetPath = "web_res/datasets/";
const std::string webPagePath = "web/";

void LoadDatasetWork(std::string image, std::string label, int slot);

#endif