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
        METHOD_LIST_END

        void GetStatus(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
        void GetDatasetList(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback);
    };

    class Command : public drogon::HttpController<Command>
    {
    public:
        METHOD_LIST_BEGIN
            METHOD_ADD(Command::LoadDataset, "/loadmodel?image={1}&label={2}&slot={3}", Get);
            //METHOD_ADD(Command::Shutdown, "/shutdown?password={}", Post);
        METHOD_LIST_END

        void LoadDataset(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string image, std::string label, int slot);
        //void Shutdown(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string password);
    };
}