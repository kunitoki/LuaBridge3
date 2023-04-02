// https://github.com/kunitoki/LuaBridge3
// Copyright 2019, Dmitry Tarakanov
// SPDX-License-Identifier: MIT

#include "TestBase.h"

#include "RefCountedObject.h"

struct RefCountedPtrTests : TestBase
{
    template<class T>
    T variable(const std::string& name)
    {
        runLua("result = " + name);
        return result<T>();
    }
};

namespace {

struct RefCounted : RefCountedObject
{
    explicit RefCounted(bool& deleted) : deleted(deleted) { deleted = false; }

    ~RefCounted() { deleted = true; }

    bool isDeleted() const { return deleted; }

    bool& deleted;
};

struct RefCountedStatic : RefCountedObject
{
    explicit RefCountedStatic() { constructed = true; }

    ~RefCountedStatic() { deleted = true; }

    static bool constructed;
    static bool deleted;
};

bool RefCountedStatic::constructed = false;
bool RefCountedStatic::deleted = false;

} // namespace

TEST_F(RefCountedPtrTests, Operators)
{
    bool deleted1 = false;
    auto* raw_ptr1 = new RefCounted(deleted1);
    RefCountedObjectPtr<RefCounted> ptr1(raw_ptr1);

    bool deleted2 = false;
    auto* raw_ptr2 = new RefCounted(deleted2);
    RefCountedObjectPtr<RefCounted> ptr2(raw_ptr2);

    ASSERT_TRUE(raw_ptr1 == ptr1.getObject());
    ASSERT_TRUE(ptr1.getObject() == raw_ptr1);
}

TEST_F(RefCountedPtrTests, LastReferenceInLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<RefCounted>("Class")
        .addProperty("deleted", &RefCounted::isDeleted)
        .endClass();

    bool deleted = false;

    RefCountedObjectPtr<RefCounted> object(new RefCounted(deleted));

    luabridge::setGlobal(L, object, "object");
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());

    object = nullptr;
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());
    ASSERT_EQ(false, deleted);

    runLua("object = nil");
    lua_gc(L, LUA_GCCOLLECT, 1);

    ASSERT_EQ(true, deleted);
}

TEST_F(RefCountedPtrTests, LastReferenceInCpp)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<RefCounted>("Class")
        .addProperty("deleted", &RefCounted::isDeleted)
        .endClass();

    bool deleted = false;

    RefCountedObjectPtr<RefCounted> object(new RefCounted(deleted));

    luabridge::setGlobal(L, object, "object");
    runLua("result = object.deleted");
    ASSERT_EQ(true, result().isBool());
    ASSERT_EQ(false, result<bool>());

    runLua("object = nil");
    lua_gc(L, LUA_GCCOLLECT, 1);
    ASSERT_EQ(false, deleted);

    object = nullptr;
    ASSERT_EQ(true, deleted);
}

TEST_F(RefCountedPtrTests, LastReferenceInstantiatedFromLua)
{
    luabridge::getGlobalNamespace(L)
        .beginClass<RefCountedStatic>("Class")
        .addConstructor<void()>()
        .endClass();

    EXPECT_FALSE(RefCountedStatic::constructed);
    EXPECT_FALSE(RefCountedStatic::deleted);

    runLua("local o = Class(); result = false");

    EXPECT_TRUE(RefCountedStatic::constructed);
    EXPECT_FALSE(RefCountedStatic::deleted);

    lua_gc(L, LUA_GCCOLLECT, 1);

    EXPECT_TRUE(RefCountedStatic::deleted);
}
