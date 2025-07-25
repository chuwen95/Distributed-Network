//
// Created by chu on 7/9/25.
//

#ifndef PAYLOADDISTANCEDETECT_H
#define PAYLOADDISTANCEDETECT_H

#include "PayloadBase.h"
#include "protocol/pb/distance_vector.pb.h"

namespace csm
{

    namespace service
    {

        class PayloadDistanceDetect : public PayloadTypeBase<protocol::DistanceDetect>
        {
        public:
            using Ptr = std::shared_ptr<PayloadDistanceDetect>;

        public:
            void setSeq(std::uint32_t seq);
            std::uint32_t seq() const;

            void setTimestamp(std::uint32_t timestamp);
            std::uint32_t timestamp() const;
        };

    } // namespace service

} // namespace csm

#endif // PAYLOADDISTANCEDETECT_H
