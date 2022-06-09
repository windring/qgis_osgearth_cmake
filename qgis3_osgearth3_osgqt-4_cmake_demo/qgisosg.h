//#pragma once
//
//#include <QtWidgets/QMainWindow>
////#include "ui_test_qgisosg.h"
//
//class test_qgisosg : public QMainWindow
//{
//    Q_OBJECT
//
//public:
//    test_qgisosg(QWidget *parent = Q_NULLPTR);
//
//private:
//    //Ui::test_qgisosgClass ui;
//};

#pragma once

#include <QtWidgets/QMainWindow>
//#include "ui_test_qgisosg.h"
#include "GraphicsWindowQt.h"

#include <QTimer>
#include <osgDB/ReadFile>
#include <osg/Group>
#include <osg/Node>
#include <osg/Camera>
#include <osg/PositionAttitudeTransform>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>

#include <osgEarth/MapNode>
#include <osgEarth/SpatialReference>

#include <osgEarth/GeoTransForm>
#include <osgEarth/GeoCommon>
#include <osgEarth/SpatialReference>
#include <osgEarth/EarthManipulator>
#include <osgEarth/Ephemeris>
#include <osgEarth/Sky>

#include <osgUtil/Tessellator>
#include <osgEarth/GLUtils>
#include <osg/Geode>
#include <osg/Geometry>

#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsrasterlayer.h>

#include <QList>
#include <QGroupBox>
#include <ctime>
#include <QStatusBar>
#include <QCheckBox>
#include <QPushButton>
#include <QSpinBox>
#include <QLineEdit>

class DigtalEarth : public QMainWindow {
Q_OBJECT

public:
    DigtalEarth(QWidget *parent = Q_NULLPTR);

    ~DigtalEarth();

private:
//    Ui::test_qgisosgClass ui;

private:
    QTimer *_timer;        // ��ʱ����ÿ5ms����һ���¼�
    osgViewer::Viewer *viewer;
    osg::ref_ptr<osg::Group> root;
    osg::ref_ptr<osg::Camera> camera;
    osg::ref_ptr<osg::Node> earthNode;
    osg::ref_ptr<osgEarth::MapNode> mapNode;
    osg::ref_ptr<osg::MatrixTransform> earthForm;
    osg::ref_ptr<osgEarth::EarthManipulator> em;
    tm *t_tm;
    osgEarth::SkyNode *m_pSkyNode;
    time_t now_time;

    QgsMapCanvas *mapCanvas;
    QList<QgsMapLayer *> layers;

private:
    void InitOSG();// ��ʼ������osg
    void InitUI();//�����ʼ��
    void InitOsgearth();//��ʼ��osgearth
    void InitSky();//��ճ�ʼ��
    void InitQgs();

private slots:

    // ��ʱ����֡�Ĳۺ���
    void updateFrame();

public:
    void InitTimer();//��Ļˢ�³�ʼ��
};