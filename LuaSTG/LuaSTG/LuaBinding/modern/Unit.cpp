#include "LuaBinding/modern/Unit.hpp"
#include "LuaBinding/LuaWrapper.hpp"
#include "Unit/UnitPool.hpp"
#include <cstring>

namespace {
	constexpr char const* kUnitMetatable = "lstg.Unit.instance";

	struct UnitUserData {
		luastg::UnitHandle handle{};
	};

	UnitUserData* check_unit_userdata(lua_State* const vm, int const index) {
		return static_cast<UnitUserData*>(luaL_checkudata(vm, index, kUnitMetatable));
	}

	luastg::Unit* check_unit(lua_State* const vm, int const index) {
		auto const ud = check_unit_userdata(vm, index);
		auto* unit = luastg::GetUnitPool().get(ud->handle);
		if (!unit) {
			luaL_error(vm, "invalid or destroyed lstg.Unit");
			return nullptr;
		}
		return unit;
	}

	void push_unit(lua_State* const vm, luastg::UnitHandle const handle) {
		auto* ud = static_cast<UnitUserData*>(lua_newuserdata(vm, sizeof(UnitUserData)));
		ud->handle = handle;
		luaL_getmetatable(vm, kUnitMetatable);
		lua_setmetatable(vm, -2);
	}

	int unit_new(lua_State* const vm) {
		auto handle = luastg::GetUnitPool().create();
		push_unit(vm, handle);
		return 1;
	}

	int unit_delete(lua_State* const vm) {
		auto const ud = check_unit_userdata(vm, 1);
		lua_pushboolean(vm, luastg::GetUnitPool().destroy(ud->handle));
		return 1;
	}

	int unit_is_valid(lua_State* const vm) {
		auto const ud = check_unit_userdata(vm, 1);
		lua_pushboolean(vm, luastg::GetUnitPool().get(ud->handle) != nullptr);
		return 1;
	}

	int unit_update_all(lua_State* const vm) {
		luastg::GetUnitPool().updateAll();
		return 0;
	}

	int unit_clear(lua_State* const vm) {
		luastg::GetUnitPool().clear();
		return 0;
	}

	int unit_count(lua_State* const vm) {
		lua_pushinteger(vm, static_cast<lua_Integer>(luastg::GetUnitPool().count()));
		return 1;
	}

	int unit_tostring(lua_State* const vm) {
		auto const ud = check_unit_userdata(vm, 1);
		auto* unit = luastg::GetUnitPool().get(ud->handle);
		if (!unit) {
			lua_pushfstring(vm, "lstg.Unit<destroyed:%u:%u>", ud->handle.id, ud->handle.generation);
		}
		else {
			lua_pushfstring(vm, "lstg.Unit<%u:%u>", unit->id, unit->generation);
		}
		return 1;
	}

	int unit_index(lua_State* const vm) {
		auto* unit = check_unit(vm, 1);
		char const* key = luaL_checkstring(vm, 2);

		if (std::strcmp(key, "id") == 0) {
			lua_pushinteger(vm, static_cast<lua_Integer>(unit->id));
			return 1;
		}
		if (std::strcmp(key, "generation") == 0) {
			lua_pushinteger(vm, static_cast<lua_Integer>(unit->generation));
			return 1;
		}
		if (std::strcmp(key, "alive") == 0) {
			lua_pushboolean(vm, unit->alive);
			return 1;
		}
		if (std::strcmp(key, "timer") == 0) {
			lua_pushinteger(vm, static_cast<lua_Integer>(unit->timer));
			return 1;
		}

		if (std::strcmp(key, "x") == 0) { lua_pushnumber(vm, unit->x); return 1; }
		if (std::strcmp(key, "y") == 0) { lua_pushnumber(vm, unit->y); return 1; }
		if (std::strcmp(key, "vx") == 0) { lua_pushnumber(vm, unit->vx); return 1; }
		if (std::strcmp(key, "vy") == 0) { lua_pushnumber(vm, unit->vy); return 1; }
		if (std::strcmp(key, "ax") == 0) { lua_pushnumber(vm, unit->ax); return 1; }
		if (std::strcmp(key, "ay") == 0) { lua_pushnumber(vm, unit->ay); return 1; }
		if (std::strcmp(key, "rot") == 0) { lua_pushnumber(vm, unit->rot); return 1; }

		// Methods exposed on instances.
		if (std::strcmp(key, "delete") == 0 || std::strcmp(key, "destroy") == 0) {
			lua_pushcfunction(vm, unit_delete);
			return 1;
		}
		if (std::strcmp(key, "isValid") == 0) {
			lua_pushcfunction(vm, unit_is_valid);
			return 1;
		}

		lua_pushnil(vm);
		return 1;
	}

	int unit_newindex(lua_State* const vm) {
		auto* unit = check_unit(vm, 1);
		char const* key = luaL_checkstring(vm, 2);

		if (std::strcmp(key, "x") == 0) { unit->x = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "y") == 0) { unit->y = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "vx") == 0) { unit->vx = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "vy") == 0) { unit->vy = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "ax") == 0) { unit->ax = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "ay") == 0) { unit->ay = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "rot") == 0) { unit->rot = luaL_checknumber(vm, 3); return 0; }
		if (std::strcmp(key, "alive") == 0) {
			unit->alive = lua_toboolean(vm, 3) != 0;
			return 0;
		}

		return luaL_error(vm, "unknown or read-only lstg.Unit field '%s'", key);
	}

	void create_unit_metatable(lua_State* const vm) {
		if (luaL_newmetatable(vm, kUnitMetatable)) {
			lua_pushcfunction(vm, unit_index);
			lua_setfield(vm, -2, "__index");
			lua_pushcfunction(vm, unit_newindex);
			lua_setfield(vm, -2, "__newindex");
			lua_pushcfunction(vm, unit_tostring);
			lua_setfield(vm, -2, "__tostring");
		}
		lua_pop(vm, 1);
	}
}

namespace luastg::binding {
	void Unit::registerClass(lua_State* const vm) {
		create_unit_metatable(vm);

		luaL_Reg const unit_api[] = {
			{ "new", &unit_new },
			{ "delete", &unit_delete },
			{ "destroy", &unit_delete },
			{ "isValid", &unit_is_valid },
			{ "updateAll", &unit_update_all },
			{ "clear", &unit_clear },
			{ "count", &unit_count },
			{ nullptr, nullptr },
		};

		luaL_register(vm, LUASTG_LUA_LIBNAME ".Unit", unit_api); // ... lstg.Unit
		luaL_register(vm, LUASTG_LUA_LIBNAME, nullptr); // ... lstg.Unit lstg
		lua_pushvalue(vm, -2); // ... lstg.Unit lstg lstg.Unit
		lua_setfield(vm, -2, "Unit"); // ... lstg.Unit lstg
		lua_pop(vm, 2);
	}
}
