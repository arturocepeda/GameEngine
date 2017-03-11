
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Rendering Engine (OpenGL ES)
//
//  --- GEOpenGLES20.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once

#include "Core/GEPlatform.h"

#if defined (GE_PLATFORM_IOS)
# include <OpenGLES/ES2/gl.h>
#elif defined (GE_PLATFORM_ANDROID)
# include <GLES2/gl2.h>
#else
# include "glew/include/GL/glew.h"
#endif

#if defined (GE_64_BIT)
typedef uint64_t uintPtrSize;
typedef GLuint64 GLuintPtrSize;
#else
typedef uint32_t uintPtrSize;
typedef GLuint GLuintPtrSize;
#endif
