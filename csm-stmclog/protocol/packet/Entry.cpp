//
// Created by ChuWen on 2024/6/22.
//

#include "Entry.h"

using namespace csm::stmclog;

std::uint64_t Entry::index() const
{
    return m_rawEntry.index();
}

void Entry::setIndex(const std::uint64_t index)
{
    m_rawEntry.set_index(index);
}

std::uint64_t Entry::term() const
{
    return m_rawEntry.term();
}

void Entry::setTerm(const std::uint64_t term)
{
    m_rawEntry.set_term(term);
}

EntryType Entry::entryType() const
{
    int typeValue = static_cast<int>(m_rawEntry.type());
    return static_cast<EntryType>(typeValue);
}

void Entry::setEntryType(const EntryType &entryType)
{
    int typeValue = static_cast<int>(entryType);
    m_rawEntry.set_type(static_cast<stmclog_proto::RawEntryType>(typeValue));
}

const std::vector<char> &Entry::data()
{
    return std::vector<char>(m_rawEntry.data().begin(), m_rawEntry.data().end();
}

void Entry::setData(const std::vector<char> &data)
{
    m_rawEntry.set_data(data.data(), data.size());
}

std::vector<char> Entry::encode()
{
    std::vector<char> data;
    data.reserve(m_rawEntry.ByteSizeLong());

    m_rawEntry.SerializeToArray(data.data(), m_rawEntry.ByteSizeLong());

    return data;
}

void Entry::decode(const std::vector<char> &data)
{
    m_rawEntry.ParseFromArray(data.data(), data.size());
}

void Entry::setData(const std::string &data)
{
    m_rawEntry.set_data(data);
}

