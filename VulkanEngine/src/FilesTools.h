/// \file FilesTools.h
#include <string_view>
#include <string>
#include <fstream>

bool FileIsExist(std::string_view filePath);
std::string LoadTextFile(std::string_view path);