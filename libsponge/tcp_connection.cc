// #include "tcp_connection.hh"

// #include <iostream>
// #include <algorithm>
// #include <limits>
// // Dummy implementation of a TCP connection

// // For Lab 4, please replace with a real implementation that passes the
// // automated checks run by `make check`.

// using namespace std;

// // 송신 버퍼에 남아 있는 총 용량(바이트 단위) return
// size_t TCPConnection::remaining_outbound_capacity() const {return _sender.stream_in().remaining_capacity();}
// // 아직 네트워크로 나가지 않은(전송 대기 중인) bytes
// size_t TCPConnection::bytes_in_flight() const {return _sender.bytes_in_flight();}
// // 수신 측 조립이 끝나지 않은(순서가 뒤섞여 도착하여 버퍼에 남아 있는) bytes
// size_t TCPConnection::unassembled_bytes() const {return _receiver.unassembled_bytes();}
// // 마지막 수신 이후 경과한 시간(ms)
// size_t TCPConnection::time_since_last_segment_received() const {return _connection_age - _last_segment_received;;}

// // --- return sending segment ---
// // _sender.segments_out() 큐에서 앞(front) 요소를 꺼내 반환
// TCPSegment TCPConnection::dequeue_sender_segment() {
//     TCPSegment seg = _sender.segments_out().front();
//     _sender.segments_out().pop();
//     return seg;
// }

// // --- set window size ---
// // 수신 창 크기를 16비트 필드 내에 맞춰 설정
// void TCPConnection::set_window(TCPSegment &seg) {
//     size_t recv_win = _receiver.window_size();
//     size_t max_uint16 = numeric_limits<uint16_t>::max();
//     seg.header().win = static_cast<uint16_t>(min(recv_win, max_uint16));
// }

// // --- ACK 필드 설정 및 관리 ---
// // if recevier has ackno, set on header
// void TCPConnection::set_ack(TCPSegment &seg) {
//     if (auto cur = _receiver.ackno()) {
//         seg.header().ack = true;
//         if (!_current_ackno.has_value() || _segments_out.empty()) {
//             _current_ackno = *cur;
//         }
//         seg.header().ackno = _current_ackno.value();
//     } else {
//         seg.header().ack = false;
//     }
// }

// // send one segment (segment from sender, insert into _segments_out)
// void TCPConnection::send_segment() {
//     if (_sender.segments_out().empty()) return;
//     TCPSegment seg = dequeue_sender_segment();
//     set_window(seg);
//     set_ack(seg);
//     _segments_out.push(seg);
// }

// // RST 플래그를 포함한 빈 세그먼트를 만들어 전송
// void TCPConnection::handle_rst() {
//     _sender.segments_out() = {};
//     _sender.send_empty_segment();
//     _sender.segments_out().front().header().rst = true; // rst flag

//     send_segment();
//     _is_reset = true;
//     // 에러 표시
//     _sender.stream_in().set_error();
//     _receiver.stream_out().set_error();
//     return;
// }

// void TCPConnection::segment_received(const TCPSegment &seg) {
//     // 연결 초기화 전: SYN 교환 전 상태 처리
//     if (!_receiver.ackno().has_value() && _sender.next_seqno_absolute() == 0) {
//         // RST 오면 즉시 종료
//         if (seg.header().rst) {
//             _is_reset = true;
//             _sender.stream_in().set_error();
//             _receiver.stream_out().set_error();
//             return;
//         }
//         // SYN 없으면 무시(연결 시작이 아님)
//         if (!seg.header().syn) {
//             return;
//         }
//     }
    
//     // TCPReceiver로 전달
//     _receiver.segment_received(seg);

//     // 마지막 유효 세그먼트 수신 시각 업데이트
//     _last_segment_received = _connection_age;

//     // 수신 스트림 종료 상태 && 송신 스트림 미종료 시 linger 중지
//     if (_receiver.stream_out().input_ended() && !_sender.stream_in().eof())
//         _linger_after_streams_finish = false;

//     // ACK field가 있으면 sender에게 전달
//     if (seg.header().ack) {
//         _sender.ack_received(seg.header().ackno, seg.header().win);
//     }

//     // SYN만 보낸 상태라면 세그먼트 하나 더 전송
//     if (_sender.next_seqno_absolute() == 0) {
//         _sender.fill_window();  
//         send_segment();
//         return;
//     }

//     // 데이터 전송 또는 keep-alive용 ACK 응답 처리
//     if (_receiver.ackno().has_value()) {
//         bool is_data = seg.length_in_sequence_space() > 0;
//         bool is_keepalive = !is_data && seg.header().seqno == _receiver.ackno().value() - 1;
//         if (is_data || is_keepalive) {
//             // 빈 ACK-only 세그먼트 생성 후 전송
//             _sender.send_empty_segment();
//             send_segment(); 
//         }
//     }

//     // window size에 여유가 생겼을 수 있으므로, 새 데이터 전송
//     _sender.fill_window();
//     while (!_sender.segments_out().empty())
//         send_segment();
// }

// bool TCPConnection::active() const {
//     // 송수신 스트림 중 아직 종료되지 않았거나
//     // 비행 중인 바이트가 남아 있으면 활성
//     bool streams_alive = !(_receiver.stream_out().eof()
//                           && _sender.stream_in().eof()
//                           && _sender.bytes_in_flight() == 0);
//     if (_is_reset) {
//         // RST 처리 후 비활성
//         return false;
//     }
//     if (streams_alive) {
//         return true;
//     }
//     // linger 지연 모드가 꺼졌으면 즉시 비활성
//     if (!_linger_after_streams_finish) {
//         return false;
//     }
//     // linger 타이머(10×RTT) 경과 전이면 활성
//     return _connection_age - _last_segment_received < 10 * _cfg.rt_timeout;
// }

// size_t TCPConnection::write(const std::string &data) {
//     size_t n = _sender.stream_in().write(data);
//     _sender.fill_window();
//     while (!_sender.segments_out().empty())
//         send_segment();
//     return n;
// }

// //! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
// void TCPConnection::tick(const size_t ms_since_last_tick) {
//     // if (_is_reset) 
//     //     return;

//     _connection_age += ms_since_last_tick;
//     _sender.tick(ms_since_last_tick); // update retransmission timer

//     // 연속 재전송 횟수 초과 시 RST
//     if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
//         handle_rst();
//     }

//     while (!_sender.segments_out().empty())
//         send_segment();
// }

// void TCPConnection::end_input_stream() {
//     _sender.stream_in().end_input();
//     _sender.fill_window();      
//     while (!_sender.segments_out().empty())
//         send_segment();
// }

// void TCPConnection::connect() {
//     _sender.fill_window();
//     while (!_sender.segments_out().empty())
//         send_segment();
// }

// TCPConnection::~TCPConnection() {
//     try {
//         if (active()) {
//             cerr << "Warning: Unclean shutdown of TCPConnection\n";
//             // Your code here: need to send a RST segment to the peer
//             // RST 세그먼트를 보내서 상대에 알림
//             handle_rst();
//         }
//     } catch (const exception &e) {
//         std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
//     }
// }

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
