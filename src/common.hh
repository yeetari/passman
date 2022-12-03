#pragma once

template <auto N, typename T>
constexpr T rotl(T x) {
    return (x << T(N)) | (x >> ((sizeof(T) * 8) - T(N)));
}

template <auto N, typename T>
constexpr T rotr(T x) {
    return (x >> T(N)) | (x << ((sizeof(T) * 8) - T(N)));
}

inline std::uint32_t to_dword_le(std::span<const std::uint8_t, 4> bytes) {
    return static_cast<std::uint32_t>(bytes[0]) | (static_cast<std::uint32_t>(bytes[1]) << 8u) |
           (static_cast<std::uint32_t>(bytes[2]) << 16u) | (static_cast<std::uint32_t>(bytes[3]) << 24u);
}

inline std::uint64_t to_qword_le(std::span<const std::uint8_t, 8> bytes) {
    return static_cast<std::uint64_t>(bytes[0]) | (static_cast<std::uint64_t>(bytes[1]) << 8u) |
           (static_cast<std::uint64_t>(bytes[2]) << 16u) | (static_cast<std::uint64_t>(bytes[3]) << 24u) |
           (static_cast<std::uint64_t>(bytes[4]) << 32u) | (static_cast<std::uint64_t>(bytes[5]) << 40u) |
           (static_cast<std::uint64_t>(bytes[6]) << 48u) | (static_cast<std::uint64_t>(bytes[7]) << 56u);
}
