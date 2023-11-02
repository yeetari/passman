#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

class Secret {
    std::uint8_t m_group_index;
    std::string m_name;
    std::string m_value; // TODO: Should be in erasable, unswappable memory

public:
    std::uint8_t group_index() const { return m_group_index; }
    std::string_view value() const { return m_value; }
};

class Entry {
    std::string m_name;
    std::vector<Secret> m_secrets;
    std::vector<std::string> m_tags;

public:
    Entry(std::string name) : m_name(std::move(name)) {}

    void add_tag(std::string tag) { m_tags.push_back(std::move(tag)); }

    const std::string &name() const { return m_name; }
    const std::vector<Secret> &secrets() const { return m_secrets; }
    const std::vector<std::string> &tags() const { return m_tags; }
};

class Database {
    std::vector<Entry> m_entries;
    std::vector<std::string> m_secret_groups;
    std::array<std::uint8_t, 32> m_key; // TODO: Should be in erasable, unswappable memory

    void serialise(std::FILE *);

public:
    Database() {
        m_entries.emplace_back("Gmail");
        m_entries.emplace_back("Steam");

        m_secret_groups.push_back("main");
    }

    void add_secret_group(std::string name) { m_secret_groups.push_back(std::move(name)); }

    void lock();
    void save(const std::string &path);

    const std::vector<Entry> &entries() const { return m_entries; }
    const std::vector<std::string> &secret_groups() const { return m_secret_groups; }
};
