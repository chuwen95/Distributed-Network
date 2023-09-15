//
// Created by root on 9/9/23.
//

#ifndef TCPNETWORK_PACKETPROCESSOR_H
#define TCPNETWORK_PACKETPROCESSOR_H

#include "libcommon/Common.h"
#include "libcomponents/ThreadPool.h"
#include "packet/PacketBase.h"

namespace packetprocess
{

    class PacketProcessor
    {
    public:
        PacketProcessor();
        ~PacketProcessor();

    public:
        int init();

        int uninit();

        int start();

        int stop();

        int processData(const int id, const packetprocess::PacketType packetType,
                        std::shared_ptr<std::vector<char>>& payloadData, const std::function<int(const int, const std::vector<char>&)>& writeHandler);

    private:
        components::ThreadPool m_threadPool;
    };

}


#endif //TCPNETWORK_PACKETPROCESSOR_H
