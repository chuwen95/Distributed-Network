//
// Created by ChuWen on 2024/3/23.
//

#include "FileSystemTools.h"

using namespace csm::utilities;

int FileSystemtool::createFolder(const std::string& path)
{
    if(false == std::filesystem::exists(path))
    {
        return true == std::filesystem::create_directories(path) ? 0 : -1;
    }
    else
    {
        return 0;
    }
}

int FileSystemtool::createFolderByFilename(const std::string &filename)
{
    std::string fileDir = std::filesystem::path(filename).parent_path();

    return createFolder(fileDir);
}