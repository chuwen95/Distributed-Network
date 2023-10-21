//
// Created by root on 9/11/23.
//

#include "PacketRawStringProcessor.h"
#include "../packet/PacketRawString.h"
#include "../packet/PacketRawStringReply.h"
#include "libcomponents/Logger.h"

namespace packetprocess
{

    int PacketRawStringProcessor::process(PacketBase::Ptr packet, PacketReplyBase::Ptr replyPacket)
    {
        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "PacketRawStringProcessor, process start");

        PacketRawString::Ptr reqRawStringPacket = std::dynamic_pointer_cast<PacketRawString>(packet);
        PacketRawStringReply::Ptr replyRawStringPacket = std::dynamic_pointer_cast<PacketRawStringReply>(replyPacket);

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Info, FILE_INFO,
                                                                     "req content: ", reqRawStringPacket->getContent());

        replyRawStringPacket->setResult(reqRawStringPacket->getContent());

        components::Singleton<components::Logger>::instance()->write(components::LogType::Log_Trace, FILE_INFO,
                                                                     "PacketRawStringProcessor, process finish");

        return 0;
    }

} // packetprocessor
