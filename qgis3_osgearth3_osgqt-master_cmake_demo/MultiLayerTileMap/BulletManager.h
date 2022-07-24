//
// Created by TictorDC on 2022/7/12.
//

#ifndef TILEMAPMANAGERDEMO_BULLETMANAGER_H
#define TILEMAPMANAGERDEMO_BULLETMANAGER_H

#include "btBulletDynamicsCommon.h"

#include <osg/Node>
#include <osg/BoundingBox>
#include <osg/Plane>
#include <osgEarth/Terrain>
#include <osgEarth/MapNode>
#include <osgEarth/TerrainEngineNode>

#include <map>

namespace MultiLayerTileMap {

    class BulletManager {

        btDefaultCollisionConfiguration *collisionConfiguration = nullptr; // 碰撞配置
        btCollisionDispatcher *dispatcher = nullptr; // 默认碰撞调度器，可切换为多进程调度器以提速
        btBroadphaseInterface *pairCache = nullptr; // 宽相
        btSequentialImpulseConstraintSolver *solver = nullptr; // 顺序脉冲解算器，可切换为多进程解算器
        btDiscreteDynamicsWorld *dynamicsWorld = nullptr; // 离散动态世界
        btHashMap<btHashInt, btRigidBody *> rigidBodyPool; // 刚体池
        btHashMap<btHashInt, btCollisionShape *> collisionShapePool; // 碰撞图形池
        osgEarth::MapNode* mapNode = nullptr;
        int rigidBodyPoolMaxKey = -1; // 刚体池的当前最大 key
        int terrainRigidBodyKey = -1; // 地形在刚体池中的 Key，地形可能消耗非常大，必须保证它是唯一的

    public:
        /// 获取世界指针进行操作，减少不必要的封装
        /// \return 世界指针
        [[nodiscard]] btDiscreteDynamicsWorld *getDynamicsWorld() const;

        explicit BulletManager(osgEarth::MapNode* node=nullptr);

        ~BulletManager();

        /// 由 osg::Node 获取简单盒
        /// \param node osg::Node 指针
        /// \param bb BB盒
        /// \return bullet 盒形
        static btBoxShape *btBoxCollisionShapeFromOSG(osg::Node *node, const osg::BoundingBox *bb = nullptr);

        /// 创建世界平面，不带有地势高低
        /// \param plane
        /// \return 平面在刚体池中的 key
        int createSimplePlaneFromOSG(const osg::Plane &plane);

        /// 创建带有地形的世界平面
        /// \return 地面在刚体池中的 key
        int createTerrainFromOSG();

        /// 根据 key 获取 collisionShapePool 中对应的碰撞盒
        /// \param key
        /// \return 碰撞盒指针
        const btCollisionShape *getCollisionShapeByKey(int key);

        /// 根据 key 获取 rigidBodyPool 中对应碰撞盒的矩阵
        /// \param key
        /// \return 碰撞盒矩阵
        osg::Matrix getRigidBodyMatrixByKey(int key);

        /// 向 rigidBodyPool 添加刚体，并返回对应的 key，该 key 用于连接物理引擎中的刚体和 OSG 场景中的节点
        /// \param pRigidBody
        /// \return key
        int registerRigidBodyAndGetKey(btRigidBody *pRigidBody);

        /// 计算 osg::Node 的三角面片
        /// \param node
        /// \return
        btTriangleMeshShape* getTriangleMeshShapeCollisionShapeFromOSG(osg::Node* node);

        class BulletTerrainChangedCallback : public osgEarth::TerrainCallback
        {
            BulletManager* _bulletManager = nullptr;
            osg::Timer_t onTileUpdateTimeTick; // 单位是 tick
        public:
            explicit BulletTerrainChangedCallback(BulletManager* bulletManager): _bulletManager(bulletManager) { }

            /// 地形瓦片更新时触发
            /// \param tileKey
            /// \param graph
            void onTileUpdate(const osgEarth::TileKey& tileKey, osg::Node* graph, osgEarth::TerrainCallbackContext& ) override;

            /// 延迟启动的地形面片生成程序
            void timeoutCreateTerrain();
        };
    };
} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_BULLETMANAGER_H
