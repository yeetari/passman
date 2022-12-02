#include "random.hh"

#include <algorithm>
#include <cstdio>

#include <errno.h>
#include <sys/random.h>

static void get_random_chunk(std::span<std::uint8_t> chunk) {
    [[maybe_unused]] ssize_t rc = getrandom(chunk.data(), chunk.size(), 0);
    if (rc != chunk.size()) {
        if (rc < 0) {
            std::perror("getrandom");
        }
        std::abort();
    }
}

void get_random_bytes(std::span<std::uint8_t> bytes) {
    auto remaining = bytes.size();
    do {
        // <= 256 bytes guaranteed to not be interrupted.
       const auto chunk_size = std::min(remaining, 256uz);
       get_random_chunk(bytes.subspan(bytes.size() - remaining));
       remaining -= chunk_size;
    } while (remaining > 0);
}
