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
#include <utility>

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"
#include "MultiLayerTileMap/OGRManager.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()

#include "catch.hpp"

using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;


const float lonF = 92.9431f;
const float latF = 34.9348f;
const double eps = 0.001;
double tankDeltaHeight;
double earthR;
osg::Group *root = nullptr;
MapLayerManager *mapLayerManager = nullptr;
osgQOpenGLWidget *widget = nullptr;
EarthManipulator *em = nullptr;
osgEarth::ModelNode *tankModelNode = nullptr;
GroundVehicle *tank = nullptr;
osgParticle::SmokeEffect *tankSmoke = nullptr;
OGRManager *ogrManager = nullptr;

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
    tankSmoke->getEmitter()->setLifeTime(100); // 一直冒灰

    auto *group = new osg::Group;
    group->addChild(trans);
    group->addChild(tankSmoke);

    REQUIRE(mapLayerManager->addEntity(group, {lon, lat, alt + tankDeltaHeight}, {0, 0, 0}));
    tankModelNode = dynamic_cast<osgEarth::ModelNode *>(mapLayerManager->getLastChild());
}

//TEST_CASE("createMovingModel") {
//    const std::string filename = "./data/demo06/cessna.osg";
//    REQUIRE(mapLayerManager->createMovingModel(filename, {0, 0, 0}, earthR+10000, 10));
//}

TEST_CASE("OGRManager") {
    const std::string filename = "./data/demo06/AGM/AGM_BeiluRiver.shp";
    const std::string datasetName = "tankShapefile";
    ogrManager = new OGRManager();
    ogrManager->loadDataset(filename, datasetName);
    REQUIRE(ogrManager->getDatasetByName(datasetName));
}

TEST_CASE("TankMove") {
    const std::string datasetName = "tankShapefile";
    tank = new GroundVehicle(tankModelNode, mapLayerManager, tankSmoke, tankDeltaHeight, ogrManager, datasetName);
    widget->getOsgViewer()->addEventHandler(dynamic_cast<osgGA::EventHandler *>(tank->moveGUIEventHandler));
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
//    osg::setNotifyLevel(osg::NotifySeverity::DEBUG_FP);
//    osgEarth::setNotifyLevel(osg::NotifySeverity::DEBUG_FP);

    QApplication app(argc, argv);

    // 初始化 osgEarth
    osgEarth::initialize();

    // 新建主的 widget 及布局
    auto *mainWidget = new QWidget;
    auto *mainLayout = new QHBoxLayout(mainWidget);

    // osgEarth widget
    osg::ArgumentParser arguments(&argc, argv);
    widget = new osgQOpenGLWidget(&arguments);
    QObject::connect(widget, &osgQOpenGLWidget::initialized,
                     [&argc, &argv] {
                         root = new osg::Group();
                         osgViewer::Viewer *viewer = widget->getOsgViewer();
                         em = new EarthManipulator();
                         viewer->setCameraManipulator(em);
                         viewer->setSceneData(root);
                         GLUtils::setGlobalDefaults(viewer->getCamera()->getOrCreateStateSet());
                         viewer->getCamera()->setSmallFeatureCullingPixelSize(-1.0f);
                         return Catch::Session().run(argc, argv);
                     });

    mainLayout->addWidget(widget);

    // 显示
    mainWidget->showMaximized();

    return QApplication::exec();
}