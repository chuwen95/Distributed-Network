//
// Created by chu on 7/18/25.
//

#ifndef PAYLOADDISTANCEVECTOR_H
#define PAYLOADDISTANCEVECTOR_H

#include "PayloadBase.h"
#include "protocol/pb/distance_vector.pb.h"

#include "csm-framework/cluster/Common.h"

namespace csm
{

    namespace service
    {

        class PayloadDistanceVector : public PayloadTypeBase<protocol::DistanceVector>
        {
        public:
            using Ptr = std::shared_ptr<PayloadDistanceVector>;

        public:
            std::vector<std::pair<NodeId, std::uint32_t>> distanceInfos() const;
            void addDistanceInfo(const NodeId& key, std::uint32_t value);
        };

    } // namespace service

} // namespace csm

#endif // PAYLOADDISTANCEVECTOR_H