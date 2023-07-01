/// \file FilesTools.h
/// \brief Few utils for working with files

#pragma once
#ifndef FILESTOOLS_H
#define FILESTOOLS_H

#include <string_view>
#include <string>
#include <fstream>

namespace Multor
{
/// \brief Read text file
/// \param[in] path path to file
std::string LoadTextFile(std::string_view path);

} // namespace Multor

#endif // FILESTOOLS_H