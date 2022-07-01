//
// Created by TictorDC on 2022/6/22.
//

#include <osgEarth/Style>
#include <osgEarth/ModelNode>
#include <osgEarth/EarthManipulator>
#include <osgEarth/ImageLayer>
#include <osgEarth/GDAL>
#include <osgDB/ReadFile>

#include "MapLayerManager.h"

using namespace osgEarth;
using namespace osg;
using namespace osgDB;
using namespace std;

namespace MultiLayerTileMap {

    bool MapLayerManager::addEntity(osg::Group *entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll) {
        return addEntity(entityNode->asNode(), lonLatAlt, picHeadingRoll);
    }

    bool MapLayerManager::addEntity(osg::Node *entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll) {
        if (entityNode == nullptr) {
            return false;
        }
        if (!mapNode.valid()) {
            return false;
        }
        Style style;
        auto *modelSymbol = style.getOrCreate<ModelSymbol>();
        modelSymbol->pitch() = picHeadingRoll.x();
        modelSymbol->heading() = picHeadingRoll.y();
        modelSymbol->roll() = picHeadingRoll.z();
        modelSymbol->setModel(entityNode);
        // TODO: maybe we can use osg::ref_ptr
        auto *modelNode = new ModelNode(mapNode, style, nullptr);
        const SpatialReference *geoSRS = mapNode->getMapSRS()->getGeographicSRS();
        modelNode->setPosition(GeoPoint(geoSRS, lonLatAlt, AltitudeMode::ALTMODE_ABSOLUTE));
        mapNode->addChild(modelNode);
        return true;
    }

    bool MapLayerManager::loadEarthFile(const string& filePath) {
        rootNode = readNodeFile(filePath);
        if (!rootNode.valid()) {
            // 读取文件失败
            return false;
        }
        mapNode = MapNode::findMapNode(rootNode);
        if (!rootNode.valid()) {
            // 加载的地球文件不包含 MapNode
            return false;
        }
        return true;
    }

    bool MapLayerManager::showMapLayer(const string& layerName) {
        return setMapLayerVisibility(layerName, true);
    }

    bool MapLayerManager::hideMapLayer(const string& layerName) {
        return setMapLayerVisibility(layerName, false);
    }

    bool MapLayerManager::setMapLayerVisibility(const string& layerName, bool visibility) {
        Layer *layer = findLayerByName(layerName);
        if (layer == nullptr) {
            // 不存在指定名字的图层
            return false;
        }
        layer->setEnabled(visibility);
        return true;
    }

    bool MapLayerManager::addImageLayer(const string& fileUrl,
                                        const string& layerName) {
        if (!mapNode.valid()) {
            // 没有地图图层
            return false;
        }
        if (findLayerByName(layerName) != nullptr) {
            // 已经存在这个名字的地图图层
            return false;
        }
        auto *imagery = new GDALImageLayer();
        imagery->setURL(fileUrl);
        imagery->setName(layerName);
        Status status = imagery->open();
        if (status.isError()) {
            // 图层有误
            return false;
        }
        mapNode->open();
        Map *map = mapNode->getMap();
        map->addLayer(imagery);
        return true;
    }

    bool MapLayerManager::addElevationLayer(const string& fileUrl,
                                            const string& layerName) {
        if (!mapNode.valid()) {
            // 没有地图图层
            return false;
        }
        auto *dem = new GDALElevationLayer();
        dem->setURL(fileUrl);
        dem->setName(layerName);
        Status status = dem->open();
        if (status.isError()) {
            // 图层有误
            return false;
        }
        mapNode->open();
        Map *map = mapNode->getMap();
        map->addLayer(dem);
        return true;
    }

    bool MapLayerManager::delLayerByName(const string& layerName) {
        Layer *layer = findLayerByName(layerName);
        if (layer == nullptr) {
            // 不存在指定名字的图层
            return false;
        }
        mapNode->open(); // TODO: 这行是否必要？
        Map *map = mapNode->getMap();
        map->removeLayer(layer);
        return true;
    }

    Layer *MapLayerManager::findLayerByName(const string& layerName) {
        if (!mapNode.valid()) {
            // 没有地图图层
            return nullptr;
        }
        mapNode->open(); // TODO: 这行是否必要？
        Map *map = mapNode->getMap();
        Layer *layer = map->getLayerByName(layerName);
        return layer;
    }
} // MultiLayerTileMap