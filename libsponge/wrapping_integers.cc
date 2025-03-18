#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    const uint64_t mod = 1ull << 32;

    uint32_t raw_isn = isn.raw_value();
    uint32_t result = (n + raw_isn) % mod;
    
    return WrappingInt32{result};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const uint64_t mod = 1ull << 32;

    uint32_t offset = n.raw_value() - isn.raw_value();
    uint64_t base = (checkpoint / mod) * mod;
    uint64_t candidate = base + offset;

    auto diff = [](uint64_t a, uint64_t b) -> uint64_t {
        return a > b ? a - b : b - a;
    };

    uint64_t diff1 = diff(candidate + mod - checkpoint, checkpoint);
    uint64_t diff2 = diff(candidate - checkpoint, checkpoint);
    uint64_t diff3 = diff(candidate - checkpoint - mod, checkpoint);

    if (diff1 <= diff2 && diff1 <= diff3) {
        return candidate + mod;
    }
    else if (diff3 <= diff2 && diff3 <= diff2) {
        return candidate - mod;
    }
    else {
        return candidate;   
    } 
}
