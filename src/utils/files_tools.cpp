/// \file files_tools.cpp

#include "files_tools.h"

#include <fstream>
#include <filesystem>
#include <set>
#include <sstream>

namespace Multor
{

std::string LoadTextFileRaw(std::string_view path)
{
    std::ifstream file;

    file.exceptions(std::ifstream::failbit);

    try
        {
            file.open(path.data(), std::ios::binary | std::ios::ate);
        }
    catch (const std::exception& exc)
        {
            throw(std::string(exc.what()) + "\n File:" + std::string(path) +
                  " doesn't exist!");
        }

    if (!file.is_open())
        throw("File isn't opened!");

    std::streamsize fileSize = file.tellg();
    file.seekg(0);
    std::string text(static_cast<std::size_t>(fileSize), '\0');
    if (fileSize > 0)
        file.read(text.data(), fileSize);

    return text;
}

namespace
{
std::string TrimLeft(std::string_view text)
{
    const auto first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos)
        return {};
    return std::string(text.substr(first));
}

std::string LoadTextFileWithIncludesImpl(const std::filesystem::path& path,
                                         std::set<std::filesystem::path>& includeStack)
{
    const auto normalizedPath = std::filesystem::weakly_canonical(path);
    if (includeStack.contains(normalizedPath))
        throw std::runtime_error("Recursive shader include detected: " +
                                 normalizedPath.string());

    includeStack.insert(normalizedPath);
    const std::string source = LoadTextFileRaw(normalizedPath.string());
    std::ostringstream out;
    std::istringstream in(source);
    std::string line;

    while (std::getline(in, line))
        {
            const std::string trimmed = TrimLeft(line);
            if (trimmed.rfind("#include \"", 0) == 0)
                {
                    const std::size_t start = trimmed.find('"');
                    const std::size_t end =
                        (start == std::string::npos) ? std::string::npos
                                                     : trimmed.find('"', start + 1);
                    if (start == std::string::npos || end == std::string::npos ||
                        end <= start + 1)
                        throw std::runtime_error("Invalid #include directive in: " +
                                                 normalizedPath.string());

                    const std::filesystem::path includePath =
                        normalizedPath.parent_path() /
                        trimmed.substr(start + 1, end - start - 1);
                    out << LoadTextFileWithIncludesImpl(includePath, includeStack);
                    if (out.tellp() > 0)
                        out << '\n';
                    continue;
                }

            out << line;
            if (!in.eof())
                out << '\n';
        }

    includeStack.erase(normalizedPath);
    return out.str();
}
} // namespace

std::string LoadTextFile(std::string_view path)
{
    std::set<std::filesystem::path> includeStack;
    return LoadTextFileWithIncludesImpl(std::filesystem::path(path), includeStack);
}

} // namespace Multor
