
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEPlatform.h ---
//
//////////////////////////////////////////////////////////////////

#pragma once


// 
//  Platform
//
#if defined (WIN32)
# if defined (WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#  define GE_PLATFORM_WP8
# else
#  define GE_PLATFORM_WINDOWS
# endif
#elif defined (ANDROID)
# define GE_PLATFORM_ANDROID
#elif defined (__APPLE__)
# if defined (TARGET_IPHONE_SIMULATOR) || defined (TARGET_OS_IPHONE)
#  define GE_PLATFORM_IOS
# else
#  define GE_PLATFORM_MACOS
# endif
#endif

#if defined (NN_SDK_BUILD_DEBUG) || defined (NN_SDK_BUILD_DEVELOP) || defined (NN_SDK_BUILD_RELEASE)
# define GE_PLATFORM_SWITCH
#endif

#if defined (GE_PLATFORM_WINDOWS) || defined (GE_PLATFORM_MACOS)
# define GE_PLATFORM_DESKTOP
#endif


//
//  32 bit / 64 bit
//
#if defined (_M_X64) || defined (__LP64__) || defined (__x86_64__) || defined (__ppc64__)
# define GE_64_BIT
#else
# define GE_32_BIT
#endif


//
//  Rendering API
//
#if (defined(GE_PLATFORM_WINDOWS) || defined(GE_PLATFORM_WP8)) && !defined(GE_WINDOWS_OPENGL)
# define GE_RENDERING_API_DIRECTX
#else
# define GE_RENDERING_API_OPENGL
#endif


//
//  Development
//
#if defined(GE_PLATFORM_WINDOWS) && defined(_DEBUG)
# define GE_DEVELOPMENT
#endif

#if defined(GE_PLATFORM_WINDOWS)
# define GE_EDITOR_SUPPORT
#endif
