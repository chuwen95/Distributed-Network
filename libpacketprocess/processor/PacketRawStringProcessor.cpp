//
// Created by root on 9/11/23.
//

#include "PacketRawStringProcessor.h"
#include "../packet/PacketRawString.h"
#include "../packet/PacketRawStringReply.h"
#include "libcomponents/Logger.h"

namespace packetprocess
{

    int PacketRawStringProcessor::process(const int fd, PacketBase::Ptr packet, const std::function<int(const int, const std::vector<char>&)>& writeHandler)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "process start");

        PacketRawString::Ptr reqPacket = std::dynamic_pointer_cast<PacketRawString>(packet);
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Debug, FILE_INFO, "req content: ", reqPacket->getContent());

        PacketRawStringReply replyPacket;
        replyPacket.setResult(reqPacket->getContent());
        std::size_t payloadLength = replyPacket.packetLength();

        PacketHeader packetHeader;
        packetHeader.setType(PacketType::PT_RawStringReply);
        packetHeader.setPayloadLength(payloadLength);
        std::size_t headerLength = packetHeader.headerLength();

        std::size_t length = headerLength + payloadLength;

        // 编码包
        std::vector<char> data;
        data.resize(length);
        packetHeader.encode(data.data(), headerLength);
        replyPacket.encode(data.data() + headerLength, length);

        if(nullptr != writeHandler)
        {
            // 将回应包写入发送缓冲区
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                         "try write reply packet to buffer, write size: ", length, ", fd: ", fd);
            int ret{0};
            while(0 != (ret = writeHandler(fd, data)))
            {
                if(-2 == ret)
                {
                    break;
                }
                else if(-1 == ret)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                         " write reply packet to buffer successfully, write size: ", length, ", fd: ", fd);
        }

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO, "process finish");

        return 0;
    }

} // packetprocessor
