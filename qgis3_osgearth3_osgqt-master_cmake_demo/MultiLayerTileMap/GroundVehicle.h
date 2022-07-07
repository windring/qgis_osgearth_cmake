//
// Created by TictorDC on 2022/7/6.
//

#ifndef TILEMAPMANAGERDEMO_GROUNDVEHICLE_H
#define TILEMAPMANAGERDEMO_GROUNDVEHICLE_H

#include <osgEarth/ModelNode>
#include <osgGA/GUIEventHandler>
#include <osgParticle/SmokeEffect>
#include <osgViewer/Viewer>

#include "MapLayerManager.h"
#include "OGRManager.h"

namespace MultiLayerTileMap {
    class GroundVehicle {
    private:
        osgEarth::ModelNode *tankGroup = nullptr;
        osg::Vec3d startWorldPos; // 当前位置
        osg::Vec3d endWorldPos; // 目标位置
        double deltaHeight;
        MapLayerManager *mapLayerManager = nullptr;
        osgParticle::SmokeEffect *tankSmoke = nullptr;

        // 这辆坦克的速度需要根据 Shapefile 进行改变
        class SpeedManager {
            std::string shapefileDatasetName = "tankShapefile";
            int layerIndex = 0; // 通过 ogrinfo 命令可以查看 shapefile 的层信息
            int fieldIndex = 2; // 数据列
            std::string preType;
            OGRManager *ogrManager = nullptr;
            const osgEarth::SpatialReference *srs = nullptr; // 坐标参考系
            double defaultSpeed = 200;
            std::map<std::string, double> speedTable = {
                    {"Sparse Coverage Grassland", 150},
                    {"Medium Coverage Grassland", 100},
                    {"River Valley",              100},
                    {"Sand Regions",              200},
                    {"Seasonal Water",            50},
                    {"Lake",                      50},
                    {"River",                     50}
            };
        public:
            SpeedManager(OGRManager *_ogrManager, const osgEarth::SpatialReference *_srs, std::string _shapefileDatasetName) :
                    ogrManager(_ogrManager), srs(_srs), shapefileDatasetName(std::move(_shapefileDatasetName)) {
            }

            double getSpeedByWorldXYZ(const osg::Vec3d xyz) {
                osgEarth::GeoPoint point;
                point.fromWorld(srs, xyz);
                return getSpeedByLonLat(point.x(), point.y());
            }

            double getSpeedByLonLat(double lon, double lat) {
                std::vector<std::string> typeList = ogrManager->getFieldByLonLat(shapefileDatasetName,
                                                                                 layerIndex, fieldIndex, lon, lat);
                if (!typeList.empty()) {
                    // 存在要素标记
                    if (preType != typeList[0]) { // 更新标记类型
                        std::cout << "new type: " << typeList[0] << std::endl;
                        preType = typeList[0];
                    }
                    if (speedTable.find(typeList[0]) != speedTable.end()) {
                        // 存在速度预设
                        return speedTable[typeList[0]];
                    } else {
                        // 不存在速度预设
                        return defaultSpeed;
                    }
                } else {
                    // 没有找到要素标记
                    std::cout << "no label" << std::endl;
                    return defaultSpeed;
                }
            }
        } *speedManager;

    public:
        GroundVehicle(osgEarth::ModelNode *_tankGroup, MapLayerManager *_manager, osgParticle::SmokeEffect *_smoke,
                      double _deltaHeight, OGRManager *_ogrManager, const std::string &_shapefileDatasetName) :
                tankGroup(_tankGroup), mapLayerManager(_manager), tankSmoke(_smoke), deltaHeight(_deltaHeight),
                speedManager(new SpeedManager(_ogrManager, _manager->getEarthSRS(), _shapefileDatasetName)) {
            startWorldPos = {-268956, 5.2314e+06, 3.63454e+06 + deltaHeight};
            endWorldPos = {-268956, 5.2314e+06, 3.63454e+06 + deltaHeight};
            moveGUIEventHandler = new MoveGUIEventHandler(this);
        }

        [[nodiscard]] double getDeltaHeight() const {
            return deltaHeight;
        }

        [[nodiscard]] double getDeltaDistance(double deltaTime) {
            return deltaTime * speedManager->getSpeedByWorldXYZ(startWorldPos);
        }

        osgEarth::GeoPoint getNextGeoPos(double deltaDistance) {
            double minDistance = (startWorldPos - endWorldPos).length();
            if (minDistance < deltaDistance) {
                startWorldPos = endWorldPos;
            } else {
                osg::Vec3d iVector3d = endWorldPos - startWorldPos;
                iVector3d.normalize();
                startWorldPos += iVector3d * deltaDistance;

                // 简化为二维问题
                iVector3d.z() = 0;
                osg::Vec3d i = {0, 1, 0};
                double dot = i * iVector3d;
                double bottom = i.length() * iVector3d.length();
                double angle = acos(dot / bottom);
                double cross = (i ^ iVector3d).z();
                if (cross < 0) {
                    angle *= -1;
                }
                tankGroup->setLocalRotation(osg::Quat(angle, osg::Vec3({0, 0, 1})));
            }
            osgEarth::GeoPoint ret;
            ret.fromWorld(mapLayerManager->getEarthSRS(), startWorldPos);

            return ret;
        }

        void setEndWorldPos(osg::Vec3d pos) {
            endWorldPos = pos;
        }

        void updateWorldPos(double deltaTime) {
            double deltaDistance = getDeltaDistance(deltaTime);
            osgEarth::GeoPoint nextGeoPos = getNextGeoPos(deltaDistance);
            tankGroup->setPosition(nextGeoPos);
            tankSmoke->setPosition(startWorldPos - osg::Vec3d(0, 0, deltaHeight));
        }

        struct MoveGUIEventHandler : public osgGA::GUIEventHandler {
            osg::Timer_t timeTick = osg::Timer::instance()->tick();
            GroundVehicle* tank = nullptr;

            explicit MoveGUIEventHandler (GroundVehicle* _tank): tank(_tank) {}

            bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override {
                osg::Timer_t curTimeTick = osg::Timer::instance()->tick();
                double deltaS = osg::Timer::instance()->delta_s(timeTick, curTimeTick);
                timeTick = curTimeTick;
                if (tank) {
                    tank->updateWorldPos(deltaS);
                }
                switch (ea.getEventType()) {
                    case (osgGA::GUIEventAdapter::PUSH):
                        printf("push\n");
                        if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) {
                            // 设定目的地
                            moveWorldPos(ea, aa);
                        }
                        return false;
                    default:
                        return false;
                }
            }

            osg::Vec3d moveWorldPos(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) const {
                osg::Vec3d pos(0, 0, 0);
                auto *pViewer = dynamic_cast<osgViewer::Viewer *>(&aa);
                if (pViewer == nullptr) {
                    return pos;
                }
                // 获取当前点
                osgUtil::LineSegmentIntersector::Intersections intersection;
                pViewer->computeIntersections(ea.getX(), ea.getY(), intersection);
                auto iter = intersection.begin();
                if (iter != intersection.end()) {
                    osg::Vec3d worldPos = iter->getWorldIntersectPoint(); // 世界坐标
                    worldPos.z() += tank->getDeltaHeight();
                    if (tank) {
                        tank->setEndWorldPos(worldPos);
                    }
                    printf("worldPos: %g %g %g\n", worldPos.x(), worldPos.y(), worldPos.z());
                }
                return pos;
            }
        } *moveGUIEventHandler;
    };
}

#endif //TILEMAPMANAGERDEMO_GROUNDVEHICLE_H
