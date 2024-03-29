#include "MainNetworkInstance.h"

#include <math.h>

// global variables
serverStatus server_status = serverStatus::Idle;
std::string serverProgressDisplay = "";
serverTask serverCurrentTask = serverTask::None;

std::string status_text = "Idle";
float serverProgress = 0.0f;

int progressSuccess = -1;

Network::Framework::FullConnNetwork* network = nullptr;

Network::NetworkDataSet datasets[MAX_DATASET_COUNT];

std::vector<TrainSnapshot> snapshots;

bool trainTerminate = false;

using ActivateFunctionType = Network::ActivateFunctionType;

// works

void LoadDatasetWork(std::string image, std::string label, int slot, std::string name, bool flip)
{
	server_status = serverStatus::Working;
	serverProgressDisplay = "Loading Dataset";
	serverProgress = -1;

	if (slot < 0 || slot >= MAX_DATASET_COUNT)
	{
		serverProgressDisplay = "Invalid Parameters!";
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}

	datasets[slot].Destroy();
	datasets[slot] = Network::NetworkDataSet(name);

	clock_t start = clock();

	ProcessState result = Network::NetworkDataParser::ReadMNISTData(&datasets[slot], datasetPath + image, datasetPath + label, Network::Algorithm::NormalizationMode::ZeroToOne);

	if (flip)
	{
		datasets[slot].FlipXY();
	}

	clock_t elapsedTime = clock() - start;

	if (result.success)
	{
		serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
		serverProgress = 0.0f;
		progressSuccess = 1;

		server_status = serverStatus::Done;

		return;
	}
	else
	{
		serverProgressDisplay = result.msg; std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
}

void ClearDataset(int slot)
{
	server_status = serverStatus::Working;
	serverProgress = -1;
	serverProgressDisplay = "Clearing Dataset";

	if (slot < 0 || slot >= MAX_DATASET_COUNT)
	{
		serverProgressDisplay = "Invalid Parameters!";
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}

	clock_t start = clock();

	datasets[slot].Destroy();
	datasets[slot] = Network::NetworkDataSet();

	clock_t elapsedTime = clock() - start;

	serverProgressDisplay = std::format("Deleted slot {}! Elapsed time: {}ms",slot, elapsedTime);
	serverProgress = 0.0f;
	progressSuccess = 1;

	server_status = serverStatus::Done;

	return;
}

void CreateModelWork(int inNeuronCount, int outNeuronCount, int layerCount, int layerNeuronCount, ActivateFunctionType func)
{
	server_status = serverStatus::Working;
	serverProgress = -1;
	serverProgressDisplay = "Creating Network";

	try
	{
		clock_t start = clock();

		// delete existed network
		if (network)
		{
			network->Destroy();
		}

		// set activate function
		Network::ActivateFunction forward, backward;

		forward = Network::forwardFuncList[(int)func];
		backward = Network::backwardFuncList[(int)func];

		network = new Network::Framework::FullConnNetwork(inNeuronCount, outNeuronCount, layerNeuronCount, layerCount, func, 0.0);
		network->RandomizeAllWeights(0.1, 0.9);

		clock_t elapsedTime = clock() - start;

		// set success status
		serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
		serverProgress = 0.0f;
		progressSuccess = 1;

		server_status = serverStatus::Done;
	}
	catch (std::exception e)
	{
		serverProgressDisplay = std::format("Unhandled exception: {}", e.what()); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
}

#define TRAIN_REPORT_RATE 100

void TrainModelWork(int slot, int maxIter, double learningRate, double threshold, int verifySlot)
{
	server_status = serverStatus::Working;
	trainTerminate = false;

	try
	{
		// error detections
		if (network == nullptr)
		{
			throw std::invalid_argument("Network not loaded!");
		}

		// verify arguments
		if (slot < 0 || slot >= MAX_DATASET_COUNT || verifySlot < 0 || verifySlot >= MAX_DATASET_COUNT || learningRate < 0.0 || maxIter < 0)
		{
			throw std::invalid_argument("Invalid Argument!");
		}

		Network::NetworkDataSet* dataset = &datasets[slot];

		// check if dataset is empty
		if (dataset->Count() <= 0)
		{
			throw std::invalid_argument("Empty Dataset!");
		}

		// verify in-neuron-count
		if (dataset->dataWidth * dataset->dataHeight != network->inNeuronCount)
		{
			throw std::invalid_argument("Dimension mismatch!");
		}

		// do shuffling
		serverProgressDisplay = "Shuffling Dataset...";
		serverProgress = -1;

		dataset->Shuffle();

		network->learningRate = learningRate;

		clock_t start = clock();

		char* progressText = new char[256];

		if (maxIter == 0) // 0 for verify only
		{
			goto VERIFY_SLOT;
		}

		for (int i = 0; i < dataset->Count(); i++)
		{
			// update progress
			serverProgress = (float)i / (float)dataset->Count();
			if (i % TRAIN_REPORT_RATE == 0)
			{
				
				snprintf(progressText, 256, "Training %d/%d. Loss: %.5f", i, dataset->Count(), network->loss);
				serverProgressDisplay = progressText;

				std::cout << serverProgressDisplay << std::endl;
			}

			network->Train(*(dataset->dataSet[i]), maxIter, threshold);

			_ASSERT(!isnan(network->loss));
		}

		VERIFY_SLOT:

		serverProgressDisplay = "Calculating Accuracy";
		serverProgress = 0.0f;
		double accuracy = network->GetAccuracyCallbackFloat(datasets[verifySlot], &serverProgress);

		clock_t elapsedTime = clock() - start;

		snprintf(progressText, 256, "Completed! <strong>Accuracy: %.4f%%</strong><br/>Elapsed time: %.3fs", accuracy * 100.0, elapsedTime / 1000.0);

		serverProgressDisplay = progressText;
		serverProgress = 1.0f;
		progressSuccess = 1;

		server_status = serverStatus::Done;
	}
	catch (std::invalid_argument e)
	{
		serverProgressDisplay = e.what(); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
	catch (std::exception e)
	{
		serverProgressDisplay = std::format("Unhandled exception: {}", e.what()); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
}

#define MODEL_SUFFIX ".network"

void LoadModelWork(std::string name)
{
	server_status = serverStatus::Working;
	serverProgress = -1;
	serverProgressDisplay = "Loading Network";

	try
	{
		clock_t start = clock();

		if (network) network->Destroy();

		// suffix needed
		//ProcessState result = Network::NetworkDataParser::ReadNetworkData(&network, modelPath + name);

		// use new json format data instead
		ProcessState result = Network::NetworkDataParser::ReadNetworkDataJSON(&network, modelPath + name);

		clock_t elapsedTime = clock() - start;

		if (result.success)
		{
			serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
			serverProgress = 0.0f;
			progressSuccess = 1;

			server_status = serverStatus::Done;

			return;
		}
		else
		{
			serverProgressDisplay = result.msg; std::cout << serverProgressDisplay << std::endl; // print out error
			serverProgress = 0.0f;
			progressSuccess = 0;

			server_status = serverStatus::Done;

			return;
		}
	}
	catch (std::exception e)
	{
		serverProgressDisplay = std::format("Unhandled exception: {}", e.what()); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
}

void SaveModelWork(std::string name)
{
	server_status = serverStatus::Working;

	try
	{
		clock_t start = clock();

		if (!network)
		{
			throw std::invalid_argument("Network Instance is empty!");
		}

		//ProcessState result = Network::NetworkDataParser::SaveNetworkData(network, modelPath + name + MODEL_SUFFIX);

		// use new json format data instead
		ProcessState result = Network::NetworkDataParser::SaveNetworkDataJSON(network, modelPath + name + MODEL_SUFFIX);

		clock_t elapsedTime = clock() - start;

		if (result.success)
		{
			serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
			serverProgress = 0.0f;
			progressSuccess = 1;

			server_status = serverStatus::Done;

			return;
		}
		else
		{
			serverProgressDisplay = result.msg; std::cout << serverProgressDisplay << std::endl; // print out error
			serverProgress = 0.0f;
			progressSuccess = 0;

			server_status = serverStatus::Done;

			return;
		}
	}
	catch (std::invalid_argument e)
	{
		serverProgressDisplay = e.what(); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
	catch (std::exception e)
	{
		serverProgressDisplay = std::format("Unhandled exception: {}", e.what()); std::cout << serverProgressDisplay << std::endl; // print out error
		serverProgress = 0.0f;
		progressSuccess = 0;

		server_status = serverStatus::Done;

		return;
	}
}