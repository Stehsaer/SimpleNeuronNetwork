#define _OS_WINDOWS_

#include "Network.h"

// detect os
#ifdef _OS_WINDOWS_
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <thread>

void PrintProgress(int* progress, int max, bool* doCycle, int interval)
{
	std::string block[] = { "=", " " };
	
	while (*doCycle)
	{
		int percent = *progress * 100 / max;

		printf("\r> %d/%d %d%% [", *progress, max, percent);

		for (int i = 0; i < 50; i++)
		{
			printf("%s", i * 2 <= percent ? block[0].c_str() : block[1].c_str());
		}
		printf("] ");

#ifdef _OS_WINDOWS_
		Sleep(interval);
#else
		usleep(interval * 1000); // using usleep function under linux-like systems
#endif
	}

	return;
}

int main()
{
	Network::NetworkDataSet dataSet, verifySet;

	printf("Reading train set\n");
	Network::NetworkDataParser::ReadMNISTData(&dataSet,
		R"(D:\gzip\emnist-byclass-train-images-idx3-ubyte)",
		R"(D:\gzip\emnist-byclass-train-labels-idx1-ubyte)",
		Network::Algorithm::ZeroToOne);

	printf("Reading verify set\n");
	Network::NetworkDataParser::ReadMNISTData(&verifySet, 
		R"(D:\gzip\emnist-byclass-test-images-idx3-ubyte)", 
		R"(D:\gzip\emnist-byclass-test-labels-idx1-ubyte)",
		Network::Algorithm::ZeroToOne);

	printf("Data fetched. \nDetail: Trainset:%d files, Verifyset:%d files\n\n", dataSet.Count(), verifySet.Count());

	Network::Framework::BackPropaNetwork network(28 * 28, 62, 64, 1, &Network::Algorithm::ReLU, &Network::Algorithm::ReLU_D, 0.1);

	network.RandomizeAllWeights(0.1, 1.0);

	int progress = 0;
	
	for (int epoch = 0; epoch < 10; epoch++)
	{
		bool updateProgress = true;

		std::thread thr(PrintProgress, &progress, dataSet.Count(), &updateProgress, 50);
		thr.detach();

		for (progress = 0; progress < dataSet.Count(); progress++)
		{
			network.Train(dataSet[progress], 1, 0.001);
		}

		updateProgress = false;

		printf("\nVerifying...");
		double accuracy = network.GetAccuracy(verifySet);

		printf("\rFinished Epoch %d, Accuracy: %.3f%%\n", epoch + 1, accuracy * 100.0);

		network.learningRate /= 2.0;

	}

	return 0;
}