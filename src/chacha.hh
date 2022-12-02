#pragma once

#include <cstdint>
#include <span>

void chacha20_block(std::span<const std::uint32_t, 16> state, std::span<std::uint8_t, 16> key_stream);
void chacha20(std::span<std::uint8_t> dst, std::span<const std::uint8_t> src, std::span<const std::uint8_t, 32> key,
              std::span<const std::uint8_t, 12> nonce);
