//
// Created by ivy on 1/8/26.
//

#include <gtest/gtest.h>

#include "csm-service/protocol/payload/PayloadDistanceVector.h"
#include "csm-service/routing_algorithm/distance_vector/DistanceVector.h"

namespace
{
    std::size_t nodeIndexInVectorTuple(const std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>>& vec,
                                       const csm::NodeId& nodeId)
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

    std::size_t nodeIndexInVectorPair(const std::vector<std::pair<csm::NodeId, std::uint32_t>>& vec,
                                      const csm::NodeId& nodeId)
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

    bool expectPairDistance(const std::vector<std::pair<csm::NodeId, std::uint32_t>>& vec,
                            const csm::NodeId& nodeId, std::uint32_t distance)
    {
        std::size_t index = nodeIndexInVectorPair(vec, nodeId);
        if (index == std::numeric_limits<std::size_t>::max())
        {
            return false;
        }
        if (vec[index].second != distance)
        {
            return false;
        }

        return true;
    }

    bool expectTupleDistanceAndNextHop(const std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>>& vec,
                                       const csm::NodeId& nodeId, std::uint32_t distance, const csm::NodeId& nextHop)
    {
        std::size_t index = nodeIndexInVectorTuple(vec, nodeId);
        if (index == std::numeric_limits<std::size_t>::max())
        {
            return false;
        }
        if (std::get<1>(vec[index]) != distance)
        {
            return false;
        }
        if (std::get<2>(vec[index]) != nextHop)
        {
            return false;
        }

        return true;
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

    // A根据C发过来的距离向量更新自己的距离向量表
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
    EXPECT_EQ(std::get<2>(dvInfos[index]), csm::c_invalidNodeId);

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

    // 根据毒性逆转，假如A要向E同步距离矢量，A应该告诉E自己到D的距离是不可达
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

    dvInfos = distanceVectorA.dvInfos();
    index = nodeIndexInVectorTuple(dvInfos, "D");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), csm::service::c_unreachableDistance);
    EXPECT_EQ(std::get<2>(dvInfos[index]), csm::c_invalidNodeId);
}

// 测试目标：当直连更贵，经过邻居更便宜时，选间接路径，并且nextHop正确
TEST(DistanceVectorTest, PreferIndirectShorterPath)
{
    csm::service::DistanceVector A({"B", "C"});
    csm::service::DistanceVector B({"A", "C"});
    csm::service::DistanceVector C({"A", "B"});

    A.updateNeighbourDistance("B", 1);
    A.updateNeighbourDistance("C", 10);
    B.updateNeighbourDistance("A", 1);
    B.updateNeighbourDistance("C", 1);

    A.updateDvInfos("B", B.dvInfo("A"));

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos = A.dvInfos();
    std::size_t index = nodeIndexInVectorTuple(dvInfos, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dvInfos[index]), 2);
    EXPECT_EQ(std::get<2>(dvInfos[index]), "B");
}

/*
 * 测试目标：多轮传播
 * A连接到B，B连接到C，C连接到E，E连接到D
 *
 */
TEST(DistanceVectorTest, MultiRoundSpread)
{
    // A节点的距离向量
    csm::NodeIds neighbourNodeIdsForA{"B", "C", "E"};
    csm::service::DistanceVector distanceVectorA(neighbourNodeIdsForA);

    // 调整与邻居节点的距离
    distanceVectorA.updateNeighbourDistance("B", 1);
    distanceVectorA.updateNeighbourDistance("C", 3);
    distanceVectorA.updateNeighbourDistance("E", 2);

    // B节点的距离向量
    csm::NodeIds neighbourNodeIdsForB{"A", "C"};
    csm::service::DistanceVector distanceVectorB(neighbourNodeIdsForB);

    // 调整与邻居节点的距离
    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("C", 2);

    // C节点的距离向量
    csm::NodeIds neighbourNodeIdsForC{"A", "B", "E"};
    csm::service::DistanceVector distanceVectorC(neighbourNodeIdsForC);

    // 调整与邻居节点的距离
    distanceVectorC.updateNeighbourDistance("A", 3);
    distanceVectorC.updateNeighbourDistance("B", 2);
    distanceVectorC.updateNeighbourDistance("E", 4);

    // D节点的距离向量
    csm::NodeIds neighbourNodeIdsForD{"E"};
    csm::service::DistanceVector distanceVectorD(neighbourNodeIdsForD);

    // 调整与邻居节点的距离
    distanceVectorD.updateNeighbourDistance("E", 3);

    // E节点的距离向量
    csm::NodeIds neighbourNodeIdsForE{"A", "C", "D"};
    csm::service::DistanceVector distanceVectorE(neighbourNodeIdsForE);

    // 调整与邻居节点的距离
    distanceVectorE.updateNeighbourDistance("A", 2);
    distanceVectorE.updateNeighbourDistance("C", 4);
    distanceVectorE.updateNeighbourDistance("D", 3);

    for (int i = 0; i < 5; ++i)
    {
        distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));
        distanceVectorB.updateDvInfos("C", distanceVectorC.dvInfo("B"));
        distanceVectorC.updateDvInfos("E", distanceVectorE.dvInfo("C"));
        distanceVectorE.updateDvInfos("D", distanceVectorD.dvInfo("E"));
    }

    std::optional<std::pair<std::uint32_t, csm::NodeId>> result = distanceVectorA.distance("D");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value().first, 10);
    EXPECT_EQ(result.value().second, "B");
}

/*
 * 测试目标：多轮传播
 * A连接到B，B连接到C，C连接到E，E连接到D
 *
 */
TEST(DistanceVectorTest, MultiRoundSpread2)
{
    // A节点的距离向量
    csm::NodeIds neighbourNodeIdsForA{"B", "C", "E"};
    csm::service::DistanceVector distanceVectorA(neighbourNodeIdsForA);

    // 调整与邻居节点的距离
    distanceVectorA.updateNeighbourDistance("B", 1);
    distanceVectorA.updateNeighbourDistance("C", 3);
    distanceVectorA.updateNeighbourDistance("E", 2);

    // B节点的距离向量
    csm::NodeIds neighbourNodeIdsForB{"A", "C"};
    csm::service::DistanceVector distanceVectorB(neighbourNodeIdsForB);

    // 调整与邻居节点的距离
    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("C", 2);

    // C节点的距离向量
    csm::NodeIds neighbourNodeIdsForC{"A", "B", "E"};
    csm::service::DistanceVector distanceVectorC(neighbourNodeIdsForC);

    // 调整与邻居节点的距离
    distanceVectorC.updateNeighbourDistance("A", 3);
    distanceVectorC.updateNeighbourDistance("B", 2);
    distanceVectorC.updateNeighbourDistance("E", 4);

    // D节点的距离向量
    csm::NodeIds neighbourNodeIdsForD{"E"};
    csm::service::DistanceVector distanceVectorD(neighbourNodeIdsForD);

    // 调整与邻居节点的距离
    distanceVectorD.updateNeighbourDistance("E", 3);

    // E节点的距离向量
    csm::NodeIds neighbourNodeIdsForE{"A", "C", "D"};
    csm::service::DistanceVector distanceVectorE(neighbourNodeIdsForE);

    // 调整与邻居节点的距离
    distanceVectorE.updateNeighbourDistance("A", 2);
    distanceVectorE.updateNeighbourDistance("C", 4);
    distanceVectorE.updateNeighbourDistance("D", 3);

    for (int i = 0; i < 5; ++i)
    {
        distanceVectorE.updateDvInfos("D", distanceVectorD.dvInfo("E"));
        distanceVectorC.updateDvInfos("E", distanceVectorE.dvInfo("C"));
        distanceVectorB.updateDvInfos("C", distanceVectorC.dvInfo("B"));
        distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));
    }

    std::optional<std::pair<std::uint32_t, csm::NodeId>> result = distanceVectorA.distance("D");
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value().first, 10);
    EXPECT_EQ(result.value().second, "B");
}

/*
 * 测试目标：链路代价上升：如果当前nextHop就是这个邻居，必须更新为更大值
 */
TEST(DistanceVectorTest, CostIncreaseMustPropagateWhenUsingThatNextHop)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    csm::service::DistanceVector distanceVectorB({"A", "C"});

    distanceVectorA.updateNeighbourDistance("B", 1);
    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("C", 1);

    // A通过B学到可以到达C，距离为2，nextHop = B
    distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dv = distanceVectorA.dvInfos();
    std::size_t index = nodeIndexInVectorTuple(dv, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dv[index]), 2);
    EXPECT_EQ(std::get<2>(dv[index]), "B");

    // B -> C 变差
    distanceVectorB.updateNeighbourDistance("C", 100);
    // A 更新 B 发过来的距离向量
    distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));

    dv = distanceVectorA.dvInfos();
    index = nodeIndexInVectorTuple(dv, "C");
    EXPECT_NE(index, std::numeric_limits<std::size_t>::max());
    EXPECT_EQ(std::get<1>(dv[index]), 101);
    EXPECT_EQ(std::get<2>(dv[index]), "B");
}

/*
 * 测试目标：在向邻居发送距离向量的时候，如果某个目的地的下一条就是该邻居，根据毒性逆转，需要告诉该邻居我到此
 * 目的地节点不可达
 */
TEST(DistanceVectorTest, PoisonReverse_ShouldAdvertiseUnreachableToNextHopNeighbour)
{
    // 拓扑： A --2-- E --1-- D
    // A学到D的路由后，A到D的nextHop = E，distance = 3
    csm::service::DistanceVector distanceVectorA({"E"});
    csm::service::DistanceVector distanceVectorE({"A", "D"});
    csm::service::DistanceVector distanceVectorD({"E"});

    distanceVectorA.updateNeighbourDistance("E", 2);

    distanceVectorE.updateNeighbourDistance("A", 2);
    distanceVectorE.updateNeighbourDistance("D", 1);

    distanceVectorD.updateNeighbourDistance("E", 1);

    // E向A通告，A学到了到达可以通过E到达D，距离是3
    distanceVectorA.updateDvInfos("E", distanceVectorE.dvInfo("A"));

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos = distanceVectorA.dvInfos();
    bool result = expectTupleDistanceAndNextHop(dvInfos, "E", 2, "E");
    EXPECT_TRUE(result);
    result = expectTupleDistanceAndNextHop(dvInfos, "D", 3, "E");
    EXPECT_TRUE(result);

    // 确保A发送给E的距离向量符合毒性逆转的规则
    std::vector<std::pair<csm::NodeId, std::uint32_t>> dvInfoASendToE = distanceVectorA.dvInfo("E");
    EXPECT_EQ(dvInfoASendToE.size(), 1);

    result = expectPairDistance(dvInfoASendToE, "D", csm::service::c_unreachableDistance);
    EXPECT_TRUE(result);
}

/*
 * 收到非邻居节点的dv更新，应返回false且不改本地表
 */
TEST(DistanceVectorTest, UpdateDvInfos_FromNonNeighbour_ShouldReturnFalseAndKeepState)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    distanceVectorA.updateNeighbourDistance("B", 1);

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> beforeDvInfos = distanceVectorA.dvInfos();
    bool result = expectTupleDistanceAndNextHop(beforeDvInfos, "B", 1, "B");
    EXPECT_TRUE(result);

    // C不是A的邻居
    result = distanceVectorA.updateDvInfos("C", {{"D", 2}});
    EXPECT_FALSE(result);

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> afterDvInfos = distanceVectorA.dvInfos();
    EXPECT_TRUE(beforeDvInfos == afterDvInfos);
}

/*
 * 收到未知目标节点时，应新增目的地
 */
TEST(DistanceVectorTest, UpdateDvInfos_UnknownDestination_ShouldBeLearned)
{
    // A只知道邻居B
    // B告诉A：我还能到X，代价4
    // A到B代价1
    // 则A应该学到X, distance=5, nextHop=B
    csm::service::DistanceVector distanceVectorA({"B"});
    distanceVectorA.updateNeighbourDistance("B", 1);

    csm::service::DistanceVector distanceVectorB({"A", "E"});
    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("E", 4);

    distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 2);
    bool result = expectTupleDistanceAndNextHop(dvInfos, "B", 1, "B");
    EXPECT_TRUE(result);
    result = expectTupleDistanceAndNextHop(dvInfos, "E", 5, "B");
    EXPECT_TRUE(result);
}

/*
 * 空DV更新应该成功，但不改变现有表
 */
TEST(DistanceVectorTest, UpdateDvInfos_EmptyVector_ShouldNotChangeState)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    distanceVectorA.updateNeighbourDistance("B", 1);

    auto before = distanceVectorA.dvInfos();

    bool result = distanceVectorA.updateDvInfos("B", {});
    EXPECT_TRUE(result);

    auto after = distanceVectorA.dvInfos();

    EXPECT_EQ(before, after);
}

/*
 * 重复收到相同dv，不应产生额外变化
 */
TEST(DistanceVectorTest, UpdateDvInfos_SameInputTwice_ShouldKeepSameRoutingTable)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    csm::service::DistanceVector distanceVectorB({"A", "C"});

    distanceVectorA.updateNeighbourDistance("B", 1);

    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("C", 2);

    bool result = distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));
    EXPECT_TRUE(result);

    auto before = distanceVectorA.dvInfos();

    result = distanceVectorA.updateDvInfos("B", distanceVectorB.dvInfo("A"));
    EXPECT_TRUE(result);

    auto after = distanceVectorA.dvInfos();

    EXPECT_EQ(before, after);
}

/*
 * updateNeighbourDistance对非邻居节点应返回false且不插入新项
 */
TEST(DistanceVectorTest, UpdateNeighbourDistance_NonNeighbour_ShouldReturnFalseAndKeepState)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    distanceVectorA.updateNeighbourDistance("B", 1);

    csm::service::DistanceVector distanceVectorB({"A", "C"});
    distanceVectorB.updateNeighbourDistance("A", 1);
    distanceVectorB.updateNeighbourDistance("C", 2);

    csm::service::DistanceVector distanceVectorC({"B"});
    distanceVectorC.updateNeighbourDistance("B", 2);

    bool result = distanceVectorA.updateNeighbourDistance("C", 3);
    EXPECT_FALSE(result);
}

/*
 * 邻居距离变得更小，应该更新；变成一样，不应该出问题
 */
TEST(DistanceVectorTest, UpdateNeighbourDistance_SmallerOrSameDistance_ShouldBehaveCorrectly)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    bool result = distanceVectorA.updateNeighbourDistance("B", 3);
    EXPECT_TRUE(result);

    auto dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);
    result = expectTupleDistanceAndNextHop(dvInfos, "B", 3, "B");
    EXPECT_TRUE(result);

    result = distanceVectorA.updateNeighbourDistance("B", 3);
    EXPECT_FALSE(result); // 相同距离，应返回false（返回值为isUpdated）

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);
    result = expectTupleDistanceAndNextHop(dvInfos, "B", 3, "B");
    EXPECT_TRUE(result);

    result = distanceVectorA.updateNeighbourDistance("B", 1);
    EXPECT_TRUE(result);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);
    result = expectTupleDistanceAndNextHop(dvInfos, "B", 1, "B");
    EXPECT_TRUE(result);
}

/*
 * 距离达到不可达阈值时，应该按不可达处理
 */
TEST(DistanceVectorTest, UpdateNeighbourDistance_UnreachableBoundary_ShouldBeStoreAsUnreachable)
{
    csm::service::DistanceVector distanceVectorA({"B"});
    bool result = distanceVectorA.updateNeighbourDistance("B", 752);
    EXPECT_TRUE(result);

    auto dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);

    result = expectTupleDistanceAndNextHop(dvInfos, "B", csm::service::c_unreachableDistance, csm::c_invalidNodeId);
    EXPECT_TRUE(result);
}

/*
 * E-D断链后，A 应把 D 设为不可达
 * Todo: 需要考虑 A 到 D 不可达后，是否直接从距离向量表中移除D
 */
TEST(DistanceVectorTest, LinkBreak_EDown_AShouldMarkDUnreachable)
{
    // 拓扑初始状态：
    // A --2-- E --1-- D
    //
    // 期望：
    // 1. 初始时 A 通过 E 学到 D = 3, nextHop = E
    // 2. 当 E-D 断链后，E 通知 A
    // 3. A 应把 D 更新为不可达
    csm::service::DistanceVector distanceVectorA({"E"});
    bool result = distanceVectorA.updateNeighbourDistance("E", 2);
    EXPECT_TRUE(result);

    csm::service::DistanceVector distanceVectorE({"A", "D"});
    result = distanceVectorE.updateNeighbourDistance("A", 2);
    EXPECT_TRUE(result);
    result = distanceVectorE.updateNeighbourDistance("D", 1);
    EXPECT_TRUE(result);

    std::vector<std::tuple<csm::NodeId, std::uint32_t, csm::NodeId>> dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 1);

    result = distanceVectorA.updateDvInfos("E", distanceVectorE.dvInfo("A"));
    EXPECT_TRUE(result);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 2);

    result = expectTupleDistanceAndNextHop(dvInfos, "E", 2, "E");
    EXPECT_TRUE(result);
    result = expectTupleDistanceAndNextHop(dvInfos, "D", 3, "E");
    EXPECT_TRUE(result);

    result = distanceVectorE.updateNeighbourDistance("D", csm::service::c_unreachableDistance);
    EXPECT_TRUE(result);

    dvInfos = distanceVectorE.dvInfos();
    EXPECT_EQ(dvInfos.size(), 2);
    result = expectTupleDistanceAndNextHop(dvInfos, "A", 2, "A");
    EXPECT_TRUE(result);
    result = expectTupleDistanceAndNextHop(dvInfos, "D", csm::service::c_unreachableDistance, csm::c_invalidNodeId);
    EXPECT_TRUE(result);

    result = distanceVectorA.updateDvInfos("E", distanceVectorE.dvInfo("A"));
    EXPECT_TRUE(result);

    dvInfos = distanceVectorA.dvInfos();
    EXPECT_EQ(dvInfos.size(), 2);
    result = expectTupleDistanceAndNextHop(dvInfos, "E", 2, "E");
    EXPECT_TRUE(result);
    result = expectTupleDistanceAndNextHop(dvInfos, "D", csm::service::c_unreachableDistance, csm::c_invalidNodeId);
    EXPECT_TRUE(result);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
