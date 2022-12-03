#pragma once

#include <functional>
#include <map>
#include <string>

class CommandLine {
    using args_list_t = std::vector<std::string> &&;
    struct Command {
        std::function<void(args_list_t)> handler;
        std::string help;
        std::string usage;
    };

    const std::string m_db_name;
    std::map<std::string, Command> m_commands;
    bool m_locked{false};
    bool m_unsaved_changes{false};

    std::string build_prompt() const;
    void encrypt(args_list_t) const;
    void hash(args_list_t) const;
    void help(args_list_t) const;

public:
    explicit CommandLine(std::string db_name);

    void run();
};
