//
// Created by TictorDC on 2022/7/15.
//

#ifndef TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
#define TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H

#include <iostream>

#include <osg/io_utils>
#include <osg/TriangleFunctor>

#include <osgEarth/AnnotationNode>
#include <osgEarth/DrapeableNode>
#include <osgEarthDrivers/engine_rex/SurfaceNode>

#include <btBulletDynamicsCommon.h>

namespace MultiLayerTileMap {

struct TriangleMeshFunc {
  TriangleMeshFunc() {
	vertices = new osg::Vec3Array;
  }

  void operator()(const osg::Vec3 v1, const osg::Vec3 v2, const osg::Vec3 v3) const {
	vertices->push_back(v1);
	vertices->push_back(v2);
	vertices->push_back(v3);
  }

  osg::ref_ptr<osg::Vec3Array> vertices;
};

struct NormalPrint {
  int triangleCount = 0;

  void operator()(const osg::Vec3 &v1, bool) const {
	std::cout << "\rpoint(" << v1 << ")" << std::endl;
  }

  void operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, bool) {
	std::cout << "\tline(" << v1 << ") (" << v2 << ")" << std::endl;
  }

  void operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool) {
	triangleCount += 1;
	osg::Vec3f normal = (v2 - v1) ^ (v3 - v2);
	normal.normalize();
	std::cout << "\ttriangle " << triangleCount << " (" << v1 << ") (" << v2 << ") (" << v3 << ") " << ") normal ("
			  << normal << ")" << std::endl;
  }

  void operator()(const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, const osg::Vec3 &v4, bool) const {
	osg::Vec3 normal = (v2 - v1) ^ (v3 - v2);
	normal.normalize();
	std::cout << "\tquad(" << v1 << ") (" << v2 << ") (" << v3 << ") (" << v4 << ") " << ")" << std::endl;
  }
};

struct FeatureNodeVisitor : public osg::NodeVisitor {
  std::vector<osg::ref_ptr<osg::Vec3Array>> list;
  osg::ref_ptr<osg::Vec3Array> mergedFeature = new osg::Vec3Array;
  osg::Vec3 origin;

  FeatureNodeVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
	setNodeMaskOverride(~0);
  }

  void apply(osg::Geode &geode) override {
	OSG_ALWAYS << "Apply osg::Geode: " << geode.getNumDrawables() << std::endl;
	for (unsigned int idx = 0; idx < geode.getNumDrawables(); idx++) {
	  applyDrawable(*geode.getDrawable(idx));
	}
  }

  void applyDrawable(osg::Drawable &drawable) {
	OSG_ALWAYS << "Visit: " << drawable.getCompoundClassName() << std::endl;
	osg::Geometry *geom = drawable.asGeometry();
	osg::TriangleFunctor<TriangleMeshFunc> tf;
	geom->accept(tf);
	list.emplace_back(tf.vertices);
  }

  void mergeAllFeature() {
	mergedFeature->clear();
	for (const auto &vertices : list) {
	  mergedFeature->insert(mergedFeature->end(), vertices->begin(), vertices->end());
	}
  }

  void reCalcCentroid() {
	float sumX = 0, sumY = 0, sumZ = 0;
	float originX = 0, originY = 0, originZ = 0;
	for (const auto &vec : *mergedFeature) {
	  sumX += vec[0];
	  sumY += vec[1];
	  sumZ += vec[2];
	}
	originX = sumX / (float)mergedFeature->size();
	originY = sumY / (float)mergedFeature->size();
	originZ = sumZ / (float)mergedFeature->size();
	for (auto &vec : *mergedFeature) {
	  vec[0] -= originX;
	  vec[1] -= originY;
	  vec[2] -= originZ;
	}
	origin = {originX, originY, originZ};
  }

  btTransform getTransform() {
	btTransform transform = btTransform::getIdentity();
	transform.setOrigin({origin[0], origin[1], origin[2]});
	return transform;
  }
};

} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
