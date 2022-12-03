#include "blake.hh"

#include "common.hh"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <span>

namespace {

constexpr std::array<std::uint64_t, 8> k_iv{
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179,
};

constexpr std::array<std::array<std::uint8_t, 16>, 12> k_sigma{{
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
    {11, 8, 12, 0, 5, 2, 15, 13, 10, 14, 3, 6, 7, 1, 9, 4},
    {7, 9, 3, 1, 13, 12, 11, 14, 2, 6, 5, 10, 4, 0, 15, 8},
    {9, 0, 5, 7, 2, 4, 10, 15, 14, 1, 11, 12, 6, 8, 3, 13},
    {2, 12, 6, 10, 0, 11, 8, 3, 4, 13, 7, 5, 15, 14, 1, 9},
    {12, 5, 1, 15, 14, 13, 4, 10, 0, 7, 6, 3, 9, 2, 8, 11},
    {13, 11, 7, 14, 12, 1, 3, 9, 5, 0, 15, 4, 8, 6, 2, 10},
    {6, 15, 14, 9, 11, 3, 0, 8, 12, 2, 13, 7, 1, 4, 10, 5},
    {10, 2, 8, 4, 7, 6, 1, 5, 15, 11, 9, 14, 3, 12, 13, 0},
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
    {14, 10, 4, 8, 9, 15, 13, 6, 1, 12, 0, 2, 11, 7, 5, 3},
}};

void mix(std::uint64_t &a, std::uint64_t &b, std::uint64_t &c, std::uint64_t &d, std::uint64_t x, std::uint64_t y) {
    a += b + x;
    d = rotr<32>(d ^ a);
    c += d;
    b = rotr<24>(b ^ c);
    a += b + y;
    d = rotr<16>(d ^ a);
    c += d;
    b = rotr<63>(b ^ c);
}

} // namespace

Blake::Blake(std::size_t hash_bits, std::span<std::uint8_t> key) : m_hash_size(hash_bits / 8u) {
    assert(hash_bits > 0 && hash_bits <= 512);
    assert(key.size() <= 64);
    std::copy(k_iv.begin(), k_iv.end(), m_state.begin());
    m_state[0] ^= 0x01010000 ^ (key.size() << 8u) ^ m_hash_size;

    if (!key.empty()) {
        update(key);
        m_head = 128;
    }
}

void Blake::compress(std::span<std::uint8_t, 128> block, bool last) {
    std::array<std::uint64_t, 16> v;
    std::copy(m_state.begin(), m_state.end(), v.begin());
    std::copy(k_iv.begin(), k_iv.end(), v.begin() + 8);

    v[12] ^= m_counter[0];
    v[13] ^= m_counter[1];
    if (last) {
        v[14] = ~v[14];
    }

    std::array<std::uint64_t, 16> m;
    for (std::size_t i = 0; i < 16; i++) {
        m[i] = to_qword_le(block.subspan(i * sizeof(std::uint64_t)).subspan<0, sizeof(std::uint64_t)>());
    }

    for (std::size_t i = 0; i < 12; i++) {
        mix(v[0], v[4], v[8], v[12], m[k_sigma[i][0]], m[k_sigma[i][1]]);
        mix(v[1], v[5], v[9], v[13], m[k_sigma[i][2]], m[k_sigma[i][3]]);
        mix(v[2], v[6], v[10], v[14], m[k_sigma[i][4]], m[k_sigma[i][5]]);
        mix(v[3], v[7], v[11], v[15], m[k_sigma[i][6]], m[k_sigma[i][7]]);
        mix(v[0], v[5], v[10], v[15], m[k_sigma[i][8]], m[k_sigma[i][9]]);
        mix(v[1], v[6], v[11], v[12], m[k_sigma[i][10]], m[k_sigma[i][11]]);
        mix(v[2], v[7], v[8], v[13], m[k_sigma[i][12]], m[k_sigma[i][13]]);
        mix(v[3], v[4], v[9], v[14], m[k_sigma[i][14]], m[k_sigma[i][15]]);
    }

    for (std::size_t i = 0; i < 8; i++) {
        m_state[i] ^= v[i] ^ v[i + 8];
    }
}

void Blake::increment_counter(std::uint64_t count) {
    m_counter[0] += count;
    m_counter[1] += (m_counter[0] < count) ? 1 : 0;
}

void Blake::digest(std::span<std::uint8_t> hash) {
    assert(hash.size() == m_hash_size);
    increment_counter(m_head);
    std::fill(m_buffer.begin() + m_head, m_buffer.end(), 0);
    compress(m_buffer, true);

    // Use hash.size() rather than m_hash_size to avoid buffer overrun.
    for (std::size_t i = 0; i < hash.size(); i++) {
        hash[i] = (m_state[i >> 3u] >> (8u * (i & 7u))) & 0xffu;
    }
}

void Blake::update(std::span<std::uint8_t> data) {
    std::size_t available = 128 - m_head;
    std::size_t remaining = data.size();
    if (remaining > available) {
        // Handle previous remainder.
        std::copy_n(data.begin(), available, m_buffer.begin() + m_head);
        increment_counter(128);
        compress(m_buffer, false);
        remaining -= available;
        m_head = 0;

        // Handle any whole blocks left.
        while (remaining > 128) {
            increment_counter(128);
            compress(data.subspan(data.size() - remaining).subspan<0, 128>(), false);
            remaining -= 128;
        }
    }

    // Save remainder.
    if (remaining > 0) {
        assert(m_head + remaining <= 128);
        std::copy_n(data.end() - remaining, remaining, m_buffer.begin() + m_head);
        m_head += remaining;
    }
}
