//
// Created by TictorDC on 2022/5/27.
//
#include "qgisosg.h"
#include <QtWidgets/QApplication>
#include <qgsapplication.h>
#include <osgViewer/Viewer>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ExampleResources>
#include "GraphicsWindowQt.h"
#include <osg/GraphicsContext>

using namespace osgEarth::Util;

int main(int argc, char *argv[]) {
    QgsApplication a(argc, argv, true);
    QgsApplication::setPrefixPath("G:/modules/Qt5.12.12+Osg3.6.5+OsgEarth3.3.0+QGIS3.22.6", true);
    QgsApplication::initQgis();    //初始化QGIS应用
    /*
    osgEarth::initialize();
    osg::ArgumentParser arguments(&argc, argv);
    osgViewer::Viewer viewer(arguments);
    viewer.setThreadingModel( viewer.SingleThreaded );
    viewer.setRunFrameScheme( viewer.ON_DEMAND );
    viewer.setCameraManipulator(new EarthManipulator());
    osg::Node *node = MapNodeHelper().load(arguments, &viewer);
    if (!node)
        return puts("Failed to load earth file.");
    viewer.setSceneData(node);
    osgQt::GraphicsWindowQt *gw = dynamic_cast<osgQt::GraphicsWindowQt*>(viewer.getCamera()->getGraphicsContext());
    */

    DigtalEarth w;    //创建一个窗体，类似于Qt
    w.show();
    w.InitTimer();
    return a.exec();
}