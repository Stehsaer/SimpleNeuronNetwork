#include "MainNetworkInstance.h"

#include <math.h>

// global variables
serverStatus server_status = serverStatus::Idle;
std::string serverProgressDisplay = "";
serverTask server_task = serverTask::None;

std::string status_text = "Idle";
float serverProgress = 0.0f;

int progressSuccess = -1;

Network::Framework::BackPropaNetwork* network = nullptr;

Network::NetworkDataSet datasets[MAX_DATASET_COUNT];

std::vector<TrainSnapshot> snapshots;

// works

void LoadDatasetWork(std::string image, std::string label, int slot, std::string name)
{
	server_status = serverStatus::Working;

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

	Network::ProcessState result = Network::NetworkDataParser::ReadMNISTData(&datasets[slot], datasetPath + image, datasetPath + label, Network::Algorithm::NormalizationMode::ZeroToOne);

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

	try
	{
		clock_t start = clock();

		// delete existed network
		if (network)
		{
			network->Destroy();
		}

		// set activate function
		Network::ActivateFunction forward = nullptr, backward = nullptr;

		switch (func)
		{
		case ActivateFunctionType::Sigmoid:
			forward = *Network::Algorithm::Sigmoid;
			backward = *Network::Algorithm::Sigmoid_D;
			break;
		case ActivateFunctionType::SigmoidShifted:
			forward = *Network::Algorithm::ShiftedSigmoid;
			backward = *Network::Algorithm::ShiftedSigmoid_D;
			break;
		case ActivateFunctionType::ReLU:
			forward = *Network::Algorithm::ReLU;
			backward = *Network::Algorithm::ReLU_D;
			break;
		default:
			forward = *Network::Algorithm::ReLU;
			backward = *Network::Algorithm::ReLU_D;
		}

		network = new Network::Framework::BackPropaNetwork(inNeuronCount, outNeuronCount, layerNeuronCount, layerCount, forward, backward, 0.0);
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

void TrainModelWork(int slot, int maxIter, double learningRate, double threshold)
{
	server_status = serverStatus::Working;

	try
	{
		if (network == nullptr)
		{
			throw std::invalid_argument("Network not loaded!");
		}

		// verify arguments
		if (slot < 0 || slot >= MAX_DATASET_COUNT || learningRate < 0.0 || maxIter <=0)
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

		network->learningRate = learningRate;

		clock_t start = clock();

		for (int i = 0; i < dataset->Count(); i++)
		{
			// update progress
			serverProgress = (float)i / (float)dataset->Count();
			if (i % TRAIN_REPORT_RATE == 0)
			{
				serverProgressDisplay = std::format("Training {}/{}. Loss: {}", i, dataset->Count(), network->loss);
				std::cout << serverProgressDisplay << std::endl;
			}

			network->Train(*(dataset->dataSet[i]), maxIter, threshold);
		}

		clock_t elapsedTime = clock() - start;

		serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
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