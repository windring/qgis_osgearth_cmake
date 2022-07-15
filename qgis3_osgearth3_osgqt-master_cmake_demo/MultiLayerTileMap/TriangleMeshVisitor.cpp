//
// Created by TictorDC on 2022/7/15.
//
#include <osg/Geode>
#include <osg/TriangleFunctor>

#include "TriangleMeshVisitor.h"

namespace MultiLayerTileMap {
    TriangleMeshVisitor::TriangleMeshVisitor(osg::NodeVisitor::TraversalMode traversalMode) :
        NodeVisitor( traversalMode) {
        mesh = new osg::Vec3Array;
    }

    void TriangleMeshVisitor::reset() {
        mesh->clear();
    }

    void TriangleMeshVisitor::apply(osg::Geode &geode) {
        for(unsigned int idx = 0; idx < geode.getNumDrawables(); idx++ ) {
            applyDrawable(geode.getDrawable(idx));
        }
    }

    void TriangleMeshVisitor::applyDrawable(osg::Drawable *drawable) {
        osg::TriangleFunctor< TriangleMeshFunc > functor;
        drawable->accept( functor );

        osg::Matrix m = osg::computeLocalToWorld( getNodePath() );
        for(auto iter = functor.vertices->begin(); iter != functor.vertices->end(); ++iter)
        {
            mesh->push_back( *iter * m );
        }
    }
} // MultiLayerTileMap