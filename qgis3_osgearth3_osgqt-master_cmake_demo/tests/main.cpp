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

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()

#include "catch.hpp"

using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;

const double lon = 107.5999770;
const double lat = 33.9887540;
const double eps = 0.001;
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
    mapLayerManager = new MapLayerManager(filename);
    REQUIRE(mapLayerManager != nullptr);
    root->addChild(mapLayerManager->getRootNode());
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
    REQUIRE(mapLayerManager->addEntity(trans, {lon, lat, 50}, {0, 0, 0}));
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
                                          lon, lat-eps*5, 100,
                                          0, -75, 3450.0));
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
    const std::string layerName = "12m-image";
    REQUIRE(mapLayerManager->hideMapLayer(layerName));
    Layer* layer = mapLayerManager->findLayerByName(layerName);
    REQUIRE_FALSE(layer->getEnabled());
}

TEST_CASE("showMapLayer") {
    PAUSE
    const std::string layerName = "12m-image";
    REQUIRE(mapLayerManager->showMapLayer(layerName));
    Layer* layer = mapLayerManager->findLayerByName(layerName);
    REQUIRE(layer->getEnabled());
}

TEST_CASE("delLayerByName") {
    PAUSE
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->delLayerByName(layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) == nullptr);
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