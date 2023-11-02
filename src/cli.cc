#include "cli.hh"

#include "blake.hh"
#include "chacha.hh"
#include "random.hh"

#include <iostream>
#include <string>

#include <readline/history.h>
#include <readline/readline.h>

CommandLine::CommandLine(std::string db_name, Database &database)
    : m_db_name(std::move(db_name)), m_database(database) {
    using namespace std::placeholders;
    m_commands["attach"] = {std::bind(&CommandLine::attach, this, _1), "attach a secret to an entry",
                            "attach <entry-name> <secret-group> <secret-name> <secret-value>"};
    m_commands["secret-groups"] = {std::bind(&CommandLine::secret_groups, this, _1), "list secret groups",
                                   "secret-groups"};
    m_commands["help"] = {std::bind(&CommandLine::help, this, _1), "shows this help message", "help"};
    m_commands["new-entry"] = {std::bind(&CommandLine::new_entry, this, _1), "create a new entry",
                               "new-entry <name> [tags ...]"};
    m_commands["new-secret-group"] = {std::bind(&CommandLine::new_secret_group, this, _1), "create a new secret group",
                                      "new-secret-group <name>"};
}

std::string CommandLine::build_prompt() const {
    if (m_locked) {
        return "\033[31;1m[locked due to inactivity] \033[0m";
    }
    std::string prompt = "\033[33m[database ";
    prompt += m_db_name;
    if (m_unsaved_changes) {
        prompt += " (unsaved changes)";
    }
    prompt += "]: \033[0m";
    return prompt;
}

void CommandLine::attach(args_list_t args) const {
    if (args.size() != 1) {
        std::cout << "invalid usage\n";
        return;
    }

    Blake blake(512);
    blake.update({reinterpret_cast<std::uint8_t *>(args[0].data()), args[0].size()});

    std::array<std::uint8_t, 64> hash;
    blake.digest(hash);

    std::cout << args[0] << ' ' << args[0].size() << '\n';
    std::cout << "hash: " << std::hex;
    for (std::uint8_t byte : hash) {
        std::cout << std::uint32_t(byte) << ' ';
    }
    std::cout << '\n';
}

void CommandLine::secret_groups(args_list_t) const {
    const auto &groups = m_database.secret_groups();
    for (std::size_t i = 0; i < groups.size(); i++) {
        std::cout << groups[i] << " - " << i << '\n';
    }
}

void CommandLine::help(args_list_t) const {
    for (const auto &[name, command] : m_commands) {
        std::cout << command.usage << " - " << command.help << '\n';
    }
}

void CommandLine::new_entry(args_list_t args) const {
}

void CommandLine::new_secret_group(args_list_t args) const {
    if (args.size() != 1) {
        std::cout << "invalid usage\n";
        return;
    }
    m_database.add_secret_group(args[0]);
}

void CommandLine::run() {
    while (true) {
        char *input = readline(build_prompt().c_str());
        if (input == nullptr) {
            break;
        }
        add_history(input);
        std::string line(input);
        free(input);

        std::vector<std::string> args;
        for (std::string_view remaining = line; !remaining.empty();) {
            const auto space_pos = remaining.find(' ');
            if (space_pos == std::string::npos) {
                args.emplace_back(remaining);
                break;
            }
            args.emplace_back(remaining.substr(0, space_pos));
            remaining = remaining.substr(space_pos + 1);
        }

        if (args.empty() || args[0].empty()) {
            continue;
        }
        const auto command = std::move(args[0]);
        args.erase(args.begin());
        if (!m_commands.contains(command)) {
            std::cout << "Unknown command '" << command << "'\n";
            continue;
        }
        m_commands.at(command).handler(std::move(args));
    }
}
