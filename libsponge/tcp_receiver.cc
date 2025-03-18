#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    const TCPHeader &header = seg.header();

    if (!_syn_received) {
        if (header.syn) {
            _syn_received = true;
            _isn = header.seqno;

            _reassembler.push_substring(string(seg.payload().str()), 0, header.fin);

            if (header.fin) {
                _fin_received = true;
            }
        }
        return;
    }

    uint64_t abs_index = unwrap(header.seqno, _isn, _reassembler.stream_out().bytes_written());
    
    if (!header.syn) {
        abs_index -= 1;
    }

    _reassembler.push_substring(string(seg.payload().str()), abs_index, header.fin);

    if (header.fin) {
        _fin_received = true;
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    if (!_syn_received)
        return {};

    uint64_t ack = _reassembler.stream_out().bytes_written();

    if (_fin_received && _reassembler.stream_out().eof())
        ack += 1;

    return wrap(ack, _isn);
}

size_t TCPReceiver::window_size() const {
    return _capacity - _reassembler.stream_out().buffer_size();
}

