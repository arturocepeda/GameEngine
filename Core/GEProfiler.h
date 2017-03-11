
//////////////////////////////////////////////////////////////////
//
//  Arturo Cepeda Pérez
//  Game Engine
//
//  Core
//
//  --- GEProfiler.h ---
//
//////////////////////////////////////////////////////////////////


#pragma once

#include "GEPlatform.h"

#if defined(GE_PLATFORM_WINDOWS)

# include "Externals/Brofiler/Brofiler.h"

# define GEProfilerThreadID(ID)  BROFILER_THREAD(ID)
# define GEProfilerMarker(ID)    BROFILER_CATEGORY(ID, Profiler::Color::Yellow)
# define GEProfilerFrame(ID)     BROFILER_FRAME(ID)

#else

# define GEProfilerThreadID(ID)
# define GEProfilerMarker(ID)
# define GEProfilerFrame(ID)

#endif
