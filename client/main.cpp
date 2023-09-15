//
// Created by root on 9/12/23.
//

#include "libcommon/Common.h"
#include "libcomponents/Socket.h"
#include "libcomponents/CellTimestamp.h"
#include "libpacketprocess/packet/PacketRawString.h"
#include "libpacketprocess/packet/PacketRawStringReply.h"
#include "libpacketprocess/packet/PacketHeartBeat.h"

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        std::cerr << "usage: ./TcpClient packetNum" << std::endl;
        std::cerr << "eg: ./TcpClient 100" << std::endl;
        return -1;
    }

    struct DataStatus
    {
        using Ptr = std::shared_ptr<DataStatus>;

        int sendLen{0};
        int dataSize{0};
        std::vector<char> sendBuffer;

        int writeLen{0};
        std::vector<char> writeBuffer;
    };
    std::unordered_map<int, DataStatus::Ptr> clients;

    for(int i = 0; i < 1; ++i)
    {
        int fd = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in servaddr;
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        servaddr.sin_port = htons(9999);

        if (-1 == connect(fd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
        {
            std::cout << "connect failed, errno: " << errno << ", " << strerror(errno) << std::endl;
            continue;
        }
        std::cout << "connect successfully" << std::endl;

        auto dataStatus = std::make_shared<DataStatus>();
        dataStatus->writeBuffer.resize(4 * 1024 * 1024);
        dataStatus->sendBuffer.resize(32 * 1024 * 1024);
        clients.emplace(fd, dataStatus);

        components::Socket::setNonBlock(fd);
    }

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

    // 心跳包
    packetHeader.setType(packetprocess::PacketType::PT_HeartBeat);

    packetprocess::PacketHeartBeat heartBeatPacket;
    heartBeatPacket.setTimestamp(0);

    headerLength = packetHeader.headerLength();
    payloadLength = heartBeatPacket.packetLength();
    int heartLength = headerLength + payloadLength;

    packetHeader.setPayloadLength(payloadLength);

    std::vector<char> heartBeatBuffer;
    heartBeatBuffer.resize(heartLength);

    packetHeader.encode(heartBeatBuffer.data(), headerLength);
    heartBeatPacket.encode(heartBeatBuffer.data() + headerLength, payloadLength);

    int packetNum = atoi(argv[1]);

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

            for (auto client: clients)
            {
                if(sendNum < packetNum)
                {
                    memcpy(client.second->sendBuffer.data() + client.second->dataSize, rawStringBuffer.data(), rawStringLength);
                    client.second->dataSize += rawStringLength;
                    ++sendNum;
                    if(sendNum == packetNum)
                    {
                        std::cout << "send finish, sendNum: " << sendNum << std::endl;
                    }
                }

                if(cellTimestamp.getElapsedTimeInSec() >= 5)
                {
                    std::cout << "add heartbeat packet, length: " << heartLength << std::endl;
                    memcpy(client.second->sendBuffer.data() + client.second->dataSize, heartBeatBuffer.data(), heartLength);
                    client.second->dataSize += heartLength;
                    cellTimestamp.update();
                }

                if(0 == client.second->dataSize)
                {
                    continue;
                }

                if(client.second->dataSize > client.second->sendBuffer.size())
                {
                    std::cout << client.second->dataSize << std::endl;
                }
                int ret = send(client.first, client.second->sendBuffer.data(), client.second->dataSize, 0);
                if (ret > 0)
                {
                    memmove(client.second->sendBuffer.data(), client.second->sendBuffer.data() + ret, client.second->dataSize - ret);
                    client.second->dataSize -= ret;
                }
            }
        }
    };
    std::thread(sendExpression).detach();

    int recvNum{0};
    while(recvNum < packetNum)
    {
        for (auto client: clients)
        {
            int ret = recv(client.first, client.second->writeBuffer.data() + client.second->writeLen, client.second->writeBuffer.size() - client.second->writeLen, 0);
            if (ret > 0)
            {
                client.second->writeLen += ret;
            }

            if (client.second->writeLen > headerLength)
            {
                packetprocess::PacketHeader packetHeader;
                packetHeader.decode(client.second->writeBuffer.data(), headerLength);
                if (packetHeader.payloadLength() <= client.second->writeLen - headerLength)
                {
                    packetprocess::PacketRawStringReply reply;
                    reply.decode(client.second->writeBuffer.data() + headerLength, packetHeader.payloadLength());
                    if(true == packetHeader.isMagicMatch() && reply.getResult() == rawStringPacket.getContent())
                    {
                        ++recvNum;
                    }
                    else if(false == packetHeader.isMagicMatch())
                    {
                        std::cout << "magic error" << std::endl;
                    }
                    else
                    {
                        std::cout << "packet error" << ", packet type: " << static_cast<int>(packetHeader.type())
                            << ", payload length: " << packetHeader.payloadLength() << ", result: " << reply.getResult()  << std::endl;
                    }

                    if(0 == recvNum % 5000 || recvNum == packetNum)
                    {
                        printf("%d\n", recvNum);
                    }

                    std::size_t len = headerLength + packetHeader.payloadLength();
                    memmove(client.second->writeBuffer.data(), client.second->writeBuffer.data() + len, client.second->writeLen - len);
                    client.second->writeLen -= len;
                }
            }
        }
    }

    return 0;
}
