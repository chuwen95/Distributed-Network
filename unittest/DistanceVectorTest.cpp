//
// Created by ivy on 1/8/26.
//

#include <gtest/gtest.h>

#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"

namespace
{
    std::size_t nodeIndexInVectorTuple(const std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>>& vec, const csm::NodeId& nodeId)
    {
        for (auto iter = vec.begin(); iter != vec.end(); ++iter)
        {
            if (std::get<0>(*iter) == nodeId)
            {
                return iter - vec.begin();
            }
        }

        return std::numeric_limits<std::size_t>::max();
    }

    std::size_t nodeIndexInVectorPair(const std::vector<std::pair<csm::NodeId, std::uint32_t>>& vec, const csm::NodeId& nodeId)
    {
        for (auto iter = vec.begin(); iter != vec.end(); ++iter)
        {
            if (std::get<0>(*iter) == nodeId)
            {
                return iter - vec.begin();
            }
        }

        return std::numeric_limits<std::size_t>::max();
    }

    bool compareNodesElement(csm::NodeIds a, csm::NodeIds b)
    {
        std::sort(a.begin(), a.end());
        std::sort(b.begin(), b.end());

        return a == b;
    }
}

TEST(DistanceVectorTest, MainTest)
{
    // 假设集群中所有的节点是："A", "B", "C", "D", "E"

    // A节点的距离向量
    csm::NodeIds neighbourNodeIdsForA{"B", "C", "E"};
    csm::service::DistanceVector distanceVectorA(neighbourNodeIdsForA);
    EXPECT_TRUE(compareNodesElement(neighbourNodeIdsForA, distanceVectorA.neighbours()));

    // 调整与邻居节点的距离
    bool isUpdated = distanceVectorA.updateNeighbourDistance("B", 1);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorA.updateNeighbourDistance("C", 3);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorA.updateNeighbourDistance("E", 2);
    EXPECT_EQ(isUpdated, true);

    // 获取A节点当前的距离向量，向量大小应该为3
    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // B应该在向量中
    std::size_t index = nodeIndexInVectorTuple(dvInfos, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");
    // C应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");
    // E应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // B节点的距离向量
    csm::NodeIds neighbourNodeIdsForB{"A", "C"};
    csm::service::DistanceVector distanceVectorB(neighbourNodeIdsForB);
    EXPECT_TRUE(compareNodesElement(neighbourNodeIdsForB, distanceVectorB.neighbours()));

    // 调整与邻居节点的距离
    isUpdated = distanceVectorB.updateNeighbourDistance("A", 1);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorB.updateNeighbourDistance("C", 2);
    EXPECT_EQ(isUpdated, true);

    // 获取A节点当前的距离向量，向量大小应该为3
    dvInfos = distanceVectorB.dvInfos();
    EXPECT_EQ(dvInfos.size(), 2);

    // A应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "A");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "A");
    // C应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");

    /*
     *********************************************************************************************************
     */

    // C节点的距离向量
    csm::NodeIds neighbourNodeIdsForC{"A", "B", "E"};
    csm::service::DistanceVector distanceVectorC(neighbourNodeIdsForC);
    EXPECT_TRUE(compareNodesElement(neighbourNodeIdsForC, distanceVectorC.neighbours()));

    // 调整与邻居节点的距离
    isUpdated = distanceVectorC.updateNeighbourDistance("A", 3);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorC.updateNeighbourDistance("B", 2);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorC.updateNeighbourDistance("E", 4);
    EXPECT_EQ(isUpdated, true);

    dvInfos = distanceVectorC.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // A应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "A");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "A");
    // C应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");
    // E应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 4);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // D节点的距离向量
    csm::NodeIds neighbourNodeIdsForD{"E"};
    csm::service::DistanceVector distanceVectorD(neighbourNodeIdsForD);
    EXPECT_TRUE(compareNodesElement(neighbourNodeIdsForD, distanceVectorD.neighbours()));

    // 调整与邻居节点的距离
    isUpdated = distanceVectorD.updateNeighbourDistance("E", 1);
    EXPECT_EQ(isUpdated, true);

    dvInfos = distanceVectorD.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);

    // B应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // E节点的距离向量
    csm::NodeIds neighbourNodeIdsForE{"A", "C", "D"};
    csm::service::DistanceVector distanceVectorE(neighbourNodeIdsForE);
    EXPECT_TRUE(compareNodesElement(neighbourNodeIdsForE, distanceVectorE.neighbours()));

    // 调整与邻居节点的距离
    isUpdated = distanceVectorE.updateNeighbourDistance("A", 2);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorE.updateNeighbourDistance("C", 4);
    EXPECT_EQ(isUpdated, true);
    isUpdated = distanceVectorE.updateNeighbourDistance("D", 1);
    EXPECT_EQ(isUpdated, true);

    dvInfos = distanceVectorE.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // B应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "A");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "A");
    // C应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 4);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");
    // E应该在向量中
    index = nodeIndexInVectorTuple(dvInfos, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "D");

    /*
     *********************************************************************************************************
     */

    // 确认B告知A的距离向量正确
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfoBSendToA = distanceVectorB.dvInfo("A");
    EXPECT_EQ(dvInfoBSendToA.size(), 1);
    EXPECT_EQ(dvInfoBSendToA[0].first, "C");
    EXPECT_EQ(dvInfoBSendToA[0].second, 2);

    // A根据B发过来的距离向量更新自己的距离向量表
    distanceVectorA.updateDvInfos("B", dvInfoBSendToA);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // 确认A更新了距离向量表后的自身的距离向量正确
    index = nodeIndexInVectorTuple(dvInfos, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");

    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");

    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // 确认C告知A的距离向量正确
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfoCSendToA = distanceVectorC.dvInfo("A");
    EXPECT_EQ(dvInfoCSendToA.size(), 2);

    index = nodeIndexInVectorPair(dvInfoCSendToA, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(dvInfoCSendToA[index].second, 2);

    index = nodeIndexInVectorPair(dvInfoCSendToA, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(dvInfoCSendToA[index].second, 4);

    // A根据B发过来的距离向量更新自己的距离向量表
    distanceVectorA.updateDvInfos("C", dvInfoCSendToA);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // 确认A更新了距离向量表后自身的距离向量正确
    index = nodeIndexInVectorTuple(dvInfos, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");

    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");

    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // 确认E告知A的距离向量正确
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfoESendToA = distanceVectorE.dvInfo("A");
    EXPECT_EQ(dvInfoESendToA.size(), 2);

    index = nodeIndexInVectorPair(dvInfoESendToA, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(dvInfoESendToA[index].second, 4);

    index = nodeIndexInVectorPair(dvInfoESendToA, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(dvInfoESendToA[index].second, 1);

    // A根据B发过来的距离向量更新自己的距离向量表
    distanceVectorA.updateDvInfos("E", dvInfoESendToA);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 4);

    // 确认A更新了距离向量表后自身的距离向量正确
    index = nodeIndexInVectorTuple(dvInfos, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 1);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");

    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");

    index = nodeIndexInVectorTuple(dvInfos, "E");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    index = nodeIndexInVectorTuple(dvInfos, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 3);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "E");

    /*
     *********************************************************************************************************
     */

    // 断开E与D的通路，让E与D不可达
    distanceVectorE.updateNeighbourDistance("D", csm::service::c_unreachableDistance);

    // 确认此时E节点的距离向量正确
    dvInfos = distanceVectorE.dvInfos();
    EXPECT_EQ(dvInfos.size(), 3);

    // 确认A更新了距离向量表后自身的距离向量正确
    index = nodeIndexInVectorTuple(dvInfos, "A");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "A");

    index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 4);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "C");

    index = nodeIndexInVectorTuple(dvInfos, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), csm::service::c_unreachableDistance);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "D");

    /* E的距离矢量发生了变化，通知邻居，这里以通知A为例 */

    // 确认E告知A的距离矢量正确
    dvInfoESendToA = distanceVectorE.dvInfo("A");
    EXPECT_EQ(dvInfoESendToA.size(), 2);

    index = nodeIndexInVectorPair(dvInfoESendToA, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfoESendToA[index]), 4);

    index = nodeIndexInVectorPair(dvInfoESendToA, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfoESendToA[index]), csm::service::c_unreachableDistance);

    // 模拟算法层A收到了E发过来的距离向量
    distanceVectorA.updateDvInfos("E", dvInfoESendToA);

    // 根据毒性逆转，假如A要向E同步距离矢量，E应该告诉A自己到D的距离是不可达
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfoASendToE = distanceVectorA.dvInfo("E");
    EXPECT_EQ(dvInfoASendToE.size(), 3);

    index = nodeIndexInVectorPair(dvInfoASendToE, "B");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfoASendToE[index]), 1);

    index = nodeIndexInVectorPair(dvInfoASendToE, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfoASendToE[index]), 3);

    index = nodeIndexInVectorPair(dvInfoASendToE, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfoASendToE[index]), csm::service::c_unreachableDistance);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}