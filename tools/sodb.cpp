#include <editline/readline.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <libsodb/error.hpp>
#include <libsodb/process.hpp>
#include <print>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
std::unique_ptr<sodb::process> attach(int argc, const char **argv) {
    if (argc == 3 && argv[1] == std::string_view("-p")) {
        pid_t pid = std::atoi(argv[2]);
        return sodb::process::attach(pid);
    } else {
        const char *program_path = argv[1];
        return sodb::process::launch(program_path);
    }
}

std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss{std::string{str}};
    std::string item;

    while (std::getline(ss, item, delimiter)) {
        out.push_back(item);
    }

    return out;
}

bool is_prefix(std::string_view str, std::string_view of) {
    if (str.size() > of.size())
        return false;
    return std::equal(str.begin(), str.end(), of.begin());
}

void print_stop_reason(const sodb::process &process, sodb::stop_reason reason) {
    std::print("Process {} ", process.pid());
    switch (reason.reason) {
    case sodb::process_state::exited:
        std::print("exited with status {}", static_cast<int>(reason.info));
        break;
    case sodb::process_state::terminated:
        std::print("terminated with signal {}", sigabbrev_np(reason.info));
        break;
    case sodb::process_state::stopped:
        std::print("stopped with signal {}", sigabbrev_np(reason.info));
        break;
    case sodb::process_state::running:
        break;
    }
    std::println("");
}

void handle_command(std::unique_ptr<sodb::process> &process,
                    std::string_view line) {
    auto args = split(line, ' ');
    auto command = args[0];

    if (is_prefix(command, "continue")) {
        process->resume();
        auto reason = process->wait_on_signal();
        print_stop_reason(*process, reason);
    } else {
        std::cerr << "Unknown command\n";
    }
}

void main_loop(std::unique_ptr<sodb::process> &process) {
    char *line = nullptr;
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
            } catch (const sodb::error &err) {
                std::cout << err.what() << '\n';
            }
        }
    }
}
} // namespace

int main(int argc, const char **argv) {
    if (argc == 1) {
        std::cerr << "No arguments given\n";
        return -1;
    }

    try {
        auto process = attach(argc, argv);
        main_loop(process);
    } catch (const sodb::error &err) {
        std::cout << err.what() << '\n';
    }
}
