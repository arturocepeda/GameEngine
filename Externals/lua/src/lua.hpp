// lua.hpp
// Lua header files for C++
// <<extern "C">> not supplied automatically because Lua also compiles as C++

#if !defined (GE_PLATFORM_IOS) && defined (__APPLE__)
# if defined (TARGET_IPHONE_SIMULATOR) || defined (TARGET_OS_IPHONE)
#  define GE_PLATFORM_IOS
# endif
#endif

#if !defined (GE_PLATFORM_IOS)
extern "C"
{
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#if !defined (GE_PLATFORM_IOS)
}
#endif
