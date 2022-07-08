//
// Created by TictorDC on 2022/6/22.
//

#include <osgEarth/Style>
#include <osgEarth/ModelNode>
#include <osgEarth/FeatureImageLayer>
#include <osgEarth/GDAL>
#include <osgEarth/ImageLayer>

// 特效
#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <osgDB/ReadFile>

#include "MapLayerManager.h"

using namespace osgEarth;
using namespace osg;
using namespace osgParticle;
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

    bool MapLayerManager::addEntity(osg::Node *entityNode) {
        if (entityNode == nullptr) {
            return false;
        }
        if (!mapNode.valid()) {
            return false;
        }
        mapNode->addChild(entityNode);
        return true;
    }

    bool MapLayerManager::addEntity(osgEarth::ModelNode *entityNode) {
        if (entityNode == nullptr) {
            return false;
        }
        if (!mapNode.valid()) {
            return false;
        }
        mapNode->addChild(entityNode);
        return true;
    }

    bool MapLayerManager::loadEarthFile(const string &filePath) {
        earthNode = readNodeFile(filePath);
        if (!earthNode.valid()) {
            // 读取文件失败
            return false;
        }
        mapNode = MapNode::findMapNode(earthNode);
        if (!earthNode.valid()) {
            // 加载的地球文件不包含 MapNode
            return false;
        }
        return true;
    }

    bool MapLayerManager::showMapLayer(const string &layerName) {
        return setMapLayerVisibility(layerName, true);
    }

    bool MapLayerManager::hideMapLayer(const string &layerName) {
        return setMapLayerVisibility(layerName, false);
    }

    bool MapLayerManager::setMapLayerVisibility(const string &layerName, bool visibility) {
        Layer *layer = findLayerByName(layerName);
        if (layer == nullptr) {
            // 不存在指定名字的图层
            return false;
        }
        layer->setEnabled(visibility);
        return true;
    }

    bool MapLayerManager::addImageLayer(const string &fileUrl,
                                        const string &layerName) {
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

    bool MapLayerManager::addElevationLayer(const string &fileUrl,
                                            const string &layerName) {
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

    bool MapLayerManager::delLayerByName(const string &layerName) {
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

    Layer *MapLayerManager::findLayerByName(const string &layerName) {
        if (!mapNode.valid()) {
            // 没有地图图层
            return nullptr;
        }
        mapNode->open(); // TODO: 这行是否必要？
        Map *map = mapNode->getMap();
        Layer *layer = map->getLayerByName(layerName);
        return layer;
    }

    bool MapLayerManager::addShapefileLayer(const string &fileUrl, const string &layerName) {
        auto *data = new OGRFeatureSource;
        data->setURL(fileUrl);
        data->options().buildSpatialIndex() = true;
        if (data->open().isError()) {
            return false;
        }

        Style style;

        auto *alt = style.getOrCreate<AltitudeSymbol>();
        alt->clamping() = alt->CLAMP_TO_TERRAIN; // 矢量贴地
        alt->binding() = alt->BINDING_VERTEX; // 矢量文件的每个顶点独立贴合

        auto *render = style.getOrCreate<RenderSymbol>();

        auto *line = style.getOrCreate<LineSymbol>();
        line->stroke()->color() = Color(Color::Yellow, 0.5f);
        line->stroke()->width() = 7.5f;
        line->stroke()->widthUnits() = Units::METERS;

        auto *polygon = style.getOrCreate<PolygonSymbol>();
        polygon->fill()->color() = Color(Color::Cyan, 0.4f);
        polygon->outline() = true;

        auto *point = style.getOrCreateSymbol<PointSymbol>();
        point->fill()->color() = Color::Blue;
        point->size() = 8;

        auto *extrusion = style.getOrCreate<ExtrusionSymbol>();
        extrusion->height() = 250000.0;

        // 矢量图层的瓦片加载配置
        // 设置一个分页布局，用于增量加载。瓦片大小系数和能见度范围共同决定了瓦片的大小
        // tile radius = max range / tile size factor
        // 适用于 FeatureModelLayer
        // FeatureDisplayLayout layout;
        // layout.tileSize() = 500;

        auto *layer = new FeatureImageLayer();
        layer->setName(layerName);
        layer->setFeatureSource(data);
        layer->setStyleSheet(new StyleSheet());
        layer->getStyleSheet()->addStyle(style);

        mapNode->open(); // TODO: 这行是否必要？
        Map *map = mapNode->getMap();
        map->addLayer(layer);
        map->addLayer(data);
        return true;
    }

    bool MapLayerManager::writeShapefile(const osgEarth::Util::OGRFeatureSource *srcFeatureSource,
                                         const string &filePath) {
        // create output shapefile
        FeatureSchema outSchema;
        outSchema = srcFeatureSource->getSchema();
        osg::ref_ptr<OGRFeatureSource> output = new OGRFeatureSource();
        output->setOGRDriver("ESRI Shapefile");
        output->setURL(filePath);
        if (output->create(srcFeatureSource->getFeatureProfile(), outSchema, srcFeatureSource->getGeometryType(),
                           nullptr).isError()) {
            return false;
        }
        return false;
    }

    bool MapLayerManager::addExplosion(const Vec3 &position, const Vec3 &wind, float scale, float intensity) {
        auto *explosion = new ExplosionEffect(position, scale, intensity);
        explosion->setWind(wind);
        return addEntity(explosion);
    }

    bool MapLayerManager::addExplosionDebris(const Vec3 &position, const Vec3 &wind, float scale, float intensity) {
        auto *explosionDebris = new ExplosionDebrisEffect(position, scale, intensity);
        explosionDebris->setWind(wind);
        explosionDebris->getEmitter()->setEndless(true);
        explosionDebris->getEmitter()->setLifeTime(1);
        return addEntity(explosionDebris);
    }

    bool MapLayerManager::addSmoke(const Vec3 &position, const Vec3 &wind, float scale, float intensity) {
        auto *smoke = new SmokeEffect(position, scale, intensity);
        smoke->setWind(wind);
        return addEntity(smoke);
    }

    bool MapLayerManager::addFire(const Vec3 &position, const Vec3 &wind, float scale, float intensity) {
        auto *fire = new FireEffect(position, scale, intensity);
        fire->setWind(wind);
        return addEntity(fire);
    }

    osg::AnimationPath *MapLayerManager::createAnimationPath(const Vec3 &center, float radius, double loopTime) {
        auto *animationPath = new osg::AnimationPath;
        animationPath->setLoopMode(osg::AnimationPath::LOOP); // 顺方向循环模式

        int numSamples = 40; // 路径上共 40 个节点
        float yaw = 0.0;
        float yaw_delta = 2.0f * osg::PIf / ((float) numSamples - 1.0f); // 偏航差分
        float roll = osg::inDegrees(30.0f); // 翻滚角

        double time = 0.0;
        double time_delta = loopTime / (double) numSamples; // 每一段的时间差
        for (int i = 0; i < numSamples; ++i) {
            osg::Vec3 position(center + osg::Vec3(sinf(yaw) * radius, cosf(yaw) * radius, 0));
            osg::Quat rotation(osg::Quat(roll, osg::Vec3(0.0, 1.0, 0.0)) *
                               osg::Quat(-(yaw + osg::inDegrees(90.0f)), osg::Vec3(0.0, 0.0, 1.0)));

            animationPath->insert(time, osg::AnimationPath::ControlPoint(position, rotation));

            yaw += yaw_delta;
            time += time_delta;

        }
        return animationPath;
    }

    osg::Node *MapLayerManager::createMovingModel(const string &filename,
                                                  const osg::Vec3 &center,
                                                  float radius,
                                                  double loopTime) {
        osg::AnimationPath *animationPath = createAnimationPath(center, radius, loopTime);
        osg::Group *group = new osg::Group;
        osg::Node *model = osgDB::readNodeFile(filename);

        if (model == nullptr) {
            // 模型加载失败
            return nullptr;
        }

        const osg::BoundingSphere &bs = model->getBound(); // 模型包围球
//        float size = radius / bs.radius() * 0.15f;
        float size = 1e4;

        auto *positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC); // 该值生命周期内静态不可变
        positioned->setMatrix(osg::Matrix::translate(-bs.center()) * // 平移到圆形路径中心
                              osg::Matrix::scale(size, size, size)); // 缩放

        positioned->addChild(model);

        auto *xform = new osg::PositionAttitudeTransform;
        xform->setDataVariance(osg::Object::DYNAMIC);
        xform->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        // 设置动画回调函数
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath, 0.0, 1.0f));
        xform->addChild(positioned);
        group->addChild(xform);

        mapNode->addChild(group);

        return group;
    }

} // MultiLayerTileMap