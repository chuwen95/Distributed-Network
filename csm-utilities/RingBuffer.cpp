//
// Created by root on 9/5/23.
//

#include "RingBuffer.h"
#include "Logger.h"

using namespace csm::utilities;

constexpr std::size_t c_reservedSpace{2};

RingBuffer::RingBuffer()
{
}

RingBuffer::~RingBuffer()
{
}

int RingBuffer::init(const std::size_t size)
{
    m_size = size;
    m_buffer.resize(m_size);

    return 0;
}

int RingBuffer::uninit()
{
    return 0;
}

int RingBuffer::writeData(const char *data, const std::size_t len)
{
    std::size_t remainSpace = space();
    LOG->write(components::LogType::Log_Trace, FILE_INFO, ", len: ", len, ", remainSpace: ", remainSpace);
    if (len > remainSpace)
    {
        return -1;
    }

    if (m_startOffset > m_endOffset)
    {
        memcpy(m_buffer.data() + m_endOffset, data, len);
    } else
    {
        int spaceAfterEndOffset = m_size - m_endOffset;
        if (spaceAfterEndOffset > len)
        {
            memcpy(m_buffer.data() + m_endOffset, data, len);
        } else
        {
            memcpy(m_buffer.data() + m_endOffset, data, spaceAfterEndOffset);
            memcpy(m_buffer.data(), data + spaceAfterEndOffset, len - spaceAfterEndOffset);
        }
    }

    return 0;
}

int RingBuffer::readData(const std::size_t size, const std::size_t offset, std::vector<char> &data)
{
    int usedDataLength = dataLength();
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "usedDataLength: ", usedDataLength, "size: ", size, ", m_startOffset: ", m_startOffset, ", m_endOffset: ", m_endOffset);
    if (usedDataLength < size || data.size() != size)
    {
        return -1;
    }

    if (m_startOffset < m_endOffset)
    {
        memcpy(data.data(), m_buffer.data() + m_startOffset + offset, size);
    } else
    {
        std::size_t firstCopySize = m_size - (m_startOffset + offset);
        if (size > firstCopySize)    // 如果需要获取的数据大于m_startOffset到缓冲区尾部的数据
        {
            // 首先拷贝从m_startOffset到缓冲区结束的所有数据
            memcpy(data.data(), m_buffer.data() + m_startOffset + offset, firstCopySize);
            // 再从缓冲区头部拷贝剩下长度的数据
            memcpy(data.data() + firstCopySize, m_buffer.data(), size - firstCopySize);
        } else    // 需要获取的数据小于等于m_startOffset到缓冲区尾部的数据
        {
            std::size_t startOffset = (m_startOffset + offset) % m_size;
            memcpy(data.data(), m_buffer.data() + startOffset, size);
        }
    }
    return 0;
}

char *RingBuffer::data()
{
    return m_buffer.data();
}

std::size_t RingBuffer::startOffset()
{
    return m_startOffset;
}

std::size_t RingBuffer::endOffset()
{
    return m_endOffset;
}

std::size_t RingBuffer::size()
{
    return m_size;
}

std::size_t RingBuffer::dataLength()
{
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "m_startOffset: ", m_startOffset, ", m_endOffset", m_endOffset);
    if (m_endOffset >= m_startOffset)
    {
        return m_endOffset - m_startOffset;
    }
    else
    {
        return m_endOffset + (m_size - m_startOffset);
    }
}

std::size_t RingBuffer::space()
{
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "m_startOffset: ", m_startOffset, ", m_endOffset", m_endOffset);
    // 保留2个字节不使用，避免m_startOffset > m_endOffset的情况下写入会导致m_endOffset == m_startOffset
    if (m_startOffset == m_endOffset)
    {
        return m_size - c_reservedSpace;
    } else if (m_startOffset < m_endOffset)
    {
        if ((m_endOffset + c_reservedSpace) % m_size == m_startOffset)
        {

            /**
             * 0 0 0 0 0 0 0 0 0 0
             * s                       e
             */
            /**
             * 0 0 0 0 0 0 0 0 0 0
             *     s                      e
             */
            return 0;
        }
        return (m_size - m_endOffset) + m_startOffset - c_reservedSpace;
    } else
    {
        /**
         * 0 0 0 0 0 0 0 0 0 0
         *        e            s
         */
        return m_startOffset - m_endOffset - c_reservedSpace;
    }
}

void RingBuffer::clear()
{
    m_startOffset = 0;
    m_endOffset = 0;
}

int RingBuffer::getBufferAndLengthForWrite(char *&buffer, std::size_t &length)
{
    std::size_t remainSpace = space();
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "remainSpace: ", remainSpace, ", length: ", length, ", m_startOffset: ", m_startOffset, ", m_endOffset: ", m_endOffset);
    if (0 == remainSpace)
    {
        return -1;
    }

    buffer = m_buffer.data() + m_endOffset;
    if (m_startOffset <= m_endOffset)
    {
        if (m_startOffset >= c_reservedSpace)
        {
            length = m_size - m_endOffset;
        } else
        {
            length = m_size - (c_reservedSpace - m_startOffset) - m_endOffset;
        }
    } else
    {
        length = m_startOffset - m_endOffset - c_reservedSpace;
    }

    return 0;
}

int RingBuffer::increaseUsedSpace(const std::size_t size)
{
    if (space() < size)
    {
        return -1;
    }

    m_endOffset += size;
    m_endOffset %= m_size;

    return 0;
}

int RingBuffer::getBufferForRead(const std::size_t size, char *&buffer)
{
    std::size_t usedDataLength = dataLength();
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "usedDataLength: ", usedDataLength, "size: ", size, ", m_startOffset: ", m_startOffset, ", m_endOffset: ", m_endOffset);
    if (dataLength() < size)
    {
        return -1;
    }

    if (m_startOffset < m_endOffset)
    {
        buffer = m_buffer.data() + m_startOffset;
    } else
    {
        if (m_size - m_startOffset < size)
        {
            return -2;
        }
        buffer = m_buffer.data() + m_startOffset;
    }
    return 0;
}

int RingBuffer::decreaseUsedSpace(const std::size_t size)
{
    std::size_t usedDataLength = dataLength();
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "usedDataLength: ", usedDataLength, ", size", size, ", m_startOffset: ", m_startOffset, ", m_endOffset: ", m_endOffset);
    if (usedDataLength < size)
    {
        return -1;
    }
    else if (dataLength() == size)
    {
        m_startOffset = 0;
        m_endOffset = 0;
    }
    else
    {
        m_startOffset += size;
        m_startOffset %= m_size;

        if (m_startOffset == m_endOffset)
        {
            m_startOffset = 0;
            m_endOffset = 0;
        }
    }
    usedDataLength = dataLength();
    LOG->write(components::LogType::Log_Trace, FILE_INFO, "usedDataLength: ", usedDataLength);

    return 0;
}

int RingBuffer::getContinuousData(char *&buffer, std::size_t &size)
{
    std::size_t usedDataLength = dataLength();
    LOG->write(components::LogType::Log_Trace, FILE_INFO,
                  "usedDataLength: ", usedDataLength, ", size", size, ", m_startOffset: ", m_startOffset, ", m_endOffset: ", m_endOffset);
    if (0 == usedDataLength)
    {
        return -1;
    }

    if (m_startOffset < m_endOffset)
    {
        buffer = m_buffer.data() + m_startOffset;
        size = m_endOffset - m_startOffset;
    } else
    {
        buffer = m_buffer.data() + m_startOffset;
        if (m_endOffset <= c_reservedSpace)
        {
            size = m_size - (c_reservedSpace - m_endOffset) - m_startOffset;
        } else
        {
            size = m_size - m_startOffset;
        }
    }

    return 0;
}