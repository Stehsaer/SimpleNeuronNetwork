#include "MainNetworkInstance.h"

serverStatus server_status = serverStatus::Idle;
std::string serverProgressDisplay = "";
serverTask server_task = serverTask::None;

std::string status_text = "Idle";
float serverProgress = 0.0f;

int progressSuccess = -1;

Network::Framework::BackPropaNetwork* network = nullptr;

Network::NetworkDataSet datasets[MAX_DATASET_COUNT];

void LoadDatasetWork(std::string image, std::string label, int slot)
{
	if (slot < 0 || slot >= MAX_DATASET_COUNT)
	{
		serverProgressDisplay = "Invalid Parameters!";
		serverProgress = 1.0f;
		progressSuccess = 0;

		server_status = serverStatus::Idle;

		return;
	}

	datasets[slot].Destroy();
	datasets[slot] = Network::NetworkDataSet();

	clock_t start = clock();

	Network::ProcessState result = Network::NetworkDataParser::ReadMNISTData(&datasets[slot], datasetPath + image, datasetPath + label, Network::Algorithm::NormalizationMode::ZeroToOne);

	clock_t elapsedTime = clock() - start;

	if (result.success)
	{
		serverProgressDisplay = std::format("Completed! Elapsed time: {}ms", elapsedTime);
		serverProgress = 1.0f;
		progressSuccess = 1;

		server_status = serverStatus::Idle;

		return;
	}
	else
	{
		serverProgressDisplay = std::format("Fail!\nMessage: {}", result.msg);
		serverProgress = 1.0f;
		progressSuccess = 0;

		server_status = serverStatus::Idle;

		return;
	}
}