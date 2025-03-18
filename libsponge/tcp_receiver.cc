#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();
    
    // syn이 들어오지 않은 상태 
    if (!_syn_received) {
        if (header.syn) {
            _syn_received = true;
            _isn = header.seqno;
        }
        else {
            return;
        }
    }

    // fin이 들어오지 않은 상태에서 fin이 들어온 경우
    if (!_fin_received && header.fin) {
        _fin_received = true;
    }

    // syn이 있는 상태 
    size_t abs_seqno = unwrap(header.seqno, _isn, _reassembler.stream_out().bytes_written());
    
    if (!header.syn) {
        abs_seqno -= 1;
    }
    _reassembler.push_substring(seg.payload().copy(), abs_seqno, header.fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received)
        return {};
    
    uint64_t _ackno = _reassembler.stream_out().bytes_written() + 1;
    
    if (_fin_received) {
        _ackno += 1;
    }

    return wrap(_ackno, _isn);
}

size_t TCPReceiver::window_size() const {
    return _reassembler.stream_out().remaining_capacity();
}
