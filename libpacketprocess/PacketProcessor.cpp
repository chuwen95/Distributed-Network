//
// Created by root on 9/9/23.
//

#include "PacketProcessor.h"
#include "PacketFactory.h"
#include "processor/PacketProcessorFactory.h"
#include "libcomponents/Logger.h"

namespace packetprocess
{

    PacketProcessor::PacketProcessor()
    {}

    PacketProcessor::~PacketProcessor()
    {}

    int PacketProcessor::init()
    {
        //m_threadPool.init(std::thread::hardware_concurrency(), "pack_proc");
        m_threadPool.init(8, "pack_proc");

        return 0;
    }

    int PacketProcessor::uninit()
    {
        return 0;
    }

    int PacketProcessor::start()
    {
        m_threadPool.start();

        return 0;
    }

    int PacketProcessor::stop()
    {
        m_threadPool.stop();

        return 0;
    }

    int PacketProcessor::processData(const int fd, const packetprocess::PacketType packetType,
                                     std::shared_ptr<std::vector<char>>& payloadData, const std::function<int(const int, const std::vector<char>&)>& writeHandler)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "create new task, packet type: ", static_cast<int>(packetType));
        const auto expression = [fd, packetType, payloadData, writeHandler]()
        {
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                         "task start, packet type: ", static_cast<int>(packetType), ", fd: ", fd);
            // 解码包
            PacketBase::Ptr packet = PacketFactory().createPacket(packetType, payloadData);
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "create packet successfully");

            // 创建处理器
            PacketProcessorBase::Ptr processor = PacketProcessorFactory().createPacketProcessor(packetType);
            if(nullptr == processor)
            {
                return ;
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "create processor successfully");
            processor->process(fd, packet, writeHandler);
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "task finish, fd: ", fd);
        };
        m_threadPool.push(expression);
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     " push new task to thread pool successfully, packet type: ", static_cast<int>(packetType), ", fd: ", fd);

        return 0;
    }

}
