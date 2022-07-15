#include <osg/ArgumentParser>
#include <osgGA/GUIEventHandler>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include "MultiLayerTileMap/BulletManager.h"

using namespace MultiLayerTileMap;

BulletManager *bulletManager = nullptr;

class SampleRigidUpdater : public osgGA::GUIEventHandler {
    typedef std::map<int, osg::observer_ptr<osg::MatrixTransform>> MTNodeMap;
    MTNodeMap _osgNodes;
    osg::observer_ptr<osg::Group> _root;
    BulletManager *_bulletManager;
public:
    SampleRigidUpdater(osg::Group *root, BulletManager *manager) : _root(root), _bulletManager(manager) {}

    void addGround() {
        osg::ref_ptr<osg::MatrixTransform> groundMT = new osg::MatrixTransform;
        osg::ref_ptr<osg::Geode> groundGeode = new osg::Geode; // 地理 node
        osg::ref_ptr<osg::Box> groundShape = new osg::Box(osg::Vec3(0.0f, 0.0f, -0.5f),
                                                          100.0f,
                                                          100.0f,
                                                          1.0f);
        osg::ref_ptr<osg::ShapeDrawable> groundDrawable = new osg::ShapeDrawable(groundShape);
        groundGeode->addDrawable(groundDrawable);
        groundMT->addChild(groundGeode.get());
        _root->addChild(groundMT.get());
    }

    int addPhysicsBox(osg::Box *shape, const osg::Vec3 &pos, const osg::Vec3 &vel, float mass) {
        const osg::Vec3 &dim = shape->getHalfLengths();
        btCollisionShape *boxShape = new btBoxShape(btVector3(dim[0], dim[1], dim[2]));
        btTransform boxTransform;
        boxTransform.setIdentity();

        btVector3 localInertia(0.0, 0.0, 0.0);
        if (mass > 0.0) {
            boxShape->calculateLocalInertia(mass, localInertia);
        }

        auto *motionState = new btDefaultMotionState(boxTransform);
        btRigidBody::btRigidBodyConstructionInfo rigidInfo(mass, motionState, boxShape, localInertia);
        auto *pRigidBody = new btRigidBody(rigidInfo);
        _bulletManager->getDynamicsWorld()->addRigidBody(pRigidBody);

        // 物理位置
        btTransform trans;
        trans.setFromOpenGLMatrix(osg::Matrixf(osg::Matrix::translate(pos)).ptr());
        pRigidBody->setWorldTransform(trans);

        // 物理速度
        pRigidBody->setLinearVelocity(btVector3(vel.x(), vel.y(), vel.z()));
        int key = _bulletManager->registerRigidBodyAndGetKey(pRigidBody);
        return key;
    }

    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override {
        auto *view = dynamic_cast<osgViewer::View *>( &aa );

        if (!view || !_root) return false;

        switch (ea.getEventType()) {
            case osgGA::GUIEventAdapter::PUSH:
                if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) {
                    printf("fire\n");
                    osg::Vec3 eye, center, up, dir;
                    view->getCamera()->getViewMatrixAsLookAt(eye, center, up);
                    dir = center - eye;
                    dir.normalize();
                    auto *pShape = new osg::Box(osg::Vec3(), 0.5f);
                    int key = addPhysicsBox(pShape, eye, dir * 60.0f, 2.0);
                    addOSGShape(key, pShape);
                }
                break;
            case osgGA::GUIEventAdapter::FRAME:
                _bulletManager->getDynamicsWorld()->stepSimulation(1.f / 60.f, 10);
                for (auto &itr: _osgNodes) {
                    osg::Matrix matrix = bulletManager->getRigidBodyMatrixByKey(itr.first); // 获取在物理引擎中的矩阵
                    itr.second->setMatrix(matrix); // 设置矩阵到 OSG 场景的对应 Node
                }
                break;
            default:
                break;
        }
        return false;
    }

    void addOSGShape(int key, osg::Shape *shape) {
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(shape));
        mt->addChild(geode.get());
        _root->addChild(mt.get());
        _osgNodes[key] = mt;
    }
};

int main(int argc, char **argv) {
    bulletManager = new BulletManager;
    bulletManager->getDynamicsWorld()->setGravity(btVector3(0.0f, 0.0f, -9.8f)); // 设置重力
    osg::Plane plane(0.0f, 0.0f, 1.0f, 0.0f);
    bulletManager->createSimplePlaneFromOSG(plane); // 创建平面

    osg::ArgumentParser arguments(&argc, argv);

    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osgGA::GUIEventHandler> updater;
    auto *rigidUpdater = new SampleRigidUpdater(root.get(), bulletManager);
    rigidUpdater->addGround();
    for (unsigned int i = 0; i < 10; ++i) {
        for (unsigned int j = 0; j < 10; ++j) {
            auto *pShape = new osg::Box(osg::Vec3(), 0.99f);
            int key = rigidUpdater->addPhysicsBox(pShape,
                                                  osg::Vec3((float) i, 0.0f, (float) j + 0.5f), osg::Vec3(), 1.0f);
            rigidUpdater->addOSGShape(key, pShape);
        }
    }
    updater = rigidUpdater;

    osgViewer::Viewer viewer;
    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    if (updater.valid())
        viewer.addEventHandler(updater.get());
    viewer.setSceneData(root.get());
    viewer.run();
    delete bulletManager;
    return 0;
}