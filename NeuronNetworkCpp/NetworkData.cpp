#include "NetworkData.h"
#include "json/json.h"

#include "NetworkAlgorithm.h"

using namespace std::filesystem;

using namespace Network;

// Modified 2023-2-20
NetworkData::NetworkData()
{
	this->data = nullptr;
	this->label = -1;
	this->dataSize = -1;
}

void NetworkData::DrawContentDebug(int width, Algorithm::NormalizationMode mode)
{
	const std::string chars[] = { "  ", "██" };

	printf("Content of data: (label %d)", label);

	switch (mode)
	{
	case Algorithm::ZeroToOne:
		for (int i = 0; i < dataSize; i++)
		{
			printf(chars[int(floor(data[i] * 2.0))].c_str()); // print string into console
			
			if (i % width == width - 1)
			{
				printf("\n");
			}
		}
		break;

	case Algorithm::MinusOneToOne:
		for (int i = 0; i < dataSize; i++)
		{
			printf(chars[int(floor(data[i] + 1.0))].c_str()); // print string into console
			
			if (i % width == width - 1)
			{
				printf("\n");
			}
		}
		break;

	case Algorithm::NoNormalization:
		for (int i = 0; i < dataSize; i++)
		{
			printf(chars[int(floor(data[i] / 128.0))].c_str()); // print string into console
			
			if (i % width == width - 1)
			{
				printf("\n");
			}
		}
		break;
	}
}

int NetworkDataSet::Count()
{
	return dataSet.size();
}

void NetworkDataSet::AddData(NetworkData* data)
{
	dataSet.push_back(data);
}

void NetworkDataSet::Destroy()
{
	for (auto data : dataSet)
	{
		free(data->data);
	}
}

NetworkData& NetworkDataSet::operator[](int index)
{
	return *dataSet[index];
}

