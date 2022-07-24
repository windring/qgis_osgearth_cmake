// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using an OpenGL 3.0 context.
// The comment block at the end of the source describes building OSG
// for use with OpenGL 3.x.

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>
#include <osgUtil/Optimizer>

class DrawCallback : public osg::Drawable::DrawCallback {

    virtual void drawImplementation(osg::RenderInfo &renderInfo, const osg::Drawable *drawable) const {
        static double dColor = 0;//颜色
        glColor3f(dColor, 0, 0);

        glBegin(GL_TRIANGLES);//在OSG中画一个opengl三角形
        glVertex3f(-0.2, 0.0, -2.0);
        glVertex3f(0.2, 0.0, -2.0);
        glVertex3f(0.0, 0.4, -2.0);
        glEnd();

        dColor += 0.01;//颜色渐变
        if (dColor > 1.0) {
            dColor = 0.0;
        }
    }
};

int main( int argc, char** argv )
{
    osg::Geometry* geometry = new osg::Geometry;
    //此处一定要把显示列表设置为false,
    //否则DrawCallback的drawImplementation()函数只会调用一次，而不是在画一帧时都动态更新opengl图形
    geometry->setUseDisplayList(false);
    // Drawable设置动态更新opengl图形
    geometry->setDrawCallback(new DrawCallback);
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::Group* root = new osg::Group;
    root->addChild(geode);

    osgUtil::Optimizer optimizer;
    optimizer.optimize(root, osgUtil::Optimizer::ALL_OPTIMIZATIONS  | osgUtil::Optimizer::TESSELLATE_GEOMETRY);

    const int width( 800 ), height( 450 );
    const std::string version( "3.0" );
    osg::ref_ptr< osg::GraphicsContext::Traits > traits = new osg::GraphicsContext::Traits();
    traits->x = 20; traits->y = 30;
    traits->width = width; traits->height = height;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->glContextVersion = version;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();
    osg::ref_ptr< osg::GraphicsContext > gc = osg::GraphicsContext::createGraphicsContext( traits.get() );
    if( !gc.valid() )
    {
        osg::notify( osg::FATAL ) << "Unable to create OpenGL v" << version << " context." << std::endl;
        return( 1 );
    }

    osgViewer::Viewer viewer;
    viewer.setSceneData(root);
    osg::Matrix mt;
    mt.makeIdentity();
    while ( !viewer.done())
    {
        viewer.getCamera()->setViewMatrix( mt);
        viewer.frame();
    }
}

/*
Building OSG for OpenGL 3.x
OSG currently support OpenGL 3.x on Windows. This comment block describes the
necessary configuration steps.
Get the draft gl3.h header file from OpenGL.org and put it in a folder called
“GL3” somewhere on your hard drive. OSG includes this header as <GL3/gl3.h>. Get
gl3.h from here:
http://www.opengl.org/registry/
Open the cmake-gui and load OSG's top-level CmakeLists.txt. You'll need to make
several changes.
 * Add the path to <GL3/gl3.h> to the CMake compiler flags, CMAKE_CXX_FLAGS and
   CMAKE_CXX_FLAGS_DEBUG (for release and debug builds; others if you use other
   build configurations). The text to add should look something like this:
     /I “C:\GLHeader”
   The folder GLHeader should contain a subfolder GL3, which in turn contains
   gl3.h.
 * Enable the following CMake variable:
     OSG_GL3_AVAILABLE
 * Disable the following CMake variables:
     OSG_GL1_AVAILABLE
     OSG_GL2_AVAILABLE
     OSG_GLES1_AVAILABLE
     OSG_GLES2_AVAILABLE
     OSG_GL_DISPLAYLISTS_AVAILABLE
     OSG_GL_FIXED_FUNCTION_AVAILABLE
     OSG_GL_MATRICES_AVAILABLE
     OSG_GL_VERTEX_ARRAY_FUNCS_AVAILABLE
     OSG_GL_VERTEX_FUNCS_AVAILABLE
Create your project files in cmake-gui as usual, and build OSG as usual.
If you have an external project that will depend on OSG built for OpenGL 3.x,
you'll need to ensure your external project also uses the compiler include
directives to find <GL3/gl3.h>.
To verify your application is using a pure OpenGL 3.x context, set
OSG_NOTIFY_LEVEL=INFO in the environment and check the console output. Context
creation displays output such as the following:
    GL3: Attempting to create OpenGL3 context.
    GL3: version: 3.1
    GL3: context flags: 0
    GL3: profile: 0
    GL3: context created successfully.
When your app begins rendering, it displays information about the actual context
it is using:
    glVersion=3.1, isGlslSupported=YES, glslLanguageVersion=1.4
*/