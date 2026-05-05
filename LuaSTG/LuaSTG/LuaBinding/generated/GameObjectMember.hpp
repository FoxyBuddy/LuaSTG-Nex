// This file was generated through a code generator, DO NOT edit it, please edit the code generator

#ifndef _LuaSTG_GameObjectMember_h_
#define _LuaSTG_GameObjectMember_h_

namespace LuaSTG {

enum class GameObjectMember : int {
    __unknown__ = -1,
    STATUS = 37,
    CLASS = 15,
    TIMER = 38,
    X = 43,
    Y = 44,
    ROT = 36,
    HSCALE = 22,
    VSCALE = 39,
    DX = 18,
    DY = 19,
    OMEGA = 30,
    AX = 11,
    AY = 12,
    VX = 40,
    VY = 41,
    NAVI = 28,
    LAYER = 24,
    HIDE = 21,
    IMG = 23,
    RES_RC = 33,
    ANI = 10,
    BOUND = 14,
    GROUP = 20,
    COLLI = 16,
    A = 8,
    B = 13,
    RECT = 34,
    COLLIDER = 17,
    VANGLE = 1,
    VSPEED = 7,
    PAUSE = 32,
    IGNORESUPERPAUSE = 29,
    RESOLVEMOVE = 35,
    WORLD = 42,
};

GameObjectMember MapGameObjectMember(char const* const key, size_t const len) noexcept;

}

#endif // _LuaSTG_GameObjectMember_h_
