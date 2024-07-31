//
// Created by ChuWen on 9/27/23.
//

#include "StringTool.h"

namespace csm
{

    namespace utilities
    {

        std::vector<std::string> split(const std::string &origin, const char delimiter)
        {
            std::stringstream ss(origin);

            std::vector<std::string> items;
            std::string item;
            while (std::getline(ss, item, delimiter))
            {
                items.emplace_back(item);
            }

            return std::move(items);
        }

    }

}