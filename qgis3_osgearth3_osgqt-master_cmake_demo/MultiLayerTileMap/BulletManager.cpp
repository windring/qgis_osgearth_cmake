
//
// Created by TictorDC on 2022/7/12.
//

#include <osg/ComputeBoundsVisitor>

#include "BulletManager.h"
#include "TriangleMeshVisitor.h"

namespace MultiLayerTileMap {

    BulletManager::BulletManager(osgEarth::MapNode* node): mapNode(node) {
        collisionConfiguration = new btDefaultCollisionConfiguration;
        dispatcher = new btCollisionDispatcher(collisionConfiguration);
        pairCache = new btDbvtBroadphase;
        solver = new btSequentialImpulseConstraintSolver;
        dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, pairCache, solver, collisionConfiguration);
    }

    btDiscreteDynamicsWorld *BulletManager::getDynamicsWorld() const {
        return dynamicsWorld;
    }

    BulletManager::~BulletManager() {
        // 将刚体从 dynamics 世界中移除，并删除它们
        if (dynamicsWorld != nullptr) {
            for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
                auto *obj = dynamicsWorld->getCollisionObjectArray()[i];
                auto *body = btRigidBody::upcast(obj);
                if (body && body->getMotionState()) {
                    delete body->getMotionState();
                }
                dynamicsWorld->removeCollisionObject(obj);
                delete obj;
            }
        }

        // 删除碰撞图形
        for (int i = 0; i < collisionShapePool.size(); i++) {
            auto **ppShape = collisionShapePool.getAtIndex(i);
            auto *pShape = *ppShape;
            *ppShape = nullptr;
            delete pShape;
        }

        // 删除 dynamics 世界
        delete dynamicsWorld;

        // 删除解算器
        delete solver;

        // 删除宽相
        delete pairCache;

        // 删除调度器
        delete dispatcher;

        // 删除刚体配置
        delete collisionConfiguration;
    }

    btBoxShape *BulletManager::btBoxCollisionShapeFromOSG(osg::Node *node, const osg::BoundingBox *bb) {
        osg::BoundingBox bbox;
        if (bb) {
            bbox = *bb;
        } else {
            osg::ComputeBoundsVisitor visitor;
            node->accept(visitor);
            bbox = visitor.getBoundingBox();
        }

        auto shape = new btBoxShape(btVector3((bbox.xMax() - bbox.xMin()) * 0.5f,
                                              (bbox.yMax() - bbox.yMin()) * 0.5f,
                                              (bbox.zMax() - bbox.zMin()) * 0.5f));
        return shape;
    }

    void BulletManager::createSimplePlaneFromOSG(const osg::Plane &plane) {
        if (dynamicsWorld == nullptr) { // 物理世界未初始化
            return;
        }
        osg::Vec3 norm = plane.getNormal();
        auto *groundShape = new btStaticPlaneShape(btVector3(norm[0], norm[1], norm[2]),
                                                   (float) plane[3]);
        btTransform groundTransform; // 旋转和移动
        groundTransform.setIdentity(); // 旋转矩阵为单位矩阵，不移动
        auto *motionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo groundRigidInfo(0.0, // 质量
                                                                 motionState,
                                                                 groundShape, // 碰撞形状
                                                                 btVector3(0.0, 0.0, 0.0)); // 惯性
        auto *groundBody = new btRigidBody(groundRigidInfo);
        dynamicsWorld->addRigidBody(groundBody);
    }

    const btCollisionShape *BulletManager::getCollisionShapeByKey(const int key) {
        auto **ppShape = collisionShapePool.find(key);
        if (ppShape == nullptr) {
            return nullptr;
        } else {
            return *ppShape;
        }
    }

    osg::Matrix BulletManager::getRigidBodyMatrixByKey(int key) {
        auto **ppShape = rigidBodyPool.find(key);
        if (ppShape == nullptr || (*ppShape) == nullptr) {
            return {};
        }
        auto *body = *ppShape;
        btTransform trans;
        trans = body->getWorldTransform();
        // if (body && body->getMotionState()) {
        //     body->getMotionState()->getWorldTransform(trans);
        // } else {
        //     trans = body->getWorldTransform();
        // }
        osg::Matrixf matrix;
        trans.getOpenGLMatrix(matrix.ptr());
        return matrix;
    }

    int BulletManager::registerRigidBodyAndGetKey(btRigidBody *pRigidBody) {
        rigidBodyPoolMaxKey += 1;
        rigidBodyPool.insert(rigidBodyPoolMaxKey, pRigidBody);
        return rigidBodyPoolMaxKey;
    }

    btTriangleMeshShape *BulletManager::getTriangleMeshShapeCollisionShapeFromOSG(osg::Node *node) {
        TriangleMeshVisitor visitor;
        node->accept( visitor );

        osg::Vec3Array* vertices = visitor.getTriangleMesh();
        if( vertices->size() < 3 )
        {
            osg::notify( osg::WARN ) << "BulletManager::getConvexHullCollisionShapeFromOSG: vertices->size() < 3" << std::endl;
            return nullptr;
        }

        auto* mesh = new btTriangleMesh;
        for( size_t i = 0; i + 2 < vertices->size(); i += 3 )
        {
            osg::Vec3& p1 = ( *vertices )[ i ];
            osg::Vec3& p2 = ( *vertices )[ i + 1 ];
            osg::Vec3& p3 = ( *vertices )[ i + 2 ];
            mesh->addTriangle( btVector3(p1.x(), p1.y(), p1.z()),
                               btVector3(p2.x(), p2.y(), p2.z()),
                               btVector3(p3.x(), p3.y(), p3.z()));
        }

        btBvhTriangleMeshShape* meshShape = new btBvhTriangleMeshShape( mesh, true );
        return meshShape;
    }

    void BulletManager::createTerrainFromOSG() {
        if (dynamicsWorld == nullptr || mapNode == nullptr) { // 物理世界和 OSG 世界未初始化
            return;
        }
        auto *node = mapNode->getTerrainEngine()->getNode();
        auto *groundShape = getTriangleMeshShapeCollisionShapeFromOSG(node);
        btTransform groundTransform; // 旋转和移动
        groundTransform.setIdentity(); // 旋转矩阵为单位矩阵，不移动
        auto *motionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo groundRigidInfo(0.0, // 质量
                                                                 motionState,
                                                                 groundShape, // 碰撞形状
                                                                 btVector3(0.0, 0.0, 0.0)); // 惯性
        auto *groundBody = new btRigidBody(groundRigidInfo);
        dynamicsWorld->addRigidBody(groundBody);
        if (terrainRigidBodyKey != -1) {
            auto pOldBody = *(rigidBodyPool[terrainRigidBodyKey]);
            dynamicsWorld->removeRigidBody(pOldBody); // 移除旧的地形
            delete pOldBody;
            *(rigidBodyPool[terrainRigidBodyKey]) = groundBody;
        } else {
            terrainRigidBodyKey = registerRigidBodyAndGetKey(groundBody);
        }
    }

    void BulletManager::BulletTerrainChangedCallback::onTileUpdate(const osgEarth::TileKey &tileKey, osg::Node *graph,
                                                                   osgEarth::TerrainCallbackContext &) {
        if (_mapNode != nullptr) {


        }
    }
} // MultiLayerTileMap