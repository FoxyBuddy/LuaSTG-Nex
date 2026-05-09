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
		//unit池最大容量
		static constexpr size_t kDefaultMaxUnits = 32768;
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
		size_t m_max_units{ kDefaultMaxUnits };
	};

	UnitPool& GetUnitPool() noexcept;
}
