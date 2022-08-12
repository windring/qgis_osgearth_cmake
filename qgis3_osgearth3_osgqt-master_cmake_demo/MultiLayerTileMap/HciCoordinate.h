//
// Created by TictorDC on 2022/8/9.
//

#ifndef TILEMAPMANAGERDEMO_MULTILAYERTILEMAP_HCICOORDINATE_H_
#define TILEMAPMANAGERDEMO_MULTILAYERTILEMAP_HCICOORDINATE_H_

#include <osgEarth/GeoData>
#include <osgViewer/Viewer>

namespace MultiLayerTileMap {

class HciCoordinate {
  // toLongitudeLatitude()
  // 转换不同格式的坐标到经纬度坐标。
  // toENU(osg::Vec3d viewpoint)
  // 转换不同格式的坐标到北天东坐标。
  // toScreen()
  // 转换不同格式的坐标到屏幕坐标。
 public:
  /// 将 OSG 世界坐标转换为 WGS84 的经纬度坐标
  /// \param worldXYZ 物体在世界坐标系中的坐标
  /// \return
  static osg::Vec3d worldToWGS84(osg::Vec3d worldXYZ) noexcept(false);

  /// 将 WGS84 经纬度坐标转换为 OSG 世界坐标
  /// \param wgsLLA
  /// \return
  static osg::Vec3d wGS84ToWorld(osg::Vec3d wgsLLA);

  /// 将 OSG 世界坐标转换为屏幕坐标
  /// 屏幕坐标系是相对与窗口左下角的
  /// \note: 即使世界坐标在摄像机背面也会返回其屏幕坐标
  /// \param worldXYZ 物体在世界坐标系中的坐标
  /// \return
  static osg::Vec3d worldToScreen(osg::Vec3d worldXYZ, const osg::ref_ptr<osg::Camera> &camera) noexcept(false);

  /// 将 OSG 世界坐标系坐标转换为站心坐标系坐标
  /// \param worldXYZ 物体在世界坐标系中的坐标
  /// \param centerXYZ 站心在世界坐标系中的坐标
  /// \return
  static osg::Vec3d worldToENU(osg::Vec3d worldXYZ, osg::Vec3d centerXYZ);

  /// 获取第一个与屏幕相交的 OSG 世界点的世界坐标
  /// \param x
  /// \param y
  /// \return
  static osg::Vec3d screenToWorld(float x, float y, osgViewer::Viewer *view);

};

} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_MULTILAYERTILEMAP_HCICOORDINATE_H_
