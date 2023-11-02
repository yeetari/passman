#include "cli.hh"
#include "db.hh"
#include "gui/window.hh"

#include <QApplication>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

#include <string.h>
#include <sys/resource.h>

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        explicit_bzero(argv[i], strlen(argv[i]));
    }

    // Disable core dumps.
    struct rlimit core_rlimit {};
    if (setrlimit(RLIMIT_CORE, &core_rlimit) < 0) {
        std::perror("setrlimit");
    }

    if (!args[0].ends_with("passman-cli") && !args[0].ends_with("passman-gui")) {
        std::cerr << "fatal: multi-call binary\n";
        return EXIT_FAILURE;
    }

    std::string_view db_path;
    for (int i = 1; i < argc; i++) {
        const auto &arg = args[i];
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

    Database database;
    if (args[0].ends_with("passman-gui")) {
        QApplication application(argc, argv);
        MainWindow window(database);
        window.show();
        return QApplication::exec();
    }

    if (db_path.empty()) {
        std::cout << "usage: " << argv[0] << " [--version] <db-file>\n";
        return EXIT_SUCCESS;
    }

    CommandLine cli(std::filesystem::path(db_path).filename().string());
    cli.run();
}
