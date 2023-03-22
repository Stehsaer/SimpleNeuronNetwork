// network lib
#include "Network.h"

// web server
#include "PageProvider.h"
#include "API.h"

// main network instance
#include "MainNetworkInstance.h"

// sys lib
#include <string_view>
#include <map>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>

// 3rd-party libs
#include <drogon/drogon.h>
#include <json/json.h>

using namespace std::filesystem;

// web service
int port = 80;

using namespace drogon;

int main()
{
	InitPageProvider();

	printf("Server starting. Parameters: port=%d\n", port);

	app().addListener("localhost", port);
	app().setLogLevel(trantor::Logger::LogLevel::kError);
	app().run();

	return 0;
}