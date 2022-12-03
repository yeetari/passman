#pragma once

#include <array>
#include <cstdint>
#include <span>

class Blake {
    const std::size_t m_hash_size;
    std::array<std::uint64_t, 8> m_state;
    std::array<std::uint64_t, 2> m_counter{};
    std::array<std::uint8_t, 128> m_buffer{};
    std::uint8_t m_head{0};

    void compress(std::span<std::uint8_t, 128> block, bool last);
    void increment_counter(std::uint64_t count);

public:
    Blake(std::size_t hash_bits, std::span<std::uint8_t> key = {});

    void digest(std::span<std::uint8_t> hash);
    void update(std::span<std::uint8_t> data);
};
