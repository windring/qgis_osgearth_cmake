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

//    void TriangleMeshVisitor::apply(osgEarth::REX::SurfaceNode &surfaceNode) {
//        applyDrawable(surfaceNode.getDrawable());
//    }

    void TriangleMeshVisitor::applyDrawable(osg::Drawable *drawable) {
        osg::TriangleFunctor< TriangleMeshFunc > functor;
        drawable->accept( functor );

        osg::Matrix m = osg::computeLocalToWorld( getNodePath() );
        for(auto iter = functor.vertices->begin(); iter != functor.vertices->end(); ++iter)
        {
            mesh->push_back( *iter * m );
        }
    }

    void TriangleMeshVisitor::apply(osg::Node& node) {
        _indent++;
//        for (int i = 0; i < _indent; i++) {
//            putchar(' ');
//        }
//        printf("apply node:  libraryName: %s className: %s compoundClassNmae: %s\n",
//               node.libraryName(), node.className(), node.getCompoundClassName().c_str());
        // ->traverse Rex::TileNode
        // ->Rex::SurfaceNode accept
        // ->osg::Group accept
        // ->osg::Group traverse *this
        // x no  REX::SurfaceNode accept, only osg::Group accept
        // ->TileDrawable accept->
        if (dynamic_cast<osgEarth::REX::SurfaceNode*>(&node) != nullptr) {
            applyDrawable(dynamic_cast<osgEarth::REX::SurfaceNode*>(&node)->getDrawable());
        } else {
            traverse(node);
        }
        _indent--;
    }
} // MultiLayerTileMap