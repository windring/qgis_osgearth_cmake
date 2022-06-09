//
// Created by TictorDC on 2022/5/27.
//
#include <QLayout>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>
#include <osg/GraphicsContext>
#include <QAction>
#include <QMessageBox>
#include <qgsapplication.h>
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/CompositeViewer>
#include <osgGA/MultiTouchTrackballManipulator>

#include "GraphicsWindowQt.h"

using namespace osgEarth::Util;

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer
{
public:
    ViewerWidget(QWidget* parent = 0, Qt::WindowFlags f = 0, osgViewer::ViewerBase::ThreadingModel threadingModel=osgViewer::CompositeViewer::SingleThreaded) : QWidget(parent, f)
    {
        setThreadingModel(threadingModel);

        // disable the default setting of viewer.done() by pressing Escape.
        setKeyEventSetsDone(0);

//        QWidget* widget1 = addViewWidget( createGraphicsWindow(0,0,800,800), osgDB::readRefNodeFile("./data/red_flag.osg") );
//        QWidget* widget2 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readRefNodeFile("glider.osgt") );
//        QWidget* widget3 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readRefNodeFile("axes.osgt") );
//        QWidget* widget4 = addViewWidget( createGraphicsWindow(0,0,100,100), osgDB::readRefNodeFile("fountain.osgt") );
        QWidget* popupWidget = addViewWidget( createGraphicsWindow(900,100,320,240,"Popup window",true), osgDB::readNodeFile("./data/red_flag.osg") );
        popupWidget->show();

        QHBoxLayout* grid = new QHBoxLayout(this);
//        grid->addWidget( widget1);
//        grid->addWidget( widget2, 0, 1 );
//        grid->addWidget( widget3, 1, 0 );
//        grid->addWidget( widget4, 1, 1 );
        setLayout( grid );

        connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
        _timer.start( 10 );
    }

    QWidget* addViewWidget( osgQt::GraphicsWindowQt* gw, osg::ref_ptr<osg::Node> scene )
    {
        osgViewer::View* view = new osgViewer::View;
        addView( view );

        osg::Camera* camera = view->getCamera();
        camera->setGraphicsContext( gw );

        const osg::GraphicsContext::Traits* traits = gw->getTraits();

        camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
        camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );

        // set the draw and read buffers up for a double buffered window with rendering going to back buffer
        camera->setDrawBuffer(GL_BACK);
        camera->setReadBuffer(GL_BACK);

        camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );

        view->setSceneData( scene );
        view->addEventHandler( new osgViewer::StatsHandler );
        view->setCameraManipulator( new osgGA::MultiTouchTrackballManipulator );
        gw->setTouchEventsEnabled( true );
        return gw->getGLWidget();
    }

    osgQt::GraphicsWindowQt* createGraphicsWindow( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false )
    {
        osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
        osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
        traits->windowName = name;
        traits->windowDecoration = windowDecoration;
        traits->x = x;
        traits->y = y;
        traits->width = w;
        traits->height = h;
        traits->doubleBuffer = true;
        traits->alpha = ds->getMinimumNumAlphaBits();
        traits->stencil = ds->getMinimumNumStencilBits();
        traits->sampleBuffers = ds->getMultiSamples();
        traits->samples = ds->getNumMultiSamples();

        return new osgQt::GraphicsWindowQt(traits.get());
    }

    virtual void paintEvent( QPaintEvent* /*event*/ )
    { frame(); }

protected:

    QTimer _timer;
};

int main(int argc, char *argv[]) {
    osg::setNotifyLevel(osg::NotifySeverity::DEBUG_FP);

    //初始化QGIS应用，前缀路径修改为 qgis 的安装路径
    QgsApplication app(argc, argv, true);
    QgsApplication::setPrefixPath("G:/program/osgeo4w/apps/qgis-ltr-dev", true);
    QgsApplication::initQgis();

    // 初始化 osgEarth
//    osgEarth::initialize();

    // 新建主的 widget 及布局
    QWidget *mainWidget = new QWidget;
    QHBoxLayout *mainLayout = new QHBoxLayout(mainWidget);

    // osgEarth widget
    osg::ArgumentParser arguments(&argc, argv);

    ViewerWidget* viewWidget = new ViewerWidget(mainWidget, Qt::Widget);

    // qgis widget，加载 world.shp
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
//    mainLayout->addWidget(viewWidget);

    // 显示
    mainWidget->show();

    return app.exec();
}