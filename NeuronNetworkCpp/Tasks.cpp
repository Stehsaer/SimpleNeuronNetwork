#include "Tasks.h"

#include "ProgressTimer.h"

namespace Tasks
{
	Progress taskProgress("Idle");

	//==== Progress

	void Progress::Idle()
	{
		state = TaskState::Idle;
		display = "Idle";
	}

	void Progress::Work(std::string txt)
	{
		state = TaskState::Working;
		display = txt;
	}

	void Progress::Done(std::string txt)
	{
		state = TaskState::Done;
		display = txt;
	}

	void Progress::Error(std::string txt)
	{
		state = TaskState::Error;
		display = txt;
	}

	bool Progress::Ready()
	{
		return state == TaskState::Idle;
	}

	void Progress::Set(float progress, std::string display)
	{
		this->state = TaskState::Working;
		this->isIndeterminate = false;
		this->progress = progress;
		this->display = display;
	}

	void Progress::SetIndeterminate(std::string display)
	{
		this->state = TaskState::Working;
		this->isIndeterminate = true;
		this->progress = 0.0f;
		this->display = display;
	}

	namespace Instances
	{
		const int maxDatasetCount = 8;
		const std::string datasetPath = "resources/datasets/";
		const std::string networkPath = "resources/models/";

		Network::Framework::FullConnNetwork* mainNetwork = NULL;
		std::vector<Network::NetworkDataSet*> datasets;

		//std::vector<FileInfo> datasetFiles;
		std::vector<std::string> datasetDirs;
		std::vector<FileInfo> networkFileList;

		void ReadFileList(std::vector<FileInfo>& list, std::string path, std::initializer_list<const char*> filters)
		{
			list.clear();

			namespace fs = std::filesystem;

			for (auto& iter : fs::directory_iterator(path))
			{
				if (!fs::is_directory(iter.status()))
				{
					FileInfo info(iter.path().string());

					bool filtered = false;
					for (auto ext = filters.begin(); ext != filters.end(); ext++)
					{
						if (info.name.ends_with(*ext))
						{
							filtered = true;
							break;
						}
					}

					if (filtered) list.push_back(info);
				}
			}
		}

		void ReadDirList(std::vector<std::string>& list, std::string path)
		{
			list.clear();

			namespace fs = std::filesystem;

			for (auto& iter : fs::directory_iterator(path))
			{
				if (fs::is_directory(iter.status()))
				{
					std::string str = iter.path().string();
					list.push_back(str.substr(str.find_last_of('/') + 1, str.size()));
				}
			}
		}

		bool RemoveDataset(int slot)
		{
			try
			{
				if (slot >= datasets.size()) return false;

				datasets[slot]->Destroy();
				delete datasets[slot];

				datasets.erase(datasets.begin() + slot);

				return true;
			}
			catch (std::exception e)
			{
				return false;
			}
		}

		bool AddDataset(Network::NetworkDataSet* dataset)
		{
			if (datasets.size() >= maxDatasetCount)
			{
				return false;
			}

			datasets.push_back(dataset);

			return true;
		}
	}

	void LoadDataset(std::string path, std::string name, bool flip)
	{
		try
		{
			ProgressTimer timer;

			taskProgress.Work();
			taskProgress.SetIndeterminate("Loading Dataset");

			if (Instances::datasets.size() >= Instances::maxDatasetCount)
				throw std::exception("Dataset slot full!");

			Network::NetworkDataSet* set = new Network::NetworkDataSet(name);
			Network::NetworkDataParser::ReadMNISTData(set, Instances::datasetPath + path + "/image", Instances::datasetPath + path + "/label", Network::Algorithm::ZeroToOne);

			// flip XY
			if (flip)
				set->FlipXY();

			Instances::AddDataset(set);

			taskProgress.Done(std::format("Dataset {} loaded in {}ms", name, timer.CountMs()));
		}
		catch (std::exception e)
		{
			taskProgress.Error(std::format("Error while loading dataset {}. Message: {}", name, e.what()).c_str());
		}
	}

	void CreateNetwork(int inNeuronCount, int outNeuronCount, int hiddenLayerCount, int hiddenNeuronCount, int func)
	{
		try
		{
			ProgressTimer timer;

			taskProgress.SetIndeterminate("Creating Network");

			if (Instances::mainNetwork)
			{
				auto* network = Instances::mainNetwork;
				Instances::mainNetwork = NULL;
				network->Destroy();
				delete network;
			}

			Network::ActivateFunctionType funcType = (Network::ActivateFunctionType)func;

			Instances::mainNetwork = new Network::Framework::FullConnNetwork(inNeuronCount, outNeuronCount, hiddenNeuronCount, hiddenLayerCount, funcType);
			Instances::mainNetwork->RandomizeAllWeights(0.1, 0.9);

			taskProgress.Done(std::format("Network created in {}ms", timer.CountMs()));
		}
		catch(std::exception e)
		{
			taskProgress.Error(std::format("Error while creating network. Message: {}", e.what()).c_str());
		}
	}

	void ReadNetwork(std::string path)
	{
		try
		{
			ProgressTimer timer;

			taskProgress.SetIndeterminate("Loading Network");

			if (Instances::mainNetwork)
			{
				Instances::mainNetwork->Destroy();
				delete Instances::mainNetwork;

				Instances::mainNetwork = NULL;
			}

			ProcessState state = Network::NetworkDataParser::ReadNetworkDataJSON(&Instances::mainNetwork, path);

			if (state.success)
			{
				taskProgress.Done(std::format("Network loaded in {}ms", timer.CountMs()));
			}
			else
			{
				throw std::exception(state.msg.c_str());
			}
		}
		catch (std::exception e)
		{
			taskProgress.Error(std::format("Error while creating network. Message: {}", e.what()).c_str());
		}
	}

	void TrainNetwork(double learningRate, double threshold, int trainDataset, int verifyDataset, int batchSize)
	{
		try
		{
			ProgressTimer timer;
			taskProgress.Work("Training Network");

			if (!Instances::mainNetwork)
			{
				taskProgress.Error("No network loaded!");
				return;
			}

			Instances::mainNetwork->learningRate = learningRate;

			auto& trainSet = *Instances::datasets[trainDataset];
			auto& verifySet = *Instances::datasets[verifyDataset];

			for (int i = 0; i < trainSet.Count(); i++)
			{
				if (i % 100 == 0)
				{
					taskProgress.Set((float)i / (float)trainSet.Count(), std::format("Training Network: {}/{}", i, trainSet.Count()));
				}

				Instances::mainNetwork->Train(trainSet[i], 1, threshold);
			}

			taskProgress.Work("Verifying");

			double accuracy = Instances::mainNetwork->GetAccuracyCallbackFloat(verifySet, &taskProgress.progress);

			taskProgress.Done(std::format("Train Completed. Time: {}ms, Accuracy: {:.2f}%%", timer.CountMs(), accuracy * 100.0));
		}
		catch (std::exception e)
		{
			taskProgress.Error(std::format("Error while training network. Message: {}", e.what()).c_str());
		}
	}

	void VerifyNetwork(int verifyDataset)
	{
		try
		{
			ProgressTimer timer;
			taskProgress.Set(0.0, "Verifying Network");

			if (!Instances::mainNetwork)
			{
				taskProgress.Error("No network loaded!");
				return;
			}

			double accuracy = Instances::mainNetwork->GetAccuracyCallbackFloat(*Instances::datasets[verifyDataset], &taskProgress.progress);

			taskProgress.Done(std::format("Verification Completed. Time: {}ms, Accuracy: {:.2f}%%", timer.CountMs(), accuracy * 100.0));
		}
		catch (std::exception e)
		{
			taskProgress.Error(std::format("Error while verifying network. Message: {}", e.what()).c_str());
		}
	}

	void SaveNetwork(std::string name)
	{
		try
		{
			ProgressTimer timer;
			taskProgress.SetIndeterminate("Saving Network");

			if (!Instances::mainNetwork)
			{
				taskProgress.Error("No network loaded!");
				return;
			}

			if (name.find('.') != std::string::npos)
			{
				taskProgress.Error("Invalid name!");
				return;
			}

			Network::NetworkDataParser::SaveNetworkDataJSON(Instances::mainNetwork, Instances::networkPath + name + ".network");

			taskProgress.Done(std::format("Network Saved. Time: {}ms", timer.CountMs()));
		}
		catch (std::exception e)
		{
			taskProgress.Error(std::format("Error while verifying network. Message: {}", e.what()).c_str());
		}
	}
}