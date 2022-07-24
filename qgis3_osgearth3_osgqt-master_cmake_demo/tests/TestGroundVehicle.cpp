//
// Created by TictorDC on 2022/7/2.
//
#include <QtGui/qopengl.h>
#include <QLayout>
#include <QAction>
#include <QApplication>
#include <QMessageBox>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgEarth/EarthManipulator>
#include <osgDB/ReadFile>
#include <osg/CoordinateSystemNode>
#include <osgGA/GUIEventHandler>
#include <osgGA/StateSetManipulator>
#include <osgEarth/GLUtils>
#include <osgEarth/Registry>
#include <osg/Quat>
#include <osg/Vec2d>

#include "osgQOpenGL/osgQOpenGLWidget.h"
#include "MultiLayerTileMap/MapLayerManager.h"
#include "MultiLayerTileMap/OGRManager.h"
#include "MultiLayerTileMap/GroundVehicle.h"

#define CATCH_CONFIG_RUNNER
#define PAUSE ;;; // std::getchar()

#include "catch.hpp"

using namespace osgEarth::Util;
using namespace osgDB;
using namespace MultiLayerTileMap;


const float lonF = 92.9431f;
const float latF = 34.9348f;
const double eps = 0.001;
osg::Group *root = nullptr;
MapLayerManager *mapLayerManager = nullptr;
osgQOpenGLWidget *widget = nullptr;
EarthManipulator *em = nullptr;
GroundVehicle *tank = nullptr;
OGRManager *ogrManager = nullptr;

TEST_CASE("MapLayerManager(std::string file)") {
    PAUSE
    const std::string filename = "./data/demo06/simple.earth";
    mapLayerManager = new MapLayerManager(filename, root);
    REQUIRE(mapLayerManager != nullptr);
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
//    REQUIRE(mapLayerManager->addShapefileLayer(filename, layerName));
    REQUIRE(mapLayerManager->addShapefileLayerFromPostGIS(
            "localhost",
            "5432",
            "postgres",
            "postgres",
            "postgis_32_sample",
            "agm_beiluriver",
            layerName
            ));
    REQUIRE(mapLayerManager->findLayerByName(layerName));
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
    // -268956 5.2314e+06 3.63454e+06
    // 92.9431 34.9348 4551.91
    const std::string filename = "./data/demo06/M60.obj";
    const osg::Vec3d lonLatAlt{92.9431, 34.9348, 4551.91};
    const osg::Vec3d picHeadingRoll{0, 0, 0};
    const osg::Vec3d scaleFactor{1e-2, 1e-2, 1e-2};
    const std::string shapefileDatasetName = "tankShapefile";
    tank = new GroundVehicle(mapLayerManager,
                             ogrManager,
                             filename,
                             lonLatAlt,
                             picHeadingRoll,
                             scaleFactor,
                             shapefileDatasetName);
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
//    osg::setNotifyLevel(osg::NotifySeverity::INFO);
//    osgEarth::setNotifyLevel(osg::NotifySeverity::INFO);

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
                         viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
                         viewer->addEventHandler(new osgViewer::StatsHandler);
                         return Catch::Session().run(argc, argv);
                     });

    mainLayout->addWidget(widget);

    // 显示
    mainWidget->showMaximized();

    return QApplication::exec();
}