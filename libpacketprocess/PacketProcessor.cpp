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
        return 0;
    }

    int PacketProcessor::uninit()
    {
        return 0;
    }

    int PacketProcessor::start()
    {
        return 0;
    }

    int PacketProcessor::stop()
    {
        return 0;
    }

    int PacketProcessor::process(const PacketType packetType, PacketBase::Ptr packet, PacketReplyBase::Ptr replyPacket)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "processPacket, packet type: ", static_cast<int>(packetType));

        // 创建处理器
        PacketProcessorBase::Ptr processor = PacketProcessorFactory().createPacketProcessor(packetType);
        if (nullptr == processor)
        {
            return -1;
        }
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "create processor successfully, start process packet");
        int ret = processor->process(packet, replyPacket);
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "finish process packet, ret: ", ret);

        return ret;
    }

}
