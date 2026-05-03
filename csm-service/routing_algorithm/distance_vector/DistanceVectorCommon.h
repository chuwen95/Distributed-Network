//
// Created by ivy on 4/20/26.
//

#ifndef COPYSTATEMACHINE_DISTANCEVECTORCOMMON_H
#define COPYSTATEMACHINE_DISTANCEVECTORCOMMON_H

#include <cstdint>

namespace csm
{

    namespace service
    {

        using Distance = std::uint32_t;
        constexpr Distance c_unreachableDistance{750};

    }

}

#endif //COPYSTATEMACHINE_DISTANCEVECTORCOMMON_H