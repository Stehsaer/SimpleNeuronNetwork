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

		printf("\r> %d/%d %d%%", *progress, max, percent);

#ifdef _OS_WINDOWS_
		Sleep(interval);
#else
		usleep(interval * 1000); // using usleep function under linux-like systems
#endif
	}

	return;
}

// For test
#ifdef _OS_WINDOWS_
#define IMAGE2 R"(D:\gzip\emnist-mnist-test-images-idx3-ubyte)"
#define LABEL2 R"(D:\gzip\emnist-mnist-test-labels-idx1-ubyte)"
#define IMAGE1 R"(D:\gzip\emnist-mnist-train-images-idx3-ubyte)"
#define LABEL1 R"(D:\gzip\emnist-mnist-train-labels-idx1-ubyte)"
#else
#define IMAGE2 "/storage/emulated/0/image2"
#define LABEL2 "/storage/emulated/0/label2"
#define IMAGE1 "/storage/emulated/0/image1"
#define LABEL1 "/storage/emulated/0/label1"
#endif

int main()
{
	Network::NetworkDataSet dataSet, verifySet;

	printf("Reading train set\n");
	Network::NetworkDataParser::ReadMNISTData(&dataSet, IMAGE1, LABEL1, Network::Algorithm::ZeroToOne);

	printf("Reading verify set\n");
	Network::NetworkDataParser::ReadMNISTData(&verifySet, IMAGE2, LABEL2, Network::Algorithm::ZeroToOne);

	printf("Data fetched. \nDetail: Trainset:%d files, Verifyset:%d files\n\n", dataSet.Count(), verifySet.Count());

	Network::Framework::BackPropaNetwork network(28 * 28, 10, 64, 1, &Network::Algorithm::ReLU, &Network::Algorithm::ReLU_D, 0.1);

	network.RandomizeAllWeights(0.1, 1.0);

	int progress = 0;

	for (int epoch = 0; epoch < 1; epoch++)
	{
		bool updateProgress = true;

		std::thread thr(PrintProgress, &progress, dataSet.Count(), &updateProgress, 100);
		thr.detach();

		for (progress = 0; progress < dataSet.Count(); progress++)
		{
			network.Train(dataSet[progress], 1, 0.001);
		}

		updateProgress = false;

		printf("\nVerifying...");
		double accuracy = network.GetAccuracy(verifySet);

		printf("\rFinished Epoch %d, Accuracy: %.2f%%\n", epoch + 1, accuracy * 100.0);

		network.learningRate /= 2.0;
	}

	Network::NetworkDataParser::SaveNetworkData(&network, "D:/out.bin");
	network = Network::NetworkDataParser::ReadNetworkData("D:/out.bin");

	double accuracy = network.GetAccuracy(verifySet);

	printf("\rRead data, Accuracy: %.2f%%\n", accuracy * 100.0);

	return 0;
}