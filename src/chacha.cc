#include "chacha.hh"

#include "common.hh"

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <span>

namespace {

constexpr std::size_t k_block_size = 16 * sizeof(std::uint32_t);

void quarter_round(std::uint32_t &a, std::uint32_t &b, std::uint32_t &c, std::uint32_t &d) {
    a += b, d ^= a, d = rotl<16>(d);
    c += d, b ^= c, b = rotl<12>(b);
    a += b, d ^= a, d = rotl<8>(d);
    c += d, b ^= c, b = rotl<7>(b);
}

void xor_stream(std::span<std::uint8_t> dst, std::span<const std::uint8_t> src,
                std::span<const std::uint32_t, 16> stream) {
    const auto full_words = src.size() / sizeof(std::uint32_t);
    for (std::size_t i = 0; i < full_words; i++) {
        std::array<std::uint8_t, 4> stream_bytes{
            static_cast<std::uint8_t>((stream[i] >> 0u) & 0xffu),
            static_cast<std::uint8_t>((stream[i] >> 8u) & 0xffu),
            static_cast<std::uint8_t>((stream[i] >> 16u) & 0xffu),
            static_cast<std::uint8_t>((stream[i] >> 24u) & 0xffu),
        };
        auto dst_word_bytes = dst.subspan(i * sizeof(std::uint32_t));
        auto src_word_bytes = src.subspan(i * sizeof(std::uint32_t));
        for (std::size_t j = 0; j < 4; j++) {
            dst_word_bytes[j] = src_word_bytes[j] ^ stream_bytes[j];
        }
    }

    if (src.size() % sizeof(std::uint32_t) == 0) {
        return;
    }

    auto dst_remainder = dst.subspan(full_words * sizeof(std::uint32_t));
    auto src_remainder = src.subspan(full_words * sizeof(std::uint32_t));
    auto stream_remainder = stream[full_words];
    switch (src.size() % sizeof(std::uint32_t)) {
    case 3:
        dst_remainder[2] = src_remainder[2] ^ static_cast<std::uint8_t>((stream_remainder >> 16u) & 0xffu);
        [[fallthrough]];
    case 2:
        dst_remainder[1] = src_remainder[1] ^ static_cast<std::uint8_t>((stream_remainder >> 8u) & 0xffu);
        [[fallthrough]];
    case 1:
        dst_remainder[0] = src_remainder[0] ^ static_cast<std::uint8_t>((stream_remainder >> 0u) & 0xffu);
        break;
    }
}

} // namespace

void chacha20_block(std::span<const std::uint32_t, 16> state, std::span<std::uint32_t, 16> key_stream) {
    std::array<std::uint32_t, 16> block{};
    std::copy(state.begin(), state.end(), block.begin());
    for (std::uint32_t i = 0; i < 10; i++) {
        quarter_round(block[0], block[4], block[8], block[12]);
        quarter_round(block[1], block[5], block[9], block[13]);
        quarter_round(block[2], block[6], block[10], block[14]);
        quarter_round(block[3], block[7], block[11], block[15]);
        quarter_round(block[0], block[5], block[10], block[15]);
        quarter_round(block[1], block[6], block[11], block[12]);
        quarter_round(block[2], block[7], block[8], block[13]);
        quarter_round(block[3], block[4], block[9], block[14]);
    }
    std::transform(block.begin(), block.end(), state.begin(), key_stream.begin(), std::plus{});
}

void chacha20(std::span<std::uint8_t> dst, std::span<const std::uint8_t> src, std::span<const std::uint8_t, 32> key,
              std::span<const std::uint8_t, 12> nonce) {
    std::array<std::uint32_t, 16> state{
        0x61707865u,
        0x3320646eu,
        0x79622d32u,
        0x6b206574u,
        to_dword_le(key.subspan<0, 4>()),
        to_dword_le(key.subspan<4, 4>()),
        to_dword_le(key.subspan<8, 4>()),
        to_dword_le(key.subspan<12, 4>()),
        to_dword_le(key.subspan<16, 4>()),
        to_dword_le(key.subspan<20, 4>()),
        to_dword_le(key.subspan<24, 4>()),
        to_dword_le(key.subspan<28, 4>()),
        1,
        to_dword_le(nonce.subspan<0, 4>()),
        to_dword_le(nonce.subspan<4, 4>()),
        to_dword_le(nonce.subspan<8, 4>()),
    };

    std::size_t remaining = src.size();
    while (remaining > 0) {
        std::array<std::uint32_t, 16> key_stream;
        chacha20_block(state, key_stream);

        const auto block_size = std::min(remaining, k_block_size);
        xor_stream(dst.subspan(src.size() - remaining), src.subspan(src.size() - remaining, block_size), key_stream);
        remaining -= block_size;

        // Increment counter.
        state[12]++;
    }
}
