//
// Created by ChuWen on 11/4/23.
//

#ifndef TCPNETWORK_PACKETCONCEPT_H
#define TCPNETWORK_PACKETCONCEPT_H

template<typename T>
concept PacketPtr = requires(T t, char* data, std::size_t size)
{
t.get();
{ t.use_count() } -> std::same_as<std::size_t>;
{ t.unique() } -> std::same_as<bool>;

{ t->encode(data, size) } -> std::same_as<int>;
};

template<typename T>
concept Packet = requires(T t, char* data, std::size_t size)
{
    { t.encode(data, size) } -> std::same_as<int>;
};

#endif //TCPNETWORK_PACKETCONCEPT_H
