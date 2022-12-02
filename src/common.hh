#pragma once

template <auto N, typename T>
constexpr T rotl(T x) {
    return (x << T(N)) | (x >> ((sizeof(T) * 8) - T(N)));
}

template <auto N, typename T>
constexpr T rotr(T x) {
    return (x >> T(N)) | (x << ((sizeof(T) * 8) - T(N)));
}
