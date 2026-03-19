#pragma once

#include <libsodb/types.hpp>
#include <memory>

namespace sodb {
template <class Stoppoint>
class stoppoint_collection {
    Stoppoint& push(std::unique_ptr<Stoppoint> bs);

    bool contains_id(typename Stoppoint::id_type id) const;
    bool contains_address(virt_addr address) const;
    bool enable_stoppoint_at_address(virt_addr address) const;

    Stoppoint& get_by_id(typename Stoppoint::id_type id);
};
}  // namespace sodb
