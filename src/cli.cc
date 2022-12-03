#include "cli.hh"

#include "blake.hh"
#include "chacha.hh"
#include "random.hh"

#include <iostream>
#include <string>

#include <readline/history.h>
#include <readline/readline.h>

using namespace std::placeholders;

CommandLine::CommandLine(std::string db_name) : m_db_name(std::move(db_name)) {
    m_commands["encrypt"] = {std::bind(&CommandLine::encrypt, this, _1), "encrypt a message", "encrypt <msg>"};
    m_commands["hash"] = {std::bind(&CommandLine::hash, this, _1), "hash a message", "hash <msg>"};
    m_commands["help"] = {std::bind(&CommandLine::help, this, _1), "shows this help message", "help"};
}

std::string CommandLine::build_prompt() const {
    if (m_locked) {
        return "[locked due to inactivity]";
    }
    std::string prompt = "[database ";
    prompt += m_db_name;
    if (m_unsaved_changes) {
        prompt += " (unsaved changes)";
    }
    prompt += "]: ";
    return prompt;
}

void CommandLine::encrypt(args_list_t args) const {
    if (args.size() != 1) {
        std::cout << "invalid usage\n";
        return;
    }

    std::array<std::uint8_t, 32> key;
    std::array<std::uint8_t, 12> nonce;
    get_random_bytes(key);
    get_random_bytes(nonce);

    std::vector<std::uint8_t> cipher(args[0].size());
    chacha20(cipher, {reinterpret_cast<std::uint8_t *>(args[0].data()), args[0].size()}, key, nonce);

    std::vector<std::uint8_t> decrypted(args[0].size());
    chacha20(decrypted, cipher, key, nonce);

    std::cout << "cipher: " << std::hex;
    for (std::uint8_t byte : cipher) {
        std::cout << std::uint32_t(byte) << ' ';
    }
    std::cout << "\ndecrypted: " << std::dec;
    for (std::uint8_t ch : decrypted) {
        std::cout << ch;
    }
    std::cout << '\n';
}

void CommandLine::hash(args_list_t args) const {
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

void CommandLine::help(args_list_t) const {
    for (const auto &[name, command] : m_commands) {
        std::cout << command.usage << " - " << command.help << '\n';
    }
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
