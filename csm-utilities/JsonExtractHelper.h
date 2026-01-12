//
// Created by chu on 3/17/25.
//

#ifndef JSONEXTRACTHELPER_H
#define JSONEXTRACTHELPER_H

#include <json/value.h>

#include "csm-utilities/Logger.h"

namespace csm
{

    namespace utilities
    {

        template<typename T>
        class JsonExtractHelper
        {};

        template<>
        class JsonExtractHelper<std::string>
        {
        public:
            static int extract(const Json::Value& root, const std::string& key, std::string& output)
            {
                if (false == root.isMember(key))
                {
                    LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "key ", key, " is not a member of json");
                    return -1;
                }

                if (Json::ValueType::stringValue != root[key].type())
                {
                    LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "key ", key, " is not a string value");
                    return -1;
                }

                output = root[key].asString();

                return 0;
            }
        };

        template<>
        class JsonExtractHelper<Json::Value>
        {
        public:
            static int extract(const Json::Value& root, const std::string& key, Json::Value& output)
            {
                if (false == root.isMember(key))
                {
                    LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "key ", key, " is not a member of json");
                    return -1;
                }

                if (Json::ValueType::objectValue != root[key].type())
                {
                    LOG->write(csm::utilities::LogType::Log_Error, FILE_INFO, "key ", key, " is not a object value");
                    return -1;
                }

                output = root[key];

                return 0;
            }
        };

    }

}


#endif //JSONEXTRACTHELPER_H
