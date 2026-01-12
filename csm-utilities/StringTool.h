//
// Created by ChuWen on 9/20/23.
//

#ifndef TCPNETWORK_STRINGTOOL_H
#define TCPNETWORK_STRINGTOOL_H

#include <vector>
#include <string>
#include <sstream>

namespace csm
{

    namespace utilities
    {

        template<typename T>
        T convertFromString(const std::string &value)
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
        concept StringConvertible = requires(T a) {
            { std::to_string(a) } -> std::convertible_to<std::string>;
        };

        template<typename T>
        concept IsStringClassOrCharString =
                std::is_same_v<std::remove_cvref_t<T>, std::string> || std::is_convertible_v<std::remove_cvref_t<T>, std::string>;

        template<typename T>
        concept isEnumClass = std::is_enum_v<T> && false == std::is_convertible_v<T, int>;

        template<typename T>
        concept ToStringAble = StringConvertible<T> ||
                IsStringClassOrCharString<T> ||
                isEnumClass<T>;

        template<ToStringAble T>
        std::string convertToString(const T &value)
        {
            if constexpr (StringConvertible<T>)
            {
                return std::to_string(value);
            }
            else if constexpr (isEnumClass<T>)
            {
                return std::to_string(static_cast<int>(value));
            }
            else
            {
                return value;
            }
        }

        std::vector<std::string> split(const std::string &origin, const char delimiter);

    }

}

#endif //TCPNETWORK_STRINGTOOL_H
