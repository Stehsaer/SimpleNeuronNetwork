#ifndef _FILE_HELPER_H_
#define _FILE_HELPER_H_

#include <string>
#include <filesystem>

class File
{
public:
	std::filesystem::path filePath;

	File(std::string path);

	bool Exists();
	size_t Size();

	bool ReadAllText(std::string* dst);
	bool ReadAllBytes(unsigned char** dst, size_t* size);
	
	bool WriteAllText(std::string txt);
	bool WriteAllBytes(unsigned char* src, size_t size);
};

#endif