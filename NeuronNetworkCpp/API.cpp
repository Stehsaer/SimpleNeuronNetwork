#include "API.h"

#include "Network.h"
#include "MainNetworkInstance.h"

#include <json/json.h>
#include <filesystem>
#include <thread>

using namespace API;
namespace fs = std::filesystem;

Json::Value GetStatusJson()
{
	Json::Value statusValue;

	statusValue["status"] = (int)server_status;
	statusValue["status_text"] = status_text;
	statusValue["max_dataset_num"] = MAX_DATASET_COUNT;

	// datasets
	for (auto& set : datasets)
	{
		Json::Value v;
		v["name"] = set.name;
		v["mode"] = (int)set.mode;
		v["data_height"] = set.dataHeight;
		v["data_width"] = set.dataWidth;
		v["data_count"] = set.Count();

		statusValue["datasets_info"].append(v);
	}

	// model
	if (network == nullptr)
	{
		statusValue["network_info"] = "null";
	}
	else
	{
		Json::Value v;
		v["in_count"] = network->inNeuronCount;
		v["out_count"] = network->outNeuronCount;
		v["hidden_neuron_count"] = network->hiddenNeuronCount;
		v["hidden_layer_count"] = network->hiddenLayerCount;
	}

	return statusValue;
}

Json::Value GetDatasetListJson()
{
	fs::path dir(datasetPath);
	Json::Value val;

	for (auto& iter : fs::directory_iterator(dir))
	{
		if (!fs::is_directory(iter.status()))
		{
			Json::Value v;
			v["name"] = iter.path().filename().string();
			v["size"] = fs::file_size(iter.path());

			val["list"].append(v);
		}
	}

	return val;
}


void Query::GetStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
	Json::Value returnValue = GetStatusJson();

	auto response = HttpResponse::newHttpJsonResponse(returnValue);
	callback(response);
}

void Query::GetDatasetList(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
	Json::Value returnValue = GetDatasetListJson();

	auto response = HttpResponse::newHttpJsonResponse(returnValue);
	callback(response);
}

void Command::LoadDataset(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string image, std::string label, int slot)
{
	auto response = HttpResponse::newHttpResponse();

	if (server_status != serverStatus::Idle)
	{
		response->setStatusCode(k403Forbidden);
		callback(response);

		return;
	}
	else
	{
		server_status = serverStatus::Query;
		server_task = serverTask::ReadDataset;
		serverProgress = 0.0f;
		serverProgressDisplay = "";

		std::thread thr(LoadDatasetWork, image, label, slot);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}