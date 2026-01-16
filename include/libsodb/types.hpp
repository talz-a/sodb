#pragma once

#include <array>
#include <cstddef>

namespace sodb {
using byte64 = std::array<std::byte, 8>;
using byte128 = std::array<std::byte, 16>;
}  // namespace sodb
