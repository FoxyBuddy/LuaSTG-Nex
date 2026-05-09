#pragma once
#include "Unit/Unit.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace luastg {
	struct UnitHandle {
		uint32_t id{};
		uint32_t generation{};
	};

	class UnitPool {
	public:
		UnitHandle create();
		bool destroy(UnitHandle handle) noexcept;
		Unit* get(UnitHandle handle) noexcept;
		Unit const* get(UnitHandle handle) const noexcept;
		void updateAll() noexcept;
		void clear() noexcept;
		[[nodiscard]] size_t count() const noexcept;

	private:
		struct Slot {
			Unit unit{};
			uint32_t generation{ 1 };
			bool occupied{};
		};

		std::vector<Slot> m_slots;
		std::vector<uint32_t> m_free_list;
		size_t m_alive_count{};
	};

	UnitPool& GetUnitPool() noexcept;
}
