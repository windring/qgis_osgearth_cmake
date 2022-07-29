//
// Created by TictorDC on 2022/7/6.
//
#include <osg/ComputeBoundsVisitor>
#include <osgDB/ReadFile>

#include <osgEarth/ModelNode>
#include <osgEarth/ModelSymbol>

#include "GroundVehicle.h"

using namespace osgDB;
using namespace osgEarth;

namespace MultiLayerTileMap {

double GroundVehicle::SpeedManager::getSpeedByWorldXYZ(const osg::Vec3d xyz) {
  osgEarth::GeoPoint point;
  point.fromWorld(srs, xyz);
  return getSpeedByLonLat(point.x(), point.y());
}

double GroundVehicle::SpeedManager::getSpeedByLonLat(double lon, double lat) {
  if (ogrManager == nullptr) {
	return defaultSpeed;
  }
  std::vector<std::string> typeList = ogrManager->getFieldByLonLat(shapefileDatasetName,
																   layerIndex, fieldIndex, lon, lat);
  if (!typeList.empty()) {
	// 存在要素标记
	if (preType != typeList[0]) { // 更新标记类型
	  std::cout << "new type: " << typeList[0] << std::endl;
	  preType = std::string(typeList[0]);
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

GroundVehicle::GroundVehicle(MapLayerManager *_mapLayerManager,
							 OGRManager *_ogrManager,
							 const std::string &filename,
							 const osg::Vec3d &lonLatAlt,
							 const osg::Vec3d &picHeadingRoll,
							 const osg::Vec3d &scaleFactor,
							 const std::string &_shapefileDatasetName) :
	mapLayerManager(_mapLayerManager),
	speedManager(new SpeedManager(_ogrManager, _mapLayerManager->getEarthSRS(), _shapefileDatasetName)) {

  pureModel = readNodeFile(filename);
  osgEarth::Registry::shaderGenerator().run(pureModel);
  pureModel->getOrCreateStateSet()->setMode(GL_RESCALE_NORMAL, osg::StateAttribute::ON);
  pureModel->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);

  tankTrans = new osg::MatrixTransform;
  tankTrans->setMatrix(osg::Matrix::scale(scaleFactor));
  tankTrans->addChild(pureModel);

  // 计算包围盒
  osg::ComputeBoundsVisitor cbv;
  tankTrans->accept(cbv);
  osg::BoundingBox modelBB = cbv.getBoundingBox();
  deltaHeight = modelBB.center().z() - modelBB.zMin();
  osg::Vec3d newLonLatAlt = lonLatAlt + osg::Vec3d{0, 0, deltaHeight};

  const osg::Vec3d smokePosition = {0, 0, 0};
  const osg::Vec3d smokeWind{0, 0, 10};
  const float smokeScale = 10;
  const float smokeIntensity = 10;
  const double smokeDuration = 4;
  tankSmoke = new osgParticle::SmokeEffect(smokePosition, smokeScale, smokeIntensity);
  tankSmoke->setWind(smokeWind);
  tankSmoke->setParticleDuration(smokeDuration);
  tankSmoke->getEmitter()->setEndless(true); // 一直冒灰
  tankSmoke->setUseLocalParticleSystem(false);
  smokeTrans = new osg::MatrixTransform;
  smokeTrans->setMatrix(osg::Matrix::identity());
  smokeTrans->addChild(tankSmoke);
  smokeGeode = new osg::Geode;
  smokeGeode->addDrawable(tankSmoke->getParticleSystem());
  mapLayerManager->getMapNode()->addChild(smokeGeode);

  tankAndSmokeGroup = new osg::Group;
  tankAndSmokeGroup->addChild(tankTrans);
  tankAndSmokeGroup->addChild(smokeTrans);

  Style style;
  auto *modelSymbol = style.getOrCreate<ModelSymbol>();
  modelSymbol->pitch() = picHeadingRoll.x();
  modelSymbol->heading() = picHeadingRoll.y();
  modelSymbol->roll() = picHeadingRoll.z();
  modelSymbol->setModel(tankAndSmokeGroup);

  auto mapNode = mapLayerManager->getMapNode();
  tankGroup = new ModelNode(mapNode, style, nullptr);
  const SpatialReference *geoSRS = mapLayerManager->getEarthSRS();
  GeoPoint tankPos(geoSRS, newLonLatAlt, AltitudeMode::ALTMODE_RELATIVE);
  tankGroup->setPosition(tankPos);

  mapLayerManager->addEntity(tankGroup);

  osg::Vec3d osgWorldPos;
  tankPos.toWorld(osgWorldPos);
  startWorldPos = osgWorldPos;
  endWorldPos = osgWorldPos;
  moveGUIEventHandler = new MoveGUIEventHandler(this);
}

[[nodiscard]] double GroundVehicle::getDeltaHeight() const {
  return deltaHeight;
}

[[nodiscard]] double GroundVehicle::getDeltaDistance(double deltaTime) {
  return deltaTime * speedManager->getSpeedByWorldXYZ(startWorldPos);
}

osgEarth::GeoPoint GroundVehicle::getNextGeoPos(double deltaDistance) {
  osgEarth::GeoPoint ret;
  if (tankGroup == nullptr) {
	// 初始化不正确
	osg::notify(osg::DEBUG_INFO) << "getNextGeoPos: tankGroup == nullptr" << std::endl;
	return ret;
  }
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
  ret.fromWorld(mapLayerManager->getEarthSRS(), startWorldPos);

  return ret;
}

void GroundVehicle::updateWorldPos(double deltaTime) {
  if (tankGroup == nullptr || tankSmoke == nullptr) {
	// 初始化不正确
	osg::notify(osg::DEBUG_INFO) << "updateWorldPos: tankGroup == nullptr || tankSmoke == nullptr" << std::endl;
	return;
  }
  double deltaDistance = getDeltaDistance(deltaTime);
  osgEarth::GeoPoint nextGeoPos = getNextGeoPos(deltaDistance);
  tankGroup->setPosition(nextGeoPos);
}

osg::ref_ptr<osg::Box> GroundVehicle::getPureBoundingBox() {
  osg::ComputeBoundsVisitor cbv;
  pureModel->accept(cbv);
  osg::BoundingBox modelBB = cbv.getBoundingBox();
  osg::ref_ptr<osg::Box> box = new osg::Box(modelBB.center(),
											std::abs(modelBB.xMax() - modelBB.xMin()),
											std::abs(modelBB.yMax() - modelBB.yMin()),
											std::abs(modelBB.zMax() - modelBB.zMin()));
  return box;
}
const osg::Vec3d &GroundVehicle::getStartWorldPos() const {
  return startWorldPos;
}
osg::MatrixTransform *GroundVehicle::getTankMatrixTransform() const {
  return tankGroup->getGeoTransform();
}

bool GroundVehicle::MoveGUIEventHandler::handle(const osgGA::GUIEventAdapter &ea,
												osgGA::GUIActionAdapter &aa) {
  osg::Timer_t curTimeTick = osg::Timer::instance()->tick();
  double deltaS = osg::Timer::instance()->delta_s(timeTick, curTimeTick);
  timeTick = curTimeTick;
  if (tank) {
	tank->updateWorldPos(deltaS);
  }
  switch (ea.getEventType()) {
	case (osgGA::GUIEventAdapter::PUSH):
	  if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON) {
		// 设定目的地
		moveWorldPos(ea, aa);
	  }
	  return false;
	default:return false;
  }
}

osg::Vec3d GroundVehicle::MoveGUIEventHandler::moveWorldPos(const osgGA::GUIEventAdapter &ea,
															osgGA::GUIActionAdapter &aa) const {
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
}