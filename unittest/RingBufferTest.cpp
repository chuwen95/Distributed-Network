//
// Created by root on 9/15/23.
//

#include "libutilities/RingBuffer.h"
#include "libpacketprocess/packet/PacketHeader.h"
#include "libpacketprocess/packet/PacketRawString.h"

int putOnePacketTest()
{
    // 创建大小32K的环形缓冲区
    utilities::RingBuffer ringBuffer;
    ringBuffer.init(32 * 1024);

    // 请求包
    packetprocess::PacketHeader packetHeader;
    packetHeader.setType(packetprocess::PacketType::PT_RawString);

    packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    int rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 放一个包进去，然后取出
    if (ringBuffer.space() < rawStringLength)
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " buffer space is not enough" << std::endl;
        return -1;
    }

    char *buffer{nullptr};
    std::size_t len{0};
    if (-1 == ringBuffer.getBufferAndLengthForWrite(buffer, len))
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferAndLengthForWrite, failed"
                  << std::endl;
        return -1;
    }
    memcpy(buffer, rawStringBuffer.data(), rawStringLength);
    if (-1 == ringBuffer.increaseUsedSpace(rawStringLength))
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace, failed" << std::endl;
        return -1;
    }

    if (ringBuffer.dataLength() != rawStringLength)
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " ring buffer used space error, use space: " << ringBuffer.dataLength() << std::endl;
        return -1;
    }

    if (-1 == ringBuffer.getBufferForRead(headerLength, buffer))
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead, size: " << headerLength << std::endl;
        return -1;
    }

    packetHeader.decode(buffer, headerLength);
    if (false == packetHeader.isMagicMatch())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " magic not match: " << std::endl;
        return -1;
    }

    std::vector<char> payloadBuffer;
    payloadBuffer.resize(packetHeader.payloadLength());
    if (-1 == ringBuffer.readData(packetHeader.payloadLength(), headerLength, payloadBuffer))
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " readData failed, len: " << packetHeader.payloadLength() << std::endl;
        return -1;
    }
    ringBuffer.decreaseUsedSpace(headerLength + packetHeader.payloadLength());

    if (0 != ringBuffer.dataLength())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " used space error, value: " << ringBuffer.dataLength() << std::endl;
        return -1;
    }
    rawStringPacket.decode(payloadBuffer.data(), payloadLength);

    if (rawStringPacket.getContent() != "hello")
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " packet content error, value: " << rawStringPacket.getContent() << std::endl;
        return -1;
    }

    return 0;
}

// 将缓冲区放满
int fullBufferTest()
{
    // 创建大小32K的环形缓冲区
    utilities::RingBuffer ringBuffer;
    ringBuffer.init(32 * 1024);

    // 请求包
    packetprocess::PacketHeader packetHeader;
    packetHeader.setType(packetprocess::PacketType::PT_RawString);

    packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    int rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 将缓冲区放满
    int lastPacketCopyLength{0};
    while(0 != ringBuffer.space())
    {
        char* buffer{nullptr};
        std::size_t len{0};
        if(-1 == ringBuffer.getBufferAndLengthForWrite(buffer, len))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " no space left" << std::endl;
            break;
        }

        if(len >= rawStringLength)
        {
            memcpy(buffer, rawStringBuffer.data(), rawStringLength);
            if(-1 == ringBuffer.increaseUsedSpace(rawStringLength))
            {
                std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace error" << std::endl;
                break;
            }
        }
        else
        {
            memcpy(buffer, rawStringBuffer.data(), len);
            if(-1 == ringBuffer.increaseUsedSpace(len))
            {
                std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace error" << std::endl;
                break;
            }
            lastPacketCopyLength = len;
        }
    }

    if(0 != ringBuffer.space())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    // 消耗包
    while(true)
    {
        if(ringBuffer.dataLength() < headerLength)
        {
            // std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        else if(-1 == ret)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead error" << std::endl;
            return -1;
        }

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        if(-1 == (ret= ringBuffer.readData(payloadLength, headerLength, payloadBuffer)))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " readData error" << std::endl;
            return -1;
        }

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);
    }

    if(lastPacketCopyLength != ringBuffer.dataLength())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    // 将之前未放完的包放进去
    int packetSum{0};
    if(0 != lastPacketCopyLength)
    {
        if(-1 == ringBuffer.writeData(rawStringBuffer.data() + lastPacketCopyLength, rawStringLength - lastPacketCopyLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferAndLengthForWrite error" << std::endl;
            return -1;
        }
        ringBuffer.increaseUsedSpace(rawStringLength - lastPacketCopyLength);

        ++packetSum;
    }

    if(rawStringLength != ringBuffer.dataLength())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    // 再放几个完整的包
    for(int i = 0; i < 50; ++i)
    {
        char* buffer{nullptr};
        std::size_t len{0};
        if(-1 == ringBuffer.getBufferAndLengthForWrite(buffer, len))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " no space left" << std::endl;
            return -1;
        }

        memcpy(buffer, rawStringBuffer.data(), rawStringLength);
        if(-1 == ringBuffer.increaseUsedSpace(rawStringLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace error" << std::endl;
            return -1;
        }

        ++packetSum;
    }

    if(packetSum * rawStringLength != ringBuffer.dataLength())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    // 消耗包
    int packetNum{0};
    while(0 != ringBuffer.dataLength())
    {
        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        else if(-1 == ret)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead error, data length: " << ringBuffer.dataLength() << std::endl;
            return -1;
        }

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        if(-1 == (ret= ringBuffer.readData(payloadLength, headerLength, payloadBuffer)))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " readData error" << std::endl;
            return -1;
        }

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        ++packetNum;
    }

    if(packetNum != packetSum || 0 != ringBuffer.dataLength() || ringBuffer.space() != 32 * 1024 - 2)
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    return 0;
}

int fullBufferUseWriteDataTest()
{
    // 创建大小32K的环形缓冲区
    utilities::RingBuffer ringBuffer;
    ringBuffer.init(32 * 1024);

    // 请求包
    packetprocess::PacketHeader packetHeader;
    packetHeader.setType(packetprocess::PacketType::PT_RawString);

    packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    int rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 添加包，直到缓冲区放不下为止
    int packetSize = 32 * 1024 / rawStringLength;
    for(int i = 0; i < packetSize; ++i)
    {
        if(-1 == ringBuffer.writeData(rawStringBuffer.data(), rawStringLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " no space left" << std::endl;
            return -1;
        }

        if(-1 == ringBuffer.increaseUsedSpace(rawStringLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace error" << std::endl;
            return -1;
        }
    }

    // 消耗包
    for(int i = 0; i < packetSize - 1; ++i)
    {
        if(ringBuffer.dataLength() < headerLength)
        {
            // std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        else if(-1 == ret)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead error" << std::endl;
            return -1;
        }

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        if(-1 == (ret= ringBuffer.readData(payloadLength, headerLength, payloadBuffer)))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " readData error" << std::endl;
            return -1;
        }

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        packetprocess::PacketRawString rawStringPacket;
        rawStringPacket.decode(payloadBuffer.data(), payloadLength);
        if(rawStringPacket.getContent() != "hello")
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " packet error" << std::endl;
            return -1;
        }
    }

    // 再放50包
    for(int i = 0; i < 50; ++i)
    {
        if(-1 == ringBuffer.writeData(rawStringBuffer.data(), rawStringLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " no space left" << std::endl;
            return -1;
        }

        if(-1 == ringBuffer.increaseUsedSpace(rawStringLength))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " increaseUsedSpace error" << std::endl;
            return -1;
        }
    }

    if(51 * rawStringLength != ringBuffer.dataLength())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    std::size_t startOffset =  ringBuffer.startOffset();
    for(std::size_t i = 0; i < 51; ++i)
    {
        for(std::size_t j = 0; j < rawStringLength; ++j)
        {
            int offset = (startOffset + i * rawStringLength + j) % (32 * 1024);
            if(*(ringBuffer.data() + offset) != rawStringBuffer[j])
            {
                std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " packet error, offset: " << offset << ", j: " << j << std::endl;
                return -1;
            }
        }
    }

    // 消耗包
    for(std::size_t i = 0; i < 51; ++i)
    {
        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        int ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            //std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead ret -2, packet num: " << i << std::endl;
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        else if(-1 == ret)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead error, data length: " << ringBuffer.dataLength() << std::endl;
            return -1;
        }

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available packet pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        if(-1 == (ret= ringBuffer.readData(payloadLength, headerLength, payloadBuffer)))
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " readData error" << std::endl;
            return -1;
        }

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        packetprocess::PacketRawString rawStringPacket;
        rawStringPacket.decode(payloadBuffer.data(), payloadLength);
        if(rawStringPacket.getContent() != "hello")
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " packet error, i: " << i << std::endl;
            return -1;
        }
    }

    if(0 != ringBuffer.dataLength() || ringBuffer.space() != 32 * 1024 - 2)
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " error" << std::endl;
        return -1;
    }

    return 0;
}

int main(int argc, char* argv[])
{
    if(-1 == putOnePacketTest())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << "putOnePacketTest failed" << std::endl;
        return -1;
    }
    std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " putOnePacketTest successfully" << std::endl;

    if(-1 == fullBufferTest())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " fullBufferTest failed" << std::endl;
        return -1;
    }
    std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " fullBufferTest successfully" << std::endl;

    if(-1 == fullBufferUseWriteDataTest())
    {
        std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " fullBufferUseWriteDataTest failed" << std::endl;
        return -1;
    }
    std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " fullBufferUseWriteDataTest successfully" << std::endl;

    std::cout << "no error" << std::endl;

    return 0;
}