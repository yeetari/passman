#include "cli.hh"

#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

#include <string.h>
#include <sys/resource.h>

int main(int argc, char **argv) {
    std::vector<std::string> args(argv + 1, argv + argc);
    for (int i = 1; i < argc; i++) {
        explicit_bzero(argv[i], strlen(argv[i]));
    }

    // Disable core dumps.
    struct rlimit core_rlimit {};
    if (setrlimit(RLIMIT_CORE, &core_rlimit) < 0) {
        std::perror("setrlimit");
    }

    std::string_view db_path;
    for (const auto &arg : args) {
        if (arg == "--version") {
            std::cout << "passman v0.1.0\n";
            return EXIT_SUCCESS;
        }
        if (arg[0] == '-') {
            std::cerr << "fatal: unknown option '" << arg << "'\n";
            return EXIT_FAILURE;
        }
        if (db_path.empty()) {
            db_path = arg;
            continue;
        }
        std::cerr << "fatal: unexpected argument '" << arg << "'\n";
        return EXIT_FAILURE;
    }

    if (db_path.empty()) {
        std::cout << "usage: " << argv[0] << " [--version] <db-file>\n";
        return EXIT_SUCCESS;
    }

    std::filesystem::path path(db_path);
    CommandLine cli(path.filename().string());
    cli.run();
}
