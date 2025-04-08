//
// Created by chu on 3/30/25.
//

#ifndef TIMETOOLS_H
#define TIMETOOLS_H

#include "csm-common/Common.h"

namespace csm
{

    namespace utilities
    {

        class TimeTools
        {
        public:
            using Ptr = std::shared_ptr<TimeTools>;

            TimeTools() = default;
            ~TimeTools() = default;

        public:
            static std::uint64_t getCurrentTimestamp();
            static double upTime();
        };

    }

}

#endif //TIMETOOLS_H
