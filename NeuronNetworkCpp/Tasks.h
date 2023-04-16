#pragma once

#ifndef _TASKS_H_
#define _TASKS_H_

#include <string>
#include <stdlib.h>

#include "Network.h"
#include "FileHelper.h"

namespace Tasks
{
	enum class TaskState
	{
		Idle,
		Working,
		Done,
		Error
	};

	struct Progress
	{
		float progress;
		bool isIndeterminate;
		TaskState state;
		std::string display;

		Progress(std::string txt = "") : progress(0.0f), isIndeterminate(true), display(txt), state(TaskState::Idle) {}

		void Idle(); void Work(std::string txt = ""); void Done(std::string txt = ""); void Error(std::string txt = "");

		bool Ready();

		void Set(float progress, std::string display = "");
		void SetIndeterminate(std::string display = "");
	};

	extern Progress taskProgress;

	namespace Instances
	{
		extern const int maxDatasetCount;
		extern const std::string datasetPath;
		extern const std::string networkPath;

		extern bool networkOccupied;
		extern Network::Framework::FullConnNetwork* mainNetwork;
		extern std::vector<Network::NetworkDataSet*> datasets;

		//extern std::vector<FileInfo> datasetFiles;
		extern std::vector<std::string> datasetDirs;
		extern std::vector<FileInfo> networkFileList;

		void ReadFileList(std::vector<FileInfo>& list, std::string path, std::initializer_list<const char*> filters);
		void ReadDirList(std::vector<std::string>& list, std::string path);

		bool RemoveDataset(int slot);
		bool AddDataset(Network::NetworkDataSet* dataset);
	}

	// work

	void LoadDataset(std::string path, std::string name, bool flip);
	void CreateNetwork(int inNeuronCount, int outNeuronCount, int hiddenLayerCount, int hiddenNeuronCount, int func);
	void ReadNetwork(std::string path);
	void SaveNetwork(std::string name);

	void TrainNetwork(double learningRate, double threshold, int trainDataset, int verifyDataset, int batchSize = 1);
	void VerifyNetwork(int verifyDataset);
	
}

#endif
