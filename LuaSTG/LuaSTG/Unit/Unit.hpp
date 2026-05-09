#pragma once
#include <cstdint>

namespace luastg {
	struct Unit {
		uint32_t id{};
		uint32_t generation{};
		bool alive{};

		double x{};
		double y{};
		double vx{};
		double vy{};
		double ax{};
		double ay{};
		double rot{};

		uint64_t timer{};
	};
}
