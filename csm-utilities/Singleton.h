//
// Created by ChuWen on 9/6/23.
//

#ifndef TCPSERVER_SINGLETON_H
#define TCPSERVER_SINGLETON_H

#include <mutex>

namespace csm
{

    namespace utilities
    {

        template<typename T>
        class Singleton
        {
        private:
            Singleton() = default;
            ~Singleton() = default;

            Singleton(const Singleton &) = delete;
            Singleton &operator=(const Singleton &) = delete;

        public:
            template<typename... Args>
            static T *instance(Args &&... args)
            {
                std::call_once(m_initFlag, [&]() { m_t = new T(std::forward<Args>(args)...); });
                return m_t;
            }

            static void destory()
            {
                std::call_once(m_destoryFlag, [&]() {
                    if (nullptr != m_t)
                    {
                        delete m_t;
                    }
                });
            }

        private:
            static T *m_t;
            static std::once_flag m_initFlag;
            static std::once_flag m_destoryFlag;
        };

        template<typename T>
        T *Singleton<T>::m_t = nullptr;

        template<typename T>
        std::once_flag Singleton<T>::m_initFlag;

        template<typename T>
        std::once_flag Singleton<T>::m_destoryFlag;
    }

}

#endif //TCPSERVER_SINGLETON_H
