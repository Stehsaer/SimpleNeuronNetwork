#pragma once

#include <string>
#include <algorithm>
#include <map>

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
