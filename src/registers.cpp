#include <cstdint>
#include <iostream>
#include <libsodb/bit.hpp>
#include <libsodb/process.hpp>
#include <libsodb/registers.hpp>
#include <type_traits>
#include <variant>

namespace {
template <class T>
sodb::byte128 widen(const sodb::register_info& info, T t) {
    using namespace sodb;
    if constexpr (std::is_floating_point_v<T>) {
        if (info.format == register_format::double_float) {
            return to_byte128(static_cast<double>(t));
        }
        if (info.format == register_format::long_double) {
            return to_byte128(static_cast<long double>(t));
        }
    } else if constexpr (std::is_signed_v<T>) {
        if (info.format == register_format::uint) {
            switch (info.size) {
                case 2:
                    return to_byte128(static_cast<std::int16_t>(t));
                case 4:
                    return to_byte128(static_cast<std::int32_t>(t));
                case 8:
                    return to_byte128(static_cast<std::int64_t>(t));
            }
        }
    }
    return to_byte128(t);
}
}  // namespace

sodb::registers::value sodb::registers::read(const register_info& info) const {
    auto bytes = as_bytes(data_);

    if (info.format == register_format::uint) {
        switch (info.size) {
            case 1:
                return from_bytes<std::uint8_t>(bytes + info.offest);
            case 2:
                return from_bytes<std::uint16_t>(bytes + info.offest);
            case 4:
                return from_bytes<std::uint32_t>(bytes + info.offest);
            case 8:
                return from_bytes<std::uint64_t>(bytes + info.offest);
            default:
                sodb::error::send("Unexpected register size.");
        }
    } else if (info.format == register_format::double_float) {
        return from_bytes<double>(bytes + info.offest);
    } else if (info.format == register_format::long_double) {
        return from_bytes<long double>(bytes + info.offest);
    } else if (info.format == register_format::vector and info.size == 8) {
        return from_bytes<byte64>(bytes + info.offest);
    } else {
        return from_bytes<byte128>(bytes + info.offest);
    }
}

void sodb::registers::write(const register_info& info, value val) {
    auto bytes = as_bytes(data_);
    std::visit(
        [&](auto& v) {
            if (sizeof(v) <= info.size) {
                auto wide = widen(info, v);
                auto val_bytes = as_bytes(wide);
                std::copy(val_bytes, val_bytes + info.size, bytes + info.offest);
            } else {
                std::println(
                    std::cerr,
                    "sodb::register::write called with mismatched register and value sizes.");
                std::terminate();
            }
        },
        val);
    auto aligned_offset = info.offest & ~0b111;

    if (info.type == register_type::fpr) {
        proc_->write_fprs(data_.i387);
    } else {
        proc_->write_user_area(aligned_offset, from_bytes<std::uint64_t>(bytes + aligned_offset));
    }
}
