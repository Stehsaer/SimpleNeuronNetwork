/*
File name: StatusQuery.h
Function: Provide api functionality for web server front-end
*/

#pragma once

#include <drogon/HttpController.h>
using namespace drogon;

namespace API
{
	class Query : public drogon::HttpController<Query>
	{
	public:
		METHOD_LIST_BEGIN
			METHOD_ADD(Query::GetStatus, "/status", Get);
		METHOD_ADD(Query::GetDatasetList, "/dataset_list", Get);
		METHOD_ADD(Query::Recognize, "/recognize", Post);
		METHOD_LIST_END

			void GetStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
		void GetDatasetList(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
		void Recognize(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
	};

	/* COMMAND LIST
	
	> /load_dataset?image={image}&label={label}&name={name}&slot={slot}: Loads a dataset into memory
		image: image path for dataset
		label: label path for dataset
		name: name for dataset
		slot: dataset slot for loading
		
	> /clear_status: Resets server status from DONE to IDLE

	> /clear_slot?slot={<slot>}: Clears data in dataset slot

	> /create_model?in={<in>}&out={<out>}&layer={<layerCount>}&count={<layerNeuronCount>}&func={<activateFunc>}: creates an empty Back Propagation Network instance
		in: InLayer neuron count
		out: OutLayer neuron count
		layerCount: number of hidden layers
		layerNeuronCount: number of neurons in each hidden layer
		activateFunc: function used in propagation inside the network

	*/

	class Command : public drogon::HttpController<Command>
	{
	public:
		METHOD_LIST_BEGIN
			METHOD_ADD(Command::LoadDataset, "/load_dataset?image={image}&label={label}&name={name}&slot={slot}", Post);
		METHOD_ADD(Command::ClearStatus, "/clear_status", Post);
		METHOD_ADD(Command::ClearDatasetSlot, "/clear_slot?slot={}", Post);
		METHOD_ADD(Command::CreateModel, "/create_model?in={}&out={}&layer={}&count={}&func={}", Post);
		METHOD_ADD(Command::TrainModel, "/train_model?slot={}&learningRate={}&maxIter={}&threshold={}", Get, Post);
		METHOD_LIST_END

		void ClearDatasetSlot(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int slot);
		void TrainModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int slot, double learningRate, int maxIter, double threshold);
		void ClearStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
		void LoadDataset(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string image, std::string label, std::string name, int slot);
		void CreateModel(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, int inNeuronCount, int outNeuronCount, int layerCount, int layerNeuronCount, int activateFunc);
	};
}