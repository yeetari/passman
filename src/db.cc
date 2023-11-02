#include "db.hh"

#include "chacha.hh"
#include "random.hh"

#include <array>
#include <cstdio>
#include <vector>

namespace {

constexpr std::array k_magic{0x27, 0xa9, 0xcd, 0x36};

} // namespace

void Database::serialise(std::FILE *file) {
    std::fwrite(k_magic.data(), 1, k_magic.size(), file);
    std::fseek(file, 64, SEEK_CUR);

    std::vector<std::vector<std::uint8_t>> secret_group_data(m_secret_groups.size());
    for (const auto &entry : m_entries) {
        for (const auto &secret : entry.secrets()) {
            auto &byte_data = secret_group_data[secret.group_index()];
            byte_data.resize(byte_data.size() + secret.value().size());
            std::copy(secret.value().begin(), secret.value().end(), byte_data.end() - secret.value().size());
        }
    }

    std::fputc(secret_group_data.size(), file);
    for (const auto &data : secret_group_data) {
        std::array<std::uint8_t, 12> nonce;
        get_random_bytes(nonce);
        std::array<std::uint8_t, 4> size{
            static_cast<std::uint8_t>((data.size() >> 24u) & 0xffu),
            static_cast<std::uint8_t>((data.size() >> 16u) & 0xffu),
            static_cast<std::uint8_t>((data.size() >> 8u) & 0xffu),
            static_cast<std::uint8_t>((data.size() >> 0u) & 0xffu),
        };

        std::vector<std::uint8_t> encrypted(data.size());
        chacha20(encrypted, data, m_key, nonce);

        std::fwrite(nonce.data(), 1, nonce.size(), file);
        std::fwrite(size.data(), 1, size.size(), file);
        std::fwrite(encrypted.data(), 1, encrypted.size(), file);
    }
}

void Database::save(const std::string &path) {
    std::string new_path = path + ".new";
    std::FILE *new_file = std::fopen(new_path.c_str(), "wb");
    serialise(new_file);
    std::rename(new_path.c_str(), path.c_str());
}
