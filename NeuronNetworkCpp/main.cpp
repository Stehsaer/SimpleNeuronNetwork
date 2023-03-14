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
int port = 17788;
const std::string webPath = R"(D:\Users\HFAI\Documents\neu\SimpleNeuronNetwork\NeuronNetworkCpp\web)";

using namespace drogon;

int main()
{
	InitPageProvider();

	app().addListener("localhost", 10086);
	app().setLogLevel(trantor::Logger::LogLevel::kDebug);
	app().run();

	return 0;
}