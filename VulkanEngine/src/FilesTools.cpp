/// \file FilesTools.cpp
#include "FilesTools.h"

bool FileIsExist(std::string_view filePath)
{
    bool isExist = false;
    std::ifstream fin(filePath.data());

    if (fin.is_open())
        isExist = true;

    fin.close();
    return isExist;
}

std::string LoadTextFile(std::string_view path)
{
    std::ifstream file;

    file.exceptions(std::ifstream::failbit);

    try
    {
        file.open(path.data(), std::ios::binary | std::ios::ate);
    }
    catch (std::exception exc)
    {
        throw(std::string(exc.what()) + "\n File:" + std::string(path) + " doesn't exist!");
    }

    if (!file.is_open())
        throw("File isn't opened!");
    
    std::streamsize fileSize = file.tellg();
    file.seekg(0);
    std::string Text(fileSize + 1, '\0');
    file.read(&Text[0], fileSize);
    return Text;
}