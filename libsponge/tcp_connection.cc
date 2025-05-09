#include "tcp_connection.hh"

#include <algorithm>
#include <iostream>
#include <limits>

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    // Reset idle timer whenever we see any segment
    _time_since_last_segment_received = 0;

    // In LISTEN state, ignore stray pure-ACKs before our SYN is sent
    if (seg.header().ack && !seg.header().syn && _sender.next_seqno_absolute() == 0) {
        return;
    }

    // RST from peer: abort both streams and disable lingering
    if (seg.header().rst) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish = false;
        return;
    }

    // Passive open: peer sent SYN first → feed to receiver and send SYN+ACK
    if (seg.header().syn && _sender.next_seqno_absolute() == 0) {
        _receiver.segment_received(seg);
        _sender.fill_window();  // SYN|ACK generated
        _send_segments();
        return;
    }

    // SYN+ACK in active-open handshake: ACK it and advance sender state
    if (seg.header().syn && seg.header().ack && _sender.next_seqno_absolute() > 0) {
        _receiver.segment_received(seg);
        _sender.ack_received(seg.header().ackno, seg.header().win);
        _sender.send_empty_segment();  // ACK the SYN+ACK
        _send_segments();
        return;
    }

    // Normal ACK: update sender’s window/ack state
    if (seg.header().ack) {
        _sender.ack_received(seg.header().ackno, seg.header().win);
    }

    // Pass payload/FIN to the receiver’s reassembler
    _receiver.segment_received(seg);

    // If we haven’t sent our FIN but peer’s FIN arrived, disable TIME-WAIT lingering
    if (_receiver.stream_out().input_ended() && !_sender.stream_in().input_ended()) {
        _linger_after_streams_finish = false;
    }

    // Offer more data (or FIN) if we can
    _sender.fill_window();

    // Determine if we need to ACK back (data or keep-alive)
    bool ack_needed =
        seg.length_in_sequence_space() > 0       // data or SYN/FIN
        || (seg.length_in_sequence_space() == 0  // keep-alive probe
            && _receiver.ackno().has_value() && seg.header().seqno.raw_value() + 1 == _receiver.ackno()->raw_value());

    if (ack_needed) {
        _sender.send_empty_segment();
    }

    // Annotate and queue all newly generated segments
    _send_segments();
}

bool TCPConnection::active() const {
    // Error on either side kills the connection
    if (_sender.stream_in().error() || _receiver.stream_out().error()) {
        return false;
    }
    // Still sending or receiving or have in-flight bytes
    if (!_sender.stream_in().input_ended() || !_receiver.stream_out().input_ended() || bytes_in_flight() > 0) {
        return true;
    }
    // If we’re past FIN but disabled lingering, done immediately
    if (!_linger_after_streams_finish) {
        return false;
    }
    // Otherwise linger for 10×RTTimeout
    return _time_since_last_segment_received < _cfg.rt_timeout * 10;
}

size_t TCPConnection::write(const string &data) {
    // Write into byte-stream, then try to send
    size_t written = _sender.stream_in().write(data);
    _sender.fill_window();
    _send_segments();
    return written;
}

void TCPConnection::tick(const size_t ms_since_last_tick) {
    // Advance sender’s retransmission timer
    _sender.tick(ms_since_last_tick);

    // If too many retransmits, send RST and enter error
    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _linger_after_streams_finish = false;
        TCPSegment rst;
        rst.header().rst = true;
        rst.header().seqno = _sender.next_seqno();
        _segments_out.push(rst);
        return;
    }

    // Flush any pending segments
    _send_segments();
    _time_since_last_segment_received += ms_since_last_tick;
}

void TCPConnection::end_input_stream() {
    // Application closed writer: send FIN
    _sender.stream_in().end_input();
    _sender.fill_window();
    _send_segments();
}

void TCPConnection::connect() {
    // Active open: send SYN
    _sender.fill_window();
    _send_segments();
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            // Send a RST segment to the peer
            TCPSegment rst;
            rst.header().rst = true;
            rst.header().seqno = _sender.next_seqno();
            _segments_out.push(rst);
        }
    } catch (const exception &e) {
        cerr << "Exception destructing TCP FSM: " << e.what() << endl;
    }
}

void TCPConnection::_send_segments() {
    // Pull every segment from the sender, annotate ack/win, and enqueue
    while (!_sender.segments_out().empty()) {
        TCPSegment seg = std::move(_sender.segments_out().front());
        _sender.segments_out().pop();
        if (auto ackno = _receiver.ackno()) {
            seg.header().ack = true;
            seg.header().ackno = *ackno;
            seg.header().win =
                static_cast<uint16_t>(min<uint64_t>(_receiver.window_size(), numeric_limits<uint16_t>::max()));
        }
        _segments_out.push(std::move(seg));
    }
}
