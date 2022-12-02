#include "chacha.hh"
#include "random.hh"

#include <cstdio>
#include <iostream>
#include <vector>

#include <string.h>
#include <sys/resource.h>

int main(int argc, char **argv) {
    std::vector<std::string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        explicit_bzero(argv[i], strlen(argv[i]));
    }

    // Disable core dumps.
    struct rlimit core_rlimit {};
    if (setrlimit(RLIMIT_CORE, &core_rlimit) < 0) {
        std::perror("setrlimit");
    }

    std::array<std::uint8_t, 32> key;
    std::array<std::uint8_t, 12> nonce;
    get_random_bytes(key);
    get_random_bytes(nonce);

    std::array<std::uint8_t, 5> plain{'H', 'E', 'L', 'L', 'O'};
    std::array<std::uint8_t, 5> cipher;
    chacha20(cipher, plain, key, nonce);

    std::array<std::uint8_t, 5> decrypted;
    chacha20(decrypted, cipher, key, nonce);

    for (int i = 0; i < 5; i++) {
        std::cout << std::hex << uint32_t(plain[i]) << ' ';
    }
    std::cout << '\n';
    for (int i = 0; i < 5; i++) {
        std::cout << std::hex << uint32_t(cipher[i]) << ' ';
    }
    std::cout << '\n';
    for (int i = 0; i < 5; i++) {
        std::cout << std::hex << uint32_t(decrypted[i]) << ' ';
    }
    std::cout << '\n';
}
