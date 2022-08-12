//
// Created by TictorDC on 2022/8/9.
//

#include "HciCoordinate.h"

#include <osg/Camera>
#include <osgUtil/LineSegmentIntersector>

using namespace osgEarth;

namespace MultiLayerTileMap {
osg::Vec3d HciCoordinate::worldToWGS84(osg::Vec3d worldXYZ) {
  const SpatialReference *wgs84 = SpatialReference::get("wgs84");
  GeoPoint geoPoint;
  geoPoint.fromWorld(wgs84, worldXYZ);
  if (geoPoint.isValid()) {
	return geoPoint.vec3d();
  } else {
	osg::notify(osg::FATAL) << "[HciCoordinate::worldToWGS84]: Invalid GeoPoint." << std::endl;
	throw std::runtime_error("[HciCoordinate::worldToWGS84]: Invalid GeoPoint.");
  }
}
osg::Vec3d HciCoordinate::worldToScreen(osg::Vec3d worldXYZ, const osg::ref_ptr<osg::Camera> &camera) {
  /// \ref osgUtil::SceneView::projectObjectIntoWindow
  if (!camera.valid()) {
	osg::notify(osg::FATAL) << "[HciCoordinate::worldToScreen]: Invalid Camera." << std::endl;
	throw std::runtime_error("[HciCoordinate::worldToScreen]: Invalid Camera.");
  }
  if (camera->getViewport() == nullptr) {
	osg::notify(osg::FATAL) << "[HciCoordinate::worldToScreen]: Invalid Camera Viewport." << std::endl;
	throw std::runtime_error("[HciCoordinate::worldToScreen]: Invalid Camera Viewport.");
  }
  osg::Matrixd vPW = camera->getViewMatrix() *
	  camera->getProjectionMatrix() *
	  camera->getViewport()->computeWindowMatrix();
  return worldXYZ * vPW;
}
osg::Vec3d HciCoordinate::worldToENU(osg::Vec3d worldXYZ, osg::Vec3d centerXYZ) {
  /// \ref https://www.cnblogs.com/charlee44/p/15382659.html
  const SpatialReference *wgs84 = SpatialReference::get("wgs84");
  osg::Matrixd worldToLocal;
  GeoPoint centerPoint;
  centerPoint.fromWorld(wgs84, centerXYZ);
  if (!centerPoint.isValid()) {
	osg::notify(osg::FATAL) << "[HciCoordinate::worldToENU]: Invalid centerXYZ." << std::endl;
	throw std::runtime_error("[HciCoordinate::worldToENU]: Invalid centerXYZ.");
  }
  centerPoint.createWorldToLocal(worldToLocal);
  return worldToLocal.preMult(worldXYZ);
}
osg::Vec3d HciCoordinate::screenToWorld(float x, float y, osgViewer::Viewer *viewer) {
  if (viewer == nullptr) {
	osg::notify(osg::FATAL) << "[HciCoordinate::screenToWorld]: Invalid Viewer." << std::endl;
	throw std::runtime_error("[HciCoordinate::screenToWorld]: Invalid Viewer.");
  }
  osgUtil::LineSegmentIntersector::Intersections intersection;
  viewer->computeIntersections(x, y, intersection, ~0);
  auto iter = intersection.begin();
  if (iter != intersection.end()) {
	osg::Vec3d worldPos = iter->getWorldIntersectPoint(); // 世界坐标
	return worldPos;
  } else {
	osg::notify(osg::FATAL) << "[HciCoordinate::screenToWorld]: Invalid Intersections." << std::endl;
	throw std::runtime_error("[HciCoordinate::screenToWorld]: Invalid Intersections.");
  }
}
osg::Vec3d HciCoordinate::wGS84ToWorld(osg::Vec3d wgsLLA) {
  const SpatialReference *wgs84 = SpatialReference::get("wgs84");
  GeoPoint geoPoint(wgs84, wgsLLA);
  osg::Vec3d result;
  geoPoint.toWorld(result);
  return result;
}
} // MultiLayerTileMap