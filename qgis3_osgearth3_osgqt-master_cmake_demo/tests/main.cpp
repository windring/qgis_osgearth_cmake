//
// Created by TictorDC on 2022/6/24.
//
#include <osgViewer/Viewer>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osgDB/ReadFile>
#include <QLayout>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <osg/CoordinateSystemNode>

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

const double lon = 107.5999770;
const double lat = 33.9887540;
const float lonF = 107.5999770f;
const float latF = 33.9887540f;
const double eps = 0.001;
double x, y, z;
double earthR;
osg::Group *root;
MapLayerManager *mapLayerManager;
osgQOpenGLWidget *widget;
EarthManipulator* em;

TEST_CASE("test 1 == 1") {
    REQUIRE(1 == 1);
}

TEST_CASE("MapLayerManager::MapLayerManager()") {
    PAUSE
    auto *it = new MapLayerManager();
    REQUIRE(it != nullptr);
}

TEST_CASE("MapLayerManager(std::string file)") {
    PAUSE
    const std::string filename = "./data/simple.earth";
    mapLayerManager = new MapLayerManager(filename, root);
    REQUIRE(mapLayerManager != nullptr);
    earthR = mapLayerManager->getMapNode()->getMapSRS()->getEllipsoid().getRadiusEquator();
    printf("earthR: %f\n", earthR);
    osg::Vec3d world;
    auto* oem = new osg::EllipsoidModel();
    oem->convertLatLongHeightToXYZ(lat, lon, 70, x, y, z);
}

TEST_CASE("loadEarthFile(std::string file)") {
    PAUSE
    const std::string filename = "./data/simple.earth";
    root->removeChild(mapLayerManager->getRootNode());
    REQUIRE(mapLayerManager->loadEarthFile(filename));
    root->addChild(mapLayerManager->getRootNode());
}

TEST_CASE("loadEarthFile(std::string file) for not exist file") {
    PAUSE
    const std::string filename = "./data/simple_not_exist.earth";
    REQUIRE_FALSE(mapLayerManager->loadEarthFile(filename));
}

TEST_CASE("addEntity for osg::Node") {
    PAUSE
    const std::string filename = "./data/cessna.osg";
    osg::Node *cessna = readNodeFile(filename);
    auto *trans = new osg::MatrixTransform();
//    trans->setMatrix(osg::Matrix::scale(1e-5, 1e-5, 1e-5));
    trans->addChild(cessna);
    REQUIRE(mapLayerManager->addEntity(trans, {lon-eps, lat, 50}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(trans, {lon+eps, lat, 50}, {0, 90, 0}));
//    mapLayerManager->getMapNode()->addChild(trans);
}

TEST_CASE("addEntity for osg::Group") {
    PAUSE
    const std::string filename = "./data/3ds/3D-Models---Military-master/BRDM3/BRDM3/BRDM3_L.3DS";
    auto* group = new osg::Group;
    osg::Node* a3ds = readNodeFile(filename);
    auto *trans = new osg::MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(1e-2, 1e-2, 1e-2));
    trans->addChild(a3ds);
    group->addChild(trans);
    REQUIRE(mapLayerManager->addEntity(group, {lon+eps*2, lat, 50}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(group, {lon+eps*3, lat, 50}, {0, 90, 0}));
//    mapLayerManager->getMapNode()->addChild(trans);
}

TEST_CASE("addImageLayer") {
    PAUSE
    em->setViewpoint(Viewpoint("12m-center",
                                          lonF, latF, 10,
                                          0, -75, 1000.0));
    const std::string filename = "./data/image/12m-image_Level_12.tif";
    const std::string layerName = "12m-image";
    REQUIRE(mapLayerManager->addImageLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

TEST_CASE("addElevationLayer") {
    PAUSE
    const std::string filename = "./data/dem/12m.tif";
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->addElevationLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) != nullptr);
}

TEST_CASE("hideMapLayer") {
    PAUSE
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->hideMapLayer(layerName));
    Layer* layer = mapLayerManager->findLayerByName(layerName);
    REQUIRE_FALSE(layer->getEnabled());
}

TEST_CASE("showMapLayer") {
    PAUSE
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->showMapLayer(layerName));
    Layer* layer = mapLayerManager->findLayerByName(layerName);
    REQUIRE(layer->getEnabled());
}

TEST_CASE("delLayerByName") {
    PAUSE
//    MESSAGE("删除 World GeoTIFF");
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->delLayerByName(layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) == nullptr);
}

//TEST_CASE("addShapefileLayer") {
//    PAUSE
//    MESSAGE("测试 addShapefileLayer");
//    const std::string filename = "./data/world.shp";
//    const std::string layerName = "shp-layer";
//    REQUIRE(mapLayerManager->addShapefileLayer(filename, layerName));
//    REQUIRE(mapLayerManager->findLayerByName(layerName));
//}

osg::Vec3 effectXYZ(x, y, z);

TEST_CASE("addExplosion") {
//    MESSAGE("测试 addExplosion");
    REQUIRE(mapLayerManager->addExplosion(effectXYZ, {0, 1, 0}, 100, 10));
}

TEST_CASE("addExplosionDebris") {
//    MESSAGE("测试 addExplosionDebris");
    REQUIRE(mapLayerManager->addExplosionDebris(effectXYZ, {0, 1, 0}, 100, 10));
}

TEST_CASE("addSmoke") {
//    MESSAGE("测试 addSmoke");
    REQUIRE(mapLayerManager->addSmoke(effectXYZ, {0, 1, 0}, 100, 10));
}

TEST_CASE("addFire") {
//    MESSAGE("测试 addSmoke");
    REQUIRE(mapLayerManager->addFire(effectXYZ, {0, 1, 0}, 100, 10));
}

TEST_CASE("createMovingModel") {
    const std::string filename = "./data/cessna.osg";
    REQUIRE(mapLayerManager->createMovingModel(filename, {0, 0, 0}, earthR+10000, 10));
}

int main(int argc, char *argv[]) {
//    osg::setNotifyLevel(osg::NotifySeverity::INFO);

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
//                         osg::Node *node = MapNodeHelper().load(arguments, viewer);
//                         if (!node) {
//                             puts("node == NULL");
//                             return -1;
//                         };
//                         root->addChild(node);
                         viewer->setSceneData(root);
                         return Catch::Session().run(argc, argv);
                     });

    mainLayout->addWidget(widget);

    // 显示
    mainWidget->show();

    return QApplication::exec();
}