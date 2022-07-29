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
  osgParticle::SmokeEffect *tankSmoke = nullptr;
  MapLayerManager *mapLayerManager = nullptr;
  osg::MatrixTransform* smokeTrans = nullptr;
  osg::Geode *smokeGeode = nullptr;
  osg::MatrixTransform* tankTrans = nullptr;
  osg::Node *pureModel = nullptr;
  osg::Group * tankAndSmokeGroup = nullptr;
  osg::Vec3d startWorldPos;
  // 当前位置
  osg::Vec3d endWorldPos; // 目标位置
  double deltaHeight;

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
		{"Sparse Coverage Grassland", 100},
		{"Medium Coverage Grassland", 200},
		{"River Valley", 200},
		{"Sand Regions", 300},
		{"Seasonal Water", 50},
		{"Lake", 50},
		{"River", 50}};

   public:
	SpeedManager(OGRManager *_ogrManager, const osgEarth::SpatialReference *_srs,
				 std::string _shapefileDatasetName) :
		ogrManager(_ogrManager), srs(_srs), shapefileDatasetName(std::move(_shapefileDatasetName)) {
	}

	/// 获取当前世界坐标所对应的速度，该速度根据 Shapefile 的标注进行改变，例如草地会更慢，标注-速率关系参见 speedTable
	/// \param xyz 世界坐标
	/// \return 速率
	double getSpeedByWorldXYZ(osg::Vec3d xyz);

	/// 获取当前 WSG84 经纬度坐标对应的速度，该速度根据 Shapefile 的标注进行改变，例如草地会更慢，标注-速率关系参见 speedTable
	/// \param lon 经度
	/// \param lat 纬度
	/// \return 速率
	double getSpeedByLonLat(double lon, double lat);
  } *speedManager = nullptr;

 public:
  /// 载具构造函数，构造载具的同时加入到地图当中
  /// \param _mapLayerManager 地图管理器
  /// \param _ogrManager OGR 资源管理器
  /// \param filename 载具模型文件
  /// \param lonLatAlt 初始地理坐标
  /// \param picHeadingRoll 姿势角度
  /// \param scaleFactor 缩放因子
  /// \param _shapefileDatasetName 速度管理所依赖的 Shapefile 数据集名字，该名字在 OGR 资源管理器中加载 Shapefile 时指定
  GroundVehicle(MapLayerManager *_mapLayerManager,
				OGRManager *_ogrManager,
				const std::string &filename,
				const osg::Vec3d &lonLatAlt,
				const osg::Vec3d &picHeadingRoll,
				const osg::Vec3d &scaleFactor,
				const std::string &_shapefileDatasetName);

  /// 获取载具包络中心点到底部的距离
  /// \return 载具包络中心点到底部的距离
  [[nodiscard]] double getDeltaHeight() const;

  /// 根据间隔时间获取移动的距离
  /// \param deltaTime 间隔时间
  /// \return 移动距离
  [[nodiscard]] double getDeltaDistance(double deltaTime);

  /// 根据移动距离获取下一个移动位置
  /// \param deltaDistance 移动距离
  /// \return 下一个位置点
  osgEarth::GeoPoint getNextGeoPos(double deltaDistance);

  /// 设置目标位置
  /// \param pos
  void setEndWorldPos(osg::Vec3d pos) {
	endWorldPos = pos;
  }

  /// 获取模型 BB 盒，不包括烟雾部分
  /// \return
  osg::ref_ptr<osg::Box> getPureBoundingBox();

  /// 根据间隔时间更新载具位置
  /// 步骤：1. 根据间隔时间获取移动距离
  ///      2. 根据移动距离获取下一位置点
  ///      3. 更新载具和特效到下一位置点
  /// \param deltaTime 间隔时间
  void updateWorldPos(double deltaTime);

  /// 获取载具当前位置
  /// \return
  const osg::Vec3d &getStartWorldPos() const;

  /// 获取坦克的 MatrixTransform
  /// \return
  osg::MatrixTransform *getTankMatrixTransform() const;

  /// 载具的事件处理对象
  /// 用例：
  /// widget->getOsgViewer()->addEventHandler(dynamic_cast<osgGA::EventHandler *>(moveGUIEventHandler));
  struct MoveGUIEventHandler : public osgGA::GUIEventHandler {
	osg::Timer_t timeTick = osg::Timer::instance()->tick();
	GroundVehicle *tank = nullptr;

	/// 构造载具事件处理对象
	/// \param _tank 载具
	explicit MoveGUIEventHandler(GroundVehicle *_tank) : tank(_tank) {}

	/// 载具事件处理函数
	/// \param ea osgGA::GUIEventAdapter
	/// \param aa osgGA::GUIActionAdapter
	/// \return
	bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) override;

	/// 根据鼠标中键选点，设定载具的目的位置
	/// \param ea osgGA::GUIEventAdapter
	/// \param aa osgGA::GUIActionAdapter
	/// \return 目的位置的世界坐标，失败时返回 (0, 0, 0)
	osg::Vec3d moveWorldPos(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa) const;
  } *moveGUIEventHandler;
};
}

#endif //TILEMAPMANAGERDEMO_GROUNDVEHICLE_H
