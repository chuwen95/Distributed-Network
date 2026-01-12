#ifndef ELAPSEDTIME_H
#define ELAPSEDTIME_H

#include <chrono>

namespace csm
{
    namespace utilities
    {
        class ElapsedTime
        {
        public:
            using Ptr = std::shared_ptr<ElapsedTime>;

            ElapsedTime() = default;
            ~ElapsedTime() = default;

        public:
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
                return std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - m_begin).count();
            }
#else

            __int64_t getElapsedTimeInMicroSec()
            {
                return std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - m_begin).count();
            }

#endif

        private:
            std::chrono::time_point<std::chrono::high_resolution_clock> m_begin{
                std::chrono::high_resolution_clock::now()
            };
        };
    }
}

#endif // ELAPSEDTIME_H
