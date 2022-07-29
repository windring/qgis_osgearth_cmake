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
        /// \note btHashMap 有针对物理模拟的对齐优化，但是对 key 有要求且用法比较古怪
        /// 使用 std map 替换加快开发速度
        // btHashMap<btHashInt, btRigidBody *> rigidBodyPool; // 刚体池
        // btHashMap<btHashInt, btCollisionShape *> collisionShapePool; // 碰撞图形池
        // int rigidBodyPoolMaxKey = -1; // 刚体池的当前最大 key
        // int terrainRigidBodyKey = -1; // 地形在刚体池中的 Key，地形可能消耗非常大，必须保证它是唯一的
        std::unordered_map<const osg::Node*, const btRigidBody*> rigidBodyMap; // 刚体池
        std::unordered_map<const osg::Node*, const btCollisionShape*> collisionShapeMap; // 碰撞图形池
        std::unordered_map<std::string, osg::Node*> tileKey2NodeMap;
        osgEarth::MapNode* mapNode = nullptr;
        int selectedTerrainTileLOD = 8; // 用于构建地形碰撞体的地图引擎 LOD 指数
    public:
        [[nodiscard]] int getSelectedTerrainTileLod() const;

        void setSelectedTerrainTileLod(int selectedTerrainTileLod);

		void setGravity(const osg::Vec3 &vec) {
		  btVector3 gravityVector(vec.x(), vec.y(), vec.z());
		  dynamicsWorld->setGravity(gravityVector);
		}

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
        /// 测试用，不会注册到 rigidBodyMap 中
        /// \param plane
        /// \return 平面刚体
        btRigidBody* createSimplePlaneForDebug(const osg::Plane &plane);

        /// 在物理引擎世界中创建地形瓦片
        /// 三个参数分别对应 osgEarth::TerrainCallback 的三个参数
        /// \param tileKey
        /// \param graph
        /// \param context
        /// \return 地形瓦片在刚体池中的 key
        const osg::Node* createTerrainTile(const osgEarth::TileKey &tileKey,
                              osg::Node *graph,
                              osgEarth::TerrainCallbackContext &context);

        /// 根据 key 获取 collisionShapePool 中对应的碰撞盒
        /// \param pNode key
        /// \return 碰撞盒指针
        const btCollisionShape *getCollisionShapeByKey(const osg::Node *pNode);

        /// 根据 key 获取 rigidBodyPool 中对应碰撞盒的矩阵
        /// \param pNode key
        /// \return 碰撞盒矩阵
        osg::Matrix getRigidBodyMatrixByKey(const osg::Node *pNode);

        /// 向 rigidBodyPool 添加刚体，并返回对应的 key，该 key 用于连接物理引擎中的刚体和 OSG 场景中的节点
        /// \param pRigidBody
        /// \return key
        const osg::Node* registerRigidBodyAndGetKey(const osg::Node* pNode, const btRigidBody *pRigidBody);

        /// 将 osg::Matrix 转换为 btTransform
        /// \param matrix
        /// \return
        static btTransform asBtTransform(const osg::Matrix &matrix);

        static btBvhTriangleMeshShape* asBtBvhTriangleMeshShape(const osg::Vec3Array &vertices);

		static btBvhTriangleMeshShape* asBtBvhTriangleMeshShape(std::vector<osg::Vec3> &vertices);

        class BulletTerrainChangedCallback : public osgEarth::TerrainCallback
        {
            BulletManager* _bulletManager = nullptr;
            osg::Timer_t onTileUpdateTimeTick = 0; // 单位是 tick
        public:
            explicit BulletTerrainChangedCallback(BulletManager* bulletManager): _bulletManager(bulletManager) { }

            /// 地形瓦片更新时触发
            /// \param tileKey
            /// \param graph
            void onTileUpdate(const osgEarth::TileKey& tileKey, osg::Node* graph, osgEarth::TerrainCallbackContext& ) override;
        };
    };
} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_BULLETMANAGER_H
