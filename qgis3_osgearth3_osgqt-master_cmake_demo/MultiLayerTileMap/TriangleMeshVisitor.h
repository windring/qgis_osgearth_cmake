//
// Created by TictorDC on 2022/7/15.
//

#ifndef TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
#define TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H

#include <osg/NodeVisitor>

namespace MultiLayerTileMap {

    class TriangleMeshVisitor : public osg::NodeVisitor {
    public:
        TriangleMeshVisitor(osg::NodeVisitor::TraversalMode traversalMode = TRAVERSE_ALL_CHILDREN);

        virtual void reset();

        osg::Vec3Array *getTriangleMesh() {
            return mesh.get();
        }

        void apply(osg::Geode &geode);

    protected:
        void applyDrawable(osg::Drawable *drawable);

        osg::ref_ptr<osg::Vec3Array> mesh;
    };

    struct TriangleMeshFunc
    {
        TriangleMeshFunc()
        {
            vertices = new osg::Vec3Array;
        }

        void operator()(const osg::Vec3 v1, const osg::Vec3 v2, const osg::Vec3 v3) const
        {
            vertices->push_back( v1 );
            vertices->push_back( v2 );
            vertices->push_back( v3 );
        }

        osg::ref_ptr< osg::Vec3Array > vertices;
    };
} // MultiLayerTileMap

#endif //TILEMAPMANAGERDEMO_TRIANGLEMESHVISITOR_H
