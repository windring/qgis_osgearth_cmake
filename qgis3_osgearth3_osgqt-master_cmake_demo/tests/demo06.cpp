//
// Created by TictorDC on 2022/7/2.
//
#include <QtGui/qopengl.h>
#include <QLayout>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <osgViewer/Viewer>
#include <osgEarth/EarthManipulator>
#include <osgDB/ReadFile>
#include <osg/CoordinateSystemNode>
#include <osg/Timer>
#include <osgGA/GUIEventHandler>
#include <osgEarth/GLUtils>
#include <osgEarth/CullingUtils>
#include <osgEarth/Registry>
#include <osgEarth/ModelNode>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>
#include <osg/ComputeBoundsVisitor>
#include <osg/Quat>
#include <osg/Vec2d>

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()
#define SHOW_MESSAGE true
#define MESSAGE_TITLE "TileMapManagerDemo catch2 测试"
#define MESSAGE(MESSAGE_STR) if(SHOW_MESSAGE) { QMessageBox::information(widget, MESSAGE_TITLE, MESSAGE_STR); }

#include "catch.hpp"

using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;

class Tank {
    osgEarth::ModelNode* tankGroup = nullptr;
    osg::Vec3d startWorldPos, endWorldPos;
    double speed = 200; // m/s
    double deltaHeight;
    MapLayerManager* manager;
    osgParticle::SmokeEffect* tankSmoke = nullptr;
public:
    Tank(osgEarth::ModelNode* _tankGroup, MapLayerManager* _manager, osgParticle::SmokeEffect* _smoke, double _deltaHeight) :
            tankGroup(_tankGroup), manager(_manager), tankSmoke(_smoke), deltaHeight(_deltaHeight) {
        startWorldPos = {-268956, 5.2314e+06, 3.63454e+06+deltaHeight};
        endWorldPos = {-268956, 5.2314e+06, 3.63454e+06+deltaHeight};
    }

    [[nodiscard]] double getDeltaDistance(double deltaTime) const {
        return deltaTime * speed;
    }

    GeoPoint getNextGeoPos(double deltaDistance) {
        double minDistance = (startWorldPos-endWorldPos).length() ;
        if (minDistance < deltaDistance) {
            startWorldPos = endWorldPos;
        } else {
            osg::Vec3d iVector3d = endWorldPos - startWorldPos;
            iVector3d.normalize();
            startWorldPos += iVector3d * deltaDistance;

            // 简化为二维问题
            iVector3d.z() = 0;
            osg::Vec3d i = {0, 1, 0};
            double dot = i*iVector3d;
            double bottom = i.length() * iVector3d.length();
            double angle = acos(dot / bottom);
            printf("angle: %g\n", osg::RadiansToDegrees(angle));
            double cross = (i^iVector3d).z();
            if (cross < 0) {
                angle *= -1;
            }
            tankGroup->setLocalRotation(osg::Quat(angle, osg::Vec3({0, 0, 1})));
        }
        GeoPoint ret;
        ret.fromWorld(manager->getEarthSRS(), startWorldPos);

        return ret;
    }

    void setEndWorldPos(osg::Vec3d pos) {
        endWorldPos = pos;
    }

    void updateWorldPos(double deltaTime) {
        double deltaDistance = getDeltaDistance(deltaTime);
        GeoPoint nextGeoPos = getNextGeoPos(deltaDistance);
        tankGroup->setPosition(nextGeoPos);
        tankSmoke->setPosition(startWorldPos-osg::Vec3d(0, 0, deltaHeight));
    }
};

const float lonF = 92.9431f;
const float latF = 34.9348f;
const double eps = 0.001;
double tankDeltaHeight;
double earthR;
osg::Group *root = nullptr;
MapLayerManager *mapLayerManager = nullptr;
osgQOpenGLWidget *widget = nullptr;
EarthManipulator *em = nullptr;
osgEarth::ModelNode* tankModelNode = nullptr;
Tank* tank = nullptr;
osgParticle::SmokeEffect* tankSmoke = nullptr;

struct Demo06GUIEventHandler : public osgGA::GUIEventHandler {
    osg::Timer_t timeTick = osg::Timer::instance()->tick();
    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override {
        osg::Timer_t curTimeTick = osg::Timer::instance()->tick();
        double deltaS = osg::Timer::instance()->delta_s(timeTick, curTimeTick);
        timeTick = curTimeTick;
        if (tank) {
            tank->updateWorldPos(deltaS);
        }
        switch (ea.getEventType()) {
            case (osgGA::GUIEventAdapter::PUSH):
                printf("push\n");
                if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) {
                    // TODO
                    osg::Vec3d pos = getPos(ea, aa);
                    printf("Pos:%g %g %g\n", pos.x(), pos.y(), pos.z());
                }
                return false;
            default:
                return false;
        }
    }

    static osg::Vec3d getPos(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) {
        osg::Vec3d pos(0, 0, 0);
        auto *pViewer = dynamic_cast<osgViewer::Viewer *>(&aa);
        if (pViewer == nullptr) {
            return pos;
        }
        // 获取当前点
        osgUtil::LineSegmentIntersector::Intersections intersection;
        pViewer->computeIntersections(ea.getX(), ea.getY(), intersection);
        auto iter = intersection.begin();
        if (iter != intersection.end()) {
            osg::Vec3d worldPos = iter->getWorldIntersectPoint(); // 世界坐标
            worldPos.z() += tankDeltaHeight;
            if (tank) {
                tank->setEndWorldPos(worldPos);
            }
            printf("worldPos: %g %g %g\n", worldPos.x(), worldPos.y(), worldPos.z());
            // 地理坐标
            GeoPoint geoPoint;
            geoPoint.fromWorld(mapLayerManager->getEarthSRS(), worldPos);
            printf("geoPoint: %g %g %g\n", geoPoint.x(), geoPoint.y(), geoPoint.z());
            pos = geoPoint.vec3d();
            // mapLayerManager->addExplosion(worldPos, {0, 0, 1}, 10, 10);
        }
        return pos;
    }
};

Demo06GUIEventHandler *guiEventHandler;

TEST_CASE("MapLayerManager(std::string file)") {
    PAUSE
    const std::string filename = "./data/demo06/simple.earth";
    mapLayerManager = new MapLayerManager(filename, root);
    REQUIRE(mapLayerManager != nullptr);
    earthR = mapLayerManager->getMapNode()->getMapSRS()->getEllipsoid().getRadiusEquator();
    printf("earthR: %f\n", earthR);
}

TEST_CASE("addImageLayer") {
    PAUSE
    em->setViewpoint(Viewpoint("12m-center",
                               lonF, latF, 4552,
                               0, -45, 400.0));
    const std::string filename = "./data/demo06/AGM/AGM_BeiluRiver_Image_Level_16.tif";
    const std::string layerName = "AGM_BeiluRiver_Image";
    REQUIRE(mapLayerManager->addImageLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

TEST_CASE("addElevationLayer") {
    PAUSE
    const std::string filename = "./data/demo06/AGM/ASTGTMV003_N34E092_dem.tif";
    const std::string layerName = "AGM_BeiluRiver_Dem";
    REQUIRE(mapLayerManager->addElevationLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

TEST_CASE("addShapefileLayer") {
    PAUSE
    const std::string filename = "./data/demo06/AGM/AGM_BeiluRiver.shp";
    const std::string layerName = "shp-layer";
    REQUIRE(mapLayerManager->addShapefileLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName));
}

TEST_CASE("addEntity") {
    PAUSE
    // -268956 5.2314e+06 3.63454e+06
    // 92.9431 34.9348 4551.91
    const double lon = 92.9431;
    const double lat = 34.9348;
    const double alt = 4561.91;
    const std::string filename = "./data/demo06/M60.obj";
    osg::Node *cessna = readNodeFile(filename);
    osgEarth::Registry::shaderGenerator().run(cessna);
    cessna->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
    cessna->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

    auto *trans = new osg::MatrixTransform;
    trans->setMatrix(osg::Matrix::scale(1e-2, 1e-2, 1e-2));
    trans->addChild(cessna);

    // 计算包围盒
    osg::ComputeBoundsVisitor cbv;
    trans->accept(cbv);
    osg::BoundingBox modelBB = cbv.getBoundingBox();
    printf("modelBB: %g %g %g\n", modelBB.zMin(), modelBB.zMax(), modelBB.center().z());
    tankDeltaHeight = modelBB.center().z() - modelBB.zMin();

    tankSmoke = new osgParticle::SmokeEffect({-268956, 5.2314e+06, 3.63454e+06}, 10, 10);
    tankSmoke->setWind({0, 30, 10});
    tankSmoke->setParticleDuration(2);
    tankSmoke->getEmitter()->setEndless(true);
    tankSmoke->getEmitter()->setLifeTime(100) ; // 一直冒灰

    auto *group = new osg::Group;
    group->addChild(trans);
    group->addChild(tankSmoke);

    REQUIRE(mapLayerManager->addEntity(group, {lon, lat, alt+tankDeltaHeight}, {0, 0, 0}));
    tankModelNode = dynamic_cast<osgEarth::ModelNode*>(mapLayerManager->getLastChild());
}

//TEST_CASE("createMovingModel") {
//    const std::string filename = "./data/demo06/cessna.osg";
//    REQUIRE(mapLayerManager->createMovingModel(filename, {0, 0, 0}, earthR+10000, 10));
//}



TEST_CASE("TankMove") {
    tank = new Tank(tankModelNode, mapLayerManager, tankSmoke, tankDeltaHeight);
}

//class GuidedMissile {
//public:
//    MapLayerManager* manager = nullptr;
//    osg::Vec3d startGeoPos;
//    osg::Vec3d endGeoPos;
//    osgEarth::ModelNode* guideMissile;
//    explicit GuidedMissile(MapLayerManager* _mapLayerManager): manager(_mapLayerManager) {
//        startGeoPos = {95, 36, 8561.91};
//        endGeoPos = {92.9431, 34.9348, 4590.91};
//        const std::string filename = "./data/demo06/huojianend.obj";
//        osg::Node *entity = readNodeFile(filename);
//        osgEarth::Registry::shaderGenerator().run(entity);
//        entity->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
//        entity->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
//        auto *trans = new osg::MatrixTransform();
//        trans->setMatrix(osg::Matrix::scale(10, 10, 10));
//        trans->addChild(entity);
//        manager->addEntity(trans, endGeoPos, startGeoPos-endGeoPos);
//        guideMissile = dynamic_cast<osgEarth::ModelNode*>(manager->getLastChild());
//    }
//};
//
//TEST_CASE("GuidedMissile") {
//    auto *guideMissile = new GuidedMissile(mapLayerManager);
//}

int main(int argc, char *argv[]) {
    osg::setNotifyLevel(osg::NotifySeverity::NOTICE);
    osgEarth::setNotifyLevel(osg::NotifySeverity::NOTICE);

    QApplication app(argc, argv);

    // 初始化 osgEarth
    osgEarth::initialize();

    // 新建主的 widget 及布局
    auto *mainWidget = new QWidget;
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // osgEarth widget
    osg::ArgumentParser arguments(&argc, argv);
    widget = new osgQOpenGLWidget(&arguments);
    guiEventHandler = new Demo06GUIEventHandler;
    QObject::connect(widget, &osgQOpenGLWidget::initialized,
                     [&argc, &argv] {
                         root = new osg::Group();
                         osgViewer::Viewer *viewer = widget->getOsgViewer();
                         em = new EarthManipulator();
                         viewer->setCameraManipulator(em);
                         viewer->setSceneData(root);
                         viewer->addEventHandler(dynamic_cast<osgGA::EventHandler*>(guiEventHandler));
                         GLUtils::setGlobalDefaults(viewer->getCamera()->getOrCreateStateSet());
                         viewer->getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
                         return Catch::Session().run(argc, argv);
                     });

    mainLayout->addWidget(widget);

    // 显示
    mainWidget->showMaximized();

    return QApplication::exec();
}