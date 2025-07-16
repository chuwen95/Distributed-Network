//
// Created by chu on 7/9/25.
//

#ifndef PAYLOADDISTANCEDETECTREPLY_H
#define PAYLOADDISTANCEDETECTREPLY_H

#include "protocol/pb/distance_vector.pb.h"
#include "PayloadBase.h"

namespace csm
{

    namespace service
    {

        class PayloadDistanceDetectReply : public PayloadTypeBase<protocol::DistanceDetectReply>
        {
        public:
            using Ptr = std::shared_ptr<PayloadDistanceDetectReply>;

        public:
            void setSeq(std::uint64_t seq);
            std::uint64_t seq() const;

            void setElapsedTime(std::uint32_t timestamp);
            std::uint32_t elapsedTime() const;
        };

    }

}

#endif //PAYLOADDISTANCEDETECTREPLY_H
