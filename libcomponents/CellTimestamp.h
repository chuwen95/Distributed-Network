#ifndef CellTimestamp_hpp
#define CellTimestamp_hpp

#include "libcommon/Common.h"

namespace components
{

    class CellTimestamp
    {
    public:
        using Ptr = std::shared_ptr<CellTimestamp>;

        CellTimestamp() = default;
        ~CellTimestamp() = default;

        void update()
        {
            m_begin = std::chrono::high_resolution_clock::now();
        }

        //获取秒
        double getElapsedTimeInSec()
        {
            return getElapsedTimeInMicroSec() * 0.000001;
        }

        //获取毫秒
        double getElapsedTimeInMilliSec()
        {
            return getElapsedTimeInMicroSec() * 0.001;
        }

        //获取微秒
#ifdef _WIN32
        __int64 getElapsedTimeInMicroSec()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
        }
#else

        __int64_t getElapsedTimeInMicroSec()
        {
            return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - m_begin).count();
        }

#endif

        static std::uint64_t getCurrentTimestamp()
        {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_begin{std::chrono::high_resolution_clock::now()};
    };

}

#endif // CellTimestamp_hpp
