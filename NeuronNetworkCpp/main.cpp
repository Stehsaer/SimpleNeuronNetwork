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

//#define TEST_CODE

#ifdef TEST_CODE

#endif

int main()
{

#ifdef TEST_CODE

	return -1;
#endif

	InitPageProvider();

	printf("Server starting. Parameters: port=%d\n", port);

	app().addListener("localhost", port);
	app().setLogLevel(trantor::Logger::LogLevel::kError);
	app().run();

	return 0;
}