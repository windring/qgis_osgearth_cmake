//
// Created by TictorDC on 2022/6/24.
//
#include <osgViewer/Viewer>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osg/GraphicsContext>
#include <osgDB/ReadFile>
#include <QLayout>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <cstdio>

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()

#include "catch.hpp"

using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;

osg::Group *root;
MapLayerManager *mapLayerManager;

TEST_CASE("test 1 == 1") {
    REQUIRE(1 == 1);
}

TEST_CASE("MapLayerManager::MapLayerManager()") {
    PAUSE;
    auto *it = new MapLayerManager();
    REQUIRE(it != nullptr);
}

TEST_CASE("MapLayerManager(std::string file)") {
    PAUSE;
    mapLayerManager = new MapLayerManager("./data/simple.earth");
    REQUIRE(mapLayerManager != nullptr);
    root->addChild(mapLayerManager->getRootNode());
}

TEST_CASE("loadEarthFile(std::string file)") {
    PAUSE;
    root->removeChild(mapLayerManager->getRootNode());
    REQUIRE(mapLayerManager->loadEarthFile("./data/simple.earth"));
    root->addChild(mapLayerManager->getRootNode());
}

TEST_CASE("loadEarthFile(std::string file) for not exist file") {
    PAUSE;
    REQUIRE_FALSE(mapLayerManager->loadEarthFile("./data/simple_not_exist.earth"));
}

TEST_CASE("addEntity for osg::Node") {
    PAUSE;
    osg::Node *cessna = readNodeFile("./data/cessna.osg");
    osg::MatrixTransform *trans = new osg::MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(1e5, 1e5, 1e5));
    trans->addChild(cessna);
    REQUIRE(mapLayerManager->addEntity(trans, {0, 0, 1e6}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(trans, {0, 90, 1e6}, {0, 90, 0}));
//    mapLayerManager->getMapNode()->addChild(trans);
}

TEST_CASE("addEntity for osg::Group") {
    PAUSE;
    osg::Group* group = new osg::Group;
    osg::Node* a3ds = readNodeFile("./data/3ds/1.3ds");
    osg::MatrixTransform *trans = new osg::MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(1e5, 1e5, 1e5));
    trans->addChild(a3ds);
    group->addChild(trans);
    REQUIRE(mapLayerManager->addEntity(group, {0, 0, 1e6}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(group, {0, 90, 1e6}, {0, 90, 0}));
//    mapLayerManager->getMapNode()->addChild(trans);
}

int main(int argc, char *argv[]) {
//    osg::setNotifyLevel(osg::NotifySeverity::INFO);


    QApplication app(argc, argv);

    // 初始化 osgEarth
    osgEarth::initialize();

    // 新建主的 widget 及布局
    QWidget *mainWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // osgEarth widget
    osg::ArgumentParser arguments(&argc, argv);
    osgQOpenGLWidget widget(&arguments);
    QObject::connect(&widget, &osgQOpenGLWidget::initialized,
                     [&widget, &arguments, &argc, &argv] {
                         root = new osg::Group();
                         osgViewer::Viewer *viewer = widget.getOsgViewer();
                         osg::ref_ptr<EarthManipulator> manipulator = new EarthManipulator();
                         viewer->setCameraManipulator(manipulator);
//                         osg::Node *node = MapNodeHelper().load(arguments, viewer);
//                         if (!node) {
//                             puts("node == NULL");
//                             return -1;
//                         };
//                         root->addChild(node);
                         viewer->setSceneData(root);
                         return Catch::Session().run(argc, argv);;
                     });

    mainLayout->addWidget(&widget);

    // 显示
    mainWidget->show();

    return app.exec();
}