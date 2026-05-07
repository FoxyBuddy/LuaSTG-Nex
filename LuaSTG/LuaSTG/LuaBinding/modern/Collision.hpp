#pragma once
#include "lua.hpp"

namespace luastg::binding {
	struct Collision {
		static void registerClass(lua_State* vm);
	};
}
