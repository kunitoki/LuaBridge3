#include "Lua/LuaLibrary.h"
#include "LuaBridge/LuaBridge.h"

#include "SharedCode.h"

#if _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif

extern "C" EXPORT_API void registerClasses(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<xyz::SharedClass>("SharedClass")
            .addConstructor<void()>()
            .addFunction("publicMethod", &xyz::SharedClass::publicMethod)
        .endClass();
}
