//
// Created by TictorDC on 2022/6/22.
//

#ifndef TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H
#define TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H

namespace MultiLayerTileMap {

#include <osg/Node>
#include <osg/Group>
#include <osg/Vec3d>
#include <osgEarth/MapNode>
#include <string>

    class MapLayerManager {

        /// 地图节点
        osg::ref_ptr<osg::Node> rootNode;
        osg::ref_ptr<osgEarth::MapNode> mapNode;

    public:
        /// 加载指定的 .earth 文件
        /// \param file
        /// \return .earth 文件是否成功加载
        ///     \retval true 是
        ///     \retval false 否
        bool loadEarthFile(std::string file);

        /// 实体添加函数，传入类型为 osg 组节点
        /// \param entityNode 实体模型节点
        /// \param lonLatAlt 经、纬、高，高度为贴地高度
        /// \param picHeadingRoll 俯仰角、翻滚角、航向角
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osg:: Group* entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll);

        /// 实体添加函数，传入类型为 osg 叶节点
        /// \param entityNode 实体模型节点
        /// \param lonLatAlt 经、纬、高，高度为贴地高度
        /// \param picHeadingRoll 俯仰角、翻滚角、航向角
        /// \return 实体是否添加成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addEntity(osg::Node* entityNode, osg::Vec3d lonLatAlt, osg::Vec3d picHeadingRoll);

        /// 显示指定名字的地球地图图层
        /// \param layerName 图层的名字
        /// \return 图层是否显示成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool showMapLayer(std::string layerName);

        /// 隐藏指定名字的地球地图图层
        /// \param layerName 图层的名字
        /// \return 图层是否隐藏成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool hideMapLayer(std::string layerName);

        /// 设置指定名字的地球地图图层的可见性
        /// \param layerName 图层的名字
        /// \param visibility 图层可见性
        /// \return 可见性是否设置成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool setMapLayerVisibility(std::string layerName, bool visibility);

        /// 在当前地球地图中添加普通图片层
        /// \param fileUrl 图片路径
        /// \param layerName 新图层的名字
        /// \return 图层添加是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addImageLayer(std::string fileUrl, std::string layerName);

        /// 在当前地球地图中添加普通高程数据层
        /// \param fileUrl 高程数据路径
        /// \param layerName 新图层的名字
        /// \return 高程数据层添加是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool addElevationLayer(std::string fileUrl, std::string layerName);

        /// 删除指定名字的图层
        /// \param layerName 图层名字
        /// \return 删除是否成功
        ///     \retval true 成功
        ///     \retval false 失败
        bool delLayerByName(std::string layerName);
    };

} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_MAPLAYERMANAGER_H
