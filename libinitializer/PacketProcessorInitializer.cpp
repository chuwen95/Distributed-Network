//
// Created by root on 10/11/23.
//

#include "PacketProcessorInitializer.h"

namespace initializer
{

    PacketProcessorInitializer::PacketProcessorInitializer()
    {
        m_packetProcessor = std::make_shared<packetprocess::PacketProcessor>();
    }

    int PacketProcessorInitializer::init()
    {
        return m_packetProcessor->init();
    }

    int PacketProcessorInitializer::uninit()
    {
        return m_packetProcessor->uninit();
    }

    packetprocess::PacketProcessor::Ptr PacketProcessorInitializer::packetProcessor()
    {
        return m_packetProcessor;
    }

} // initializer