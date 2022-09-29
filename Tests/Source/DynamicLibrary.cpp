#include "Lua/LuaLibrary.h"
#include "LuaBridge/LuaBridge.h"

#include "SharedCode.h"

void registerClasses(lua_State* L)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<xyz::SharedClass>("SharedClass")
            .addConstructor<void()>()
            .addFunction("publicMethod", &xyz::SharedClass::publicMethod)
        .endClass();
}
