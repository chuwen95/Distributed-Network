//
// Created by root on 9/20/23.
//

#ifndef TCPNETWORK_STRINGTOOL_H
#define TCPNETWORK_STRINGTOOL_H

#include "libcommon/Common.h"

namespace components
{
    namespace string_tools
    {
        template<typename T>
        T convertFromString(const std::string& value)
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return value;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return "true" == value ? true : false;
            }
            else
            {
                T convertedValue;

                std::istringstream iss(value);
                iss >> convertedValue;
                return convertedValue;
            }
        }

#if 0
        template<typename T>
        static T convertFromString(const std::string& value)
        {
            T convertedValue;

            std::istringstream iss(value);
            iss >> convertedValue;
            return convertedValue;
        }

        template<>
        static std::string convertFromString<std::string>(const std::string& value)
        {
            return value;
        }
#endif

        template<typename T>
        std::string convertToString(const T& value)
        {
            if constexpr (std::is_convertible_v<T, std::string>)
            {
                return std::to_string(value);
            }
            else if constexpr (std::is_same_v<T, unsigned short>)
            {
                return std::to_string(value);
            }
            else
            {
                return value;
            }
        }

        std::vector<std::string> split(const std::string& origin, const char delimiter);
    }
}

#endif //TCPNETWORK_STRINGTOOL_H
