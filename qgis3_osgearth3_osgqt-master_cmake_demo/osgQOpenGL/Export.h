#ifndef OSGQOPENGL_EXPORT_H
#define OSGQOPENGL_EXPORT_H

#define OSGQOPENGL_STATIC_DEFINE 0

#ifdef OSGQOPENGL_STATIC_DEFINE
#  define OSGQOPENGL_EXPORT
#  define OSGQOPENGL_NO_EXPORT
#else
#  ifndef OSGQOPENGL_EXPORT
#    ifdef osgQOpenGL_EXPORTS
/* We are building this library */
#      define OSGQOPENGL_EXPORT __declspec(dllexport)
#    else
/* We are using this library */
#      define OSGQOPENGL_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef OSGQOPENGL_NO_EXPORT
#    define OSGQOPENGL_NO_EXPORT
#  endif
#endif

#ifndef OSGQOPENGL_DEPRECATED
#  define OSGQOPENGL_DEPRECATED __declspec(deprecated)
#endif

#ifndef OSGQOPENGL_DEPRECATED_EXPORT
#  define OSGQOPENGL_DEPRECATED_EXPORT OSGQOPENGL_EXPORT OSGQOPENGL_DEPRECATED
#endif

#ifndef OSGQOPENGL_DEPRECATED_NO_EXPORT
#  define OSGQOPENGL_DEPRECATED_NO_EXPORT OSGQOPENGL_NO_EXPORT OSGQOPENGL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef OSGQOPENGL_NO_DEPRECATED
#    define OSGQOPENGL_NO_DEPRECATED
#  endif
#endif

#endif /* OSGQOPENGL_EXPORT_H */
