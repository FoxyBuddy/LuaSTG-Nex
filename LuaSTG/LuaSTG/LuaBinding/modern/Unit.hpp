#pragma once
#include "lua.hpp"

namespace luastg::binding {
	struct Unit {
		static void registerClass(lua_State* vm);
	};
}
