#include "Network.h"
#include <Windows.h>
#include <thread>

void PrintProgress(int* progress, int max, bool* doCycle, int interval)
{
	std::string block[] = { "=", " " };
	int prevProgress = *progress;
	int avgSpeed = 0;
	
	while (*doCycle)
	{
		int percent = *progress * 100 / max;
		int speed = *progress - prevProgress;
		speed = speed * 1000.0f / (float)interval;

		avgSpeed = speed * 0.1 + avgSpeed * 0.9;

		printf("\r> %d/%d %d%% [", *progress, max, percent);

		for (int i = 0; i < 50; i++)
		{
			printf("%s", i * 2 <= percent ? block[0].c_str() : block[1].c_str());
		}
		printf("] %d/s       ", avgSpeed);
		prevProgress = *progress;

		Sleep(interval);
	}

	return;
}

int main()
{
	Network::NetworkDataSet dataSet, verifySet;

	printf("Reading train set\n");
	Network::NetworkDataParser::ReadMNISTData(&dataSet,
		R"(D:\gzip\emnist-mnist-train-images-idx3-ubyte)",
		R"(D:\gzip\emnist-mnist-train-labels-idx1-ubyte)",
		Network::Algorithm::ZeroToOne);

	printf("Reading verify set\n");
	Network::NetworkDataParser::ReadMNISTData(&verifySet, 
		R"(D:\gzip\emnist-mnist-test-images-idx3-ubyte)", 
		R"(D:\gzip\emnist-mnist-test-labels-idx1-ubyte)",
		Network::Algorithm::ZeroToOne);

	printf("Data fetched. \nDetail: Trainset:%d files, Verifyset:%d files\n\n", dataSet.Count(), verifySet.Count());

	Network::Framework::BackPropaNetwork network(28 * 28, 10, 64, 1, &Network::Algorithm::ReLU, &Network::Algorithm::ReLU_D, 0.1);

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