#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
<<<<<<< HEAD
    const TCPHeader &header = seg.header();

    // LISTEN state
    if (!_syn_received) {
        if (header.syn) {
            _syn_received = true;
            _isn = header.seqno;
        } else {
            return;
        }
    }

    // SYN_RECEV state
    // 디벅깅이 필요할수도 있음
    size_t abs_seqno = unwrap(header.seqno, _isn, _reassembler.stream_out().bytes_written());

    if (!header.syn) {
        abs_seqno -= 1;
    }

    _reassembler.push_substring(seg.payload().copy(), abs_seqno, header.fin);

    // FIN_RECEV state
    if (!_fin_received && header.fin) {
        _fin_received = true;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received)
        return {};

    uint64_t _ackno = _reassembler.stream_out().bytes_written() + 1;

    if (_fin_received && _reassembler.empty()) {
        _ackno += 1;
    }

    return wrap(_ackno, _isn);
}

size_t TCPReceiver::window_size() const { return _reassembler.stream_out().remaining_capacity(); }
=======
    DUMMY_CODE(seg);
}

optional<WrappingInt32> TCPReceiver::ackno() const { return {}; }

size_t TCPReceiver::window_size() const { return {}; }
>>>>>>> upstream/lab5-startercode
