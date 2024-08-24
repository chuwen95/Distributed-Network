//
// Created by ChuWen on 2024/8/24.
//

#include "PayloadModuleMessage.h"

using namespace csm::service;

PayloadModuleMessage::PayloadModuleMessage()
{
    m_payload = std::make_shared<std::vector<char>>();
}

void PayloadModuleMessage::setPayload(const char *data, const std::size_t len)
{
    m_payload->resize(len);
    memcpy(m_payload->data(), data, len);
}

void PayloadModuleMessage::setPayload(const std::vector<char> &data)
{
    *m_payload = data;
}

std::shared_ptr<std::vector<char>> PayloadModuleMessage::payload()
{
    return m_payload;
}
