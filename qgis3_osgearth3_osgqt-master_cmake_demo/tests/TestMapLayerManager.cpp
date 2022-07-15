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
const float epsF = 0.001f;
double x, y, z;
double earthR;
osg::Vec3d effectXYZ;
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
    trans->addChild(cessna);
    REQUIRE(mapLayerManager->addEntity(trans, {lon-eps, lat, 50}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(trans, {lon+eps, lat, 50}, {0, 90, 0}));
}

TEST_CASE("addEntity for osg::Group") {
    PAUSE
    const std::string filename = "./data/demo06/M60.obj";
    auto* group = new osg::Group;
    osg::Node* a3ds = readNodeFile(filename);
    auto *trans = new osg::MatrixTransform();
    trans->setMatrix(osg::Matrix::scale(1e-2, 1e-2, 1e-2));
    trans->addChild(a3ds);
    group->addChild(trans);
    REQUIRE(mapLayerManager->addEntity(group, {lon+eps*2, lat, 50}, {0, 0, 0}));
    REQUIRE(mapLayerManager->addEntity(group, {lon+eps*3, lat, 50}, {0, 90, 0}));
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
    const std::string layerName = "12m-image";
    REQUIRE(mapLayerManager->hideMapLayer(layerName));
    auto* layer = dynamic_cast<VisibleLayer*>(mapLayerManager->findLayerByName(layerName));
    REQUIRE_FALSE(layer->getVisible());
}

TEST_CASE("showMapLayer") {
    PAUSE
    const std::string layerName = "12m-image";
    REQUIRE(mapLayerManager->showMapLayer(layerName));
    auto* layer = dynamic_cast<VisibleLayer*>(mapLayerManager->findLayerByName(layerName));
    REQUIRE(layer->getVisible());
}

TEST_CASE("delLayerByName") {
    PAUSE
    const std::string layerName = "12m-elevation";
    REQUIRE(mapLayerManager->delLayerByName(layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName) == nullptr);
}

TEST_CASE("addShapefileLayer") {
    PAUSE
    const std::string filename = "./data/world.shp";
    const std::string layerName = "shp-layer";
    REQUIRE(mapLayerManager->addShapefileLayer(filename, layerName));
    REQUIRE(mapLayerManager->findLayerByName(layerName));
}

TEST_CASE("WorldPos To GeoPos") {
    PAUSE
    GeoPoint geoPos(mapLayerManager->getEarthSRS(), lon, lat, 0);
    geoPos.toWorld(effectXYZ);
    printf("worldPos: %g %g %g\n", effectXYZ.x(), effectXYZ.y(), effectXYZ.z());
}

TEST_CASE("addExplosion") {
    PAUSE
    osg::Vec3 position{lonF, latF-epsF, 0};
    osg::Vec3 wind{0, 1, 0};
    float scale = 10;
    float intensity = 10;
    REQUIRE(mapLayerManager->addExplosion(position, wind, scale, intensity));
}

TEST_CASE("addExplosionDebris") {
    PAUSE
    osg::Vec3 position{lonF, latF, 0};
    osg::Vec3 wind{0, 1, 0};
    float scale = 10;
    float intensity = 10;
    REQUIRE(mapLayerManager->addExplosionDebris(position, wind, scale, intensity));
}

TEST_CASE("addSmoke") {
    PAUSE
    osg::Vec3 position{lonF, latF+epsF, 0};
    osg::Vec3 wind{0, 1, 0};
    float scale = 10;
    float intensity = 10;
    REQUIRE(mapLayerManager->addSmoke(position, wind, scale, intensity));
}

TEST_CASE("addSmokeTrailEffect") {
    PAUSE
    osg::Vec3 position{lonF, latF+2*epsF, 0};
    osg::Vec3 wind{0, 1, 0};
    float scale = 10;
    float intensity = 10;
    REQUIRE(mapLayerManager->addSmokeTrailEffect(position, wind, scale, intensity));
}

TEST_CASE("addFire") {
    PAUSE
    osg::Vec3 position{lonF, latF+3*epsF, 0};
    osg::Vec3 wind{0, 1, 0};
    float scale = 10;
    float intensity = 10;
    REQUIRE(mapLayerManager->addFire(position, wind, scale, intensity));
}

TEST_CASE("createMovingModel") {
    PAUSE
    const std::string filename = "./data/cessna.osg";
    REQUIRE(mapLayerManager->createMovingModel(filename, {0, 0, 0}, earthR+10000, 10));
}

struct TestEventHandler : osgGA::GUIEventHandler {
    bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override {
        switch (ea.getEventType()) {
            case (osgGA::GUIEventAdapter::PUSH):
                if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) {
                    // 设定目的地
                    osgUtil::LineSegmentIntersector::Intersections intersection;
                    auto *pViewer = dynamic_cast<osgViewer::Viewer *>(&aa);
                    pViewer->computeIntersections(ea.getX(), ea.getY(), intersection);
                    auto iter = intersection.begin();
                    if (iter != intersection.end()) {
                        osg::Vec3d worldPos = iter->getWorldIntersectPoint(); // 世界坐标
                        printf("worldPos: %g %g %g\n", worldPos.x(), worldPos.y(), worldPos.z());
                    }
                }
                return false;
            default:
                return false;
        }
    }
};

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
                         auto eventHandler = new TestEventHandler;
                         viewer->addEventHandler(eventHandler);
                         return Catch::Session().run(argc, argv);
                     });

    mainLayout->addWidget(widget);

    // 显示
    mainWidget->show();

    return QApplication::exec();
}