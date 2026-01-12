//
// Created by chu on 3/30/25.
//

#ifndef TIMETOOLS_H
#define TIMETOOLS_H

#include <cstdint>

namespace csm
{

    namespace utilities
    {

        class TimeTools
        {
        public:
            TimeTools() = default;
            ~TimeTools() = default;

        public:
            static std::uint64_t getCurrentTimestamp();
            static double upTime();
        };

    }

}

#endif //TIMETOOLS_H
