//
// Created by ivy on 1/8/26.
//

#include <gtest/gtest.h>

#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"

TEST(DistanceVectorTest, MainTest)
{
    csm::NodeIds nodeIds{"A", "B", "C", "D", "E", "F", "G", "H"};

    csm::service::DistanceVector distanceVector(nodeIds);
    EXPECT_EQ(nodeIds, distanceVector.neighbours());


}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}