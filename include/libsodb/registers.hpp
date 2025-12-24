#pragma once

#include <sys/user.h>
#include <libsodb/register_info.hpp>

namespace sodb {
class process;
class registers {
public:
    registers() = delete;
    registers(const registers&) = delete;
    registers& operator=(const registers&) = delete;

    // using value = /*?*/;
};
};  // namespace sodb
