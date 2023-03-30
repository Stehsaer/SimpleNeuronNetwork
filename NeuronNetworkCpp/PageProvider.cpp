#include "PageProvider.h"
#include "MainNetworkInstance.h"

#include <drogon/drogon.h>

#include <iostream>
#include <fstream>
#include <filesystem>

using namespace drogon;

namespace fs = std::filesystem;

const std::map<std::string, std::string> contentTypeList =
{
	{"png", "_image/png"},
	{"jpg","_image/jpg"},
	{"html", "text/html"},
	{"css", "text/css"},
	{"js", "text/javascript"},
	{"ttf", "_font/ttf"},
};

std::string GetContentType(std::string suffix)
{
	try
	{
		std::string type = contentTypeList.at(suffix);
		return type;
	}
	catch (std::exception e) { return ""; }
}

inline std::string GetFileExt(std::string& strFile, int isLower = 0)
{
	if (isLower == 1)
	{
		std::string strTemp = strFile;
		std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
		std::string::size_type pos = strTemp.rfind('.');
		std::string strExt = strTemp.substr(pos == std::string::npos ? strTemp.length() : pos + 1);
		return strExt;
	}
	else
	{
		std::string::size_type pos = strFile.rfind('.');
		std::string strExt = strFile.substr(pos == std::string::npos ? strFile.length() : pos + 1);
		return strExt;
	}
}

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