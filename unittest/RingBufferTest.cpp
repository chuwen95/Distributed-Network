//
// Created by ChuWen on 9/15/23.
//

#include <gtest/gtest.h>

#include "csm-utilities/RingBuffer.h"
#include "csm-service/protocol/header/PacketHeader.h"
#include "csm-packetprocess/packet/PacketRawString.h"

constexpr std::size_t c_bufferSize{32 * 1024};

TEST(RingBufferTest, PutOnePacketTest)
{
    // 创建大小32K的环形缓冲区
    csm::utilities::RingBuffer ringBuffer(c_bufferSize);
    int ret = ringBuffer.init();
    EXPECT_NE(-1, ret);

    // 请求包
    csm::service::PacketHeader packetHeader;
    packetHeader.setType(csm::service::PacketType::PT_ModuleMessage);

    csm::packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    std::size_t rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 放一个包进去，然后取出
    EXPECT_GT (ringBuffer.space(), rawStringLength);

    char *buffer{nullptr};
    std::size_t len{0};
    ret = ringBuffer.getBufferAndLengthForWrite(buffer, len);
    EXPECT_NE(-1, ret);

    memcpy(buffer, rawStringBuffer.data(), rawStringLength);
    ret = ringBuffer.increaseUsedSpace(rawStringLength);
    EXPECT_NE (-1, ret);

    EXPECT_EQ (ringBuffer.dataLength(), rawStringLength);

    ret = ringBuffer.getBufferForRead(headerLength, buffer);
    EXPECT_NE (-1, ret);

    packetHeader.decode(buffer, headerLength);
    EXPECT_TRUE(packetHeader.isMagicMatch());

    std::vector<char> payloadBuffer;
    payloadBuffer.resize(packetHeader.payloadLength());
    ret = ringBuffer.readData(packetHeader.payloadLength(), headerLength, payloadBuffer);
    EXPECT_NE (-1, ret);

    ringBuffer.decreaseUsedSpace(headerLength + packetHeader.payloadLength());
    EXPECT_EQ (0, ringBuffer.dataLength());

    rawStringPacket.decode(payloadBuffer.data(), payloadLength);
    EXPECT_EQ(rawStringPacket.getContent(), "hello");
}

// 将缓冲区放满
TEST(RingBufferTest, fullBufferTest)
{
    // 创建大小32K的环形缓冲区
    csm::utilities::RingBuffer ringBuffer(c_bufferSize);
    int ret = ringBuffer.init();
    EXPECT_NE(-1, ret);

    // 请求包
    csm::service::PacketHeader packetHeader;
    packetHeader.setType(csm::service::PacketType::PT_ModuleMessage);

    csm::packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    std::size_t rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 将缓冲区放满
    std::size_t lastPacketCopyLength{0};
    while(0 != ringBuffer.space())
    {
        char* buffer{nullptr};
        std::size_t len{0};
        ret = ringBuffer.getBufferAndLengthForWrite(buffer, len);
        EXPECT_NE(-1, ret);

        if(len >= rawStringLength)
        {
            memcpy(buffer, rawStringBuffer.data(), rawStringLength);
            ret = ringBuffer.increaseUsedSpace(rawStringLength);
            EXPECT_NE(-1, ret);
        }
        else
        {
            memcpy(buffer, rawStringBuffer.data(), len);
            ret = ringBuffer.increaseUsedSpace(len);
            EXPECT_NE(-1, ret);

            lastPacketCopyLength = len;
        }
    }

    EXPECT_EQ(0, ringBuffer.space());

    // 消耗包
    while(true)
    {
        if(ringBuffer.dataLength() < headerLength)
        {
            // std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        EXPECT_NE(-1, ret);

        packetHeader.decode(buffer, headerLength);
        EXPECT_TRUE(packetHeader.isMagicMatch());

        // 尝试复制包数据
        std::size_t payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        ret = ringBuffer.readData(payloadLength, headerLength, payloadBuffer);
        EXPECT_NE(-1, ret);

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);
    }

    EXPECT_EQ(lastPacketCopyLength, ringBuffer.dataLength());

    // 将之前未放完的包放进去
    int packetSum{0};
    if(0 != lastPacketCopyLength)
    {
        ret = ringBuffer.writeData(rawStringBuffer.data() + lastPacketCopyLength, rawStringLength - lastPacketCopyLength);
        EXPECT_NE(-1, ret);

        ringBuffer.increaseUsedSpace(rawStringLength - lastPacketCopyLength);
        ++packetSum;
    }

    EXPECT_EQ(rawStringLength, ringBuffer.dataLength());

    // 再放几个完整的包
    for(int i = 0; i < 50; ++i)
    {
        char* buffer{nullptr};
        std::size_t len{0};
        ret = ringBuffer.getBufferAndLengthForWrite(buffer, len);
        EXPECT_NE(-1, ret);

        memcpy(buffer, rawStringBuffer.data(), rawStringLength);
        ret = ringBuffer.increaseUsedSpace(rawStringLength);
        EXPECT_NE(-1, ret);

        ++packetSum;
    }

    EXPECT_EQ(packetSum * rawStringLength, ringBuffer.dataLength());

    // 消耗包
    int packetNum{0};
    while(0 != ringBuffer.dataLength())
    {
        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        EXPECT_NE(-1, ret);

        packetHeader.decode(buffer, headerLength);
        EXPECT_TRUE(packetHeader.isMagicMatch());

        // 尝试复制包数据
        payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        ret = ringBuffer.readData(payloadLength, headerLength, payloadBuffer);
        EXPECT_NE(-1, ret);

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        ++packetNum;
    }

    EXPECT_EQ(packetNum, packetSum);
    EXPECT_EQ(0, ringBuffer.dataLength());
    EXPECT_EQ(ringBuffer.space(), c_bufferSize - 2);
}

TEST(RingBufferTest, FullBufferUseWriteDataTest)
{
    // 创建大小32K的环形缓冲区
    csm::utilities::RingBuffer ringBuffer(c_bufferSize);
    int ret = ringBuffer.init();

    // 请求包
    csm::service::PacketHeader packetHeader;
    packetHeader.setType(csm::service::PacketType::PT_ModuleMessage);

    csm::packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");

    std::size_t headerLength = packetHeader.headerLength();
    std::size_t payloadLength = rawStringPacket.packetLength();
    std::size_t rawStringLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 添加包，直到缓冲区放不下为止
    std::size_t packetSize = c_bufferSize / rawStringLength;
    for(int i = 0; i < packetSize; ++i)
    {
        ret = ringBuffer.writeData(rawStringBuffer.data(), rawStringLength);
        EXPECT_NE(-1, ret);

        ret = ringBuffer.increaseUsedSpace(rawStringLength);
        EXPECT_NE(-1, ret);
    }

    // 消耗包
    for(int i = 0; i < packetSize - 1; ++i)
    {
        if(ringBuffer.dataLength() < headerLength)
        {
            // std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        EXPECT_NE(-1, ret);

        packetHeader.decode(buffer, headerLength);
        EXPECT_TRUE(packetHeader.isMagicMatch());

        // 尝试复制包数据
        payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        ret = ringBuffer.readData(payloadLength, headerLength, payloadBuffer);
        EXPECT_NE(-1, ret);

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        csm::packetprocess::PacketRawString rawStringPacket;
        rawStringPacket.decode(payloadBuffer.data(), payloadLength);
        EXPECT_EQ(rawStringPacket.getContent(), "hello");
    }

    // 再放50包
    for(int i = 0; i < 50; ++i)
    {
        ret = ringBuffer.writeData(rawStringBuffer.data(), rawStringLength);
        EXPECT_NE(-1, ret);

        ret = ringBuffer.increaseUsedSpace(rawStringLength);
        EXPECT_NE(-1, ret);
    }

    EXPECT_EQ(51 * rawStringLength, ringBuffer.dataLength());

    std::size_t startOffset =  ringBuffer.startOffset();
    for(std::size_t i = 0; i < 51; ++i)
    {
        for(std::size_t j = 0; j < rawStringLength; ++j)
        {
            std::size_t offset = (startOffset + i * rawStringLength + j) % (c_bufferSize);
            EXPECT_EQ(*(ringBuffer.data() + offset), rawStringBuffer[j]);
        }
    }

    // 消耗包
    for(std::size_t i = 0; i < 51; ++i)
    {
        char *buffer{nullptr};
        std::vector<char> bufferForBackspaceLessThanHeaderLength;

        ret = ringBuffer.getBufferForRead(headerLength, buffer);
        if(-2 == ret)
        {
            //std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " getBufferForRead ret -2, payload num: " << i << std::endl;
            bufferForBackspaceLessThanHeaderLength.resize(headerLength);
            ringBuffer.readData(headerLength, 0, bufferForBackspaceLessThanHeaderLength);
            buffer = bufferForBackspaceLessThanHeaderLength.data();
        }
        EXPECT_NE(-1, ret);

        packetHeader.decode(buffer, headerLength);
        assert(true == packetHeader.isMagicMatch());

        // 尝试复制包数据
        payloadLength = packetHeader.payloadLength();
        if(ringBuffer.dataLength() - headerLength < payloadLength)
        {
            std::cerr << __FILE__ << " " << __FUNCTION__ << " " << __LINE__ << " available payload pop finish" << std::endl;
            break;
        }

        std::vector<char> payloadBuffer;
        payloadBuffer.resize(payloadLength);
        ret = ringBuffer.readData(payloadLength, headerLength, payloadBuffer);
        EXPECT_NE(-1, ret);

        ringBuffer.decreaseUsedSpace(headerLength + payloadLength);

        csm::packetprocess::PacketRawString rawStringPacket;
        rawStringPacket.decode(payloadBuffer.data(), payloadLength);
        EXPECT_EQ(rawStringPacket.getContent(), "hello");
    }

    EXPECT_EQ(0, ringBuffer.dataLength());
    EXPECT_EQ(ringBuffer.space(), c_bufferSize - 2);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}