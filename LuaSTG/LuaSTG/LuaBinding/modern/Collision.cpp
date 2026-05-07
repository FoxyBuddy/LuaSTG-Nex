#include "LuaBinding/modern/Collision.hpp"
#include "LuaBinding/modern/GameObject.hpp"
#include "LuaBinding/LuaWrapper.hpp"
#include "GameObject/GameObjectPool.h"
#include "AppFrame.h"
#include "XCollision.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using std::string_view_literals::operator ""sv;

namespace {
	constexpr double pi = 3.141592653589793238462643383279502884;
	constexpr double deg_to_rad = pi / 180.0;

	enum class ColliderShape : uint8_t {
		Circle,
		Ellipse,
		OBB,
	};

	struct ColliderDef {
		double x{};
		double y{};
		double rot{}; // radian
		double a{ 0.0 };
		double b{ 0.0 };
		ColliderShape shape{ ColliderShape::Circle };
		bool enabled{ true };
		bool inherit_rot{ true };
	};

	struct Collider {
		uint32_t id{};
		uint32_t profile{};
		luastg::GameObject* owner{};
		uint64_t owner_unique_id{};
		ColliderDef def{};
		bool alive{ true };
	};

	struct CollisionRule {
		uint32_t id{};
		uint32_t profile_a{};
		uint32_t profile_b{};
		int callback_ref{ LUA_NOREF };
		bool enabled{ true };
		bool allow_same_owner{ false };
	};

	struct WorldCollider {
		Collider* collider{};
		float x{};
		float y{};
		float rot{};
		float a{};
		float b{};
		float r{};
		xmath::collision::ColliderType type{};
	};

	struct DetectionResult {
		uint32_t rule_id{};
		uint32_t collider_a{};
		uint32_t collider_b{};
		uint64_t owner_a_uid{};
		uint64_t owner_b_uid{};
	};

	int abs_index(lua_State* const vm, int const index) noexcept {
		return index < 0 ? lua_gettop(vm) + index + 1 : index;
	}

	bool get_bool_field(lua_State* const vm, int table, char const* const key, bool const default_value) {
		table = abs_index(vm, table);
		lua_getfield(vm, table, key);
		bool value = default_value;
		if (!lua_isnil(vm, -1)) {
			value = lua_toboolean(vm, -1) != 0;
		}
		lua_pop(vm, 1);
		return value;
	}

	double get_number_field(lua_State* const vm, int table, char const* const key, double const default_value) {
		table = abs_index(vm, table);
		lua_getfield(vm, table, key);
		double value = default_value;
		if (!lua_isnil(vm, -1)) {
			value = luaL_checknumber(vm, -1);
		}
		lua_pop(vm, 1);
		return value;
	}

	std::string get_string_field(lua_State* const vm, int table, char const* const key, std::string_view const default_value) {
		table = abs_index(vm, table);
		lua_getfield(vm, table, key);
		std::string value(default_value);
		if (!lua_isnil(vm, -1)) {
			value = luaL_checkstring(vm, -1);
		}
		lua_pop(vm, 1);
		return value;
	}

	ColliderDef read_collider_def(lua_State* const vm, int const index) {
		ColliderDef def{};
		if (lua_isnumber(vm, index)) {
			def.a = luaL_checknumber(vm, index);
			def.b = def.a;
			def.shape = ColliderShape::Circle;
			return def;
		}

		luaL_checktype(vm, index, LUA_TTABLE);
		int const table = abs_index(vm, index);

		def.x = get_number_field(vm, table, "x", 0.0);
		def.y = get_number_field(vm, table, "y", 0.0);
		def.rot = get_number_field(vm, table, "rot", 0.0) * deg_to_rad;
		def.enabled = get_bool_field(vm, table, "enabled", true);
		def.inherit_rot = get_bool_field(vm, table, "inherit_rot", true);

		lua_getfield(vm, table, "radius");
		if (!lua_isnil(vm, -1)) {
			def.a = luaL_checknumber(vm, -1);
			def.b = def.a;
		}
		lua_pop(vm, 1);

		def.a = get_number_field(vm, table, "a", def.a);
		def.b = get_number_field(vm, table, "b", def.b == 0.0 ? def.a : def.b);

		if (def.a < 0.0 || def.b < 0.0) {
			luaL_error(vm, "collider half size should not be negative");
		}
		if (def.b == 0.0) {
			def.b = def.a;
		}

		auto const shape = get_string_field(vm, table, "shape", "");
		auto const is_rect = get_bool_field(vm, table, "is_rect", get_bool_field(vm, table, "rect", false));
		if (is_rect || shape == "rect" || shape == "obb") {
			def.shape = ColliderShape::OBB;
		}
		else if (shape == "ellipse" || std::abs(def.a - def.b) > std::numeric_limits<double>::epsilon()) {
			def.shape = ColliderShape::Ellipse;
		}
		else {
			def.shape = ColliderShape::Circle;
		}
		return def;
	}

	xmath::collision::ColliderType to_xcollision_type(ColliderShape const shape) noexcept {
		switch (shape) {
		case ColliderShape::OBB:
			return xmath::collision::ColliderType::OBB;
		case ColliderShape::Ellipse:
			return xmath::collision::ColliderType::Ellipse;
		case ColliderShape::Circle:
		default:
			return xmath::collision::ColliderType::Circle;
		}
	}

	bool is_owner_alive(Collider const& c) noexcept {
		return c.owner != nullptr
			&& c.owner->unique_id == c.owner_unique_id
			&& c.owner->status == luastg::GameObjectStatus::Active;
	}

	bool make_world_collider(Collider& src, WorldCollider& out) noexcept {
		if (!src.alive || !src.def.enabled || !is_owner_alive(src)) {
			return false;
		}

		auto const owner = src.owner;
		auto const owner_rot = src.def.inherit_rot ? owner->rot : 0.0;
		auto const cos_r = std::cos(owner_rot);
		auto const sin_r = std::sin(owner_rot);
		auto const local_x = src.def.x;
		auto const local_y = src.def.y;

		out.collider = &src;
		out.x = static_cast<float>(owner->x + local_x * cos_r - local_y * sin_r);
		out.y = static_cast<float>(owner->y + local_x * sin_r + local_y * cos_r);
		out.rot = static_cast<float>(owner_rot + src.def.rot);
		out.a = static_cast<float>(src.def.a);
		out.b = static_cast<float>(src.def.b);
		out.r = static_cast<float>(std::hypot(src.def.a, src.def.b));
		out.type = to_xcollision_type(src.def.shape);
		return true;
	}

	bool is_aabb_not_intersect(WorldCollider const& a, WorldCollider const& b) noexcept {
		return (a.x + a.r) < (b.x - b.r)
			|| (a.x - a.r) > (b.x + b.r)
			|| (a.y + a.r) < (b.y - b.r)
			|| (a.y - a.r) > (b.y + b.r);
	}

	bool is_intersect(WorldCollider const& a, WorldCollider const& b) noexcept {
		if (is_aabb_not_intersect(a, b)) {
			return false;
		}

		cocos2d::Vec2 const xy1(a.x, a.y);
		cocos2d::Vec2 const xy2(b.x, b.y);

		if (!xmath::collision::check(
			xy1, a.r, a.r, a.rot, xmath::collision::ColliderType::Circle,
			xy2, b.r, b.r, b.rot, xmath::collision::ColliderType::Circle)) {
			return false;
		}

		return xmath::collision::check(
			xy1, a.a, a.b, a.rot, a.type,
			xy2, b.a, b.b, b.rot, b.type);
	}

	class CollisionWorld {
	public:
		void bindLuaState(lua_State* const vm) noexcept {
			m_vm = vm;
		}

		uint32_t profile(std::string_view const name) {
			auto const it = m_profile_to_id.find(std::string(name));
			if (it != m_profile_to_id.end()) {
				return it->second;
			}

			auto const id = static_cast<uint32_t>(m_profile_names.size() + 1);
			m_profile_names.emplace_back(name);
			m_profile_to_id.emplace(m_profile_names.back(), id);
			m_buckets.emplace_back();
			return id;
		}

		std::string_view profileName(uint32_t const id) const noexcept {
			if (id == 0 || id > m_profile_names.size()) {
				return {};
			}
			return m_profile_names[id - 1];
		}

		uint32_t addCollider(luastg::GameObject* owner, std::string_view const profile_name, ColliderDef def) {
			auto const profile_id = profile(profile_name);
			auto const id = static_cast<uint32_t>(m_colliders.size() + 1);
			m_colliders.push_back(Collider{
				.id = id,
				.profile = profile_id,
				.owner = owner,
				.owner_unique_id = owner->unique_id,
				.def = def,
				.alive = true,
			});
			m_buckets[profile_id - 1].push_back(id);
			return id;
		}

		bool removeCollider(uint32_t const id) noexcept {
			auto* c = getCollider(id);
			if (c == nullptr) {
				return false;
			}
			c->alive = false;
			c->def.enabled = false;
			return true;
		}

		uint32_t removeOwner(luastg::GameObject const* owner) noexcept {
			if (owner == nullptr) {
				return 0;
			}
			uint32_t count = 0;
			for (auto& c : m_colliders) {
				if (c.alive && c.owner == owner && c.owner_unique_id == owner->unique_id) {
					c.alive = false;
					c.def.enabled = false;
					count += 1;
				}
			}
			return count;
		}

		bool setEnabled(uint32_t const id, bool const enabled) noexcept {
			auto* c = getCollider(id);
			if (c == nullptr) {
				return false;
			}
			c->def.enabled = enabled;
			return true;
		}

		uint32_t addRule(lua_State* const vm, std::string_view const a, std::string_view const b, int callback_ref, bool const allow_same_owner) {
			auto const id = static_cast<uint32_t>(m_rules.size() + 1);
			m_rules.push_back(CollisionRule{
				.id = id,
				.profile_a = profile(a),
				.profile_b = profile(b),
				.callback_ref = callback_ref,
				.enabled = true,
				.allow_same_owner = allow_same_owner,
			});
			m_vm = vm;
			return id;
		}

		bool removeRule(lua_State* const vm, uint32_t const id) {
			auto* rule = getRule(id);
			if (rule == nullptr) {
				return false;
			}
			if (rule->callback_ref != LUA_NOREF) {
				luaL_unref(vm, LUA_REGISTRYINDEX, rule->callback_ref);
				rule->callback_ref = LUA_NOREF;
			}
			rule->enabled = false;
			return true;
		}

		void clearRules(lua_State* const vm) {
			for (auto& rule : m_rules) {
				if (rule.callback_ref != LUA_NOREF) {
					luaL_unref(vm, LUA_REGISTRYINDEX, rule.callback_ref);
					rule.callback_ref = LUA_NOREF;
				}
				rule.enabled = false;
			}
			m_rules.clear();
		}

		void reset(lua_State* const vm) {
			clearRules(vm);
			m_colliders.clear();
			m_buckets.clear();
			m_profile_to_id.clear();
			m_profile_names.clear();
			m_last_checks = 0;
			m_last_hits = 0;
		}

		Collider* getCollider(uint32_t const id) noexcept {
			if (id == 0 || id > m_colliders.size()) {
				return nullptr;
			}
			auto& c = m_colliders[id - 1];
			return c.alive ? &c : nullptr;
		}

		CollisionRule* getRule(uint32_t const id) noexcept {
			if (id == 0 || id > m_rules.size()) {
				return nullptr;
			}
			auto& r = m_rules[id - 1];
			return r.enabled ? &r : nullptr;
		}

		std::pair<uint64_t, uint64_t> update(lua_State* const vm) {
			if (m_updating) {
				luaL_error(vm, "lstg.Collision.update is not reentrant");
			}
			m_updating = true;
			m_vm = vm;
			m_last_checks = 0;
			m_last_hits = 0;
			m_results.clear();

			for (auto const& rule : m_rules) {
				if (!rule.enabled || rule.callback_ref == LUA_NOREF) {
					continue;
				}
				if (rule.profile_a == 0 || rule.profile_a > m_buckets.size()
					|| rule.profile_b == 0 || rule.profile_b > m_buckets.size()) {
					continue;
				}
				auto const& bucket_a = m_buckets[rule.profile_a - 1];
				auto const& bucket_b = m_buckets[rule.profile_b - 1];
				for (size_t i = 0; i < bucket_a.size(); ++i) {
					auto* const collider_a = getCollider(bucket_a[i]);
					if (collider_a == nullptr) {
						continue;
					}
					WorldCollider world_a{};
					if (!make_world_collider(*collider_a, world_a)) {
						continue;
					}

					size_t const begin_b = rule.profile_a == rule.profile_b ? i + 1 : 0;
					for (size_t j = begin_b; j < bucket_b.size(); ++j) {
						auto* const collider_b = getCollider(bucket_b[j]);
						if (collider_b == nullptr || collider_a == collider_b) {
							continue;
						}
						if (!rule.allow_same_owner && collider_a->owner == collider_b->owner) {
							continue;
						}
						WorldCollider world_b{};
						if (!make_world_collider(*collider_b, world_b)) {
							continue;
						}
						m_last_checks += 1;
						if (!is_intersect(world_a, world_b)) {
							continue;
						}
						m_last_hits += 1;
						m_results.push_back(DetectionResult{
							.rule_id = rule.id,
							.collider_a = collider_a->id,
							.collider_b = collider_b->id,
							.owner_a_uid = collider_a->owner_unique_id,
							.owner_b_uid = collider_b->owner_unique_id,
						});
					}
				}
			}

			auto const checks = m_last_checks;
			auto const hits = m_last_hits;
			for (auto const& result : m_results) {
				auto* const rule = getRule(result.rule_id);
				auto* const collider_a = getCollider(result.collider_a);
				auto* const collider_b = getCollider(result.collider_b);
				if (rule == nullptr || collider_a == nullptr || collider_b == nullptr) {
					continue;
				}
				if (!is_owner_alive(*collider_a) || !is_owner_alive(*collider_b)) {
					continue;
				}
				if (collider_a->owner_unique_id != result.owner_a_uid || collider_b->owner_unique_id != result.owner_b_uid) {
					continue;
				}
				callRule(vm, *rule, *collider_a, *collider_b);
			}

			m_results.clear();
			m_updating = false;
			return { checks, hits };
		}

		uint64_t lastChecks() const noexcept { return m_last_checks; }
		uint64_t lastHits() const noexcept { return m_last_hits; }
		uint32_t colliderCount() const noexcept {
			uint32_t count = 0;
			for (auto const& c : m_colliders) {
				if (c.alive) {
					count += 1;
				}
			}
			return count;
		}
		uint32_t ruleCount() const noexcept {
			uint32_t count = 0;
			for (auto const& r : m_rules) {
				if (r.enabled) {
					count += 1;
				}
			}
			return count;
		}

	private:
		static void pushOwner(lua_State* const vm, luastg::GameObject const* const owner) {
			luastg::binding::GameObject::pushGameObjectTable(vm); // ... object_table
			lua_rawgeti(vm, -1, static_cast<int32_t>(owner->id + 1)); // ... object_table object
			lua_remove(vm, -2); // ... object
		}

		void callRule(lua_State* const vm, CollisionRule const& rule, Collider const& a, Collider const& b) {
			lua_rawgeti(vm, LUA_REGISTRYINDEX, rule.callback_ref);
			pushOwner(vm, a.owner);
			pushOwner(vm, b.owner);
			lua_pushinteger(vm, static_cast<lua_Integer>(a.id));
			lua_pushinteger(vm, static_cast<lua_Integer>(b.id));
			lua_pushlstring(vm, profileName(a.profile).data(), profileName(a.profile).size());
			lua_pushlstring(vm, profileName(b.profile).data(), profileName(b.profile).size());
			lua_call(vm, 6, 0);
		}

	private:
		lua_State* m_vm{};
		std::unordered_map<std::string, uint32_t> m_profile_to_id;
		std::vector<std::string> m_profile_names;
		std::vector<std::vector<uint32_t>> m_buckets;
		std::vector<Collider> m_colliders;
		std::vector<CollisionRule> m_rules;
		std::vector<DetectionResult> m_results;
		uint64_t m_last_checks{};
		uint64_t m_last_hits{};
		bool m_updating{};
	};

	CollisionWorld& world() {
		static CollisionWorld instance;
		return instance;
	}

	struct CollisionCallbacks final : luastg::IGameObjectManagerCallbacks {
		std::string_view getCallbacksName() const noexcept override {
			return "collision"sv;
		}
		void onCreate(luastg::GameObject*) override {}
		void onDestroy(luastg::GameObject* object) override {
			world().removeOwner(object);
		}
		void onBeforeBatchDestroy() override {}
		void onAfterBatchDestroy() override {}
		void onBeforeBatchUpdate() override {}
		void onAfterBatchUpdate() override {}
		void onBeforeBatchRender() override {}
		void onAfterBatchRender() override {}
		void onBeforeBatchOutOfWorldBoundCheck() override {}
		void onAfterBatchOutOfWorldBoundCheck() override {}
		void onBeforeBatchIntersectDetect() override {}
		void onAfterBatchIntersectDetect() override {}

		static CollisionCallbacks& getInstance() {
			static CollisionCallbacks instance;
			return instance;
		}
	};

	int l_profile(lua_State* const vm) {
		auto const name = luaL_checkstring(vm, 1);
		lua_pushinteger(vm, static_cast<lua_Integer>(world().profile(name)));
		return 1;
	}

	int l_addCollider(lua_State* const vm) {
		auto* const owner = luastg::binding::GameObject::as(vm, 1);
		auto const profile = luaL_checkstring(vm, 2);
		auto const def = read_collider_def(vm, 3);
		lua_pushinteger(vm, static_cast<lua_Integer>(world().addCollider(owner, profile, def)));
		return 1;
	}

	int l_removeCollider(lua_State* const vm) {
		auto const id = static_cast<uint32_t>(luaL_checkinteger(vm, 1));
		lua_pushboolean(vm, world().removeCollider(id));
		return 1;
	}

	int l_clearOwner(lua_State* const vm) {
		auto* const owner = luastg::binding::GameObject::as(vm, 1);
		lua_pushinteger(vm, static_cast<lua_Integer>(world().removeOwner(owner)));
		return 1;
	}

	int l_setEnabled(lua_State* const vm) {
		auto const id = static_cast<uint32_t>(luaL_checkinteger(vm, 1));
		auto const enabled = lua_toboolean(vm, 2) != 0;
		lua_pushboolean(vm, world().setEnabled(id, enabled));
		return 1;
	}

	bool read_allow_same_owner(lua_State* const vm, int const index) {
		if (lua_isnoneornil(vm, index)) {
			return false;
		}
		if (lua_isboolean(vm, index)) {
			return lua_toboolean(vm, index) != 0;
		}
		if (lua_istable(vm, index)) {
			return get_bool_field(vm, index, "allow_same_owner", false);
		}
		return false;
	}

	int l_addRule(lua_State* const vm) {
		auto const profile_a = luaL_checkstring(vm, 1);
		auto const profile_b = luaL_checkstring(vm, 2);
		luaL_checktype(vm, 3, LUA_TFUNCTION);
		lua_pushvalue(vm, 3);
		auto const callback_ref = luaL_ref(vm, LUA_REGISTRYINDEX);
		auto const allow_same_owner = read_allow_same_owner(vm, 4);
		lua_pushinteger(vm, static_cast<lua_Integer>(world().addRule(vm, profile_a, profile_b, callback_ref, allow_same_owner)));
		return 1;
	}

	int l_removeRule(lua_State* const vm) {
		auto const id = static_cast<uint32_t>(luaL_checkinteger(vm, 1));
		lua_pushboolean(vm, world().removeRule(vm, id));
		return 1;
	}

	int l_clearRules(lua_State* const vm) {
		world().clearRules(vm);
		return 0;
	}

	int l_reset(lua_State* const vm) {
		world().reset(vm);
		return 0;
	}

	int l_update(lua_State* const vm) {
		auto const [checks, hits] = world().update(vm);
		lua_pushinteger(vm, static_cast<lua_Integer>(checks));
		lua_pushinteger(vm, static_cast<lua_Integer>(hits));
		return 2;
	}

	int l_getStatistics(lua_State* const vm) {
		lua_createtable(vm, 0, 4);
		lua_pushinteger(vm, static_cast<lua_Integer>(world().lastChecks()));
		lua_setfield(vm, -2, "checks");
		lua_pushinteger(vm, static_cast<lua_Integer>(world().lastHits()));
		lua_setfield(vm, -2, "hits");
		lua_pushinteger(vm, static_cast<lua_Integer>(world().colliderCount()));
		lua_setfield(vm, -2, "colliders");
		lua_pushinteger(vm, static_cast<lua_Integer>(world().ruleCount()));
		lua_setfield(vm, -2, "rules");
		return 1;
	}
}

namespace luastg::binding {
	void Collision::registerClass(lua_State* const vm) {
		world().bindLuaState(vm);
		LPOOL.addCallbacks(&CollisionCallbacks::getInstance());

		luaL_Reg const lib[] = {
			{ "profile", &l_profile },
			{ "addCollider", &l_addCollider },
			{ "removeCollider", &l_removeCollider },
			{ "clearOwner", &l_clearOwner },
			{ "setEnabled", &l_setEnabled },
			{ "addRule", &l_addRule },
			{ "removeRule", &l_removeRule },
			{ "clearRules", &l_clearRules },
			{ "reset", &l_reset },
			{ "update", &l_update },
			{ "getStatistics", &l_getStatistics },
			{ nullptr, nullptr },
		};

		luaL_register(vm, LUASTG_LUA_LIBNAME ".Collision", lib); // ... lstg.Collision
		luaL_register(vm, LUASTG_LUA_LIBNAME, nullptr); // ... lstg.Collision lstg
		lua_pushvalue(vm, -2); // ... lstg.Collision lstg lstg.Collision
		lua_setfield(vm, -2, "Collision"); // ... lstg.Collision lstg
		lua_pop(vm, 2); // ...
	}
}
