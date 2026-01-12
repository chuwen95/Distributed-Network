//
// Created by ChuWen on 2024/3/23.
//

#ifndef COPYSTATEMACHINE_FILESYSTEMTOOLS_H
#define COPYSTATEMACHINE_FILESYSTEMTOOLS_H

#include <string>
#include <filesystem>

namespace csm
{

    namespace utilities
    {

        class FileSystemTools
        {
        public:
            FileSystemTools() = default;
            ~FileSystemTools() = default;

        public:
            /**
             * @brief 创建文件夹，如果文件夹存在，则直接返回成功
             *
             * @param path      [in] 文件夹路径
             * @return               失败返回-1，成功返回0
             */
            int createFolder(const std::string& path);

            /**
             * @brief 根据文件名创建文件夹，即创建文件所在的文件夹
             *             比如：/data/tmp/tmp.txt，会创建/data/tmp文件夹
             *
             * @param path          [in] 文件路径
             * @return                   失败返回-1，成功返回0
             */
            int createFolderByFilename(const std::string& filename);
        };

    }

}

#endif //COPYSTATEMACHINE_FILESYSTEMTOOLS_H
