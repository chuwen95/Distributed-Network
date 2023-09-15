//
// Created by root on 9/11/23.
//

#include "PacketProcessorFactory.h"

#include "PacketRawStringProcessor.h"

namespace packetprocess
{

    PacketProcessorBase::Ptr PacketProcessorFactory::createPacketProcessor(packetprocess::PacketType type)
    {
        switch(type)
        {
            case PacketType::PT_RawString:
                return std::make_shared<PacketRawStringProcessor>();
            default:
                return nullptr;
        }
    }

} // packetprocess