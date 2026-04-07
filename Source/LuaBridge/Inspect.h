// https://github.com/kunitoki/LuaBridge3
// Copyright 2026, kunitoki
// SPDX-License-Identifier: MIT

#pragma once

#if ! LUABRIDGE_ENABLE_REFLECT
#error "This header is only for use when LUABRIDGE_ENABLE_REFLECT is active."
#endif

#include "detail/CFunctions.h"
#include "detail/ClassInfo.h"
#include "detail/LuaHelpers.h"

#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace luabridge {

//=================================================================================================
// Forward declarations

struct NamespaceInspectInfo;
struct ClassInspectInfo;
class InspectVisitor;

void accept(const NamespaceInspectInfo& ns, InspectVisitor& v);
void accept(const ClassInspectInfo& cls, InspectVisitor& v);

//=================================================================================================
/**
 * @brief Kind of a registered member.
 */
enum class MemberKind
{
    Method,
    StaticMethod,
    Property,
    ReadOnlyProperty,
    StaticProperty,
    StaticReadOnlyProperty,
    Constructor,
    Metamethod,
};

//=================================================================================================
/**
 * @brief Type and optional name hint for one Lua-visible parameter of an overload.
 */
struct ParamInfo
{
    std::string typeName; ///< C++ type name (e.g. "float", "int", "MyClass"). Empty if unavailable.
    std::string hint;     ///< Optional user-provided name (e.g. "damage"). Empty if not provided.
};

//=================================================================================================
/**
 * @brief Type information for one overload of a registered function or constructor.
 */
struct OverloadInfo
{
    std::string returnType;        ///< C++ return type name. Empty if unavailable.
    std::vector<ParamInfo> params; ///< One entry per Lua-visible parameter.
    bool isConst = false;          ///< True when this is a const member function.
};

//=================================================================================================
/**
 * @brief Information about one registered member (method, property, constructor, …).
 */
struct MemberInfo
{
    std::string name;
    MemberKind kind = MemberKind::Method;
    std::vector<OverloadInfo> overloads; ///< At least 1 entry; may contain empty OverloadInfo.
};

//=================================================================================================
/**
 * @brief Inspection result for one registered class.
 */
struct ClassInspectInfo
{
    std::string name;
    std::vector<std::string> baseClasses; ///< Names of all registered ancestor classes.
    std::vector<MemberInfo> members;

    void accept(InspectVisitor& v) const;
};

//=================================================================================================
/**
 * @brief Inspection result for a namespace (may contain classes and sub-namespaces).
 */
struct NamespaceInspectInfo
{
    std::string name;
    std::vector<MemberInfo> freeMembers;                  ///< Free functions and namespace-level properties.
    std::vector<ClassInspectInfo> classes;
    std::vector<NamespaceInspectInfo> subNamespaces;

    void accept(InspectVisitor& v) const;
};

//=================================================================================================
/**
 * @brief Visitor interface for traversing an inspection result tree.
 *
 * Override the methods you care about. Default implementations are no-ops so you only
 * need to override what you need.
 */
class InspectVisitor
{
public:
    virtual ~InspectVisitor() = default;

    virtual void beginNamespace(const NamespaceInspectInfo& /*ns*/) {}
    virtual void endNamespace(const NamespaceInspectInfo& /*ns*/) {}
    virtual void visitFreeMember(const NamespaceInspectInfo& /*ns*/, const MemberInfo& /*m*/) {}

    virtual void beginClass(const ClassInspectInfo& /*cls*/) {}
    virtual void endClass(const ClassInspectInfo& /*cls*/) {}
    virtual void visitMember(const ClassInspectInfo& /*cls*/, const MemberInfo& /*m*/) {}
};

//=================================================================================================
/**
 * @brief Traverse a namespace inspection result, calling visitor callbacks in order.
 */
inline void accept(const NamespaceInspectInfo& ns, InspectVisitor& v)
{
    v.beginNamespace(ns);
    for (const auto& m : ns.freeMembers)
        v.visitFreeMember(ns, m);
    for (const auto& cls : ns.classes)
        accept(cls, v);
    for (const auto& sub : ns.subNamespaces)
        accept(sub, v);
    v.endNamespace(ns);
}

inline void accept(const ClassInspectInfo& cls, InspectVisitor& v)
{
    v.beginClass(cls);
    for (const auto& m : cls.members)
        v.visitMember(cls, m);
    v.endClass(cls);
}

inline void ClassInspectInfo::accept(InspectVisitor& v) const { luabridge::accept(*this, v); }
inline void NamespaceInspectInfo::accept(InspectVisitor& v) const { luabridge::accept(*this, v); }

//=================================================================================================
// Internal helpers

namespace detail {

// Read all string keys from a Lua table at tableIdx into a set
inline std::set<std::string> collectTableKeys(lua_State* L, int tableIdx)
{
    std::set<std::string> keys;
    tableIdx = lua_absindex(L, tableIdx);
    lua_pushnil(L);
    while (lua_next(L, tableIdx))
    {
        if (lua_type(L, -2) == LUA_TSTRING)
            keys.insert(lua_tostring(L, -2));
        lua_pop(L, 1); // pop value, keep key
    }
    return keys;
}

// Strip a "const " prefix from a type name string
inline std::string stripConst(const char* name)
{
    std::string s = name ? name : "";
    if (s.substr(0, 6) == "const ")
        s = s.substr(6);
    return s;
}

// Return the overload infos for a function/closure sitting at funcIdx.
// If there's no OverloadSet upvalue (bare cfunction), returns 1 entry with empty type strings.
inline std::vector<OverloadInfo> getOverloadInfos(lua_State* L, int funcIdx)
{
    funcIdx = lua_absindex(L, funcIdx);

    if (!lua_isfunction(L, funcIdx))
        return { OverloadInfo{} };

    // Check upvalue 1 for an OverloadSet userdata identified by its magic cookie.
    // This handles two layouts:
    //   Multi-overload:           upvalue[1]=OverloadSet, upvalue[2]=table of functions
    //   Single-overload REFLECT:  upvalue[1]=OverloadSet, upvalue[2]=function data directly
    const char* uv1name = lua_getupvalue(L, funcIdx, 1);
    if (uv1name == nullptr)
    {
        // No upvalues — bare cfunction, no metadata available
        return { OverloadInfo{} };
    }

    OverloadSet* overload_set = nullptr;
    if (isfulluserdata(L, -1))
    {
        auto* candidate = align<OverloadSet>(lua_touserdata(L, -1));
        if (candidate && candidate->magic == OverloadSet::kMagic)
            overload_set = candidate;
    }
    lua_pop(L, 1); // pop upvalue 1

    if (!overload_set || overload_set->entries.empty())
        return { OverloadInfo{} };

    std::vector<OverloadInfo> result;
    result.reserve(overload_set->entries.size());

    for (const auto& entry : overload_set->entries)
    {
        OverloadInfo info;

#if LUABRIDGE_ENABLE_REFLECT
        info.returnType = entry.returnType;
        for (std::size_t i = 0; i < entry.paramTypes.size(); ++i)
        {
            ParamInfo p;
            p.typeName = entry.paramTypes[i];
            if (i < entry.paramHints.size())
                p.hint = entry.paramHints[i];
            info.params.push_back(std::move(p));
        }
#else
        // Without LUABRIDGE_ENABLE_REFLECT: emit synthetic params based on arity only
        if (entry.arity >= 0)
        {
            for (int i = 0; i < entry.arity; ++i)
                info.params.push_back(ParamInfo{});
        }
#endif

        result.push_back(std::move(info));
    }

    return result;
}

// Returns true if the table at tableIdx is a class static table (has a metatable with getClassKey).
// Also leaves the class metatable (mt) on the stack top if true (caller must pop it).
// Leaves nothing extra on the stack if false.
inline bool isClassStaticTable(lua_State* L, int tableIdx)
{
    tableIdx = lua_absindex(L, tableIdx);

    if (!lua_getmetatable(L, tableIdx)) // always works from C regardless of __metatable
        return false;

    lua_rawgetp_x(L, -1, getClassKey()); // mt[getClassKey()] = cl ?
    bool result = lua_istable(L, -1);
    lua_pop(L, 1); // pop cl / nil

    if (!result)
    {
        lua_pop(L, 1); // pop mt
        return false;
    }

    // Leave mt on stack (caller needs it)
    return true;
}

// Inspect a class from its static table (st at stIdx).
// Assumes isClassStaticTable(L, stIdx) returned true and left mt on the stack.
inline ClassInspectInfo inspectClassFromStaticTable(lua_State* L, int stIdx)
{
    stIdx = lua_absindex(L, stIdx);

    // mt is on top of the stack (left there by isClassStaticTable)
    int mtIdx = lua_absindex(L, -1);

    // Get the class table cl = mt[getClassKey()]
    lua_rawgetp_x(L, mtIdx, getClassKey());
    int clIdx = lua_absindex(L, -1);

    ClassInspectInfo cls;

    // 1. Class name
    lua_rawgetp_x(L, clIdx, getTypeKey());
    if (lua_isstring(L, -1))
        cls.name = stripConst(lua_tostring(L, -1));
    lua_pop(L, 1);

    // 2. Base classes from parent list
    lua_rawgetp_x(L, clIdx, getParentKey());
    if (lua_istable(L, -1))
    {
        int parIdx = lua_absindex(L, -1);
        int len = get_length(L, parIdx);
        for (int i = 1; i <= len; ++i)
        {
            lua_rawgeti(L, parIdx, i);
            lua_rawgetp_x(L, -1, getTypeKey());
            if (lua_isstring(L, -1))
            {
                std::string baseName = stripConst(lua_tostring(L, -1));
                if (!baseName.empty() && baseName != cls.name)
                    cls.baseClasses.push_back(std::move(baseName));
            }
            lua_pop(L, 2); // pop typename + parent mt
        }
    }
    lua_pop(L, 1); // pop parent list / nil

    // 3. Collect instance property names
    std::set<std::string> instPropget;
    lua_rawgetp_x(L, clIdx, getPropgetKey());
    if (lua_istable(L, -1))
        instPropget = collectTableKeys(L, -1);
    lua_pop(L, 1);

    // Collect instance property names that have a real (non-readonly-sentinel) setter
    std::set<std::string> instPropsetReal;
    lua_rawgetp_x(L, clIdx, getPropsetKey());
    if (lua_istable(L, -1))
    {
        int psIdx = lua_absindex(L, -1);
        for (const auto& k : instPropget)
        {
            lua_getfield(L, psIdx, k.c_str());
            if (lua_isfunction(L, -1))
            {
                lua_CFunction fn = lua_tocfunction(L, -1);
                if (fn != &detail::read_only_error)
                    instPropsetReal.insert(k);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // Collect instance property type names (may be absent if not registered with type info)
    std::map<std::string, std::string> instPropTypes;
    lua_rawgetp_x(L, clIdx, getPropTypeKey());
    if (lua_istable(L, -1))
    {
        int ptIdx = lua_absindex(L, -1);
        for (const auto& k : instPropget)
        {
            lua_getfield(L, ptIdx, k.c_str());
            if (lua_isstring(L, -1))
                instPropTypes[k] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // 4. Emit instance properties
    for (const auto& propName : instPropget)
    {
        MemberInfo m;
        m.name = propName;
        m.kind = instPropsetReal.count(propName) ? MemberKind::Property : MemberKind::ReadOnlyProperty;
        OverloadInfo ov;
        auto it = instPropTypes.find(propName);
        if (it != instPropTypes.end())
            ov.returnType = it->second;
        m.overloads.push_back(std::move(ov));
        cls.members.push_back(std::move(m));
    }

    // 5. Iterate class table for instance methods / metamethods
    static const std::set<std::string> skipKeys{ "__index", "__newindex", "__metatable" };

    lua_pushnil(L);
    while (lua_next(L, clIdx))
    {
        if (lua_type(L, -2) == LUA_TSTRING)
        {
            std::string key = lua_tostring(L, -2);
            if (!skipKeys.count(key) && !instPropget.count(key))
            {
                if (lua_isfunction(L, -1) || lua_isuserdata(L, -1))
                {
                    MemberInfo m;
                    m.name = key;
                    m.kind = (key.size() >= 2 && key[0] == '_' && key[1] == '_')
                        ? MemberKind::Metamethod
                        : MemberKind::Method;
                    m.overloads = getOverloadInfos(L, -1);
                    cls.members.push_back(std::move(m));
                }
            }
        }
        lua_pop(L, 1); // pop value
    }

    // 6. Collect static property names
    std::set<std::string> stPropget;
    lua_rawgetp_x(L, mtIdx, getPropgetKey());
    if (lua_istable(L, -1))
        stPropget = collectTableKeys(L, -1);
    lua_pop(L, 1);

    // Collect static property names that have a real (non-readonly-sentinel) setter
    std::set<std::string> stPropsetReal;
    lua_rawgetp_x(L, mtIdx, getPropsetKey());
    if (lua_istable(L, -1))
    {
        int psIdx = lua_absindex(L, -1);
        for (const auto& k : stPropget)
        {
            lua_getfield(L, psIdx, k.c_str());
            if (lua_isfunction(L, -1))
            {
                lua_CFunction fn = lua_tocfunction(L, -1);
                if (fn != &detail::read_only_error)
                    stPropsetReal.insert(k);
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // Collect static property type names
    std::map<std::string, std::string> stPropTypes;
    lua_rawgetp_x(L, mtIdx, getPropTypeKey());
    if (lua_istable(L, -1))
    {
        int ptIdx = lua_absindex(L, -1);
        for (const auto& k : stPropget)
        {
            lua_getfield(L, ptIdx, k.c_str());
            if (lua_isstring(L, -1))
                stPropTypes[k] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // 7. Emit static properties
    for (const auto& propName : stPropget)
    {
        MemberInfo m;
        m.name = propName;
        m.kind = stPropsetReal.count(propName) ? MemberKind::StaticProperty : MemberKind::StaticReadOnlyProperty;
        OverloadInfo ov;
        auto it = stPropTypes.find(propName);
        if (it != stPropTypes.end())
            ov.returnType = it->second;
        m.overloads.push_back(std::move(ov));
        cls.members.push_back(std::move(m));
    }

    // 8. Iterate mt for static methods + constructor
    static const std::set<std::string> staticSkipKeys{ "__index", "__newindex", "__metatable" };

    lua_rawgetp_x(L, mtIdx, getClassKey()); // just to see mt's string keys, iterate mt directly
    lua_pop(L, 1);

    lua_pushnil(L);
    while (lua_next(L, mtIdx))
    {
        if (lua_type(L, -2) == LUA_TSTRING)
        {
            std::string key = lua_tostring(L, -2);
            if (!staticSkipKeys.count(key) && !stPropget.count(key))
            {
                if (lua_isfunction(L, -1) || lua_isuserdata(L, -1))
                {
                    MemberInfo m;
                    m.name = key;
                    if (key == "__call")
                        m.kind = MemberKind::Constructor;
                    else if (key.size() >= 2 && key[0] == '_' && key[1] == '_')
                        m.kind = MemberKind::Metamethod;
                    else
                        m.kind = MemberKind::StaticMethod;
                    m.overloads = getOverloadInfos(L, -1);
                    cls.members.push_back(std::move(m));
                }
            }
        }
        lua_pop(L, 1);
    }

    lua_pop(L, 2); // pop cl, mt
    return cls;
}

// Inspect a namespace table at nsIdx
inline NamespaceInspectInfo inspectNamespaceTable(lua_State* L, int nsIdx, std::string name);

inline NamespaceInspectInfo inspectNamespaceTable(lua_State* L, int nsIdx, std::string name)
{
    nsIdx = lua_absindex(L, nsIdx);
    NamespaceInspectInfo info;
    info.name = std::move(name);

    // Namespace-level properties (stored directly on the namespace table via getPropgetKey)
    std::set<std::string> nsPropget;
    lua_rawgetp_x(L, nsIdx, getPropgetKey());
    if (lua_istable(L, -1))
        nsPropget = collectTableKeys(L, -1);
    lua_pop(L, 1);

    // Get the propset table so we can check each setter individually
    int nsPropsetTableIdx = 0;
    lua_rawgetp_x(L, nsIdx, getPropsetKey());
    if (lua_istable(L, -1))
        nsPropsetTableIdx = lua_absindex(L, -1);
    // leave propset table on stack; we pop it after the loop

    // Collect namespace property type names
    std::map<std::string, std::string> nsPropTypes;
    lua_rawgetp_x(L, nsIdx, getPropTypeKey());
    if (lua_istable(L, -1))
    {
        int ptIdx = lua_absindex(L, -1);
        for (const auto& k : nsPropget)
        {
            lua_getfield(L, ptIdx, k.c_str());
            if (lua_isstring(L, -1))
                nsPropTypes[k] = lua_tostring(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1); // pop proptype table or nil

    for (const auto& propName : nsPropget)
    {
        MemberInfo m;
        m.name = propName;

        // A getter-only namespace property still gets a read_only_error sentinel in propset.
        // Detect the sentinel by comparing the setter's underlying C function pointer.
        bool isReadOnly = true;
        if (nsPropsetTableIdx)
        {
            lua_getfield(L, nsPropsetTableIdx, propName.c_str());
            if (lua_isfunction(L, -1))
            {
                lua_CFunction fn = lua_tocfunction(L, -1);
                isReadOnly = (fn == &detail::read_only_error);
            }
            lua_pop(L, 1);
        }

        m.kind = isReadOnly ? MemberKind::ReadOnlyProperty : MemberKind::Property;
        OverloadInfo ov;
        auto it = nsPropTypes.find(propName);
        if (it != nsPropTypes.end())
            ov.returnType = it->second;
        m.overloads.push_back(std::move(ov));
        info.freeMembers.push_back(std::move(m));
    }

    lua_pop(L, 1); // pop propset table (or nil)

    // Iterate namespace table entries
    static const std::set<std::string> nsSkipKeys{ "__index", "__newindex", "__metatable", "__gc" };

    lua_pushnil(L);
    while (lua_next(L, nsIdx))
    {
        if (lua_type(L, -2) != LUA_TSTRING)
        {
            lua_pop(L, 1);
            continue;
        }

        std::string key = lua_tostring(L, -2);

        if (nsSkipKeys.count(key) || nsPropget.count(key))
        {
            lua_pop(L, 1);
            continue;
        }

        if (lua_istable(L, -1))
        {
            int valIdx = lua_absindex(L, -1);

            if (isClassStaticTable(L, valIdx))
            {
                // mt is now on top of the stack (left by isClassStaticTable)
                auto cls = inspectClassFromStaticTable(L, valIdx);
                // inspectClassFromStaticTable pops mt and cl
                info.classes.push_back(std::move(cls));
            }
            else
            {
                auto sub = inspectNamespaceTable(L, valIdx, key);
                info.subNamespaces.push_back(std::move(sub));
            }
        }
        else if (lua_isfunction(L, -1))
        {
            if (!nsPropget.count(key))
            {
                MemberInfo m;
                m.name = key;
                m.kind = MemberKind::Method;
                m.overloads = getOverloadInfos(L, -1);
                info.freeMembers.push_back(std::move(m));
            }
        }

        lua_pop(L, 1); // pop value
    }

    return info;
}

} // namespace detail

//=================================================================================================
/**
 * @brief Inspect a single registered class by its C++ type.
 *
 * Uses the compile-time registry key — no namespace traversal required.
 *
 * @returns Inspection result, or an empty ClassInspectInfo if T is not registered.
 */
template <class T>
[[nodiscard]] ClassInspectInfo inspect(lua_State* L)
{
    // Load the class table (cl) directly from the Lua registry
    lua_rawgetp_x(L, LUA_REGISTRYINDEX, detail::getClassRegistryKey<T>());
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return {};
    }
    // Stack: ..., cl

    // cl[getStaticKey()] = st  (the internal static table that acts as a metatable)
    // NOTE: this is NOT the user-visible ns["ClassName"] table; it is the metatable of that table.
    // isClassStaticTable() cannot be used here because st itself has no metatable.
    lua_rawgetp_x(L, -1, detail::getStaticKey());
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 2);
        return {};
    }
    // Stack: ..., cl, st

    // Verify st actually belongs to a registered class: st[getClassKey()] must be a table
    lua_rawgetp_x(L, -1, detail::getClassKey());
    const bool isClass = lua_istable(L, -1);
    lua_pop(L, 1);

    if (!isClass)
    {
        lua_pop(L, 2);
        return {};
    }
    // Stack: ..., cl, st  (st is at top — inspectClassFromStaticTable reads it as "mt")

    ClassInspectInfo result = detail::inspectClassFromStaticTable(L, lua_gettop(L));
    // inspectClassFromStaticTable pops st (used as mt) and the cl it fetched from st[getClassKey()]
    // Stack: ..., cl
    lua_pop(L, 1); // pop cl
    return result;
}

//=================================================================================================
/**
 * @brief Inspect a namespace by name.
 *
 * @param namespaceName The global name of the namespace (e.g. "MyNS"), or nullptr / "" for the
 *                      global namespace. Supports dotted paths ("Outer.Inner").
 *
 * @returns Inspection result, or an empty NamespaceInspectInfo if the name is not found.
 */
[[nodiscard]] inline NamespaceInspectInfo inspectNamespace(lua_State* L, const char* namespaceName = nullptr)
{
    if (namespaceName == nullptr || namespaceName[0] == '\0')
    {
        lua_getglobal(L, "_G");
    }
    else
    {
        // Support dotted paths by navigating component by component
        std::string path = namespaceName;
        std::string first;
        std::string::size_type dot = path.find('.');
        if (dot == std::string::npos)
            first = path;
        else
            first = path.substr(0, dot);

        lua_getglobal(L, first.c_str());
        if (!lua_istable(L, -1))
        {
            lua_pop(L, 1);
            return {};
        }

        while (dot != std::string::npos)
        {
            path = path.substr(dot + 1);
            dot = path.find('.');
            std::string component = (dot == std::string::npos) ? path : path.substr(0, dot);
            lua_getfield(L, -1, component.c_str());
            lua_remove(L, -2); // remove previous table
            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);
                return {};
            }
        }
    }

    // Use only the leaf component of a dotted path as the display name
    std::string label;
    if (namespaceName && namespaceName[0])
    {
        std::string_view sv(namespaceName);
        auto dot = sv.rfind('.');
        label = std::string(dot == std::string_view::npos ? sv : sv.substr(dot + 1));
    }
    else
    {
        label = "_G";
    }
    auto result = detail::inspectNamespaceTable(L, -1, label);
    lua_pop(L, 1);
    return result;
}

//=================================================================================================
/**
 * @brief Collect inspection data for a namespace and drive a visitor over it.
 */
inline void inspectAccept(lua_State* L, const char* namespaceName, InspectVisitor& visitor)
{
    auto ns = inspectNamespace(L, namespaceName);
    accept(ns, visitor);
}

//=================================================================================================
/**
 * @brief Visitor that emits TypeScript-style pseudo-code to an ostream.
 *
 * Useful for quick debugging and documentation.
 *
 * Example output:
 * @code
 * namespace MyNS {
 *     class Foo extends Base {
 *         constructor();
 *         method(p1: float): void;
 *         readonly value: int;
 *         static create(): Foo;
 *     }
 * }
 * @endcode
 */
class ConsoleVisitor : public InspectVisitor
{
public:
    explicit ConsoleVisitor(std::ostream& out = std::cerr)
        : out_(out)
    {
    }

    void beginNamespace(const NamespaceInspectInfo& ns) override
    {
        indent();
        out_ << "namespace " << ns.name << " {\n";
        ++depth_;
    }

    void endNamespace([[maybe_unused]] const NamespaceInspectInfo& ns) override
    {
        --depth_;
        indent();
        out_ << "}\n";
    }

    void visitFreeMember([[maybe_unused]] const NamespaceInspectInfo& ns, const MemberInfo& m) override
    {
        emitMember(m, "");
    }

    void beginClass(const ClassInspectInfo& cls) override
    {
        indent();
        out_ << "class " << cls.name;
        if (!cls.baseClasses.empty())
        {
            out_ << " extends ";
            for (std::size_t i = 0; i < cls.baseClasses.size(); ++i)
            {
                if (i) out_ << ", ";
                out_ << cls.baseClasses[i];
            }
        }
        out_ << " {\n";
        ++depth_;
    }

    void endClass([[maybe_unused]] const ClassInspectInfo& cls) override
    {
        --depth_;
        indent();
        out_ << "}\n";
    }

    void visitMember(const ClassInspectInfo& cls, const MemberInfo& m) override
    {
        emitMember(m, cls.name);
    }

private:
    void indent() const
    {
        for (int i = 0; i < depth_; ++i)
            out_ << "    ";
    }

    static std::string paramStr(const OverloadInfo& ov)
    {
        std::string s;
        for (std::size_t i = 0; i < ov.params.size(); ++i)
        {
            if (i) s += ", ";
            const auto& p = ov.params[i];
            s += p.hint.empty() ? ("p" + std::to_string(i + 1)) : p.hint;
            s += ": ";
            s += p.typeName.empty() ? "any" : p.typeName;
        }
        return s;
    }

    static std::string retStr(const OverloadInfo& ov, const std::string& constructorClass = "")
    {
        if (!constructorClass.empty())
            return constructorClass;
        return ov.returnType.empty() ? "any" : ov.returnType;
    }

    void emitMember(const MemberInfo& m, [[maybe_unused]] const std::string& className) const
    {
        switch (m.kind)
        {
        case MemberKind::Property:
            indent();
            out_ << m.name << ": any;\n";
            break;

        case MemberKind::ReadOnlyProperty:
            indent();
            out_ << "readonly " << m.name << ": any;\n";
            break;

        case MemberKind::StaticProperty:
            indent();
            out_ << "static " << m.name << ": any;\n";
            break;
        
        case MemberKind::StaticReadOnlyProperty:
            indent();
            out_ << "static readonly " << m.name << ": any;\n";
            break;

        case MemberKind::Metamethod:
            // Skip metamethods in TypeScript-style output
            break;

        case MemberKind::Constructor:
            for (const auto& ov : m.overloads)
            {
                indent();
                out_ << "constructor(" << paramStr(ov) << ");\n";
            }
            break;

        case MemberKind::Method:
        case MemberKind::StaticMethod:
        {
            const bool isStatic = (m.kind == MemberKind::StaticMethod);
            for (std::size_t i = 0; i < m.overloads.size(); ++i)
            {
                indent();
                if (isStatic) out_ << "static ";
                out_ << m.name << "(" << paramStr(m.overloads[i]) << "): " << retStr(m.overloads[i]) << ";\n";
            }
            break;
        }

        default:
            break;
        }
    }

    std::ostream& out_;
    int depth_ = 0;
};

//=================================================================================================
/**
 * @brief Visitor that emits LuaLS (Lua Language Server) / LuaCATS annotation stubs.
 *
 * Suitable for generating `.lua` type stub files consumed by lua-language-server.
 *
 * @see https://luals.github.io/wiki/annotations/
 */
class LuaLSVisitor : public InspectVisitor
{
public:
    explicit LuaLSVisitor(std::ostream& out)
        : out_(out)
    {
    }

    void beginNamespace(const NamespaceInspectInfo& ns) override
    {
        ns_ = ns.name == "_G" ? "" : ns.name;
    }

    void endNamespace([[maybe_unused]] const NamespaceInspectInfo& ns) override {}

    void visitFreeMember(const NamespaceInspectInfo& ns, const MemberInfo& m) override
    {
        if (m.kind != MemberKind::Method && m.kind != MemberKind::StaticMethod)
            return; // properties not directly emittable as LuaLS annotations easily

        for (const auto& ov : m.overloads)
        {
            emitParamsTo(out_, ov.params);
            if (!ov.returnType.empty() && ov.returnType != "void")
                out_ << "---@return " << luaType(ov.returnType) << "\n";
        }

        std::string qual = ns_.empty() ? "" : (ns_ + ".");
        out_ << "function " << qual << m.name << "(";
        if (!m.overloads.empty())
            out_ << paramNames(m.overloads[0].params);
        out_ << ") end\n\n";
    }

    void beginClass(const ClassInspectInfo& cls) override
    {
        curClass_ = cls.name;
        out_ << "---@class " << qualifiedName(cls.name);
        if (!cls.baseClasses.empty())
        {
            out_ << " : " << cls.baseClasses[0];
            for (std::size_t i = 1; i < cls.baseClasses.size(); ++i)
                out_ << ", " << cls.baseClasses[i];
        }
        out_ << "\n";
        methodBuf_.str({});
        methodBuf_.clear();
    }

    void endClass([[maybe_unused]] const ClassInspectInfo& cls) override
    {
        out_ << "local " << curClass_ << " = {}\n";
        const std::string methods = methodBuf_.str();
        if (!methods.empty())
            out_ << "\n" << methods;
        else
            out_ << "\n";
        curClass_.clear();
    }

    void visitMember(const ClassInspectInfo& cls, const MemberInfo& m) override
    {
        auto propType = [&]() -> std::string {
            if (!m.overloads.empty() && !m.overloads[0].returnType.empty())
                return luaType(m.overloads[0].returnType);
            return "any";
        };

        switch (m.kind)
        {
        case MemberKind::Property:
            out_ << "---@field " << m.name << " " << propType() << "\n";
            break;

        case MemberKind::ReadOnlyProperty:
            out_ << "---@field " << m.name << " " << propType() << " # readonly\n";
            break;

        case MemberKind::StaticProperty:
            out_ << "---@field " << m.name << " " << propType() << "\n";
            break;

        case MemberKind::StaticReadOnlyProperty:
            out_ << "---@field " << m.name << " " << propType() << " # readonly\n";
            break;

        case MemberKind::Constructor:
        {
            // Represent the constructor as @overload annotations before the local declaration.
            // LuaBridge constructors are called as ClassName(args), not ClassName.new(args).
            std::string qname = qualifiedName(cls.name);
            for (const auto& ov : m.overloads)
                out_ << "---@overload fun(" << overloadParams(ov.params) << "): " << qname << "\n";
            break;
        }

        case MemberKind::Method:
        {
            std::string qname = qualifiedName(cls.name);
            for (std::size_t i = 0; i < m.overloads.size(); ++i)
            {
                const auto& ov = m.overloads[i];
                if (i == 0)
                {
                    emitParamsTo(methodBuf_, ov.params, /*hasSelf=*/true, qname);
                    if (!ov.returnType.empty() && ov.returnType != "void")
                        methodBuf_ << "---@return " << luaType(ov.returnType) << "\n";
                    methodBuf_ << "function " << qname << ":" << m.name << "(" << paramNames(ov.params) << ") end\n\n";
                }
                else
                {
                    methodBuf_ << "---@overload fun(self: " << qname << ", " << overloadParams(ov.params) << ")"
                               << (ov.returnType.empty() ? "" : (": " + luaType(ov.returnType))) << "\n";
                }
            }
            break;
        }

        case MemberKind::StaticMethod:
        {
            std::string qname = qualifiedName(cls.name);
            for (std::size_t i = 0; i < m.overloads.size(); ++i)
            {
                const auto& ov = m.overloads[i];
                if (i == 0)
                {
                    emitParamsTo(methodBuf_, ov.params);
                    if (!ov.returnType.empty() && ov.returnType != "void")
                        methodBuf_ << "---@return " << luaType(ov.returnType) << "\n";
                    methodBuf_ << "function " << qname << "." << m.name << "(" << paramNames(ov.params) << ") end\n\n";
                }
                else
                {
                    methodBuf_ << "---@overload fun(" << overloadParams(ov.params) << ")"
                               << (ov.returnType.empty() ? "" : (": " + luaType(ov.returnType))) << "\n";
                }
            }
            break;
        }

        default:
            break;
        }
    }

private:
    // Minimal C++ → Lua type name mapping
    static std::string luaType(const std::string& cppType)
    {
        if (cppType == "void") return "nil";

        if (cppType == "bool") return "boolean";

        if (cppType == "int" || cppType == "long" || cppType == "short" ||
            cppType == "unsigned int" || cppType == "unsigned long" ||
            cppType == "unsigned short" || cppType == "int64_t" || cppType == "uint64_t" ||
            cppType == "int32_t" || cppType == "uint32_t" || cppType == "size_t")
            return "integer";

        if (cppType == "float" || cppType == "double") return "number";

        if (cppType == "std::string" || cppType == "const char *" || cppType == "const char*")
            return "string";

        return cppType.empty() ? "any" : cppType;
    }

    std::string qualifiedName(const std::string& name) const
    {
        return ns_.empty() ? name : (ns_ + "." + name);
    }

    static std::string paramNames(const std::vector<ParamInfo>& params)
    {
        std::string s;
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            if (i) s += ", ";
            s += params[i].hint.empty() ? ("p" + std::to_string(i + 1)) : params[i].hint;
        }
        return s;
    }

    static std::string overloadParams(const std::vector<ParamInfo>& params)
    {
        std::string s;
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            if (i) s += ", ";

            const auto& p = params[i];
            std::string pname = p.hint.empty() ? ("p" + std::to_string(i + 1)) : p.hint;
            s += pname + ": " + luaType(p.typeName.empty() ? "any" : p.typeName);
        }
        return s;
    }

    static void emitParamsTo(std::ostream& os, const std::vector<ParamInfo>& params,
                             bool hasSelf = false, const std::string& selfType = {})
    {
        if (hasSelf)
            os << "---@param self " << selfType << "\n";

        for (std::size_t i = 0; i < params.size(); ++i)
        {
            const auto& p = params[i];
            std::string pname = p.hint.empty() ? ("p" + std::to_string(i + 1)) : p.hint;
            std::string ptype = p.typeName.empty() ? "any" : luaType(p.typeName);
            os << "---@param " << pname << " " << ptype << "\n";
        }
    }

    std::ostream& out_;
    std::ostringstream methodBuf_;
    std::string ns_;
    std::string curClass_;
};

//=================================================================================================
/**
 * @brief Visitor that emits a real Lua stub file (require()-able, mockable in tests).
 *
 * Each class is emitted as a module-local table with stub methods.
 * Namespace-level functions are emitted as module-level functions.
 */
class LuaProxyVisitor : public InspectVisitor
{
public:
    explicit LuaProxyVisitor(std::ostream& out)
        : out_(out)
    {
    }

    void beginNamespace(const NamespaceInspectInfo& ns) override
    {
        curNs_ = (ns.name == "_G") ? "" : ns.name;
        if (!curNs_.empty())
            out_ << "local " << curNs_ << " = {}\n\n";
    }

    void endNamespace(const NamespaceInspectInfo& ns) override
    {
        if (ns.name != "_G")
            out_ << "return " << ns.name << "\n";
        curNs_.clear();
    }

    void visitFreeMember(const NamespaceInspectInfo& ns, const MemberInfo& m) override
    {
        if (m.kind != MemberKind::Method && m.kind != MemberKind::StaticMethod)
            return;
        std::string qual = (ns.name == "_G") ? "" : (ns.name + ".");
        if (!m.overloads.empty())
        {
            out_ << "function " << qual << m.name
                 << "(" << paramNames(m.overloads[0].params) << ") end\n";
        }
    }

    void beginClass(const ClassInspectInfo& cls) override
    {
        curClass_ = cls.name;
        std::string qname = curNs_.empty() ? cls.name : (curNs_ + "." + cls.name);
        out_ << qname << " = {}\n";
        out_ << qname << ".__index = " << qname << "\n\n";
    }

    void endClass([[maybe_unused]] const ClassInspectInfo& cls) override
    {
        curClass_.clear();
        out_ << "\n";
    }

    void visitMember([[maybe_unused]] const ClassInspectInfo& cls, const MemberInfo& m) override
    {
        if (m.overloads.empty())
            return;

        std::string qname = curNs_.empty() ? curClass_ : (curNs_ + "." + curClass_);

        switch (m.kind)
        {
        case MemberKind::Constructor:
            // LuaBridge constructors are called as ClassName(args) via __call metamethod.
            out_ << "setmetatable(" << qname << ", {__call = function(t";
            if (!m.overloads[0].params.empty())
                out_ << ", " << paramNames(m.overloads[0].params);
            out_ << ")\n    return setmetatable({}, t)\nend})\n\n";
            break;

        case MemberKind::Method:
            out_ << "function " << qname << ":" << m.name
                 << "(" << paramNames(m.overloads[0].params) << ") end\n\n";
            break;

        case MemberKind::StaticMethod:
            out_ << "function " << qname << "." << m.name
                 << "(" << paramNames(m.overloads[0].params) << ") end\n\n";
            break;

        case MemberKind::Property:
        case MemberKind::ReadOnlyProperty:
        case MemberKind::StaticProperty:
        case MemberKind::StaticReadOnlyProperty:
            // Properties are part of the table, no explicit function needed
            break;

        default:
            break;
        }
    }

private:
    static std::string paramNames(const std::vector<ParamInfo>& params)
    {
        std::string s;
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            if (i) s += ", ";
            s += params[i].hint.empty() ? ("p" + std::to_string(i + 1)) : params[i].hint;
        }
        return s;
    }

    std::ostream& out_;
    std::string curNs_;
    std::string curClass_;
};

//=================================================================================================
/**
 * @brief Visitor that builds a structured Lua table on the Lua stack.
 *
 * After calling accept() with this visitor, the stack top is a table with structure:
 * @code
 * {
 *   name = "MyNS",
 *   freeMembers = { {name=…, kind=…, overloads=…}, … },
 *   classes = {
 *     { name=…, bases={…}, members={ {name=…, kind=…, overloads=N}, … } },
 *     …
 *   },
 *   subNamespaces = { … }
 * }
 * @endcode
 */
class LuaTableVisitor : public InspectVisitor
{
public:
    explicit LuaTableVisitor(lua_State* L)
        : L_(L)
    {
    }

    void beginNamespace(const NamespaceInspectInfo& ns) override
    {
        lua_newtable(L_); // the namespace table
        lua_pushstring(L_, ns.name.c_str());
        lua_setfield(L_, -2, "name");

        lua_newtable(L_);
        lua_setfield(L_, -2, "freeMembers");
        freeMemberIdx_ = 1;

        lua_newtable(L_);
        lua_setfield(L_, -2, "classes");
        classIdx_ = 1;

        lua_newtable(L_);
        lua_setfield(L_, -2, "subNamespaces");
        subNsIdx_ = 1;
    }

    void endNamespace([[maybe_unused]] const NamespaceInspectInfo& ns) override
    {
        // namespace table stays on stack as the result
    }

    void visitFreeMember([[maybe_unused]] const NamespaceInspectInfo& ns, const MemberInfo& m) override
    {
        lua_getfield(L_, -1, "freeMembers");
        pushMemberInfo(m);
        lua_rawseti(L_, -2, freeMemberIdx_++);
        lua_pop(L_, 1);
    }

    void beginClass(const ClassInspectInfo& cls) override
    {
        lua_newtable(L_);
        lua_pushstring(L_, cls.name.c_str());
        lua_setfield(L_, -2, "name");

        lua_newtable(L_);
        for (std::size_t i = 0; i < cls.baseClasses.size(); ++i)
        {
            lua_pushstring(L_, cls.baseClasses[i].c_str());
            lua_rawseti(L_, -2, static_cast<int>(i + 1));
        }
        lua_setfield(L_, -2, "bases");

        lua_newtable(L_);
        lua_setfield(L_, -2, "members");
        memberIdx_ = 1;
    }

    void endClass([[maybe_unused]] const ClassInspectInfo& cls) override
    {
        // class table is on top; store it in the namespace's "classes" array
        lua_getfield(L_, -2, "classes");
        lua_pushvalue(L_, -2);
        lua_rawseti(L_, -2, classIdx_++);
        lua_pop(L_, 2); // pop classes table + class table
    }

    void visitMember([[maybe_unused]] const ClassInspectInfo& cls, const MemberInfo& m) override
    {
        lua_getfield(L_, -1, "members");
        pushMemberInfo(m);
        lua_rawseti(L_, -2, memberIdx_++);
        lua_pop(L_, 1);
    }

private:
    void pushMemberInfo(const MemberInfo& m)
    {
        lua_newtable(L_);
        lua_pushstring(L_, m.name.c_str());
        lua_setfield(L_, -2, "name");
        lua_pushstring(L_, kindStr(m.kind));
        lua_setfield(L_, -2, "kind");
        lua_pushinteger(L_, static_cast<lua_Integer>(m.overloads.size()));
        lua_setfield(L_, -2, "overloads");

        // Emit overload details when type info is available
        lua_newtable(L_);
        for (std::size_t i = 0; i < m.overloads.size(); ++i)
        {
            const auto& ov = m.overloads[i];
            lua_newtable(L_);
            lua_pushstring(L_, ov.returnType.c_str());
            lua_setfield(L_, -2, "returnType");

            lua_newtable(L_);
            for (std::size_t j = 0; j < ov.params.size(); ++j)
            {
                lua_newtable(L_);
                lua_pushstring(L_, ov.params[j].typeName.c_str());
                lua_setfield(L_, -2, "type");
                lua_pushstring(L_, ov.params[j].hint.c_str());
                lua_setfield(L_, -2, "hint");
                lua_rawseti(L_, -2, static_cast<int>(j + 1));
            }
            lua_setfield(L_, -2, "params");
            lua_rawseti(L_, -2, static_cast<int>(i + 1));
        }
        lua_setfield(L_, -2, "overloadDetails");
    }

    static const char* kindStr(MemberKind k)
    {
        switch (k)
        {
        case MemberKind::Method: return "method";
        case MemberKind::StaticMethod: return "static_method";
        case MemberKind::Property: return "property";
        case MemberKind::ReadOnlyProperty: return "readonly_property";
        case MemberKind::StaticProperty: return "static_property";
        case MemberKind::StaticReadOnlyProperty: return "static_readonly_property";
        case MemberKind::Constructor: return "constructor";
        case MemberKind::Metamethod: return "metamethod";
        default: return "unknown";
        }
    }

    lua_State* L_;
    int freeMemberIdx_ = 1;
    int classIdx_ = 1;
    int subNsIdx_ = 1;
    int memberIdx_ = 1;
};

//=================================================================================================
/**
 * @brief Print a TypeScript-style inspection of a namespace to an ostream.
 *
 * Convenience wrapper around ConsoleVisitor.
 */
inline void inspectPrint(lua_State* L, const char* namespaceName = nullptr, std::ostream& stream = std::cerr)
{
    auto ns = inspectNamespace(L, namespaceName);
    ConsoleVisitor v(stream);
    accept(ns, v);
}

//=================================================================================================
/**
 * @brief Push a structured Lua table describing a namespace onto the Lua stack.
 *
 * Convenience wrapper around LuaTableVisitor.
 * Pushes exactly 1 value (the result table).
 */
inline void inspectToLua(lua_State* L, const char* namespaceName = nullptr)
{
    auto ns = inspectNamespace(L, namespaceName);
    LuaTableVisitor v(L);
    accept(ns, v);
}

} // namespace luabridge
