#define _WINDOWS_

#include <stdio.h>
#include "BP_Network.h"
#include "BP_Network_Set.h"


#ifdef _ANDROID_
#define TEST_PATH "/sdcard/learnNN/learnNN/dataset/test/"
#define TRAIN_PATH "C:\\Users\\HFAI\\source\\repos\\learnNN\\dataset\\mnist_train"
#define TEST_LABEL_FILE "/sdcard/learnNNProj/data/t10k-labels.idx1-ubyte"
#define TEST_ORG_FILE "/sdcard/learnNNProj/data/t10k-images.idx3-ubyte"
#define TRAIN_LABEL_FILE "/sdcard/learnNNProj/data/train-labels.idx1-ubyte"
#define TRAIN_ORG_FILE "/sdcard/learnNNProj/data/train-images.idx3-ubyte"
#endif

#ifdef _WINDOWS_
#define TEST_PATH R"(C:\Users\Stehs\source\repos\dataset\mnist_test_bmp\)"
#define TRAIN_PATH R"(C:\Users\Stehs\source\repos\dataset\mnist_train_bmp\)"
#define TEST_LABEL_FILE "D:/t10k-labels.idx1-ubyte"
#define TEST_ORG_FILE "D:/t10k-images.idx3-ubyte"
#define TRAIN_LABEL_FILE "D:/train-labels.idx1-ubyte"
#define TRAIN_ORG_FILE "D:/train-images.idx3-ubyte"
#endif

int main()
{
	BPNetwork network(28*28, 10, 1, 128, .01, 0.0);
	
	double accuracy = 0.0;

	BPNetworkVerifySet verifySet;
	
	printf("Reading Verify Set...\n");
	verifySet.GetMNISTDataSet(path(TEST_ORG_FILE), path(TEST_LABEL_FILE));
	
	printf("Reading Train Set...\n");
	BPNetworkTrainSet trainset;
	
	trainset.GetMNISTDataSet(TRAIN_ORG_FILE,TRAIN_LABEL_FILE);

trainagain:
	trainset.TrainAll(&network, verifySet);

	/*BPNetworkSetData data(path(R"(D:\Users\HFAI\Desktop\�ޱ���.png)"), 1);
	printf("test: %d", network.GetResult(data.data));*/
	
	printf("Verifying...\n");
	accuracy = verifySet.GetAccuracy(&network);    
	printf("Accuracy: %.2f\n", accuracy * 100.0);
	//system("pause");

	if (accuracy < 0.8)
	{
		//if (accuracy > 0.6)
		//	network.learningRate /= 2.;
		//network.learningRate = fmax(.00001,network.learningRate);

		////std::cin>>network.learningRate;
		//printf("Learning rate tweaked to %.8f\n", network.learningRate);
		goto trainagain;
	}
}