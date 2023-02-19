#include "BP_Network_Set.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define _WINDOWS_

#define NOISE_AMPTITUDE 0.1

double NormalizeData(unsigned char *data)
{
	double brightness = 0.3 * *data + 0.59 * *(data + 1) + 0.11 * *(data + 2);
	brightness /= 128. - 1.;
	return brightness;
}

double NormalizeData(unsigned char data)
{
	double brightness = data;
	brightness /= 255.;
	return brightness;
}

double AddNoiseToData(double in)
{
	return in - NOISE_AMPTITUDE / 2.0 + RandFloat() * NOISE_AMPTITUDE;
}

void reverse(int* x) // convert MSB & LSB
{
	int y = 0;
	y = y | (0xff000000 & *x) >> 24;
	y = y | (0x00ff0000 & *x) >> 8;
	y = y | (0x0000ff00 & *x) << 8;
	y = y | (0x000000ff & *x) << 24;
	*x = y;
}

BPNetworkSetData::BPNetworkSetData(path _path, int label)
{
	//printf("Reading file: %s (label: %d)\n", _path.filename().string().c_str(), label);

	int x, y, comp;
	unsigned char *data = stbi_load(_path.string().c_str(), &x, &y, &comp, 3);

	//for(int emm=0;emm <)

	if (!data)
	{
		printf("%s", _path.string().c_str());
		PAUSE;
		printf("Failed to read file: %s\n", _path.filename().string().c_str());
		return;
	}

	double *normalizedData = new double[x * y];

	for (int i = 0; i < x * y; i++)
	{
		normalizedData[i] = AddNoiseToData(NormalizeData(data + 3 * i));
	}

	this->data = normalizedData;
	this->label = label;
	this->dataSize = x * y;

	free(data);
}

BPNetworkSetData::BPNetworkSetData(unsigned char *originalData, int offset, int size, int label)
{
	dataSize = size;
	this->label = label;
	data = new double[size];

	unsigned char *dat = originalData + offset;

	for (int i = 0; i < size; i++)
	{
		data[i] = AddNoiseToData(NormalizeData(dat[i]));
	}
}

void BPNetworkSetData::Train(BPNetwork *network)
{
	double *target = new double[network->outNeuronCount];

	for (int i = 0; i < network->outNeuronCount; i++)
		target[i] = i == label ? target[i] = 1.0 : target[i] = -0.0;

	network->Train(data, target);
	
	free(target);
}

void BPNetworkDataSet::AddLabelPath(std::string relPath, int label)
{
	path p = basePath / relPath;

	if (exists(p))
	{
		for (const auto &iter : directory_iterator(p))
		{
			if (is_regular_file(iter.path()))
			{
				dataset.push_back(BPNetworkSetData(iter.path().string(), label));
			}
		}
	}
	else
	{
		printf("Such path doesn't exist: %s\n", p.string().c_str());
		return;
	}
}

void BPNetworkDataSet::DestroyAll()
{
	for (auto &set : dataset)
	{
		free(set.data);
	}
}

void BPNetworkDataSet::GetMNISTDataSet(path data_path, path label_path)
{
	if (!is_regular_file(data_path) || !is_regular_file(label_path))
	{
		std::cout << "Not a valid path!\n";
		PAUSE;
	}

	int dataFileSize = file_size(data_path);
	int labelFileSize = file_size(label_path);

	unsigned char *data = new unsigned char[dataFileSize];
	unsigned char *label = new unsigned char[labelFileSize];

	FILE* dataFile;
	FILE* labelFile;

#ifdef _ANDROID_
	dataFile = fopen(data_path.string().c_str(), "r");
	labelFile = fopen(label_path.string().c_str(), "r");
#endif

#ifdef _WINDOWS_
	fopen_s(&dataFile, data_path.string().c_str(), "r");
	fopen_s(&labelFile, label_path.string().c_str(), "r");
#endif

	if (!dataFile || !labelFile)
	{
		throw "Read Error!";
		exit(-1);
	}

	fread(data, sizeof(unsigned char), dataFileSize, dataFile);
	fread(label, sizeof(unsigned char), labelFileSize, labelFile);

	fclose(dataFile);
	fclose(labelFile);

	int dataCount, labelCount;

	memcpy(&dataCount, data + MNIST_DATACOUNT_OFFSET, sizeof(int)); reverse(&dataCount);
	memcpy(&labelCount, label + MNIST_DATACOUNT_OFFSET, sizeof(int)); reverse(&labelCount);

	if (dataCount != labelCount)
	{
		throw "Data sizes don't match!";
	}

	int imageWidth, imageHeight;

	memcpy((unsigned char *)&imageWidth, data + MNIST_DATAWIDTH_OFFSET, sizeof(int)); reverse(&imageWidth);
	memcpy((unsigned char *)&imageHeight, data + MNIST_DATAHEIGHT_OFFSET, sizeof(int)); reverse(&imageHeight);

	for (int i = 0; i < dataCount; i++)
	{
		dataset.push_back(BPNetworkSetData(data, 
			i * imageWidth * imageHeight + MNIST_DATA_IMAGE_OFFSET, 
			imageWidth * imageHeight, 
			label[MNIST_DATA_LABEL_OFFSET + i]));
		printf("Reading File %d, label %d\r", i, label[MNIST_DATA_LABEL_OFFSET +i]);
	}

	printf("\n");

	delete[](data);
	delete[](label);
}

auto get_URBG()
{
	std::random_device rd;
	std::mt19937 g(rd());
	return g;
}

void BPNetworkTrainSet::TrainAll(BPNetwork *network)
{
	std::shuffle(dataset.begin(), dataset.end(), get_URBG());

	for (int i = 0; i < dataset.size(); i++)
	{
		auto &dataObj = dataset[i];
		dataObj.Train(network);

		if (i % 100 == 0)
			printf("Trained %d, error: %f            \r", i, network->errorSum);
		//network->hiddenLayers[0].PrintDataDebug();
		//network->inLayer.PrintDataDebug();
		//system("pause");
	}
}

void BPNetworkTrainSet::TrainAll(BPNetwork *network, BPNetworkVerifySet set)
{
	printf("Train Start, shuffling...\n");
	std::shuffle(dataset.begin(), dataset.end(), get_URBG());

	for (int i = 0; i < dataset.size(); i++)
	{
		auto &dataObj = dataset[i];
		dataObj.Train(network);

		if (i % 5000 == 0)
		{
			double acc = set.GetAccuracy(network);
			printf("Trained %d, error: %f, acc: %f           \r", i, network->errorSum, acc * 100);
		}

		//network->hiddenLayers[0].PrintDataDebug();
		//network->inLayer.PrintDataDebug();
		//system("pause");
	}
}

double BPNetworkVerifySet::GetAccuracy(BPNetwork *network)
{
	int correctCount = 0;

	std::vector<int> countLabel;
	std::vector<int> countResult;

	for (int i = 0; i < network->outNeuronCount; i++)
	{
		countLabel.push_back(0);
		countResult.push_back(0);
	}

	for (auto &dataObj : dataset)
	{
		int labelOut = network->GetResult(dataObj.data);

		if (labelOut == dataObj.label)
		{
			correctCount++;
			countLabel[labelOut]++;
		}

		countResult[labelOut]++;

		//else // debug
		//{
		//	network->outLayer.PrintDataDebug();
		//	system("pause");
		//}
	}

	/*printf("\n[");
	for (int i : countLabel)
	{
		printf("%d ", i);
	}
	printf("\b]\n");
	printf("\n[");
	for (int i : countResult)
	{
		printf("%d ", i);
	}
	printf("\b]\n");*/

	//network->hiddenLayers[0].PrintDataDebug();
	//network->outLayer.PrintDataDebug();

	countLabel.clear();
	countResult.clear();

	return (double)correctCount / (double)dataset.size();
}