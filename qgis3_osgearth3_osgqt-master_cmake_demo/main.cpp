//
// Created by TictorDC on 2022/5/27.
//
#include <QLayout>
#include <osgViewer/Viewer>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include <osg/GraphicsContext>
#include <QAction>
#include <QMessageBox>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include "osgQOpenGL/osgQOpenGLWidget.h"

using namespace osgEarth::Util;

int main(int argc, char *argv[]) {
    //初始化QGIS应用，前缀路径修改为 qgis 的安装路径
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication app(argc, argv, true);
    QgsApplication::initQgis();

    // 初始化 osgEarth
    osgEarth::initialize();

    // 新建主的 widget 及布局
    QWidget *mainWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // osgEarth widget
    osg::ArgumentParser arguments(&argc, argv);
    osgQOpenGLWidget widget(&arguments);
    QObject::connect(&widget, &osgQOpenGLWidget::initialized,
                     [&widget, &arguments] {
                         osg::ref_ptr<osg::Group> root = new osg::Group();
                         osgViewer::Viewer* viewer = widget.getOsgViewer();
                         osg::ref_ptr<EarthManipulator> manipulator = new EarthManipulator();
                         viewer->setCameraManipulator(manipulator);
                         osg::Node *node = MapNodeHelper().load(arguments, viewer);
                         if (!node) {
                             puts("node == NULL");
                             return -1;
                         };
                         root->addChild(node);
                         viewer->setSceneData(root);
                         return 0;
                     });

    // qgis widget，加载 world.tif
    QgsMapCanvas *mapCanvas = new QgsMapCanvas;
    QList<QgsMapLayer *> layers;
    QString fileName = "./data/world.shp";
    QString basename = "world.shp";
    QgsVectorLayer* vecLayer = new QgsVectorLayer(fileName, basename, "ogr");
    if (!vecLayer->isValid())
    {
        QMessageBox::critical(mapCanvas, "error", QString("layer is invalid: \n") + fileName);
    }
    mapCanvas->setExtent(vecLayer->extent());
    layers.append(vecLayer);
    mapCanvas->setLayers(layers);
    mapCanvas->refresh();

    // 设置布局
    mainLayout->addWidget(mapCanvas);
    mainLayout->addWidget(&widget);

    // 显示
    mainWidget->show();

    return app.exec();
}