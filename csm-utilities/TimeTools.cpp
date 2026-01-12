//
// Created by chu on 3/30/25.
//

#include "TimeTools.h"

#include <chrono>
#include <fstream>

using namespace csm::utilities;

std::uint64_t TimeTools::getCurrentTimestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

double TimeTools::upTime()
{
#ifdef __linux__
    std::ifstream uptimeFile("/proc/uptime");
    if (false == uptimeFile.is_open())
    {
        return 0;
    }

    double uptimeSeconds{ 0 };
    uptimeFile >> uptimeSeconds;

    uptimeFile.close();

    return uptimeSeconds;
#endif
}
