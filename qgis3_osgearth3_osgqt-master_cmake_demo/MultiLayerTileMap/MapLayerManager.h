//
// Created by TictorDC on 2022/6/22.
//

#ifndef TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H
#define TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H

#include <osg/Node>
#include <osg/Group>
#include <osg/Vec3d>
#include <osg/AnimationPath>
#include <osgEarth/MapNode>
#include <osgEarth/ModelNode>
#include <osgEarth/Layer>
#include <osgEarth/OGRFeatureSource>
#include <osgEarth/FeatureImageLayer>
#include <osgEarth/Ellipsoid>
#include <string>

namespace MultiLayerTileMap {

    class MapLayerManager {

        /// 地图节点
        osg::ref_ptr<osg::Node> earthNode;
        osg::ref_ptr<osgEarth::MapNode> mapNode;

    public:
        MapLayerManager() = default;

        /// 初始化地图管理器，向根群组添加地球节点
        /// \param file osgEarth 地球文件
        /// \param rootGroup 根群组
        explicit MapLayerManager(const std::string &file, osg::Group *rootGroup) {
            loadEarthFile(file);
            rootGroup->addChild(earthNode);
        }

        /// 返回主节点（地球节点）的智能指针
        /// \return 主节点的智能指针
        osg::ref_ptr<osg::Node> getRootNode() {
            return earthNode;
        }

        /// 返回地图节点的智能指针
        /// \return 地图节点的智能指针
        osg::ref_ptr<osgEarth::MapNode> getMapNode() {
            return mapNode;
        }

        /// 返回地球的 Ellipsoid
        /// \return 地球 Ellipsoid 的常量引用
        const osgEarth::Ellipsoid &getEarthEllipsoid() {
            return mapNode->getMapSRS()->getGeocentricSRS()->getEllipsoid();
        }

        /// 返回地球的 SRS（空间参考）
        /// \return
        const osgEarth::SpatialReference *getEarthSRS() {
            return mapNode->getMapSRS();
        }

        /// 加载指定的 .earth 文件
        /// \param file
        /// \return .earth 文件是否成功加载
        ///     \retval true 是
        ///     \retval false 否
        bool loadEarthFile(const std::string &file);

        /// 实体添加函数，传入类型为 osg 组节点
        /// \param entityNode 实体模型节点
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osg::Node *entityNode);

        /// 实体添加函数，传入类型为 osg 组节点
        /// \param entityNode 实体模型节点
        /// \param lonLatAlt 经、纬、高，高度为贴地高度
        /// \param picHeadingRoll 俯仰角、翻滚角、航向角
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osg::Group *entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll);

        /// 实体添加函数，传入类型为 osg 叶节点
        /// \param entityNode 实体模型节点
        /// \param lonLatAlt 经、纬、高，高度为贴地高度
        /// \param picHeadingRoll 俯仰角、翻滚角、航向角
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osg::Node *entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll);

        /// 实体添加函数，传入类型为 osgEarth::ModelNode 节点
        /// \param entityNode osgEarth::ModelNode 实体模型节点
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osgEarth::ModelNode *entityNode);

        /// 显示指定名字的地球地图图层
        /// \param layerName 图层的名字
        /// \return 图层是否显示成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool showMapLayer(const std::string &layerName);

        /// 隐藏指定名字的地球地图图层
        /// \param layerName 图层的名字
        /// \return 图层是否隐藏成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool hideMapLayer(const std::string &layerName);

        /// 设置指定名字的地球地图图层的可见性
        /// \param layerName 图层的名字
        /// \param visibility 图层可见性
        /// \return 可见性是否设置成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool setMapLayerVisibility(const std::string &layerName, bool visibility);

        /// 在当前地球地图中添加普通图片层
        /// \param fileUrl 图片路径
        /// \param layerName 新图层的名字
        /// \return 图层添加是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addImageLayer(const std::string &fileUrl, const std::string &layerName);

        /// 在当前地球地图中添加普通高程数据层
        /// \param fileUrl 高程数据路径
        /// \param layerName 新图层的名字
        /// \return 高程数据层添加是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addElevationLayer(const std::string &fileUrl, const std::string &layerName);

        /// 删除指定名字的图层
        /// \param layerName 图层名字
        /// \return 删除是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool delLayerByName(const std::string &layerName);

        /// 根据指定的图层名字查找地图图层
        /// \param layerName 图层名字
        /// \return 所查找的图层
        ///     \retval nullptr 查找失败，没有找到指定的图层
        ///     \retval 非空 指向地图图层的指针
        osgEarth::Layer *findLayerByName(const std::string &layerName);

        /// 向地图添加 PostGIS 中的 Shapefile 图层
        /// 根据字段生成类似：
        /// PG:"dbname='databasename' host='addr' port='5432' user='x' password='y' tables='table1'"
        /// 的连接字符串
        /// 关于 PG 连接字符串的更多信息：https://gdal.org/drivers/vector/pg.html
        /// \example
        /// \code
        ///     mapLayerManager->addShapefileLayerFromPostGIS(
        //          "host",
        //          "port",
        //          "userName",
        //          "password",
        //          "dbName",
        //          "tableName",
        //          "layerName")
        /// \param host 数据库主机地址
        /// \param port 数据库端口
        /// \param user 数据库用户名
        /// \param password 数据库密码
        /// \param dbname 数据库名
        /// \param table 显示的数据表名，默认 tables 和 table 相同，在底层实现中，tables 用于加载数据表，table 指定要素层使用的数据表
        /// \param layerName 图层名
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addShapefileLayerFromPostGIS(
                const std::string &host,
                const std::string &port,
                const std::string &user,
                const std::string &password,
                const std::string &dbname,
                const std::string &table,
                const std::string &layerName
                );

        /// 向地图添加 Shapefile 图层
        /// \param fileUrl 文件路径
        /// \param layerName 图层名字
        /// \return
		osgEarth::FeatureImageLayer* addShapefileLayer(const std::string &fileUrl, const std::string &layerName);

        /// 根据 OGRFeatureSource 对象，写出 shapefile 文件
        /// \param srcFeatureSource 源对象
        /// \param filePath 写出的文件路径
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool writeShapefile(const osgEarth::OGRFeatureSource *srcFeatureSource,
                            const std::string &filePath);


        /// 添加特效-爆炸
        /// \param position 经纬度位置
        /// \param wind 风向，采用 osg 世界坐标系
        /// \param scale 缩放
        /// \param intensity 密度
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addExplosion(const osg::Vec3 &position, const osg::Vec3 &wind, float scale, float intensity);

        /// 添加特效-爆炸碎片
        /// \param position 经纬度位置
        /// \param wind 风向，采用 osg 世界坐标系
        /// \param scale 缩放
        /// \param intensity 密度
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addExplosionDebris(const osg::Vec3 &position, const osg::Vec3 &wind, float scale, float intensity);

        /// 添加特效-烟雾
        /// \param position 经纬度位置
        /// \param wind 风向，采用 osg 世界坐标系
        /// \param scale 缩放
        /// \param intensity 密度
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addSmoke(const osg::Vec3 &position, const osg::Vec3 &wind, float scale, float intensity);

        /// 添加特效-烟雾尾迹
        /// \param position 经纬度位置
        /// \param wind 风向，采用 osg 世界坐标系
        /// \param scale 缩放
        /// \param intensity 密度
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addSmokeTrailEffect(const osg::Vec3 &position, const osg::Vec3 &wind, float scale, float intensity);

        /// 添加特效-火
        /// \param position 经纬度位置
        /// \param wind 风向，采用 osg 世界坐标系
        /// \param scale 缩放
        /// \param intensity 密度
        /// \return
        ///     \retval true 成功
        ///     \retval false 失败
        bool addFire(const osg::Vec3 &position, const osg::Vec3 &wind, float scale, float intensity);


        /// 创建圆形动画轨迹
        /// \param center 圆形轨迹中心点
        /// \param radius 圆形轨迹半径
        /// \param loopTime 循环时间
        /// \return
        static osg::AnimationPath *createAnimationPath(const osg::Vec3 &center, float radius, double loopTime);

        /// 创建沿圆形路径移动的模型
        /// \param filename 模型文件路径
        /// \param center 圆形路径中心点
        /// \param radius 圆形路径半径
        /// \param loopTime 循环时间
        /// \return
        osg::Node *
        createMovingModel(const std::string &filename, const osg::Vec3 &center, float radius, double loopTime);

        /// 返回最后一次向地图节点添加的子节点
        /// \return 最后一个子节点
        osg::Node *getLastChild() {
            return mapNode->getChild(mapNode->getNumChildren() - 1);
        }
    };
} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H
