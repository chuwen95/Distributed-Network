//
// Created by root on 9/12/23.
//

#include "libcommon/Common.h"
#include "libcomponents/Socket.h"
#include "libcomponents/CellTimestamp.h"
#include "libpacketprocess/packet/PacketRawString.h"
#include "libpacketprocess/packet/PacketRawStringReply.h"
#include "libpacketprocess/packet/PacketHeartBeat.h"
#include "libpacketprocess/packet/PacketClientInfo.h"
#include "libpacketprocess/packet/PacketClientInfoReply.h"

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        std::cerr << "usage: ./TcpClient name packetNum" << std::endl;
        std::cerr << "eg: ./TcpClient client1 100" << std::endl;
        return -1;
    }

    struct DataStatus
    {
        int sendLen{0};
        std::vector<char> sendBuffer;

        int writeLen{0};
        std::vector<char> writeBuffer;
    };
    DataStatus dataStatus;

    int fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(9999);

    if (-1 == connect(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
    {
        std::cout << "connect failed, errno: " << errno << ", " << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "connect successfully" << std::endl;

    components::Socket::setNonBlock(fd);

    dataStatus.writeBuffer.resize(32 * 1024 * 1024);
    dataStatus.sendBuffer.resize(32 * 1024 * 1024);

    packetprocess::PacketHeader packetHeader;
    std::size_t headerLength = packetHeader.headerLength();

    // 发送ClientInfo包
    packetHeader.setType(packetprocess::PacketType::PT_ClientInfo);

    packetprocess::PacketClientInfo clientInfoPacket;
    clientInfoPacket.setId(argv[1]);

    std::size_t payloadLength = clientInfoPacket.packetLength();
    packetHeader.setPayloadLength(payloadLength);

    packetHeader.encode(dataStatus.sendBuffer.data(), headerLength);
    dataStatus.sendLen += headerLength;
    clientInfoPacket.encode(dataStatus.sendBuffer.data() + dataStatus.sendLen, payloadLength);
    dataStatus.sendLen += payloadLength;

    int ret = send(fd, dataStatus.sendBuffer.data(), dataStatus.sendLen, 0);
    if(ret != dataStatus.sendLen)
    {
        std::cout << "send client info packet failed" << std::endl;
        return -1;
    }
    dataStatus.sendLen -= (headerLength + payloadLength);

    packetprocess::PacketClientInfoReply clientInfoReply;
    while(true)
    {
        ret = recv(fd, dataStatus.writeBuffer.data() + dataStatus.writeLen, dataStatus.writeBuffer.size() - dataStatus.writeLen, 0);
        if(ret < (int)headerLength)
        {
            continue;
        }
        dataStatus.writeLen += ret;

        packetHeader.decode(dataStatus.writeBuffer.data(), headerLength);
        if(packetHeader.type() != packetprocess::PacketType::PT_ClientInfoReply)
        {
            std::cout << "error type" << std::endl;
            return -1;
        }
        if(dataStatus.writeLen - headerLength < packetHeader.payloadLength())
        {
            continue;
        }

        clientInfoReply.decode(dataStatus.writeBuffer.data() + headerLength, packetHeader.payloadLength());
        dataStatus.writeLen -= (headerLength + packetHeader.payloadLength());
        break;
    }
    std::cout << "dataStatus.dataSize: " << dataStatus.writeLen << std::endl;

    // 请求包
    packetprocess::PacketRawString rawStringPacket;
    rawStringPacket.setContent("hello");
    payloadLength = rawStringPacket.packetLength();

    packetHeader.setType(packetprocess::PacketType::PT_RawString);
    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> rawStringBuffer;
    int rawStringLength = headerLength + payloadLength;
    rawStringBuffer.resize(rawStringLength);

    packetHeader.encode(rawStringBuffer.data(), headerLength);
    rawStringPacket.encode(rawStringBuffer.data() + headerLength, payloadLength);

    // 心跳包
    packetprocess::PacketHeartBeat heartBeatPacket;
    heartBeatPacket.setTimestamp(0);
    payloadLength = heartBeatPacket.packetLength();

    packetHeader.setType(packetprocess::PacketType::PT_HeartBeat);
    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> heartBeatBuffer;
    int heartLength = headerLength + payloadLength;
    heartBeatBuffer.resize(heartLength);

    packetHeader.encode(heartBeatBuffer.data(), headerLength);
    heartBeatPacket.encode(heartBeatBuffer.data() + headerLength, payloadLength);

    int packetNum = atoi(argv[2]);

    // 发送线程
    const auto sendExpression = [&]()
    {
        components::CellTimestamp cellTimestamp;
        cellTimestamp.update();
        int sendNum{0};
        while(true)
        {
            if(sendNum >= packetNum)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            if(sendNum < packetNum)
            {
                memcpy(dataStatus.sendBuffer.data() + dataStatus.sendLen, rawStringBuffer.data(), rawStringLength);
                dataStatus.sendLen += rawStringLength;
                ++sendNum;
                if(sendNum == packetNum)
                {
                    std::cout << "send finish, sendNum: " << sendNum << std::endl;
                }
            }

            if(cellTimestamp.getElapsedTimeInSec() >= 5)
            {
                std::cout << "add heartbeat packet, length: " << heartLength << std::endl;
                memcpy(dataStatus.sendBuffer.data() + dataStatus.sendLen, heartBeatBuffer.data(), heartLength);
                dataStatus.sendLen += heartLength;
                cellTimestamp.update();
            }

            if(0 == dataStatus.sendLen)
            {
                continue;
            }

            int ret = send(fd, dataStatus.sendBuffer.data(), dataStatus.sendLen, 0);
            if (ret > 0)
            {
                memmove(dataStatus.sendBuffer.data(), dataStatus.sendBuffer.data() + ret, dataStatus.sendLen - ret);
                dataStatus.sendLen -= ret;
            }
        }
    };
    std::thread(sendExpression).detach();

    components::CellTimestamp timestamp;
    timestamp.update();
    int recvNum{0};
    while(recvNum < packetNum)
    {
        int ret = recv(fd, dataStatus.writeBuffer.data() + dataStatus.writeLen, dataStatus.writeBuffer.size() - dataStatus.writeLen, 0);
        if (ret > 0)
        {
            dataStatus.writeLen += ret;
        }

        if (dataStatus.writeLen > headerLength)
        {
            packetprocess::PacketHeader packetHeader;
            packetHeader.decode(dataStatus.writeBuffer.data(), headerLength);
            if (packetHeader.payloadLength() <= dataStatus.writeLen - headerLength)
            {
                packetprocess::PacketRawStringReply reply;
                reply.decode(dataStatus.writeBuffer.data() + headerLength, packetHeader.payloadLength());
                if (true == packetHeader.isMagicMatch() && reply.getResult() == rawStringPacket.getContent())
                {
                    ++recvNum;
                }
                else if (false == packetHeader.isMagicMatch())
                {
                    std::cout << "magic error" << std::endl;
                }
                else
                {
                    std::cout << "packet error" << ", packet type: " << static_cast<int>(packetHeader.type())<< ", payload length: " << packetHeader.payloadLength() << ", result: " << reply.getResult() << std::endl;
                }

                if (0 == recvNum % 5000 || recvNum == packetNum)
                {
                    printf("%d\n", recvNum);
                }

                std::size_t len = headerLength + packetHeader.payloadLength();
                memmove(dataStatus.writeBuffer.data(), dataStatus.writeBuffer.data() + len,dataStatus.writeLen - len);
                dataStatus.writeLen -= len;
            }
        }
    }
    std::cout << packetNum / timestamp.getElapsedTimeInMilliSec() << std::endl;

    close(fd);

    return 0;
}
