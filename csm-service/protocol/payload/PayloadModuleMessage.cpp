//
// Created by ChuWen on 2024/8/24.
//

#include "PayloadModuleMessage.h"

#include <algorithm>

using namespace csm::service;

std::size_t PayloadModuleMessage::packetLength() const
{
    return 0;
}

int PayloadModuleMessage::encode( char *data, const std::size_t len) const
{
    return 0;
}

int PayloadModuleMessage::decode(const char *buffer, const std::size_t length)
{
    m_payload.resize(length);
    std::copy_n(buffer, length, m_payload.begin());

    return 0;
}

const std::vector<char>& PayloadModuleMessage::payload() const
{
    return m_payload;
}
