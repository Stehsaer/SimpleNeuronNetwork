#include "PageProvider.h"
#include "Helper.h"
#include "MainNetworkInstance.h"

#include <drogon/drogon.h>

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace drogon;

namespace fs = std::filesystem;

void PageProviderFunc(const HttpRequestPtr& req, std::function<void(const HttpResponsePtr&)>&& callback, std::string filename)
{
	// check if file exists
	std::string filePath = fs::current_path().string() + "/" + webPagePath + filename;
	std::string ext = GetFileExt(filePath);

	if (filePath.find("..") != std::string::npos)
	{
		// unauthorized
		auto response = HttpResponse::newHttpResponse();
		response->setStatusCode(k403Forbidden);
		callback(response);

		return;
	}

	if (!fs::exists(filePath) || !fs::is_regular_file(filePath))
	{
		// return 404
		auto response = HttpResponse::newHttpResponse();
		response->setStatusCode(k404NotFound);
		callback(response);

		return;
	}

	//uintmax_t fileSize = fs::file_size(filePath); // reserved for future use

	auto response = HttpResponse::newFileResponse(filePath);

	response->setContentTypeCode(req->getContentType());
	callback(response);
}

void InitPageProvider()
{
	app().registerHandler("/{}", std::function(PageProviderFunc), { Get });
}