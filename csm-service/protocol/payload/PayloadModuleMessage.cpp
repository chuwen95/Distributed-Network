//
// Created by ChuWen on 2024/8/24.
//

#include "PayloadModuleMessage.h"

using namespace csm::service;

PayloadModuleMessage::PayloadModuleMessage()
{
    m_payload = std::make_shared<std::vector<char>>();
}

std::size_t PayloadModuleMessage::packetLength() const
{
    return 0;
}


int PayloadModuleMessage::encode( char *data, const std::size_t len) const
{
    return -1;
}

int PayloadModuleMessage::decode(const char *buffer, const std::size_t length)
{
    m_payload->resize(length);
    memcpy(m_payload->data(), buffer, length);

    return 0;
}


std::shared_ptr<std::vector<char>> PayloadModuleMessage::payload()
{
    return m_payload;
}
