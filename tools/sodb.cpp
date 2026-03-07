#include <editline/readline.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <libsodb/error.hpp>
#include <libsodb/parse.hpp>
#include <libsodb/process.hpp>
#include <libsodb/register_info.hpp>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace {
std::unique_ptr<sodb::process> attach(int argc, const char** argv) {
    if (argc == 3 && argv[1] == std::string_view("-p")) {
        pid_t pid = std::atoi(argv[2]);
        return sodb::process::attach(pid);
    } else {
        const char* program_path = argv[1];
        return sodb::process::launch(program_path);
    }
}

std::vector<std::string> split(std::string_view str, char delimiter) {
    return str | std::views::split(delimiter) | std::ranges::to<std::vector<std::string>>();
}

void print_stop_reason(const sodb::process& process, sodb::stop_reason reason) {
    std::string message;
    switch (reason.reason) {
        case sodb::process_state::exited:
            message = std::format("Exited with status {}", static_cast<int>(reason.info));
            break;
        case sodb::process_state::terminated:
            message = std::format("Terminated with signal {}", sigabbrev_np(reason.info));
            break;
        case sodb::process_state::stopped:
            message = std::format("Stopped with signal {} at {:#x}",
                                  sigabbrev_np(reason.info),
                                  process.get_pc().addr());
            break;
        case sodb::process_state::running:
            break;
    }
    std::println("Process {} {}", process.pid(), message);
}

void print_help(const std::vector<std::string>& args) {
    if (args.empty()) return;
    if (args.size() == 1) {
        std::println(std::cerr, R"(Available commands:
        continue - Resume the process
        register - Commands for operating on registers
        )");
    } else if (args.size() >= 2 && args[1].starts_with("register")) {
        std::println(std::cerr, R"(Available commands:
        read
        read <register>
        read all
        write <register> <value>
        )");
    } else {
        std::println(std::cerr, "No help available for that.");
    }
}

void handle_register_read(sodb::process& process, const std::vector<std::string>& args) {
    auto format = []<typename T>(const T& t) {
        if constexpr (std::floating_point<T>) {
            return std::format("{}", t);
        } else if constexpr (std::integral<T>) {
            return std::format("{:#0{}x}", t, sizeof(T) * 2 + 2);
        } else {
            auto as_ints = t | std::views::transform([](auto val) {
                               using ValT = std::remove_cvref_t<decltype(val)>;
                               if constexpr (std::is_same_v<ValT, std::byte>) {
                                   return std::to_integer<unsigned int>(val);
                               } else {
                                   return val;
                               }
                           });
            return std::format("{::#04x}", as_ints);
        }
    };

    if (args.size() == 2 || (args.size() == 3 && args[2] == "all")) {
        for (auto& info : sodb::g_register_infos) {
            auto should_print = (args.size() == 3 || info.type == sodb::register_type::gpr) &&
                                info.name != "orig_rax";
            if (!should_print) continue;
            auto value = process.get_registers().read(info);
            std::println("{}:\t{}", info.name, std::visit(format, value));
        }
    } else if (args.size() == 3) {
        try {
            auto info = sodb::register_info_by_name(args[2]);
            auto value = process.get_registers().read(info);
            std::println("{}:\t{}", info.name, std::visit(format, value));
        } catch (sodb::error& err) {
            std::println(std::cerr, "No such register.");
            return;
        }
    } else {
        print_help({"help", "register"});
    }
}

sodb::registers::value parse_register_value(sodb::register_info info, std::string_view text) {
    try {
        if (info.format == sodb::register_format::uint) {
            switch (info.size) {
                case 1:
                    return sodb::to_integral<std::uint8_t>(text, 16).value();
                case 2:
                    return sodb::to_integral<std::uint16_t>(text, 16).value();
                case 4:
                    return sodb::to_integral<std::uint32_t>(text, 16).value();
                case 8:
                    return sodb::to_integral<std::uint64_t>(text, 16).value();
            }
        } else if (info.format == sodb::register_format::double_float) {
            return sodb::to_float<double>(text).value();
        } else if (info.format == sodb::register_format::long_double) {
            return sodb::to_float<long double>(text).value();
        } else if (info.format == sodb::register_format::vector) {
            if (info.size == 8) {
                return sodb::parse_vector<8>(text);
            } else if (info.size == 16) {
                return sodb::parse_vector<16>(text);
            }
        }
    } catch (...) {
    }
    sodb::error::send("Invalid format");
}

void handle_register_write(sodb::process& process, const std::vector<std::string>& args) {
    if (args.size() != 4) {
        print_help({"help", "register"});
        return;
    }

    try {
        auto info = sodb::register_info_by_name(args[2]);
        auto value = parse_register_value(info, args[3]);
        process.get_registers().write(info, value);
    } catch (sodb::error& err) {
        std::println(std::cerr, "{}", err.what());
        return;
    }
}

void handle_register_command(sodb::process& process, const std::vector<std::string>& args) {
    if (args.size() < 2) {
        print_help({"help", "register"});
        return;
    }

    if (args[1].starts_with("read")) {
        handle_register_read(process, args);
    } else if (args[1].starts_with("write")) {
        handle_register_write(process, args);
    } else {
        print_help({"help", "register"});
    }
}

void handle_command(std::unique_ptr<sodb::process>& process, std::string_view line) {
    auto args = split(line, ' ');
    if (args.empty()) return;
    auto command = args[0];

    if (command.starts_with("continue")) {
        process->resume();
        auto reason = process->wait_on_signal();
        print_stop_reason(*process, reason);
    } else if (command.starts_with("help")) {
        print_help(args);
    } else if (command.starts_with("register")) {
        handle_register_command(*process, args);
    } else {
        std::println(std::cerr, "Unknown command");
    }
}

void main_loop(std::unique_ptr<sodb::process>& process) {
    char* line = nullptr;
    while ((line = readline("sodb> ")) != nullptr) {
        std::string line_str;

        if (line == std::string_view("")) {
            free(line);
            if (history_length > 0) {
                line_str = history_list()[history_length - 1]->line;
            }
        } else {
            line_str = line;
            add_history(line);
            free(line);
        }

        if (!line_str.empty()) {
            try {
                handle_command(process, line_str);
            } catch (const sodb::error& err) {
                std::println("{}", err.what());
            }
        }
    }
}
}  // namespace

int main(int argc, const char** argv) {
    if (argc == 1) {
        std::println(std::cerr, "No arguments given");
        return -1;
    }

    try {
        auto process = attach(argc, argv);
        main_loop(process);
    } catch (const sodb::error& err) {
        std::println("{}", err.what());
    }
}
