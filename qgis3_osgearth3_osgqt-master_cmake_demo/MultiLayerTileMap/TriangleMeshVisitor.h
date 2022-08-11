//
// Created by TictorDC on 2022/7/15.
//

#ifndef TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
#define TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H

#include <osgEarthDrivers/engine_rex/SurfaceNode>
#include <iostream>
#include <osg/io_utils>
#include <osg/TriangleFunctor>
#include <osgEarth/AnnotationNode>
#include <osgEarth/DrapeableNode>

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

  FeatureNodeVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
	setNodeMaskOverride(~0);
  }

  void apply(osg::Geode &geode) override {
	OSG_ALWAYS << "Apply osg::Geode: " << geode.getNumDrawables() << std::endl;
	for (int idx = 0; idx < geode.getNumDrawables(); idx++) {
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
};

} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
