//
// Created by TictorDC on 2022/7/15.
//
#include "TriangleMeshVisitor.h"

namespace MultiLayerTileMap {
    /// \note:
    /// ->traverse Rex::TileNode
    /// ->Rex::SurfaceNode accept
    /// ->osg::Group accept
    /// ->osg::Group traverse *this
    /// x no  REX::SurfaceNode accept, only osg::Group accept
    /// ->TileDrawable accept->
} // MultiLayerTileMap