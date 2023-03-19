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

	// basic status info
	statusValue["status"] = (int)server_status;
	statusValue["status_text"] = serverProgressDisplay;
	statusValue["progress"] = serverProgress;
	statusValue["success"] = progressSuccess;
	statusValue["max_dataset_num"] = MAX_DATASET_COUNT;
	statusValue["current_task"] = (int)server_task;

	// datasets
	int index = 0;
	for (auto& set : datasets)
	{
		Json::Value v;
		v["name"] = set.name;
		v["mode"] = (int)set.mode;
		v["data_height"] = set.dataHeight;
		v["data_width"] = set.dataWidth;
		v["data_count"] = set.Count();
		v["index"] = index, index++;

		statusValue["datasets_info"].append(v);
	}

	// model
	if (network == nullptr)
	{
		statusValue["network_info"] = Json::nullValue;
	}
	else
	{
		Json::Value v;
		v["in_count"] = network->inNeuronCount;
		v["out_count"] = network->outNeuronCount;
		v["hidden_neuron_count"] = network->hiddenNeuronCount;
		v["hidden_layer_count"] = network->hiddenLayerCount;
		v["loss"] = network->loss;
		v["learning_rate"] = network->learningRate;

		statusValue["network_info"].append(v);
	}

	return statusValue;
}

Json::Value GetFileList(std::string path)
{
	fs::path dir(path);
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
	Json::Value returnValue = GetFileList(datasetPath);

	auto response = HttpResponse::newHttpJsonResponse(returnValue);
	callback(response);
}

void Query::GetModelList(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
	Json::Value returnValue = GetFileList(modelPath);

	auto response = HttpResponse::newHttpJsonResponse(returnValue);
	callback(response);
}

void Command::LoadDataset(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string image, std::string label, std::string name, int slot)
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

		std::cout << std::format("Load Dataset: image={}; label={}; slot={}; name={}", image, label, slot, name) << std::endl;

		std::thread thr(LoadDatasetWork, image, label, slot, name);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}

void Command::ClearStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
	auto response = HttpResponse::newHttpResponse();

	if (server_status == serverStatus::Done)
	{
		server_status = serverStatus::Idle;
		progressSuccess = -1;
		serverProgressDisplay = "";
		serverProgress = 0.0f;

		response->setStatusCode(k200OK);
	}
	else
	{
		response->setStatusCode(k403Forbidden);
	}

	callback(response);
}

void Command::ClearDatasetSlot(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int slot)
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

		std::cout << std::format("Clear Dataset: slot={}", slot) << std::endl;

		std::thread thr(ClearDataset, slot);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}

void Command::CreateModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int inNeuronCount, int outNeuronCount, int layerCount, int layerNeuronCount, int activateFunc)
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
		server_task = serverTask::ReadModel;
		serverProgress = 0.0f;
		serverProgressDisplay = "";

		std::cout << std::format("Create Model: {},{},{},{},{}", inNeuronCount,outNeuronCount,layerCount,layerNeuronCount,activateFunc) << std::endl;

		std::thread thr(CreateModelWork, inNeuronCount, outNeuronCount, layerCount, layerNeuronCount, (Network::ActivateFunctionType)activateFunc);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}

void Command::TrainModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int slot, double learningRate, int maxIter, double threshold)
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
		server_task = serverTask::TrainModel;
		serverProgress = 0.0f;
		serverProgressDisplay = "";

		std::cout << std::format("Training Model: slot={}, maxIter={}, learningRate={}, threshold={}", slot, maxIter, learningRate, threshold ) << std::endl;

		std::thread thr(TrainModelWork, slot, maxIter, learningRate, threshold);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}

void Query::Recognize(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback)
{
	std::cout << "Recognize." << std::endl;

	try
	{
		if (!network)
		{
			throw std::invalid_argument("No network loaded!");
		}

		std::string body = std::string(req->getBody());

		Json::Value value;
		Json::Reader reader; reader.parse(body, value, false); // parse json value

		int size = value.size();

		if (!value.isArray() || size != network->inNeuronCount)
		{
			throw std::invalid_argument("Invalid arguments");
		}

		unsigned char* rawData = new unsigned char[size];

		for (int i = 0; i < size; i++)
			rawData[i] = value[i].asInt();

		// normalize data
		float* normalizedData = Network::Algorithm::NormalizeData(rawData, 0, size, Network::Algorithm::ZeroToOne);

		// push data into network
		network->PushDataFloat(normalizedData);
		delete[] normalizedData;
		delete[] rawData;

		network->ForwardTransmit(); // run network

		Json::Value outValue;

		for (int i = 0; i < network->outNeuronCount; i++)
		{
			outValue.append(Json::Value(network->outLayer[i].value));
		}

		auto response = HttpResponse::newHttpJsonResponse(outValue);
		callback(response);
	}
	catch (std::invalid_argument e)
	{
		auto response = HttpResponse::newHttpResponse();

		response->setStatusCode(k400BadRequest);
		response->setBody(e.what());
		callback(response);
	}
	catch (std::exception e)
	{
		auto response = HttpResponse::newHttpResponse();

		response->setStatusCode(k500InternalServerError);
		response->setBody(std::format("Unhandled Error: {}", e.what()));
		callback(response);
	}
}

void Command::SaveModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string name)
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
		server_task = serverTask::TrainModel;
		serverProgress = 0.0f;
		serverProgressDisplay = "";

		std::cout << std::format("Save Model: name={}", name) << std::endl;

		std::thread thr(SaveModelWork, name);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}

void Command::LoadModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string name)
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
		server_task = serverTask::TrainModel;
		serverProgress = 0.0f;
		serverProgressDisplay = "";

		std::cout << std::format("Load Model: name={}", name) << std::endl;

		std::thread thr(LoadModelWork, name);
		thr.detach();

		response->setStatusCode(k200OK);
		callback(response);
	}
}