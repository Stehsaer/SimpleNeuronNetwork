#ifndef _BPNETWORK_TRAIN_SET_H_
#define _BPNETWORK_TRAIN_SET_H_

#include <vector>
#include <string>
//#include <boost/filesystem.hpp>
#include <filesystem>
#include <algorithm>

#include "BP_Network.h"

#include <random>
#include <filesystem>

using namespace std::filesystem;

#define MNIST_DATACOUNT_OFFSET 4
#define MNIST_DATAWIDTH_OFFSET 12
#define MNIST_DATAHEIGHT_OFFSET 8
#define MNIST_DATA_LABEL_OFFSET 8
#define MNIST_DATA_IMAGE_OFFSET 16

inline double RandFloat()
{
	return (rand()) / (RAND_MAX + 1.0);
}

struct BPNetworkSetData
{
	int label;
	double *data;
	int dataSize;

	BPNetworkSetData(path _path, int label);
	BPNetworkSetData(unsigned char *originalData, int offset, int size, int label);
	void Train(BPNetwork *network);
};

class BPNetworkDataSet
{
  public:
	std::vector<BPNetworkSetData> dataset;
	path basePath;

	void AddLabelPath(std::string relPath, int label);
	void GetMNISTDataSet(path data_path, path label_path);
	void DestroyAll();
};

class BPNetworkVerifySet;

class BPNetworkTrainSet : public BPNetworkDataSet
{
  public:
	BPNetworkTrainSet(std::string inPath) { basePath = path(inPath); }
	BPNetworkTrainSet() {}

	void TrainAll(BPNetwork *network);
	void TrainAll(BPNetwork *network, BPNetworkVerifySet);
};

class BPNetworkVerifySet : public BPNetworkDataSet
{
  public:
	BPNetworkVerifySet(std::string inPath) { basePath = path(inPath); }
	BPNetworkVerifySet(){}

	double GetAccuracy(BPNetwork *network);
};

#endif