//
// Created by chu on 7/17/25.
//

#ifndef PACKETTYPE_H
#define PACKETTYPE_H

enum class PacketType
{
    PT_None,

    // handshake
    PT_ClientInfo,
    PT_ClientInfoReply,

    // heartbeat
    PT_HeartBeat,
    PT_HeartBeatReply,

    // distance_vector -> distance detect
    PT_DistanceDetect,
    PT_DistanceDetectReply,

    // message
    PT_ModuleMessage
};

#endif //PACKETTYPE_H
